
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

#ifndef DROP_ACTIVITY_CONTROL_ELEMENT_IMPL_HPP_
#define DROP_ACTIVITY_CONTROL_ELEMENT_IMPL_HPP_

#include <memory>
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
#include <future>

#include "activity/control_element.hpp"

#include "router/port_status.hpp"

#include "activity/concurrent_activity.hpp"

#include "util/external_interface_map.hpp"

#include "dispatch.hpp"
#include "mac_address_fwd.hpp"
#include "ip_address.hpp"
#include "range.hpp"

namespace tnt {

struct Protocol;

namespace event {

class ConnectionReset;

} // namespace event
} // namespace tnt

namespace pugi {

class xml_node;

} // namespace pugi

namespace drop {
namespace event {

struct LCPStarted;

class ServiceElementConnected;
class NewInterconnectionElement;
class PacketIn;
class PortStatsRequest;
class PortStatsReply;
class PortStatusChange;
class ArpRequest;
class DethOutPacket;
class OspfMulticastChange;
class TrafficProfileChange;
class LocalPortAdded;
class LocalPortDeleted;
class RouteList;
class LocalRouteAdded;
class LocalRouteRemoved;
class LocalAddressAdded;
class LocalAddressRemoved;
class LocalPortAdminUp;
class LocalPortAdminDown;
class SlotChange;
class ExternalInterfacesDataUpdate;

} // namespace event

namespace router {

class Interface;
class Route;
class Address;

} // namespace router

namespace ce {

class ServiceElement;

} // namespace ce

namespace protocol {

class Flow;

} // namespace protocol

struct RouteInfo;
struct AddressInfo;
struct PortInfo;
struct Protocol;
struct ArpPacket;
struct Service;

namespace activity {

class ControlElement::ControlElementImpl: private tnt::ConcurrentActivity
{
    // The key is the target ip, the value tuples are: [external src ip, fe ip, fe mac, ie, port]
    //using NatTable = std::multimap<tnt::ip::Address, std::tuple<tnt::ip::Address, tnt::ip::Address, tnt::MacAddress, std::shared_ptr<ce::InterconnectionElement>, uint16_t>>;
    using ElementsContainer = std::vector<std::shared_ptr<ce::ServiceElement>>;
    using ElementsIterator = ElementsContainer::iterator;
    using ElementsProtocolTable = std::unordered_map<tnt::Protocol*, ElementsContainer::iterator>;
    using ElementsProtocolIterator = ElementsProtocolTable::iterator;
public:
    ControlElementImpl() = default;
    ~ControlElementImpl();

    void operator()();
private:
    void configure_network();

    void load_nodes(const pugi::xml_node& root);
    void load_node(const pugi::xml_node& root);
    void load_interfaces(const pugi::xml_node& root);

    void register_listeners();

    bool is_sequential();

    tnt::Range<ElementsIterator> for_each_element();
    void for_each_element(std::function<void(const std::shared_ptr<ce::ServiceElement>&)> func);

    void for_each_external_interface(std::function<void(const std::shared_ptr<router::Interface>&)> func);
    void for_each_interface(std::function<void(const std::shared_ptr<router::Interface>&)> func);
    void for_each_address(std::function<void(const std::shared_ptr<router::Address>&)> func);
    void for_each_route(std::function<void(const std::shared_ptr<router::Route>&)> func);

    void add(const RouteInfo& route);
    void add(const AddressInfo& address);

    void add_routes(const std::vector<std::shared_ptr<Service>>& services);

    void remove(const RouteInfo& route);
    void remove(const AddressInfo& address);

    void set_state_up(const PortInfo& port);
    void set_state_down(const PortInfo& port);

    std::string get_interface_name(uint32_t index);
    uint32_t get_interface_index(const std::string& name);

    uint32_t get_on_link_interface(const RouteInfo& route);

    std::shared_ptr<router::Interface> get_interface(uint32_t index);
    std::shared_ptr<router::Interface> get_interface(const tnt::ip::Address& address);

    std::shared_ptr<router::Address> address_by_network(const tnt::ip::Address& address);

    ElementsIterator get_element(const std::string& name);

    void manage_arp(const protocol::Flow& flow, const std::string& buffer, const std::shared_ptr<tnt::Protocol>& proto);
    void manage_ip(const protocol::Flow& flow, const std::string& buffer, const std::shared_ptr<tnt::Protocol>& proto);

    /*void manage_ospf(const protocol::Flow& flow, const std::string& buffer, const std::shared_ptr<ce::InterconnectionElement>& ie);
    void manage_bgp(const protocol::Flow& flow, const std::string& buffer, const std::shared_ptr<ce::InterconnectionElement>& ie);
    void manage_multicast(int iface);

    void arp_external_request(const std::shared_ptr<ce::InterconnectionElement>& ie, const ArpPacket& pkt, uint16_t port);
    void arp_internal_request(const std::shared_ptr<ce::InterconnectionElement>& ie, const ArpPacket& pkt, uint16_t port);
    void arp_external_reply(const ArpPacket& pkt);

    void send_packet(uint16_t iface, const std::string& packet);
    void send_packet(const std::shared_ptr<ce::InterconnectionElement>& ie, uint16_t port, const tnt::MacAddress& src, const tnt::MacAddress& dst, L2Proto proto, const std::string& packet);*/

    template <class T> void create_multicast_flow(uint16_t iface, const tnt::ip::Address& address);

    void join_multicast_group(uint16_t iface, const tnt::ip::Address& address);
    void leave_multicast_group(uint16_t iface, const tnt::ip::Address& address);

	// Events handlers
	void connection_reset(const tnt::event::ConnectionReset& event);
    void lcp_started(const event::LCPStarted& event);
	void connect_service_element(const event::ServiceElementConnected& event);

    void port_status_change(const event::PortStatusChange& event);

	void packet_in(const std::shared_ptr<event::PacketIn>& event);
	void port_stats_request(const std::shared_ptr<event::PortStatsRequest>& event);
	void port_stats_reply(const std::shared_ptr<event::PortStatsReply>& event);
	void arp_request(const std::shared_ptr<event::ArpRequest>& event);
	void deth_out_packet(const std::shared_ptr<event::DethOutPacket>& event);
	void ospf_multicast_change(const std::shared_ptr<event::OspfMulticastChange>& event);
	void traffic_profile_change(const std::shared_ptr<event::TrafficProfileChange>& event);
    void traffic_profile_change(const std::shared_ptr<event::SlotChange>& event);

    void local_port_added(const event::LocalPortAdded& event);
	void local_port_removed(const event::LocalPortDeleted& event);
    void local_routes_list(const event::RouteList& event);
	void local_route_added(const event::LocalRouteAdded& event);
	void local_route_removed(const event::LocalRouteRemoved& event);
	void local_address_added(const event::LocalAddressAdded& event);
	void local_address_removed(const event::LocalAddressRemoved& event);
    void local_port_admin_up(const event::LocalPortAdminUp& event);
	void local_port_admin_down(const event::LocalPortAdminDown& event);

    void update_external_port_stats(const event::ExternalInterfacesDataUpdate& event);

    void slot_change(int num);

    void start_http_server();
    void start_lcp();
private:
    bool lcp_started_ = false;

    std::vector<std::future<void>> activities_;

    std::shared_ptr<tnt::Protocol> kernel_;
    std::shared_ptr<tnt::Protocol> module_;

	int current_profile_slot_ = 1;

    std::thread::id thread_id_;

    ElementsContainer elements_;

    std::vector<std::shared_ptr<router::Interface>> interfaces_;
    std::vector<std::shared_ptr<router::Address>> addresses_;
    std::vector<std::shared_ptr<router::Route>> routes_;

    ElementsProtocolTable elements_table_;

    //NatTable nat_table_;

    ExternalInterfaceMap external_interfaces_;
};

} // namespace activity
} // namespace drop

#endif
