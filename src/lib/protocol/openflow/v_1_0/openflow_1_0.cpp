
/*

Copyright (c) 2013, Giulio Scancarello (giulioscanca@hotmail.it), Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#include "openflow_1_0.hpp"

#include <cassert>
#include <cstring>

#include "event/network/port_status_change.hpp"

#include "protocol/openflow/v_1_0/structs.hpp"
#include "protocol/openflow/v_1_0/values.hpp"
#include "protocol/openflow/utils.hpp"
#include "protocol/openflow/openflow.hpp"
#include "protocol/openflow/flow.hpp"
#include "protocol/openflow/flow_flags.hpp"
#include "protocol/openflow/flow_filters.hpp"
#include "protocol/openflow/flow_actions.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "mac_address.hpp"
#include "ip_address.hpp"
#include "endianness.hpp"
#include "log.hpp"

using namespace std::literals;

namespace drop {
namespace protocol {
namespace {

static const uint8_t version = 0x01;

void set_wildcard(ofp_flow_mod_1_0& ofp_flow, ofp_flow_wildcards_1_0 flag)
{
    using E = std::underlying_type_t<ofp_flow_wildcards_1_0>;
    auto wildcards = static_cast<E>(ofp_flow.match.wildcards) & ~(static_cast<E>(flag));

    ofp_flow.match.wildcards = static_cast<ofp_flow_wildcards_1_0>(wildcards); // Setting the wildcard is clearing the bit(s).
}

void unset_wildcard(ofp_flow_mod_1_0& ofp_flow, ofp_flow_wildcards_1_0 flag)
{
    using E = std::underlying_type_t<ofp_flow_wildcards_1_0>;
    auto wildcards = static_cast<E>(ofp_flow.match.wildcards) | static_cast<E>(flag);

    ofp_flow.match.wildcards = static_cast<ofp_flow_wildcards_1_0>(wildcards); // Unsetting the wildcard is setting the bit(s).
}

template <class F> bool is_flag_set(F value, F flag)
{
    using E = typename std::underlying_type_t<F>;

    return (network_to_host_underlying(value) & static_cast<E>(flag)) != 0;
}

bool is_port_state_flag_set(ofp_phy_port_1_0 port, ofp_port_state_1_0 flag)
{
    return is_flag_set(port.state, flag);
}

void set_type(ofp_action_header_1_0& ofp_action, ofp_action_type_1_0 value)
{
    ofp_action.type = host_to_network(value);
}

void set_value(ofp_action_header_1_0& ofp_action, uint16_t value)
{
    auto val = htons(value);
    memcpy(ofp_action.pad, &val, 2);
    ofp_action.len = htons(8);
}

void set_value(ofp_action_header_1_0& ofp_action, ofp_port_1_0 port)
{
    auto val = host_to_network(port);
    memcpy(ofp_action.pad, &val, 2);
    ofp_action.len = htons(8);
}

void set_value2(ofp_action_header_1_0& ofp_action, uint16_t value)
{
    auto val = htons(value);
    memcpy(ofp_action.pad + 2, &val, 2);
    ofp_action.len = htons(8);
}
    
/*void set_value(ofp_action_header_1_0& ofp_action, uint32_t value)
{
    uint32_t val = htonl(value);
    memcpy(ofp_action.pad, &val, 4);
    ofp_action.len = htons(8);
}
    
void set_value(ofp_action_header_1_0& ofp_action, uint8_t value)
{
    memcpy(ofp_action.pad, &value, 1);
    ofp_action.len = htons(8);
}*/

ofp_action_header_1_0 set_value(ofp_action_header_1_0& ofp_action, const tnt::MacAddress& mac)
{
    auto raw = mac.raw();
    const char* ptr = raw.data();
    memcpy(ofp_action.pad, ptr, 4);

    auto action_cont = ofp_action_header_1_0();
    memcpy(reinterpret_cast<char*>(&action_cont.type), ptr + 4, sizeof(action_cont.type));
    ofp_action.len = htons(16);

    return action_cont;
}

// Filters
ofp_flow_mod_1_0& from_port(ofp_flow_mod_1_0& ofp_flow, uint16_t src)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::IN_PORT);
    ofp_flow.match.in_port = htons(src);

    return ofp_flow;
}

ofp_flow_mod_1_0& from_vlan(ofp_flow_mod_1_0& ofp_flow, uint16_t tag)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_VLAN);
    ofp_flow.match.dl_vlan = htons(tag);

    return ofp_flow;
}

ofp_flow_mod_1_0& from_proto(ofp_flow_mod_1_0& ofp_flow, L2Proto proto)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_TYPE);
    ofp_flow.match.dl_type = htons(static_cast<uint16_t>(proto));

    return ofp_flow;
}

ofp_flow_mod_1_0& from_proto(ofp_flow_mod_1_0& ofp_flow, L3Proto proto)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_TYPE);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::NW_PROTO);
    ofp_flow.match.dl_type = htons(static_cast<uint16_t>(L2Proto::IPv4));
	ofp_flow.match.nw_proto = static_cast<uint8_t>(proto);

    return ofp_flow;
}

ofp_flow_mod_1_0& from_hw_src(ofp_flow_mod_1_0& ofp_flow, const tnt::MacAddress& mac)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_SRC);
    auto raw_mac = mac.raw();
    std::copy_n(std::begin(raw_mac), 6, ofp_flow.match.dl_src);

    return ofp_flow;
}

ofp_flow_mod_1_0& from_hw_dst(ofp_flow_mod_1_0& ofp_flow, const tnt::MacAddress& mac)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_DST);
    auto raw_mac = mac.raw();
    std::copy_n(std::begin(raw_mac), 6, ofp_flow.match.dl_dst);

    return ofp_flow;
}

ofp_flow_mod_1_0& from_ip_src(ofp_flow_mod_1_0& ofp_flow, const tnt::ip::Address& ip, int prefix)
{
    using E = std::underlying_type_t<ofp_flow_wildcards_1_0>;

    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_TYPE);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::NW_SRC_MASK);
    unset_wildcard(ofp_flow, static_cast<ofp_flow_wildcards_1_0>((32 - prefix) << static_cast<E>(ofp_flow_wildcards_1_0::NW_SRC_SHIFT)));
    ofp_flow.match.dl_type = htons(static_cast<uint16_t>(L2Proto::IPv4));
    ofp_flow.match.nw_src = static_cast<uint32_t>(ip.to_net_order_ulong());

    return ofp_flow;
}

ofp_flow_mod_1_0& from_ip_dst(ofp_flow_mod_1_0& ofp_flow, const tnt::ip::Address& ip, int prefix)
{
    using E = std::underlying_type_t<ofp_flow_wildcards_1_0>;

    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_TYPE);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::NW_DST_MASK);
    unset_wildcard(ofp_flow, static_cast<ofp_flow_wildcards_1_0>((32 - prefix) << static_cast<E>(ofp_flow_wildcards_1_0::NW_DST_SHIFT)));
    ofp_flow.match.dl_type = htons(static_cast<uint16_t>(L2Proto::IPv4));
    ofp_flow.match.nw_dst = static_cast<uint32_t>(ip.to_net_order_ulong());

    return ofp_flow;
}

ofp_flow_mod_1_0& from_transport_port_src(ofp_flow_mod_1_0& ofp_flow, L3Proto proto, uint16_t port)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_TYPE);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::NW_PROTO);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::TP_SRC);
    ofp_flow.match.dl_type = htons(static_cast<uint16_t>(L2Proto::IPv4));
    ofp_flow.match.nw_proto = static_cast<uint8_t>(proto);
    ofp_flow.match.tp_src = htons(port);

        return ofp_flow;
}

ofp_flow_mod_1_0& from_transport_port_dst(ofp_flow_mod_1_0& ofp_flow, L3Proto proto, uint16_t port)
{
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::DL_TYPE);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::NW_PROTO);
    set_wildcard(ofp_flow, ofp_flow_wildcards_1_0::TP_DST);
    ofp_flow.match.dl_type = htons(static_cast<uint16_t>(L2Proto::IPv4));
    ofp_flow.match.nw_proto = static_cast<uint8_t>(proto);
    ofp_flow.match.tp_dst = htons(port);

    return ofp_flow;
}

// Actions
ofp_action_header_1_0 to_controller(uint16_t size)
{
    auto action = ofp_action_header_1_0();
    set_type(action, ofp_action_type_1_0::OUTPUT);
    set_value(action, ofp_port_1_0::CONTROLLER);
    set_value2(action, size);

    return action;
}

std::vector<ofp_action_header_1_0> to_ports(const std::vector<uint16_t>& dst)
{
    std::vector<ofp_action_header_1_0> actions;

    auto action = ofp_action_header_1_0();
    set_type(action, ofp_action_type_1_0::OUTPUT);
    
    for (auto p : dst)
    {
        uint16_t val16 = p;
        set_value(action, val16);

        actions.push_back(action);
    }

    return actions;
}

std::pair<ofp_action_header_1_0, ofp_action_header_1_0> set_hw_src(const tnt::MacAddress& mac)
{
    auto action = ofp_action_header_1_0();
    set_type(action, ofp_action_type_1_0::SET_DL_SRC);
    auto action_cont = set_value(action, mac);

    return { action, action_cont };
}

std::pair<ofp_action_header_1_0, ofp_action_header_1_0> set_hw_dst(const tnt::MacAddress& mac)
{
    auto action = ofp_action_header_1_0();
    set_type(action, ofp_action_type_1_0::SET_DL_DST);
    auto action_cont = set_value(action, mac);

    return { action, action_cont };
}

ofp_action_header_1_0 set_vlan(uint16_t tag)
{
    auto action = ofp_action_header_1_0();
    set_type(action, ofp_action_type_1_0::SET_VLAN_VID);
	set_value(action, tag);

    return action;
}

ofp_action_header_1_0 strip_vlan()
{
    auto action = ofp_action_header_1_0();
    set_type(action, ofp_action_type_1_0::STRIP_VLAN);

    return action;
}

std::string actions_to_string(const std::vector<ofp_action_header_1_0>& actions)
{
    std::string str;

    for (const auto& a : actions)
    {
        str.append(to_mem_buffer(a));
    }

    return str;
}

void add_flag(FlowFlag* flag, ofp_flow_mod_1_0& ofp_flow)
{
    if (tnt::visit<Priority>(flag, [&] (auto f) { ofp_flow.priority = f->value(); }))
    {
        return;
    }

    if (tnt::visit<Buffer>(flag, [&] (auto f) { ofp_flow.buffer_id = f->value(); }))
    {
        return;
    }

    if (tnt::visit<Cookie>(flag, [&] (auto f) { ofp_flow.cookie = f->value(); }))
    {
        return;
    }

    if (tnt::visit<IdleTimeout>(flag, [&] (auto f) { ofp_flow.idle_timeout = f->value(); }))
    {
        return;
    }

    if (tnt::visit<HardTimeout>(flag, [&] (auto f) { ofp_flow.hard_timeout = f->value(); }))
    {
        return;
    }

    if (tnt::visit<OutPort>(flag, [&] (auto f) { ofp_flow.out_port = f->value(); }))
    {
        return;
    }

    if (tnt::visit<Flags>(flag, [&] (auto f) { ofp_flow.flags = f->value(); }))
    {
        return;
    }
}

void add_flags(const Flow& flow, ofp_flow_mod_1_0& ofp_flow)
{
    for (const auto& f : flow.flags())
    {
        add_flag(f.get(), ofp_flow);
    }
}

void add_filter(ofp_flow_mod_1_0& ofp_flow, FlowFilter* filter)
{
    if (tnt::visit<FromPort>(filter, [&] (auto f) { from_port(ofp_flow, f->port()); }))
    {
        return;
    }

    if (tnt::visit<FromVlan>(filter, [&] (auto f) { from_vlan(ofp_flow, f->tag()); }))
    {
        return;
    }

    if (tnt::visit<FromL2Proto>(filter, [&] (auto f) { from_proto(ofp_flow, f->proto()); }))
    {
        return;
    }
       
    if (tnt::visit<FromL3Proto>(filter, [&] (auto f) { from_proto(ofp_flow, f->proto()); }))
    {
        return;
    }
       
    if (tnt::visit<FromHwSrc>(filter, [&] (auto f) { from_hw_src(ofp_flow, f->mac()); }))
    {
        return;
    }
       
    if (tnt::visit<FromHwDst>(filter, [&] (auto f) { from_hw_dst(ofp_flow, f->mac()); }))
    {
        return;
    }
       
    if (tnt::visit<FromIpSrc>(filter, [&] (auto f) { from_ip_src(ofp_flow, f->ip(), f->prefix()); }))
    {
        return;
    }
       
    if (tnt::visit<FromIpDst>(filter, [&] (auto f) { from_ip_dst(ofp_flow, f->ip(), f->prefix()); }))
    {
        return;
    }
       
    if (tnt::visit<FromTransportPortSrc>(filter, [&] (auto f) { from_transport_port_src(ofp_flow, f->proto(), f->port()); }))
    {
        return;
    }
       
    if (tnt::visit<FromTransportPortDst>(filter, [&] (auto f) { from_transport_port_dst(ofp_flow, f->proto(), f->port()); }))
    {
        return;
    }
}

void add_filters(const Flow& flow, ofp_flow_mod_1_0& ofp_flow)
{
    ofp_flow.match.wildcards = ofp_flow_wildcards_1_0::ALL;

    for (const auto& f : flow.filters())
    {
        add_filter(ofp_flow, f.get());
    }

    ofp_flow.match.wildcards = host_to_network(ofp_flow.match.wildcards);
}

void get_action(FlowAction* action, std::vector<ofp_action_header_1_0>& actions)
{
    if (tnt::visit<ToController>(action, [&] (auto a)
    {
        actions.push_back(to_controller(a->size()));
    }))
    {
        return;
    }

    if (tnt::visit<ToPorts>(action, [&] (auto a)
    {
        for (const auto& p : to_ports(a->ports()))
        {
            actions.push_back(p);
        }
    }))
    {
        return;
    }

    if (tnt::visit<Loop>(action, [&] (auto /*a*/)
    {
        actions.push_back(to_ports({ static_cast<uint16_t>(ofp_port_1_0::IN_PORT) })[0]);
    }))
    {
        return;
    }

    if (tnt::visit<Flood>(action, [&] (auto /*a*/)
    {
        actions.push_back(to_ports({ static_cast<uint16_t>(ofp_port_1_0::FLOOD) })[0]);
    }))
    {
        return;
    }

    if (tnt::visit<SetHwSrc>(action, [&] (auto a)
    {
        auto p = set_hw_src(a->mac());
        actions.push_back(p.first);
        actions.push_back(p.second);
    }))
    {
        return;
    }

    if (tnt::visit<SetHwDst>(action, [&] (auto a)
    {
        auto p = set_hw_dst(a->mac());
        actions.push_back(p.first);
        actions.push_back(p.second);
    }))
    {
        return;
    }

    if (tnt::visit<SetVlan>(action, [&] (auto a)
    {
        actions.push_back(set_vlan(a->tag()));
    }))
    {
        return;
    }

    if (tnt::is<StripVlan>(action))
    {
        actions.push_back(strip_vlan());

        return;
    }

    tnt::Log::error("Unsupported Action");
}

std::vector<ofp_action_header_1_0> get_actions(const Flow& flow)
{
    std::vector<ofp_action_header_1_0> actions;

    for (const auto& f : flow.actions())
    {
        get_action(f.get(), actions);
    }

    return actions;
}

std::string get_flow(const Flow& flow, uint32_t xid, ofp_flow_mod_command_1_0 command)
{
    auto ofp_flow = ofp_flow_mod_1_0();
    
    ofp_flow.header.type = ofp_type_1_0::FLOW_MOD;
    ofp_flow.header.version = version;
    ofp_flow.header.xid = htonl(xid);
    
    ofp_flow.command = host_to_network(command);
    ofp_flow.buffer_id = htonl(OFP_NO_BUFFER);
    ofp_flow.out_port = host_to_network_underlying(ofp_port_1_0::NONE);
    ofp_flow.flags = host_to_network_underlying(ofp_flow_mod_flags_1_0::SEND_FLOW_REM);
    ofp_flow.priority = htons(OFP_DEFAULT_PRIORITY);
    
    add_flags(flow, ofp_flow);
    add_filters(flow, ofp_flow);

    auto actions = get_actions(flow);
    ofp_flow.header.length = htons(sizeof(ofp_flow_mod_1_0) + sizeof(ofp_action_header_1_0) * actions.size());

    return to_mem_buffer(ofp_flow) + actions_to_string(actions);
}

} // namespace

Openflow_1_0::Openflow_1_0(Openflow* parent): parent_(parent)
{
    assert(parent_);
    register_handlers();
}

void Openflow_1_0::init()
{
    auto packet = ofp_header_1_0();
    packet.version = version;
    packet.type = ofp_type_1_0::FEATURES_REQUEST;
    packet.length = htons(sizeof(packet));
    packet.xid = htonl(create_xid());

    parent_->write(to_mem_buffer(packet));

    std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.

    auto sw_config = ofp_switch_config_1_0();

    sw_config.header.version = version;
    sw_config.header.type = ofp_type_1_0::SET_CONFIG;
    sw_config.header.length = htons(sizeof(sw_config));
    sw_config.header.xid = htonl(create_xid());
    sw_config.flags = ofp_config_flags::FRAG_NORMAL;
    sw_config.miss_send_len = 0x0080;

    parent_->write(to_mem_buffer(sw_config));
}

void Openflow_1_0::add(const Flow& flow)
{
    auto xid = create_xid();
    parent_->write(get_flow(flow, xid, ofp_flow_mod_command_1_0::ADD));
}

void Openflow_1_0::remove(const Flow& flow)
{
    auto xid = create_xid();
    parent_->write(get_flow(flow, xid, ofp_flow_mod_command_1_0::DELETE));
}

void Openflow_1_0::remove_all()
{
    auto pkt = ofp_flow_mod_1_0();

    pkt.header.type = ofp_type_1_0::FLOW_MOD;
    pkt.header.version = version;
    pkt.header.xid = htonl(create_xid());
    pkt.header.length = htons(sizeof(pkt));

    pkt.match.wildcards = ofp_flow_wildcards_1_0::ALL; // 0xffffffff;

    pkt.command = host_to_network(ofp_flow_mod_command_1_0::DELETE);
    pkt.out_port = host_to_network_underlying(ofp_port_1_0::NONE);
    pkt.flags = host_to_network_underlying(ofp_flow_mod_flags_1_0::SEND_FLOW_REM);

    parent_->write(to_mem_buffer(pkt));
}

void Openflow_1_0::send_packet(const std::string& buffer, uint16_t port)
{
    auto pkt = ofp_packet_out_1_0();

    pkt.header.type = ofp_type_1_0::PACKET_OUT;
    pkt.header.version = version;
    pkt.header.xid = htonl(create_xid());
    pkt.header.length = htons(sizeof(pkt) + buffer.size());

    pkt.in_port = htons(port);

    parent_->write(to_mem_buffer(pkt) + buffer);
}

void Openflow_1_0::request_port_stats(uint16_t /*port*/)
{

}

void Openflow_1_0::echo(const std::string& message)
{
    auto ptr = message.data();
    auto request = reinterpret_cast<const ofp_header_1_0*>(ptr);

    auto reply = ofp_header_1_0();
    reply.length = request->length;
    reply.type = ofp_type_1_0::ECHO_REPLY;
    reply.version = version;
    reply.xid = request->xid;

    auto payload_len = ntohs(request->length) - sizeof(ofp_header_1_0);

    parent_->write(to_mem_buffer(reply) + std::string(ptr + sizeof(ofp_header_1_0), payload_len));
}

void Openflow_1_0::port_status(const std::string& message)
{
    auto temp = reinterpret_cast<const ofp_port_status_1_0*>(message.data());

    if (temp->reason == ofp_port_reason::MODIFY)
    {
        tnt::Application::raise(event::PortStatusChange(parent_, ntohs(temp->desc.port_no), is_port_state_flag_set(temp->desc, ofp_port_state_1_0::LINK_DOWN) != 0 ? PortStatus::Down : PortStatus::Up));
    }
    else
    {
        tnt::Log::info(colors::blue, "PORT_STATUS: Reason #", static_cast<unsigned int>(temp->reason));
    }
}

void Openflow_1_0::get_config(const std::string& /*message*/)
{
    tnt::Log::info(colors::blue, "GET_CONFIG_REPLY");
}

void Openflow_1_0::stats(const std::string& /*message*/)
{
    tnt::Log::info(colors::blue, "STATS_REPLY");
}

void Openflow_1_0::flow_removed(const std::string& /*message*/)
{
    tnt::Log::info(colors::blue, "FLOW_REMOVED");
}
    
void Openflow_1_0::packet_in(const std::string& message)
{
    auto temp = reinterpret_cast<const ofp_packet_in_1_0*>(message.data());
    tnt::Log::info(colors::blue, "PACKET_IN SW: Reason #", static_cast<unsigned int>(temp->reason));
}

void Openflow_1_0::unmanaged_packet(const ofp_header_1_0* pkt)
{
    tnt::Log::info(colors::blue, "Received a packet with type: #", static_cast<unsigned int>(pkt->type));
}

void Openflow_1_0::error(const std::string& message)
{
    auto ptr = message.data();
    auto pkt = reinterpret_cast<const ofp_error_msg_1_0*>(ptr);

    try
    {
        error_dispatcher_.inject_object(network_to_host(pkt->type), pkt, ptr);
    }
    catch (tnt::ListenerNotFound&)
    {
        tnt::Log::error("Error type not defined in the Openflow Protocol version ", static_cast<unsigned int>(version));
    }
}

void Openflow_1_0::features(const std::string& message)
{
    auto pkt = reinterpret_cast<const ofp_switch_features_1_0*>(message.data());

    // TODO: Parse the packet, and get the features and the status of the switch.
    parent_->set_features(pkt->datapath_id);
}
    
void Openflow_1_0::packet(const std::string& message)
{
    auto pkt = reinterpret_cast<const ofp_header_1_0*>(message.data());

    try
    {
        message_dispatcher_.inject_object(pkt->type, message);
    }
    catch (tnt::ListenerNotFound&)
    {
        unmanaged_packet(pkt);
    }
}

void Openflow_1_0::error_hello_failed(const ofp_error_msg_1_0* pkt, const std::string& /*message*/)
{
    tnt::Log::error("Error type: HELLO_FAILED");
                
    if (ntohs(pkt->code) == OFPHFC_INCOMPATIBLE)
    {
        tnt::Log::error("Error code: OFPHFC_INCOMPATIBLE, No compatible version.");
    }
    else if (ntohs(pkt->code) == OFPHFC_EPERM)
    {
        tnt::Log::error("Error code: OFPHFC_EPERM, Permissions error.");
    }
    else
    {
        tnt::Log::error("Error code non definito nella versione OF 1.0.0");
    }
}

void Openflow_1_0::error_bad_request(const ofp_error_msg_1_0* pkt, const std::string& message)
{
    auto data_err = reinterpret_cast<const ofp_header_1_0*>(message.data() + sizeof(ofp_error_msg_1_0));

	switch (network_to_host_enum<ofp_bad_request_code_1_0>(pkt->code))
	{
    case ofp_bad_request_code_1_0::BAD_VERSION:
		tnt::Log::error("Error code: OFPBRC_BAD_VERSION, ofp_header_1_0.version not supported.");
		break;
	case ofp_bad_request_code_1_0::BAD_TYPE:
		tnt::Log::error("Error code: OFPBRC_BAD_TYPE, ofp_header_1_0.type not supported.");
		break;
	case ofp_bad_request_code_1_0::BAD_STAT:
		tnt::Log::error("Error code: OFPBRC_BAD_STAT, ofp_stats_request_1_0.type not supported.");
		break;
	case ofp_bad_request_code_1_0::BAD_VENDOR:
		tnt::Log::error("Error code: OFPBRC_BAD_VENDOR, Vendor not supported.");
		break;
	case ofp_bad_request_code_1_0::BAD_SUBTYPE:
		tnt::Log::error("Error code: OFPBRC_BAD_SUBTYPE, Vendor subtype not supported.");
		break;
	case ofp_bad_request_code_1_0::PERMISSION:
		tnt::Log::error("Error code: OFPBRC_EPERM, Permissions error.");
		break;
	case ofp_bad_request_code_1_0::BAD_LEN:
		tnt::Log::error("Error code: OFPBRC_BAD_LEN, Wrong request length for type.");
		break;
	case ofp_bad_request_code_1_0::BUFFER_EMPTY:
		tnt::Log::error("Error code: OFPBRC_BUFFER_EMPTY, Specified buffer has already been used.");
		break;
	case ofp_bad_request_code_1_0::BUFFER_UNKNOWN:
		tnt::Log::error("Error code: OFPBRC_BUFFER_UNKNOWN Specified buffer does not exist.");
		break;
	default:
		tnt::Log::error("Error code non definito nella versione OF 1.0.0");
	};

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_0::error_bad_action(const ofp_error_msg_1_0* pkt, const std::string& message)
{
    auto data_err = reinterpret_cast<const ofp_header_1_0*>(message.data() + sizeof(ofp_error_msg_1_0));

	switch (network_to_host_enum<ofp_bad_action_code_1_0>(pkt->code))
	{
    case ofp_bad_action_code_1_0::BAD_TYPE:
		tnt::Log::error("Error code: OFPBAC_BAD_TYPE, Unknown action type.");
		break;
	case ofp_bad_action_code_1_0::BAD_LEN:
		tnt::Log::error("Error code: OFPBAC_BAD_LEN, Length problem in actions.");
		break;
	case ofp_bad_action_code_1_0::BAD_VENDOR:
		tnt::Log::error("Error code: OFPBAC_BAD_VENDOR, Unknown vendor id specified.");
		break;
	case ofp_bad_action_code_1_0::BAD_VENDOR_TYPE:
		tnt::Log::error("Error code: OFPBAC_BAD_VENDOR_TYPE, Unknown action type for vendor id.");
		break;
	case ofp_bad_action_code_1_0::BAD_OUT_PORT:
		tnt::Log::error("Error code: OFPBAC_BAD_OUT_PORT, Problem validating output action.");
		break;
	case ofp_bad_action_code_1_0::BAD_ARGUMENT:
		tnt::Log::error("Error code: OFPBAC_BAD_ARGUMENT, Bad action argument.");
		break;
	case ofp_bad_action_code_1_0::PERMISSION:
		tnt::Log::error("Error code: OFPBAC_EPERM, Permissions error.");
		break;
	case ofp_bad_action_code_1_0::TOO_MANY:
		tnt::Log::error("Error code: OFPBAC_TOO_MANY, Can t handle this many actions.");
		break;
	case ofp_bad_action_code_1_0::BAD_QUEUE:
		tnt::Log::error("Error code: OFPBAC_BAD_QUEUE, Problem validating output queue.");
		break;
	default:
		tnt::Log::error("Error code non definito nella versione OF 1.0.0");
	}

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_0::error_flow_mod_failed(const ofp_error_msg_1_0* pkt, const std::string& message)
{
    auto data_err = reinterpret_cast<const ofp_header_1_0*>(message.data() + sizeof(ofp_error_msg_1_0));

	switch (network_to_host_enum<ofp_flow_mod_failed_code_1_0>(pkt->code))
	{
    case ofp_flow_mod_failed_code_1_0::ALL_TABLES_FULL:
		tnt::Log::error("Error code: OFPFMFC_ALL_TABLES_FULL, Flow not added because of full tables.");
		break;
	case ofp_flow_mod_failed_code_1_0::OVERLAP:
		tnt::Log::error("Error code: OFPFMFC_OVERLAP, Attempted to add overlapping flow.");
		break;
	case ofp_flow_mod_failed_code_1_0::PERMISSION:
		tnt::Log::error("Error code: OFPFMFC_EPERM, Permissions error.");
		break;
	case ofp_flow_mod_failed_code_1_0::BAD_EMERG_TIMEOUT:
		tnt::Log::error("Error code: OFPFMFC_BAD_EMERG_TIMEOUT, Flow not added because of non-zero idle/hard timeout.");
		break;
	case ofp_flow_mod_failed_code_1_0::BAD_COMMAND:
		tnt::Log::error("Error code: OFPFMFC_BAD_COMMAND, Unknown command.");
		break;
	case ofp_flow_mod_failed_code_1_0::UNSUPPORTED:
		tnt::Log::error("Error code: OFPFMFC_UNSUPPORTED, Unsupported action list.");
		break;
	default:
		tnt::Log::error("Error code non definito nella versione OF 1.0.0");
	}

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_0::error_port_mod_failed(const ofp_error_msg_1_0* pkt, const std::string& /*message*/)
{
    tnt::Log::error("Error type: PORT_MOD_FAILED (", ntohs(pkt->code), ")");
}

void Openflow_1_0::error_queue_op_failed(const ofp_error_msg_1_0* pkt, const std::string& /*message*/)
{
    tnt::Log::error("Error type: QUEUE_OP_FAILED (", ntohs(pkt->code), ")");
}

void Openflow_1_0::register_handlers()
{
    message_dispatcher_.register_listener(ofp_type_1_0::FEATURES_REPLY,    [this] (const std::string& message) { features(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::ERROR,             [this] (const std::string& message) { error(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::ECHO_REQUEST,      [this] (const std::string& message) { echo(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::PACKET_IN,         [this] (const std::string& message) { packet_in(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::GET_CONFIG_REPLY,  [this] (const std::string& message) { get_config(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::FLOW_REMOVED,      [this] (const std::string& message) { flow_removed(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::PORT_STATUS,       [this] (const std::string& message) { port_status(message); });
    message_dispatcher_.register_listener(ofp_type_1_0::STATS_REPLY,       [this] (const std::string& message) { stats(message); });

    error_dispatcher_.register_listener(ofp_error_type_1_0::HELLO_FAILED,     [this] (const ofp_error_msg_1_0* pkt, const std::string& message) { error_hello_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_0::BAD_REQUEST,      [this] (const ofp_error_msg_1_0* pkt, const std::string& message) { error_bad_request(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_0::BAD_ACTION,       [this] (const ofp_error_msg_1_0* pkt, const std::string& message) { error_bad_action(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_0::FLOW_MOD_FAILED,  [this] (const ofp_error_msg_1_0* pkt, const std::string& message) { error_flow_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_0::PORT_MOD_FAILED,  [this] (const ofp_error_msg_1_0* pkt, const std::string& message) { error_port_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_0::QUEUE_OP_FAILED,  [this] (const ofp_error_msg_1_0* pkt, const std::string& message) { error_queue_op_failed(pkt, message); });
}

} // namespace protocol
} // namespace drop
