
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

#include <algorithm>
#include <sstream>
#include <future>
#include <unordered_map>
#include <cassert>

#include "activity/network/arp_request.hpp"
#include "activity/gal/ce_lcp.hpp"

#include "network/service_element.hpp"

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

#include "exception/drop_exception.hpp"

#include "message/network/management.hpp"
#include "message/network/interface_list.hpp"
#include "message/network/port_stats_request.hpp"
#include "message/network/openflow_messages.hpp"
#include "message/virtual_interfaces_messages.hpp"

#include "router/address.hpp"
#include "router/route.hpp"
#include "router/interface.hpp"
#include "protocol/openflow/flow.hpp"
#include "router/flow_query.hpp"
#include "router/ports.hpp"
#include "router/arp_packet.hpp"
#include "router/arp_packet_factory.hpp"
#include "router/eth_factory.hpp"

#include "protocol/protocol.hpp"

#include "service/forwarding.hpp"
#include "service/interconnection.hpp"
#include "service/openflow_management.hpp"
#include "service/port_status_notification.hpp"

#include "util/configuration.hpp"

#include "application.hpp"
#include "dynamic_pointer_visitor.hpp"
#include "range.hpp"
#include "ip_address.hpp"
#include "mac_address.hpp"
#include "multicast.hpp"
#include "containers.hpp"
#include "log.hpp"
#include "dump.hpp"
#include "containers.hpp"
#include "async.hpp"

namespace drop {
namespace activity {

void ControlElement::ControlElementImpl::register_listeners()
{
    register_handler([this] (const tnt::event::ConnectionReset& event)
    {
		connection_reset(event);
    });

    register_handler([this] (const event::LCPStarted& event)
    {
        lcp_started(event);
    });

    register_handler([this] (const event::ServiceElementConnected& event)
    {
		connect_service_element(event);
    });

    register_handler([this] (const std::shared_ptr<event::InterfacesRequest>& event)
    {
		for_each_interface(event->func());
        event->notify();
    });

    register_handler([this] (const std::shared_ptr<event::ExternalInterfacesRequest>& event)
    {
		external_interfaces_.for_each(event->func());
        event->notify();
    });

    register_handler([this] (const std::shared_ptr<event::AddressesRequest>& event)
    {
		for_each_address(event->func());
        event->notify();
    });

    register_handler([this] (const std::shared_ptr<event::RoutesRequest>& event)
    {
		for_each_route(event->func());
        event->notify();
    });

    register_handler([this] (const std::shared_ptr<event::ServiceElementsRequest>& event)
    {
		for_each_element(event->func());
        event->notify();
    });

    register_handler([this] (const event::ExternalInterfacesDataUpdate& event)
    {
		update_external_port_stats(event);
    });

    register_handler([this] (const event::DethOutPacket& /*event*/)
    {
		//deth_out_packet(event);
    });

    register_handler([this] (const event::LocalPortAdded& event)
    {
        local_port_added(event);
    });

    register_handler([this] (const event::LocalPortDeleted& event)
    {
        local_port_removed(event);
    });

    register_handler([this] (const event::RouteList& /*event*/)
    {
		//local_routes_list(event);
    });

	register_handler([this] (const event::LocalRouteAdded& event)
    {
        local_route_added(event);
    });

    register_handler([this] (const event::LocalRouteRemoved& event)
    {
        local_route_removed(event);
    });

    register_handler([this] (const event::LocalAddressAdded& event)
    {
        local_address_added(event);
    });

    register_handler([this] (const event::LocalAddressRemoved& event)
    {
        local_address_removed(event);
    });

    register_handler([this] (const event::LocalPortAdminUp& event)
    {
        local_port_admin_up(event);
    });

    register_handler([this] (const event::LocalPortAdminDown& event)
    {
        local_port_admin_down(event);
    });

    register_handler([this] (const event::PortStatusChange& event)
    {
		port_status_change(event);
    });

    register_handler([this] (const event::OspfMulticastChange& /*event*/)
    {
		//ospf_multicast_change(event);
    });

    /*register_handler([this] (const event::PacketIn& event)
    {
		packet_in(event);
    });

    register_handler([this] (const event::PortStatsRequest& event)
    {
		port_stats_request(event);
    });

    register_handler([this] (const event::PortStatsReply& event)
    {
		port_stats_reply(event);
    });
    
    register_handler([this] (const event::ArpRequest& event)
	{
		arp_request(event);
	});

	register_handler([&] (const event::TrafficProfileChange& event)
    {
		traffic_profile_change(event);
    });

    register_handler([this] (const event::AddRouteSuccess& event)
    {
        assert(event);

        // TODO:
    });

    register_handler([this] (const event::AddRouteFailure& event)
    {
        assert(event);

        // TODO:
    });

    register_handler([this] (const event::MigrationRequest& event)
    {
        tnt::sync([this, event] ()
        {
            notify_all(user_interfaces_, std::string(R"(<ElementSuspended element=")") + event->fe() + R"(" />)");
        });
    });*/
}

void ControlElement::ControlElementImpl::port_status_change(const event::PortStatusChange& event)
{
    auto it = elements_table_.find(event.protocol());

    if (it != std::end(elements_table_))
    {
        const auto& se = *it->second;
        assert(se);

        auto index = external_interfaces_.get_index(se->display_name(), event.port(), std::nothrow);

        if (index > 0)
        {
            module_->send(std::make_unique<message::DethLinkChange>(index, event.status() == PortStatus::Up));
        }

	    tnt::Log::info(colors::red, "Port ", event.port(), " status changed: ", event.status() == PortStatus::Up ? "UP" : "DOWN");
    }
}

void ControlElement::ControlElementImpl::packet_in(const std::shared_ptr<event::PacketIn>& /*event*/)
{
	/*assert(event->protocol);

    const auto& flow = event->flow();
    auto l2 = get_l2_proto(flow);

    async([=] ()
    {
        if (l2 == L2Proto::Arp)
        {
            manage_arp(flow, event->buffer(), event->protocol);
        }
        else if (l2 == L2Proto::IPv4)
        {
            manage_ip(flow, event->buffer(), event->protocol);
        }
    });*/
}

void ControlElement::ControlElementImpl::port_stats_request(const std::shared_ptr<event::PortStatsRequest>& /*event*/)
{
	/*auto p = ie_and_port_by_interface(event->interface());
    auto& ie = p.first;

    if (ie)
    {
        ie->send(std::make_unique<message::PortStatsRequest>(p.second));
    }*/
}

void ControlElement::ControlElementImpl::port_stats_reply(const std::shared_ptr<event::PortStatsReply>& /*event*/)
{
	/*for (const auto& s : event->stats())
    {
        tnt::Log::info(colors::red, "Port #", s.port_no, " rx_packets: ", s.rx_packets);
    }*/
}

void ControlElement::ControlElementImpl::arp_request(const std::shared_ptr<event::ArpRequest>& /*event*/)
{
    activities_.push_back(tnt::async([] ()
    {
        //activity::ArpRequest(event)();
    }));
}

void ControlElement::ControlElementImpl::deth_out_packet(const std::shared_ptr<event::DethOutPacket>& /*event*/)
{
	//send_packet(event->index(), event->packet());
}

void ControlElement::ControlElementImpl::ospf_multicast_change(const std::shared_ptr<event::OspfMulticastChange>& /*event*/)
{
	//manage_multicast(event->interface());
}

void ControlElement::ControlElementImpl::traffic_profile_change(const std::shared_ptr<event::TrafficProfileChange>& /*event*/)
{
    //slot_change(event->slot());
}

void ControlElement::ControlElementImpl::traffic_profile_change(const std::shared_ptr<event::SlotChange>& /*event*/)
{
    //slot_change(event->num());
}

void ControlElement::ControlElementImpl::slot_change(int /*num*/)
{
    /*assert(ie_);

	auto flow_sleep = tnt::Configuration::get("topology.delay").as<int>();

	for (++current_profile_slot_; current_profile_slot_ <= num; ++current_profile_slot_)
	{
        auto cmds = commands_.equal_range(current_profile_slot_);

        for (auto it = cmds.first; it != cmds.second; ++it)
		{
            std::system(it->second.c_str());
		}

		auto range = flows_.equal_range(current_profile_slot_);

		for (auto it = range.first; it != range.second; ++it)
		{
            tnt::Log::info(colors::cyan, "Sending a new flow to the OF controller.");
			std::this_thread::sleep_for(std::chrono::milliseconds(flow_sleep));
			ie_->send(std::make_unique<message::AddFlow>(it->second));
		}
	}*/
}

void ControlElement::ControlElementImpl::local_port_added(const event::LocalPortAdded& event)
{
    tnt::Log::info(colors::blue, "Added local port ", event.port()->info().name);
}

void ControlElement::ControlElementImpl::local_port_removed(const event::LocalPortDeleted& event)
{
    tnt::Log::info(colors::blue, "Deleted local port ", event.port()->info().name);
}

void ControlElement::ControlElementImpl::local_routes_list(const event::RouteList& event)
{
    tnt::Log::info(colors::cyan, "Local routes list: #", event.routes().size(), " routes");

    for (const auto& r : event.routes())
    {
        add(r);
    }
}

void ControlElement::ControlElementImpl::local_route_added(const event::LocalRouteAdded& event)
{
    add(event.route());
}

void ControlElement::ControlElementImpl::local_route_removed(const event::LocalRouteRemoved& event)
{
    tnt::Log::info(colors::blue, "Removed local route: ", event.route());

	//remove(event->route());
}

void ControlElement::ControlElementImpl::local_address_added(const event::LocalAddressAdded& event)
{
    tnt::Log::info(colors::blue, "Added local address: ", event.address());
}

void ControlElement::ControlElementImpl::local_address_removed(const event::LocalAddressRemoved& event)
{
    tnt::Log::info(colors::blue, "Removed local address: ", event.address());
}

void ControlElement::ControlElementImpl::local_port_admin_up(const event::LocalPortAdminUp& event)
{
    tnt::Log::info(colors::blue, "Local port ", event.port()->info().name, " UP");
}

void ControlElement::ControlElementImpl::local_port_admin_down(const event::LocalPortAdminDown& event)
{
    tnt::Log::info(colors::blue, "Local port ", event.port()->info().name, " DOWN");
}

void ControlElement::ControlElementImpl::update_external_port_stats(const event::ExternalInterfacesDataUpdate& event)
{
    for (const auto& d : event.data())
    {
        // DethLinkStats(uint16_t iface, uint64_t rxpkts, uint64_t rxbytes, uint64_t txpkts, uint64_t txbytes);
        module_->send(std::make_unique<message::DethLinkStats>(d.iface, d.rxpkts, d.rxbytes, d.txpkts, d.txbytes));
    }
}

} // namespace activity
} // namespace drop
