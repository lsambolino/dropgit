
/*

Copyright (c) 2013, Sergio Mangialardi (sergio@reti.dist.unige.it)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this 
list of conditions and the following disclaimer in the documentation and/or 
other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "control_element_impl.hpp"

#include <vector>
#include <utility>
#include <cstdint>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <tuple>
#include <thread>
#include <map>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <future>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <atomic>

#include "activity/network/configuration_persistence.hpp"
#include "activity/network/discover_service_elements.hpp"
#include "activity/network/discover_openflow_elements.hpp"
#include "activity/http/http_server.hpp"
#include "activity/gal/ce_lcp.hpp"

#include "exception/drop_exception.hpp"

#include "message/network/management.hpp"
#include "message/network/openflow_messages.hpp"
#include "message/virtual_interfaces_messages.hpp"

#include "network/service_element.hpp"
#include "network/service_element.hpp"

#include "protocol/drop/drop.hpp"

#include "event/quit.hpp"
#include "event/network/address_events.hpp"
#include "event/network/route_events.hpp"
#include "event/network/port_events.hpp"
#include "event/gal/traffic_profile_change.hpp"
#include "event/network/service_element_active.hpp"
#include "event/router_requests.hpp"
#include "event/service_request.hpp"
#include "event/network/arp.hpp"
#include "event/network/service_element_connected.hpp"
#include "event/network/service_element_active.hpp"
#include "event/network/service_element_disconnected.hpp"
#include "event/network/connection_reset.hpp"
#include "event/network/route_events.hpp"
#include "event/network/packet_in.hpp"
#include "event/network/port_events.hpp"
#include "event/network/address_events.hpp"
#include "event/network/route_events.hpp"
#include "event/network/port_stats_request.hpp"
#include "event/network/port_stats_reply.hpp"
#include "event/network/port_status_change.hpp"
#include "event/network/arp_packet_in.hpp"
#include "event/virtual_interfaces_events.hpp"
#include "event/gal/traffic_profile_change.hpp"
#include "event/gal/lcp_started.hpp"
#include "event/gal/data_update.hpp"

#include "router/interface.hpp"
#include "router/route.hpp"
#include "router/address.hpp"
#include "router/address_info.hpp"
#include "protocol/openflow/flow.hpp"
#include "router/flow_query.hpp"
#include "router/arp_packet.hpp"
#include "router/arp_packet_factory.hpp"
#include "router/eth_factory.hpp"
#include "router/port_status.hpp"

#include "service/service.hpp"
#include "service/forwarding.hpp"
#include "service/interconnection.hpp"
#include "service/forwarding.hpp"
#include "service/interconnection.hpp"
#include "service/openflow_management.hpp"
#include "service/port_status_notification.hpp"

#include "util/configuration.hpp"
#include "util/pugixml.hpp"
#include "util/path.hpp"
#include "util/factory.hpp"
#include "util/io_factory.hpp"
#include "util/interfaces.hpp"

#include "io/io.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "containers.hpp"
#include "range.hpp"
#include "log.hpp"
#include "dump.hpp"
#include "endianness.hpp"
#include "async.hpp"
#include "mac_address.hpp"
#include "demangle.hpp"

namespace drop {
namespace activity {
namespace {

//const char* igmp_path = "/proc/net/igmp";

enum MulticastAddress
{
    OSPF_ALLSPFROUTERS = 0xe0000005, // 224.0.0.5
    OSPF_ALLDROUTERS = 0xe0000006,   // 224.0.0.6
};

enum class PortType
{
    Internal,
    External,
    Service
};

/*std::vector<uint32_t> read_multicast_addresses(uint32_t iface)
{
    std::ifstream file(igmp_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string line;
    auto pattern = std::to_string(iface);
    auto in = false;

	std::string addr(4, '\0');
    std::vector<uint32_t> addresses;

    while (std::getline(file, line))
    {
        std::string val;
        std::stringstream ss(line);

        ss >> val;

        if (in)
        {
            if (std::isdigit(line[0]))
            {
                break;
            }
        }
        else
        {
            if (val == pattern)
            {
                in = true;
            }

            continue;
        }


        for (int i=0, j=0; i<8; i+=2, ++j)
        {
            int n = std::stoi(val.substr(i, 2), 0, 16);

            addr[j] = n;
        }

        addresses.push_back(*reinterpret_cast<const uint32_t*>(addr.data()));
    }

    return addresses;
}

RouteInfo on_link_route(const AddressInfo& address)
{
    auto netmask = std::numeric_limits<uint32_t>::max() << (32 - address.prefix);

    RouteInfo ri;
    ri.origin = RouteOrigin::Kernel;
    ri.port_index = address.port_index;
    ri.prefix = address.prefix;
    ri.destination = htonl(ntohl(address.address) & netmask);
    ri.gateway = 0;
    ri.source = 0;

    return ri;
}*/

} // namespace

ControlElement::ControlElementImpl::~ControlElementImpl()
{
    for (auto& a : activities_)
    {
        try
        {
            a.get();
        }
        catch (std::exception& ex)
        {
            tnt::Log::error("ControlElement::dtor error: ", ex.what(), " (", tnt::get_name(ex), ")");
        }
        catch (...)
        {
            tnt::Log::error("ControlElement::dtor error: Unknown exception");
        }
    }

    tnt::Log::debug(colors::green, "ControlElement activity terminated.");
}

void ControlElement::ControlElementImpl::configure_network()
{
    pugi::xml_document doc;
    auto result = doc.load_file(util::config_file_path("network").c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("ControlElement::ControlElementImpl: ") + result.description());
    }

    auto root = doc.child("network");
	assert(root);

    load_nodes(root.child("topology"));
    load_interfaces(root.child("interfaces"));
    
    /*activities_.push_back(tnt::async([] ()
    {
        activity::ConfigurationPersistence()();
    }));*/
}

void ControlElement::ControlElementImpl::load_nodes(const pugi::xml_node& root)
{
	assert(root);
    auto nodes = root.child("nodes");
	assert(nodes);

    for (const auto& node : nodes.children("node"))
    {
        load_node(node);
    }
}

void ControlElement::ControlElementImpl::load_node(const pugi::xml_node& root)
{
    assert(root);

    auto services_node = root.child("services");

    std::vector<std::string> services;

    if (services_node)
    {
        for (auto s : services_node)
        {
            services.push_back(s.attribute("type").as_string());
        }
    }

    auto name = root.attribute("name").as_string();
    auto dp = root.attribute("display");

    tnt::insert_back(elements_, std::make_shared<ce::ServiceElement>(name, dp.as_string(name), services, root.attribute("force_connected").as_bool(), root.attribute("address").as_string("")));
}

void ControlElement::ControlElementImpl::load_interfaces(const pugi::xml_node& root)
{
    assert(root);

    for (const auto& iface : root.children("interface"))
    {
        auto name = iface.attribute("name").as_string();
        auto mac = iface.attribute("mac").as_string();

		// TODO: Use Netlink to create the links and to set them up and to add addresses and routes.
        std::stringstream ss;
        ss << "ip link add name " << name << " address " << mac << " mtu 3000 type deth";

        std::system(ss.str().c_str());
        ss.str("");
        
        ss << "ip link set up dev " << name;

        std::system(ss.str().c_str());
        ss.str("");

        auto index = tnt::name_to_index(name);

        tnt::insert_back(interfaces_, std::make_shared<router::Interface>(index, name, tnt::MacAddress(mac)));

		auto node = iface.attribute("node").as_string();
        auto port = iface.attribute("port").as_int();

        external_interfaces_.add(index, name, node, port);

        auto addresses = iface.child("addresses");

        for (const auto& address : addresses.children("address"))
        {
            auto addr = address.attribute("value").as_string();

            auto ai = AddressInfo();
            ai.address = inet_addr(addr);
            ai.prefix = address.attribute("prefix").as_int();
            ai.port_index = index;

            add(ai);
        }

        auto routes = iface.child("routes");

        for (const auto& route : routes.children("route"))
        {
            auto ri = RouteInfo();

            ri.destination = inet_addr(route.attribute("dest").as_string("0.0.0.0"));
            ri.source = inet_addr(route.attribute("src").as_string("0.0.0.0"));
            ri.gateway = inet_addr(route.attribute("gw").as_string("0.0.0.0"));
            ri.prefix = route.attribute("prefix").as_uint();
            ri.metric = route.attribute("metric").as_uint();
            ri.origin = route_origin(route.attribute("origin").as_string());
            ri.port_index = index;

			add(ri);
        }
    }
}

bool ControlElement::ControlElementImpl::is_sequential()
{
    return thread_id_ == std::this_thread::get_id();
}

void ControlElement::ControlElementImpl::for_each_interface(std::function<void(const std::shared_ptr<router::Interface>&)> func)
{
    assert(is_sequential());

    tnt::for_all(interfaces_, func);
}

void ControlElement::ControlElementImpl::for_each_address(std::function<void(const std::shared_ptr<router::Address>&)> func)
{
    assert(is_sequential());

    tnt::for_all(addresses_, func);
}

void ControlElement::ControlElementImpl::for_each_route(std::function<void(const std::shared_ptr<router::Route>&)> func)
{
    assert(is_sequential());

    tnt::for_all(routes_, func);
}

tnt::Range<ControlElement::ControlElementImpl::ElementsIterator> ControlElement::ControlElementImpl::for_each_element()
{
    return tnt::make_range(elements_);
}

void ControlElement::ControlElementImpl::for_each_element(std::function<void(const std::shared_ptr<ce::ServiceElement>&)> func)
{
    assert(is_sequential());

    tnt::for_all(elements_, func);
}

void ControlElement::ControlElementImpl::add(const RouteInfo& route)
{
	if (tnt::contains_if(routes_, [&] (const auto& r) { return *r == route; }))
    {
        tnt::Log::info(colors::blue, "Local Route already in the system, skipping adding: ", route);

        return;
    }

    tnt::Log::info(colors::blue, "Added Local Route: ", route);

    routes_.push_back(std::make_shared<router::Route>(route));

    auto fe = false;

    for (const auto& element : for_each_element())
    {
        fe |= tnt::visit_any_of<service::Forwarding>(element->services(), [&] (const auto& s)
        {
            try
            {
                s->add(route);
            }
            catch (std::exception& ex)
            {
                tnt::Log::error("ControlElement::ControlElementImpl error sending a message to a SE: ", ex.what());
            }
        });
    }

    if (!fe)
    {
        tnt::Log::info(colors::red, "No Forwarding Element connected");
    }
}

void ControlElement::ControlElementImpl::add(const AddressInfo& address)
{
    if (tnt::contains_if(addresses_, [&] (const auto& a) { return *a == address; }))
    {
        return;
    }

    tnt::insert_back(addresses_, std::make_shared<router::Address>(address));
}

void ControlElement::ControlElementImpl::add_routes(const std::vector<std::shared_ptr<Service>>& services)
{
    for (const auto& service : services)
    {
        tnt::visit<service::Forwarding>(service, [&] (const auto& s)
        {
            for (const auto& route : routes_)
            {
                tnt::Log::info(colors::blue, "Adding route: ", *route);

                try
                {
                    s->add(static_cast<const RouteInfo>(*route));
                }
                catch (std::exception& ex)
                {
                    tnt::Log::error("ControlElement::ControlElementImpl error sending a message to a SE: ", ex.what());
                }
            }
        });
    }
}

void ControlElement::ControlElementImpl::remove(const RouteInfo& /*route*/)
{
    /*auto it = std::find_if(std::begin(routes_), std::end(routes_), [&] (const auto& r) { return *r == route; });

    if (it == std::end(routes_))
    {
        return;
    }

    auto r = *it;
    routes_.erase(it);

    for_each_forwarding_element([&] (const auto& fe)
    {
        try
        {
			fe->send(std::make_unique<message::DelRoute>(on_link_route(r)));
        }
        catch (std::exception& ex)
        {
            tnt::Log::error("ControlElement::ControlElementImpl error sending a message to the FE ", fe->name(), ": ", ex.what());
        }
    });*/
}

void ControlElement::ControlElementImpl::remove(const AddressInfo& address)
{
    tnt::erase_if(addresses_, [&] (const auto& a) { return *a == address; });
}

void ControlElement::ControlElementImpl::set_state_up(const PortInfo& /*port*/)
{
    //change_state<event::PortAdminUp, message::SetPortUp>(port);
}

void ControlElement::ControlElementImpl::set_state_down(const PortInfo& /*port*/)
{
    //change_state<event::PortAdminDown, message::SetPortDown>(port);
}

std::string ControlElement::ControlElementImpl::get_interface_name(uint32_t index)
{
    assert(index != 0); // 0 is not a valid index.

    auto it = tnt::find_if(interfaces_, [index] (const auto& i)
    {
        return i->index() == index;
    });

    assert(it != std::end(interfaces_));

    return (*it)->name();
}

uint32_t ControlElement::ControlElementImpl::get_interface_index(const std::string& name)
{
    auto it = tnt::find_if(interfaces_, [&name] (const auto& i)
    {
        return i->name() == name;
    });

    assert(it != std::end(interfaces_));

    return (*it)->index();
}

uint32_t ControlElement::ControlElementImpl::get_on_link_interface(const RouteInfo& route)
{
    AddressInfo gw;
    gw.address = route.gateway;
    gw.prefix = 24;

    for (const auto& a : addresses_)
    {
        const auto& ai = static_cast<const AddressInfo>(*a);

        if (same_network(ai, gw))
        {
            return ai.port_index;
        }
    }

    return 0;
}

std::shared_ptr<router::Interface> ControlElement::ControlElementImpl::get_interface(uint32_t index)
{
    std::shared_ptr<router::Interface> interface;

    for_each_interface([&] (const auto& iface)
    {
        if (iface->index() == index)
        {
            interface = iface;

            return;
        }
    });

    return interface;
}

std::shared_ptr<router::Interface> ControlElement::ControlElementImpl::get_interface(const tnt::ip::Address& address)
{
    std::shared_ptr<router::Interface> interface;

    for_each_address([&] (const auto& addr)
    {
        const auto& ai = static_cast<const AddressInfo>(*addr);

        if (ai.address == address.to_net_order_ulong())
        {
            for_each_interface([&] (const auto& iface)
            {
                if (iface->index() == ai.port_index)
                {
                    interface = iface;

                    return;
                }
            });
        }
    });

    return interface;
}

std::shared_ptr<router::Address> ControlElement::ControlElementImpl::address_by_network(const tnt::ip::Address& address)
{
    auto it = tnt::find_if(addresses_, [&] (const auto& addr)
    {
        return same_network(static_cast<const AddressInfo>(*addr), address);
    });

    return it != std::end(addresses_) ? *it : nullptr;
}

ControlElement::ControlElementImpl::ElementsIterator ControlElement::ControlElementImpl::get_element(const std::string& name)
{
    for (auto it = std::begin(elements_); it != std::end(elements_); ++it)
    {
        if ((*it)->name() == name || (*it)->display_name() == name)
        {
            return it;
        }
    }

    return std::end(elements_);
}

void ControlElement::ControlElementImpl::manage_arp(const protocol::Flow& /*flow*/, const std::string& /*buffer*/, const std::shared_ptr<tnt::Protocol>& /*proto*/)
{
    /*auto ie = get_ie(proto);

    if (!ie)
    {
        return;
    }

    auto idx = external_interfaces_.get_index(ie->name(), flow.in_port_index, std::nothrow);
    auto pkt = arp_packet(buffer.data() + 14);

    if (idx == 0)
    {
        arp_internal_request(ie, pkt, flow.in_port_index);

        return;
    }

    module_->send(std::make_unique<message::DethInPacket>(idx, buffer));

    if (arp_operation(pkt) == ArpPacketOperation::Reply)
    {
        arp_external_reply(pkt);
    }*/
}

void ControlElement::ControlElementImpl::manage_ip(const protocol::Flow& /*flow*/, const std::string& /*buffer*/, const std::shared_ptr<tnt::Protocol>& /*proto*/)
{
    /*auto l3 = get_l3_proto(flow);
    auto ie = get_ie(proto);

    if (!ie)
    {
        return;
    }

    if (l3 == L3Proto::Ospf)
    {
        manage_ospf(flow, buffer, ie);
    }
    else if (l3 == L3Proto::Tcp && get_tp_dst(flow) == TransportPort::Bgp) // TODO: Check if BGP packets can arrive on other TCP ports.
    {
        manage_bgp(flow, buffer, ie);
    }*/

    // Default: drop packet
}

/*void ControlElement::ControlElementImpl::manage_ospf(const protocol::Flow& flow, const std::string& buffer, const std::shared_ptr<ce::InterconnectionElement>& ie)
{
    try
    {
        auto idx = external_interfaces_.get_index(ie->name(), flow.in_port_index);
        module_->send(std::make_unique<message::DethInPacket>(idx, buffer));
    }
    catch (NonExistentInterface& ex)
    {
        tnt::Log::error("ControlElement::ControlElementImpl::manage_ospf error: ", ex.what());
    }
}

void ControlElement::ControlElementImpl::manage_bgp(const protocol::Flow& flow, const std::string& buffer, const std::shared_ptr<ce::InterconnectionElement>& ie)
{
    tnt::Log::info(colors::red, "Got a BGP packet from switch port ", flow.in_port_index);
}

void ControlElement::ControlElementImpl::manage_multicast(int iface)
{
    auto addresses = read_multicast_addresses(iface);

    if (tnt::contains(addresses, OSPF_ALLSPFROUTERS))
    {
        tnt::Log::info(colors::blue, "OSPF: join ALL SPF ROUTERS group on interface #", iface);
        join_multicast_group(iface, tnt::ip::Address::from_host_order_ulong(OSPF_ALLSPFROUTERS));
    }
    else
    {
        tnt::Log::info(colors::blue, "OSPF: leave ALL SPF ROUTERS group on interface #", iface);
        leave_multicast_group(iface, tnt::ip::Address::from_host_order_ulong(OSPF_ALLSPFROUTERS));
    }

    if (tnt::contains(addresses, OSPF_ALLDROUTERS))
    {
        tnt::Log::info(colors::blue, "OSPF: join ALL DR ROUTERS group on interface #", iface);
        join_multicast_group(iface, tnt::ip::Address::from_host_order_ulong(OSPF_ALLDROUTERS));
    }
    else
    {
        tnt::Log::info(colors::blue, "OSPF: leave ALL DR ROUTERS group on interface #", iface);
        leave_multicast_group(iface, tnt::ip::Address::from_host_order_ulong(OSPF_ALLDROUTERS));
    }
}

void ControlElement::ControlElementImpl::arp_external_request(const std::shared_ptr<ce::InterconnectionElement>& ie, const ArpPacket& pkt, uint16_t port)
{
    auto proto_src = arp_proto_target(pkt);
    auto iface = get_interface(proto_src);

    if (iface)
    {
        auto hw_src = iface->hw_address();
        auto hw_dst = arp_hw_source(pkt);
        auto out_pkt = arp_packet(ArpPacketOperation::Reply, hw_src, hw_dst, proto_src, arp_proto_source(pkt));

        send_packet(ie, port, hw_src, hw_dst, L2Proto::Arp, std::string(reinterpret_cast<const char*>(&out_pkt), sizeof(out_pkt)));
    }
}

void ControlElement::ControlElementImpl::arp_internal_request(const std::shared_ptr<ce::InterconnectionElement>& ie, const ArpPacket& pkt, uint16_t port)
{
    auto ip = arp_proto_target(pkt);
    auto fe_ip = arp_proto_source(pkt);
    auto fe_hw = arp_hw_source(pkt);
    auto address = address_by_network(ip);

    if (address)
    {
        try
        {
            auto idx = static_cast<const AddressInfo>(*address).port_index;
            const auto& p = external_interfaces_.get_port(idx);

            auto ext_src_ip = tnt::ip::Address::from_string(address->str());
        
            nat_table_.insert(std::make_pair(ip, std::make_tuple(ext_src_ip, fe_ip, fe_hw, ie, port)));

            const auto& iface = get_interface(idx);

            if (!iface)
            {
                return;
            }

            auto hw_src = iface->hw_address();

            auto out_pkt = arp_packet(ArpPacketOperation::Request, hw_src, tnt::MacAddress::broadcast(), ext_src_ip, ip);
            send_packet(get_ie(p.ie), p.port, hw_src, tnt::MacAddress::broadcast(), L2Proto::Arp, std::string(reinterpret_cast<const char*>(&out_pkt), sizeof(out_pkt)));
        }
        catch (NonExistentInterface& ex)
        {
            tnt::Log::error("ControlElement::ControlElementImpl::arp_internal_request error: ", ex.what());
        }
    }
}

void ControlElement::ControlElementImpl::arp_external_reply(const ArpPacket& pkt)
{
    auto ip = arp_proto_source(pkt);
    auto hw_src = arp_hw_source(pkt);

    // The key is the target ip, the value tuples are: [external src ip, fe ip, fe mac, ie, port]
    auto range = tnt::equal_range(nat_table_, ip);

    for (const auto& v : range)
    {
        auto out_pkt = arp_packet(ArpPacketOperation::Reply, hw_src, std::get<2>(v.second), ip, std::get<1>(v.second));
        send_packet(std::get<3>(v.second), std::get<4>(v.second), hw_src, std::get<2>(v.second), L2Proto::Arp, std::string(reinterpret_cast<const char*>(&out_pkt), sizeof(out_pkt)));
    }

    if (std::begin(range) != std::end(range))
    {
        nat_table_.erase(ip);
    }
}

void ControlElement::ControlElementImpl::send_packet(uint16_t iface, const std::string& packet)
{
    try
    {
        const auto& p = external_interfaces_.get_port(iface);
        const auto& ie = get_ie(p.ie);

        if (ie)
        {
            ie->send(std::make_unique<message::PacketOut>(packet, p.port));
        }
    }
    catch (NonExistentInterface& ex)
    {
        tnt::Log::error("ControlElement::ControlElementImpl::send_packet error: ", ex.what());
    }
}

void ControlElement::ControlElementImpl::send_packet(const std::shared_ptr<ce::InterconnectionElement>& ie, uint16_t port, const tnt::MacAddress& src, const tnt::MacAddress& dst, L2Proto proto, const std::string& packet)
{
    if (!ie)
    {
        return;
    }

    //ie->send(std::make_unique<message::PacketOut>(create_eth_packet(packet, src, dst, proto), port));
}*/

template <class T> void ControlElement::ControlElementImpl::create_multicast_flow(uint16_t /*iface*/, const tnt::ip::Address& /*address*/)
{
    /*try
    {
        const auto& p = external_interfaces_.get_port(iface);
        const auto& ie = get_ie(p.ie);

        if (!ie)
        {
            tnt::Log::error("No ie for interface ", iface);

            return;
        }

        auto flow = protocol::Flow().filter(from_ip_dst(address)).filter(from_port(p.port)).add_action(to_controller());

        ie->send(std::make_shared<T>(flow));
    }
    catch (NonExistentInterface& ex)
    {
        tnt::Log::error("ControlElement::ControlElementImpl::send_packet error: ", ex.what());
    }*/
}

void ControlElement::ControlElementImpl::join_multicast_group(uint16_t iface, const tnt::ip::Address& address)
{
    create_multicast_flow<message::AddFlow>(iface, address);
}

void ControlElement::ControlElementImpl::leave_multicast_group(uint16_t iface, const tnt::ip::Address& address)
{
    create_multicast_flow<message::RemoveFlow>(iface, address);
}

void ControlElement::ControlElementImpl::start_http_server()
{
    activities_.push_back(tnt::async([] ()
    {
        tnt::activity::HttpServer server;
        server(factory::create_io_end_point("httpd"), "html");
    }));
}

void ControlElement::ControlElementImpl::start_lcp()
{
   if (!lcp_started_)
    {
        auto all_connected = tnt::all_of(elements_, [] (const auto& e)
        {
            return e->connected();
        });

        if (all_connected)
        {
            activities_.push_back(tnt::async([] ()
            {
                activity::ControlElementLCP lcp;
                lcp();
            }));
        }
    } 
}

void ControlElement::ControlElementImpl::lcp_started(const event::LCPStarted& /*event*/)
{
    lcp_started_ = true;

    for_each_element([&] (const auto& se)
    {
        if (se->connected())
        {
            tnt::Application::raise(event::ServiceElementActive(se));
            tnt::Log::info(colors::red, "Connected Service Element ", se->display_name());
        }

        if (tnt::any_of_is<service::Interconnection>(se->services()))
        {
            auto notify_enabled = tnt::visit_any_of<service::PortStatusNotification>(se->services(), [this] (const auto& service)
            {
                service->subscribe([this] (const std::string& port, PortStatus status)
                {
                    tnt::Log::info(colors::blue, "Local port ", port, status == PortStatus::Up ? " UP" : " DOWN");
                });
            });

            if (!notify_enabled)
            {
                // The IE doesn't have any notification for the port status change,
                // so we need to set the carrier up "manually".
                external_interfaces_.for_each(se->display_name(), [&] (int index)
                {
                    module_->send(std::make_unique<message::DethLinkChange>(index, true));
                });
            }
        }
    });

    start_http_server();
}

void ControlElement::ControlElementImpl::connection_reset(const tnt::event::ConnectionReset& event)
{
    auto it = elements_table_.find(event.protocol());

    if (it != std::end(elements_table_))
    {
        const auto& se = *it->second;
        assert(se);

        se->disconnect();
        elements_table_.erase(it);

        tnt::Log::info(colors::red, "Disconnected element ", se->display_name());
        tnt::Application::raise(event::ServiceElementDisconnected(se));
    }
}

void ControlElement::ControlElementImpl::connect_service_element(const event::ServiceElementConnected& event)
{
    auto it = get_element(event.name());

    if (it == std::end(elements_))
    {
        tnt::Log::info(colors::red, "Unknown Service Element ", event.name(), ", refusing connection.");

        return;
    }

    auto se = *it;
    se->connect(event.address(), event.proto(), event.services());

    elements_table_[event.proto().get()] = it;

    add_routes(se->services());

    start_lcp();
}

void ControlElement::ControlElementImpl::operator()()
{
    thread_id_ = std::this_thread::get_id();

	std::atomic_bool running{ true };

	register_handler([&] (const tnt::event::Quit& /*event*/)
	{
		running = false;
	});

    configure_network();

    activities_.push_back(tnt::async([] ()
    {
        activity::DiscoverServiceElements()();
    }));

    activities_.push_back(tnt::async([] ()
    {
        activity::DiscoverOpenflowElements()();
    }));

	kernel_ = factory::create_kernel_protocol();
	kernel_->start();

	register_listeners();

	tnt::for_all(addresses_, [&] (const auto& address)
	{
		const auto& ai = static_cast<const AddressInfo>(*address);
		const auto& iface = get_interface(ai.port_index);

		std::ostringstream ss;
		ss << "ip address add " << address->str() << "/" << address->prefix() << " dev " << iface->name();
		std::system(ss.str().c_str());
	});

    // Initialize the module_ protocol only after the devices (deth0, etc...) have been created, so the Netlink socket 
    // in the module will be created in the correct namespace.
	module_ = factory::create_deth_protocol();
    module_->start();

    start_lcp();

    while (running)
    {
        wait_event();
    }

	tnt::for_all(interfaces_, [&] (const auto& iface)
	{
        std::ostringstream ss;
        ss << "ip link del " << iface->name();
        std::system(ss.str().c_str());
	});
}

} // namespace activity
} // namespace drop

