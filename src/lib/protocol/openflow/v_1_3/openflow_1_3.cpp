
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

#include "openflow_1_3.hpp"

#include <cstring>

#include "protocol/openflow/v_1_3/structs.hpp"
#include "protocol/openflow/v_1_3/values.hpp"
#include "protocol/openflow/utils.hpp"
#include "protocol/openflow/openflow.hpp"
#include "protocol/openflow/flow.hpp"
#include "protocol/openflow/flow_flags.hpp"
#include "protocol/openflow/flow_filters.hpp"
#include "protocol/openflow/flow_actions.hpp"

#include "util/random.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "mac_address.hpp"
#include "ip_address.hpp"
#include "endianness.hpp"
#include "log.hpp"
#include "dump.hpp"

using namespace std::literals;

namespace drop {
namespace protocol {
namespace {

static const uint8_t version = 0x04;

int get_pad_len(int len)
{
    return (len + 7) / 8 * 8 - len;
}

std::string create_pad(int len)
{
    return std::string(len, '\0');
}

uint32_t oxm_header(ofp_oxm_class_1_3 oxm_class, oxm_ofb_match_fields_1_3 field, bool has_mask, uint8_t len)
{
    uint32_t header = 0;

    header |= (static_cast<int16_t>(oxm_class) << 16);
    header |= (static_cast<uint8_t>(field) << 9);

    if (has_mask)
    {
        header |= (1 << 8);
    }

    header |= len;

    return htonl(header);
}

uint32_t oxm_basic_header(oxm_ofb_match_fields_1_3 field, bool has_mask, uint8_t len)
{
    return oxm_header(ofp_oxm_class_1_3::OPENFLOW_BASIC, field, has_mask, len);
}

std::string tlv_8(oxm_ofb_match_fields_1_3 field, uint8_t value)
{
    uint8_t len = sizeof(value);

    return to_mem_buffer(oxm_basic_header(field, false, len), value);
}

std::string tlv_16(oxm_ofb_match_fields_1_3 field, uint16_t value)
{
    value = htons(value);
    uint8_t len = sizeof(value);

    return to_mem_buffer(oxm_basic_header(field, false, len), value);
}

std::string tlv_32(oxm_ofb_match_fields_1_3 field, uint32_t value)
{
    value = htonl(value);
    uint8_t len = sizeof(value);

    return to_mem_buffer(oxm_basic_header(field, false, len), value);
}

std::string tlv_ip_mask(oxm_ofb_match_fields_1_3 field, const tnt::ip::Address& ip, int prefix)
{
    uint32_t value = ip.to_net_order_ulong();
    uint32_t mask = htonl(std::numeric_limits<uint32_t>::max() << (32 - prefix));
    uint8_t len = sizeof(value) + sizeof(mask);

    return to_mem_buffer(oxm_basic_header(field, true, len), value, mask);
}

std::string tlv_ip(oxm_ofb_match_fields_1_3 field, const tnt::ip::Address& ip)
{
    uint32_t value = ip.to_net_order_ulong();
    uint8_t len = sizeof(value);

    return to_mem_buffer(oxm_basic_header(field, false, len), value);
}

std::string tlv_ip(oxm_ofb_match_fields_1_3 field, const tnt::ip::Address& ip, int prefix)
{
    return prefix == 32 ? tlv_ip(field, ip): tlv_ip_mask(field, ip, prefix);
}

std::string tlv_mac(oxm_ofb_match_fields_1_3 field, const tnt::MacAddress& value)
{
    assert(field == oxm_ofb_match_fields_1_3::ETH_SRC || field == oxm_ofb_match_fields_1_3::ETH_DST);

    uint32_t val0 = 0;
    uint16_t val1 = 0;
    auto str = value.raw();
    std::memcpy(&val0, str.data(), 4);
    std::memcpy(&val1, str.data() + 4, 2);

    uint8_t len = sizeof(val0) + sizeof(val1);

    return to_mem_buffer(oxm_basic_header(field, false, len), val0, val1);
}

std::string tlv_tcp_src(uint16_t port)
{
    return tlv_16(oxm_ofb_match_fields_1_3::TCP_SRC, port);
}

std::string tlv_tcp_dst(uint16_t port)
{
    return tlv_16(oxm_ofb_match_fields_1_3::TCP_DST, port);
}

std::string tlv_udp_src(uint16_t port)
{
    return tlv_16(oxm_ofb_match_fields_1_3::UDP_SRC, port);
}

std::string tlv_udp_dst(uint16_t port)
{
    return tlv_16(oxm_ofb_match_fields_1_3::UDP_DST, port);
}

std::string tlv_vid(uint16_t tag)
{
    return tlv_16(oxm_ofb_match_fields_1_3::VLAN_VID, tag | ofp_vlan_id_1_3::PRESENT);
}

// Filters
std::string from_port(uint16_t src)
{
    return tlv_32(oxm_ofb_match_fields_1_3::IN_PORT, static_cast<uint32_t>(src));
}

std::string from_vlan(uint16_t tag)
{
    return tlv_vid(tag);
}

std::string from_proto(L2Proto proto)
{
    return tlv_16(oxm_ofb_match_fields_1_3::ETH_TYPE, static_cast<uint16_t>(proto));
}

std::string from_proto(L3Proto proto)
{
    return from_proto(L2Proto::IPv4) + tlv_8(oxm_ofb_match_fields_1_3::IP_PROTO, static_cast<uint8_t>(proto));
}

std::string from_hw_src(const tnt::MacAddress& mac)
{
    return tlv_mac(oxm_ofb_match_fields_1_3::ETH_SRC, mac);
}

std::string from_hw_dst(const tnt::MacAddress& mac)
{
    return tlv_mac(oxm_ofb_match_fields_1_3::ETH_DST, mac);
}

std::string from_ip_src(const tnt::ip::Address& ip, int prefix)
{
    return from_proto(L2Proto::IPv4) + tlv_ip(oxm_ofb_match_fields_1_3::IPV4_SRC, ip, prefix);
}

std::string from_ip_dst(const tnt::ip::Address& ip, int prefix)
{
    return from_proto(L2Proto::IPv4) + tlv_ip(oxm_ofb_match_fields_1_3::IPV4_DST, ip, prefix);
}

std::string from_transport_port_src(L3Proto proto, uint16_t port)
{
    return from_proto(proto) + (proto == L3Proto::Tcp ? tlv_tcp_src(port) : tlv_udp_src(port));
}

std::string from_transport_port_dst(L3Proto proto, uint16_t port)
{
    return from_proto(proto) + (proto == L3Proto::Tcp ? tlv_tcp_dst(port) : tlv_udp_dst(port));
}

// Actions
std::string to_port(uint32_t port, uint16_t size = 0)
{
    auto action = ofp_action_output_1_3();
    action.type = host_to_network(ofp_action_type_1_3::OUTPUT);
    action.port = htonl(port);
    action.len = htons(sizeof(action));
    action.max_len = size;

    return to_mem_buffer(action);
}

std::string to_port(ofp_port_no_1_3 port, uint16_t size = 0)
{
    return to_port(static_cast<uint32_t>(port), size);
}

std::string to_ports(const std::vector<uint32_t>& dst, uint16_t size = 0)
{
    std::ostringstream os;

    for (auto p : dst)
    {
        os << to_port(p, size);
    }

    return os.str();
}

std::string to_controller(uint16_t size)
{
    return to_port(ofp_port_no_1_3::CONTROLLER, size);
}

std::string set_hw(oxm_ofb_match_fields_1_3 field, const tnt::MacAddress& mac)
{
    assert(field == oxm_ofb_match_fields_1_3::ETH_SRC || field == oxm_ofb_match_fields_1_3::ETH_DST);

    auto action = ofp_action_set_field_1_3();
    action.type = host_to_network(ofp_action_type_1_3::SET_FIELD);

    auto tlv = tlv_mac(field, mac);

    std::copy_n(std::begin(tlv), 4, action.field);
    tlv = tlv.substr(4);

    auto len = sizeof(action) + tlv.size();
    auto pad_len = get_pad_len(len);

    action.len = htons(len + pad_len);

    return to_mem_buffer(action) + tlv + create_pad(pad_len);
}

std::string set_hw_src(const tnt::MacAddress& mac)
{
    return set_hw(oxm_ofb_match_fields_1_3::ETH_SRC, mac);
}

std::string set_hw_dst(const tnt::MacAddress& mac)
{
    return set_hw(oxm_ofb_match_fields_1_3::ETH_DST, mac);
}

std::string set_ip(oxm_ofb_match_fields_1_3 field, const tnt::ip::Address& ip)
{
    assert(field == oxm_ofb_match_fields_1_3::IPV4_SRC || field == oxm_ofb_match_fields_1_3::IPV4_DST);

    auto action = ofp_action_set_field_1_3();
    action.type = host_to_network(ofp_action_type_1_3::SET_FIELD);

    auto tlv = tlv_ip(field, ip);

    std::copy_n(std::begin(tlv), 4, action.field);
    tlv = tlv.substr(4);

    auto len = sizeof(action) + tlv.size();
    auto pad_len = get_pad_len(len);

    action.len = htons(len + pad_len);

    return to_mem_buffer(action) + tlv + create_pad(pad_len);
}

std::string set_ip_src(const tnt::ip::Address& ip)
{
    return set_ip(oxm_ofb_match_fields_1_3::IPV4_SRC, ip);
}

std::string set_ip_dst(const tnt::ip::Address& ip)
{
    return set_ip(oxm_ofb_match_fields_1_3::IPV4_DST, ip);
}

std::string set_vlan(uint16_t tag)
{
    auto action = ofp_action_set_field_1_3();
    action.type = host_to_network(ofp_action_type_1_3::SET_FIELD);

    auto tlv = tlv_vid(tag);

    std::copy_n(std::begin(tlv), 4, action.field);
    tlv = tlv.substr(4);

    auto len = sizeof(action) + tlv.size();
    auto pad_len = get_pad_len(len);

    action.len = htons(len + pad_len);

    return to_mem_buffer(action) + tlv + create_pad(pad_len);
}

/*std::string add_vlan(uint16_t tag)
{
    auto action = ofp_action_header_1_3();
    action.type = host_to_network(ofp_action_type_1_3::POP_VLAN);
    action.len = htons(sizeof(action));

    return to_mem_buffer(action);
}*/

std::string strip_vlan()
{
    auto action = ofp_action_header_1_3();
    action.type = host_to_network(ofp_action_type_1_3::SET_FIELD);
    action.len = htons(sizeof(action));

    return to_mem_buffer(action);
}

void add_flag(FlowFlag* flag, ofp_flow_mod_1_3& ofp_flow)
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

void add_flags(const Flow& flow, ofp_flow_mod_1_3& ofp_flow)
{
    for (const auto& f : flow.flags())
    {
        add_flag(f.get(), ofp_flow);
    }
}

void add_filter(FlowFilter* filter, std::ostringstream& os)
{
    if (tnt::visit<FromPort>(filter, [&] (auto f) { os << from_port(f->port()); }))
    {
        return;
    }

    if (tnt::visit<FromVlan>(filter, [&] (auto f) { os << from_vlan(f->tag()); }))
    {
        return;
    }

    if (tnt::visit<FromL2Proto>(filter, [&] (auto f) { os << from_proto(f->proto()); }))
    {
        return;
    }
       
    if (tnt::visit<FromL3Proto>(filter, [&] (auto f) { os << from_proto(f->proto()); }))
    {
        return;
    }
       
    if (tnt::visit<FromHwSrc>(filter, [&] (auto f) { os << from_hw_src(f->mac()); }))
    {
        return;
    }
       
    if (tnt::visit<FromHwDst>(filter, [&] (auto f) { os << from_hw_dst(f->mac()); }))
    {
        return;
    }
       
    if (tnt::visit<FromIpSrc>(filter, [&] (auto f) { os << from_ip_src(f->ip(), f->prefix()); }))
    {
        return;
    }
       
    if (tnt::visit<FromIpDst>(filter, [&] (auto f) { os << from_ip_dst(f->ip(), f->prefix()); }))
    {
        return;
    }
       
    if (tnt::visit<FromTransportPortSrc>(filter, [&] (auto f) { os << from_transport_port_src(f->proto(), f->port()); }))
    {
        return;
    }
       
    if (tnt::visit<FromTransportPortDst>(filter, [&] (auto f) { os << from_transport_port_dst(f->proto(), f->port()); }))
    {
        return;
    }
}

std::string get_filters(const Flow& flow)
{
    std::ostringstream os;

    for (const auto& f : flow.filters())
    {
        add_filter(f.get(), os);
    }

    return os.str();
}

void get_action(FlowAction* action, std::ostringstream& os)
{
    if (tnt::visit<ToController>(action, [&] (auto a) { os << to_controller(a->size()); }))
    {
        return;
    }

    if (tnt::visit<ToPorts>(action, [&] (auto a)
    {
        std::vector<uint32_t> ports;
        std::copy(std::begin(a->ports()), std::end(a->ports()), std::back_inserter(ports));

        os << to_ports(ports);
    }))
    {
        return;
    }

    if (tnt::visit<Loop>(action, [&] (auto /*a*/) { os << to_port(ofp_port_no_1_3::IN_PORT); }))
    {
        return;
    }

    if (tnt::visit<Flood>(action, [&] (auto /*a*/) { os << to_port(ofp_port_no_1_3::FLOOD); }))
    {
        return;
    }

    if (tnt::visit<SetHwSrc>(action, [&] (auto a) { os << set_hw_src(a->mac()); }))
    {
        return;
    }

    if (tnt::visit<SetHwDst>(action, [&] (auto a) { os << set_hw_dst(a->mac()); }))
    {
        return;
    }

    if (tnt::visit<SetIpSrc>(action, [&] (auto a) { os << set_ip_src(a->ip()); }))
    {
        return;
    }

    if (tnt::visit<SetIpDst>(action, [&] (auto a) { os << set_ip_dst(a->ip()); }))
    {
        return;
    }

    if (tnt::visit<SetVlan>(action, [&] (auto a) { os << set_vlan(a->tag()); }))
    {
        return;
    }

    if (tnt::is<StripVlan>(action))
    {
        os << strip_vlan();

        return;
    }

    tnt::Log::error("Unsupported Action");
}

std::string get_actions(const Flow& flow)
{
    std::ostringstream os;

    for (const auto& f : flow.actions())
    {
        get_action(f.get(), os);
    }

    return os.str();
}

std::string get_flow(const Flow& flow, uint32_t xid, ofp_flow_mod_command_1_3 command)
{
    auto ofp_flow = ofp_flow_mod_1_3();
    
    ofp_flow.header.type = ofp_type_1_3::FLOW_MOD;
    ofp_flow.header.version = version;
    ofp_flow.header.xid = htonl(xid);
    
    ofp_flow.command = host_to_network(command);
    ofp_flow.buffer_id = htonl(OFP_NO_BUFFER);
    ofp_flow.out_port = host_to_network_underlying(ofp_port_no_1_3::ANY);
    ofp_flow.flags = host_to_network_underlying(ofp_flow_mod_flags_1_3::SEND_FLOW_REM);
    ofp_flow.priority = htons(OFP_DEFAULT_PRIORITY);
    
    add_flags(flow, ofp_flow);

    auto filters = get_filters(flow);
    auto instructions = ofp_instruction_actions_1_3();
    instructions.type = host_to_network(ofp_instruction_type_1_3::APPLY_ACTIONS);

    auto actions = get_actions(flow);
    auto len = sizeof(ofp_flow.match) + filters.size() - 4; // Subtract 4 bytes because the first filter is copied in the last 4 bytes of the ofp_match_1_3 structure

    ofp_flow.match.type = host_to_network(ofp_match_type_1_3::OXM);
    ofp_flow.match.length = htons(len);

    std::copy_n(std::begin(filters), 4, ofp_flow.match.oxm_fields);
    filters = filters.substr(4);

    instructions.len = htons(sizeof(instructions) + actions.size());

    auto pad_len = get_pad_len(len);

    ofp_flow.header.length = htons(sizeof(ofp_flow_mod_1_3) + filters.size() + pad_len + sizeof(ofp_instruction_actions_1_3) + actions.size());

    return to_mem_buffer(ofp_flow) + filters + create_pad(pad_len) + to_mem_buffer(instructions) + actions;
}

} // namespace

Openflow_1_3::Openflow_1_3(Openflow* parent): parent_(parent)
{
    assert(parent_);
    register_handlers();
}

void Openflow_1_3::init()
{
    auto packet = ofp_header_1_3();
    packet.version = version;
    packet.type = ofp_type_1_3::FEATURES_REQUEST;
    packet.length = htons(sizeof(packet));
    packet.xid = htonl(create_xid());

    parent_->write(to_mem_buffer(packet));

    std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.

    auto sw_config = ofp_switch_config_1_3();

    sw_config.header.version = version;
    sw_config.header.type = ofp_type_1_3::SET_CONFIG;
    sw_config.header.length = htons(sizeof(sw_config));
    sw_config.header.xid = htonl(create_xid());
    sw_config.flags = ofp_config_flags::FRAG_NORMAL;
    sw_config.miss_send_len = 0x0080;

    parent_->write(to_mem_buffer(sw_config));
}

void Openflow_1_3::add(const Flow& flow)
{
    auto xid = create_xid();
    parent_->write(get_flow(flow, xid, ofp_flow_mod_command_1_3::ADD));
}

void Openflow_1_3::remove(const Flow& flow)
{
    auto xid = create_xid();
    parent_->write(get_flow(flow, xid, ofp_flow_mod_command_1_3::DELETE));
}

void Openflow_1_3::remove_all()
{
    auto pkt = ofp_flow_mod_1_3();

    pkt.header.type = ofp_type_1_3::FLOW_MOD;
    pkt.header.version = version;
    pkt.header.xid = htonl(create_xid());
    pkt.header.length = htons(sizeof(pkt));

    pkt.command = ofp_flow_mod_command_1_3::DELETE;
    pkt.out_port = host_to_network_underlying(ofp_port_no_1_3::ANY);
    pkt.out_group = host_to_network_underlying(ofp_group_no_1_3::ANY);
    pkt.flags = host_to_network_underlying(ofp_flow_mod_flags_1_3::SEND_FLOW_REM);
    pkt.table_id = static_cast<uint8_t>(ofp_table_1_3::ALL);

    pkt.match.type = host_to_network(ofp_match_type_1_3::OXM);
    pkt.match.length = htons(sizeof(pkt.match) - 4);

    parent_->write(to_mem_buffer(pkt));
}

void Openflow_1_3::packet(const std::string& message)
{
    auto pkt = reinterpret_cast<const ofp_header_1_3*>(message.data());

    try
    {
        message_dispatcher_.inject_object(pkt->type, message);
    }
    catch (tnt::ListenerNotFound&)
    {
        unmanaged_packet(pkt);
    }
}

void Openflow_1_3::send_packet(const std::string& /*buffer*/, uint16_t /*port*/)
{

}

void Openflow_1_3::request_port_stats(uint16_t /*port*/)
{

}

void Openflow_1_3::features(const std::string& message)
{
    auto pkt = reinterpret_cast<const ofp_switch_features_1_3*>(message.data());

    // TODO: Parse the packet, and get the features and the status of the switch.
    parent_->set_features(pkt->datapath_id);
}

void Openflow_1_3::error(const std::string& message)
{
    auto ptr = message.data();
    auto pkt = reinterpret_cast<const ofp_error_msg_1_3*>(ptr);

    try
    {
        error_dispatcher_.inject_object(network_to_host(pkt->type), pkt, ptr);
    }
    catch (tnt::ListenerNotFound&)
    {
        tnt::Log::error("Error type (", network_to_host(pkt->type), ") not defined in OF version 1.3");
    }
}

void Openflow_1_3::echo(const std::string& message)
{
    auto ptr = message.data();
    auto request = reinterpret_cast<const ofp_header_1_3*>(ptr);

    auto reply = ofp_header_1_3();
    reply.length = request->length;
    reply.type = ofp_type_1_3::ECHO_REPLY;
    reply.version = version;
    reply.xid = request->xid;

    auto payload_len = ntohs(request->length) - sizeof(ofp_header_1_3);

    parent_->write(to_mem_buffer(reply) + std::string(ptr + sizeof(ofp_header_1_3), payload_len));
}

void Openflow_1_3::port_status(const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "port_status");
}

void Openflow_1_3::get_config(const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "get_config");
}

void Openflow_1_3::stats(const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "stats");
}

void Openflow_1_3::flow_removed(const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "flow_removed");
}

void Openflow_1_3::packet_in(const std::string& /*message*/)
{

}

void Openflow_1_3::unmanaged_packet(const ofp_header_1_3* /*pkt*/)
{
    tnt::Log::info(colors::magenta, "unmanaged_packet");
}

void Openflow_1_3::error_hello_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "error_hello_failed");
}

void Openflow_1_3::error_bad_request(const ofp_error_msg_1_3* pkt, const std::string& message)
{
    auto data_err = reinterpret_cast<const ofp_header_1_3*>(message.data() + sizeof(ofp_error_msg_1_3));

	switch (network_to_host_enum<ofp_bad_request_code_1_3>(pkt->code))
	{
    case ofp_bad_request_code_1_3::BAD_VERSION:
		tnt::Log::error("Error code: OFPBRC_BAD_VERSION, ofp_header_1_3.version not supported.");
		break;
	case ofp_bad_request_code_1_3::BAD_TYPE:
		tnt::Log::error("Error code: OFPBRC_BAD_TYPE, ofp_header_1_3.type not supported.");
		break;
	case ofp_bad_request_code_1_3::BAD_MULTIPART:
		tnt::Log::error("Error code: BAD_MULTIPART, ofp_stats_request_1_3.type not supported.");
		break;
	case ofp_bad_request_code_1_3::BAD_EXPERIMENTER:
		tnt::Log::error("Error code: BAD_EXPERIMENTER, Experimenter not supported.");
		break;
	case ofp_bad_request_code_1_3::BAD_EXP_TYPE:
		tnt::Log::error("Error code: BAD_EXP_TYPE, Experimenter subtype not supported.");
		break;
	case ofp_bad_request_code_1_3::PERMISSION:
		tnt::Log::error("Error code: OFPBRC_EPERM, Permissions error.");
		break;
	case ofp_bad_request_code_1_3::BAD_LEN:
		tnt::Log::error("Error code: OFPBRC_BAD_LEN, Wrong request length for type.");
		break;
	case ofp_bad_request_code_1_3::BUFFER_EMPTY:
		tnt::Log::error("Error code: OFPBRC_BUFFER_EMPTY, Specified buffer has already been used.");
		break;
	case ofp_bad_request_code_1_3::BUFFER_UNKNOWN:
		tnt::Log::error("Error code: OFPBRC_BUFFER_UNKNOWN Specified buffer does not exist.");
		break;
    case ofp_bad_request_code_1_3::BAD_TABLE_ID:
        tnt::Log::error("Error code: BAD_TABLE_ID, Specified table-id invalid or does not exist.");
		break;
    case ofp_bad_request_code_1_3::IS_SLAVE:
        tnt::Log::error("Error code: IS_SLAVE, Denied because controller is slave.");
		break;
    case ofp_bad_request_code_1_3::BAD_PORT:
        tnt::Log::error("Error code: BAD_PORT, Invalid port.");
		break;
    case ofp_bad_request_code_1_3::BAD_PACKET:
        tnt::Log::error("Error code: BAD_PACKET, Invalid packet in packet-out.");
		break;
    case ofp_bad_request_code_1_3::MULTIPART_BUFFER_OVERFLOW:
        tnt::Log::error("Error code: MULTIPART_BUFFER_OVERFLOW, ofp_multipart_request overflowed the assigned buffer.");
		break;
	default:
		tnt::Log::error("Error code (", ntohs(pkt->code), ") not defined in OF version 1.3");
	};

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_3::error_bad_action(const ofp_error_msg_1_3* pkt, const std::string& message)
{
    auto data_err = reinterpret_cast<const ofp_header_1_3*>(message.data() + sizeof(ofp_error_msg_1_3));

	switch (network_to_host_enum<ofp_bad_action_code_1_3>(pkt->code))
	{
    case ofp_bad_action_code_1_3::BAD_TYPE:
		tnt::Log::error("Error code: OFPBAC_BAD_TYPE, Unknown or unsupported action type.");
		break;
	case ofp_bad_action_code_1_3::BAD_LEN:
		tnt::Log::error("Error code: OFPBAC_BAD_LEN, Length problem in actions.");
		break;
	case ofp_bad_action_code_1_3::BAD_EXPERIMENTER:
		tnt::Log::error("Error code: BAD_EXPERIMENTER, Unknown experimenter id specified.");
		break;
	case ofp_bad_action_code_1_3::BAD_EXP_TYPE:
		tnt::Log::error("Error code: BAD_EXP_TYPE, Unknown action type for experimenter id.");
		break;
	case ofp_bad_action_code_1_3::BAD_OUT_PORT:
		tnt::Log::error("Error code: OFPBAC_BAD_OUT_PORT, Problem validating output action.");
		break;
	case ofp_bad_action_code_1_3::BAD_ARGUMENT:
		tnt::Log::error("Error code: OFPBAC_BAD_ARGUMENT, Bad action argument.");
		break;
	case ofp_bad_action_code_1_3::PERMISSION:
		tnt::Log::error("Error code: OFPBAC_EPERM, Permissions error.");
		break;
	case ofp_bad_action_code_1_3::TOO_MANY:
		tnt::Log::error("Error code: OFPBAC_TOO_MANY, Can t handle this many actions.");
		break;
	case ofp_bad_action_code_1_3::BAD_QUEUE:
		tnt::Log::error("Error code: OFPBAC_BAD_QUEUE, Problem validating output queue.");
		break;
    case ofp_bad_action_code_1_3::BAD_OUT_GROUP:
        tnt::Log::error("Error code: BAD_OUT_GROUP, Invalid group id in forward action.");
		break;
    case ofp_bad_action_code_1_3::MATCH_INCONSISTENT:
        tnt::Log::error("Error code: MATCH_INCONSISTENT, Action can't apply for this match, or Set-Field missing prerequisite.");
		break;
    case ofp_bad_action_code_1_3::UNSUPPORTED_ORDER:
        tnt::Log::error("Error code: UNSUPPORTED_ORDER, Action order is unsupported for the action list in an Apply-Actions instruction.");
		break;
    case ofp_bad_action_code_1_3::BAD_TAG:
        tnt::Log::error("Error code: BAD_TAG, Actions uses an unsupported tag/encap.");
		break;
    case ofp_bad_action_code_1_3::BAD_SET_TYPE:
        tnt::Log::error("Error code: BAD_SET_TYPE, Unsupported type in SET_FIELD actio.");
		break;
    case ofp_bad_action_code_1_3::BAD_SET_LEN:
        tnt::Log::error("Error code: BAD_SET_LEN, Length problem in SET_FIELD action.");
		break;
    case ofp_bad_action_code_1_3::BAD_SET_ARGUMENT:
        tnt::Log::error("Error code: BAD_SET_ARGUMENT, Bad argument in SET_FIELD action.");
		break;
	default:
		tnt::Log::error("Error code (", ntohs(pkt->code), ") not defined in OF version 1.3");
	}

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_3::error_flow_mod_failed(const ofp_error_msg_1_3* pkt, const std::string& message)
{
    auto data_err = reinterpret_cast<const ofp_header_1_3*>(message.data() + sizeof(ofp_error_msg_1_3));

	switch (network_to_host_enum<ofp_flow_mod_failed_code_1_3>(pkt->code))
	{
    case ofp_flow_mod_failed_code_1_3::UNKNOWN:
		tnt::Log::error("Error code: UNKNOWN, Unspecified error.");
		break;
	case ofp_flow_mod_failed_code_1_3::OVERLAP:
		tnt::Log::error("Error code: OFPFMFC_OVERLAP, Attempted to add overlapping flow.");
		break;
	case ofp_flow_mod_failed_code_1_3::PERMISSION:
		tnt::Log::error("Error code: OFPFMFC_EPERM, Permissions error.");
		break;
	case ofp_flow_mod_failed_code_1_3::TABLE_FULL:
		tnt::Log::error("Error code: TABLE_FULL, Flow not added because table was full.");
		break;
	case ofp_flow_mod_failed_code_1_3::BAD_COMMAND:
		tnt::Log::error("Error code: OFPFMFC_BAD_COMMAND, Unknown command.");
		break;
    case ofp_flow_mod_failed_code_1_3::BAD_TABLE_ID:
		tnt::Log::error("Error code: BAD_TABLE_ID, Table does not exist.");
		break;
    case ofp_flow_mod_failed_code_1_3::BAD_TIMEOUT:
		tnt::Log::error("Error code: BAD_TIMEOUT, Flow not added because of unsupported idle/hard timeout.");
		break;
    case ofp_flow_mod_failed_code_1_3::BAD_FLAGS:
		tnt::Log::error("Error code: BAD_FLAGS, Unsupported or unknown flags.");
		break;
	default:
		tnt::Log::error("Error code (", ntohs(pkt->code), ") not defined in OF version 1.3");
	}

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_3::error_port_mod_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "error_port_mod_failed");
}

void Openflow_1_3::error_queue_op_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "error_queue_op_failed");
}

void Openflow_1_3::error_bad_instruction(const ofp_error_msg_1_3* pkt, const std::string& message) 
{
    auto data_err = reinterpret_cast<const ofp_header_1_3*>(message.data() + sizeof(ofp_error_msg_1_3));
    
    switch (network_to_host_enum<ofp_bad_instruction_code_1_3>(pkt->code))
    {
    case ofp_bad_instruction_code_1_3::UNKNOWN_INST:
        tnt::Log::error("Error code: UNKNOWN_INST, Unknown instruction.");
		break;
    case ofp_bad_instruction_code_1_3::UNSUP_INST:
        tnt::Log::error("Error code: UNSUP_INST, Switch or table does not support the instruction.");
		break;
    case ofp_bad_instruction_code_1_3::BAD_TABLE_ID:
        tnt::Log::error("Error code: BAD_TABLE_ID, Invalid Table-ID specified.");
		break;
    case ofp_bad_instruction_code_1_3::UNSUP_METADATA:
        tnt::Log::error("Error code: UNSUP_METADATA, Metadata value unsupported by datapath.");
		break;
    case ofp_bad_instruction_code_1_3::UNSUP_METADATA_MASK:
        tnt::Log::error("Error code: UNSUP_METADATA_MASK, Metadata mask value unsupported by datapath.");
		break;
    case ofp_bad_instruction_code_1_3::BAD_EXPERIMENTER:
        tnt::Log::error("Error code: BAD_EXPERIMENTER, Unknown experimenter id specified.");
		break;
    case ofp_bad_instruction_code_1_3::BAD_EXP_TYPE:
        tnt::Log::error("Error code: BAD_EXP_TYPE, Unknown instruction for experimenter id.");
		break;
    case ofp_bad_instruction_code_1_3::BAD_LEN:
        tnt::Log::error("Error code: BAD_LEN, Length problem in instructions.");
		break;
    case ofp_bad_instruction_code_1_3::PERMISSION:
        tnt::Log::error("Error code: PERMISSION, Permissions error.");
		break;
    default:
        tnt::Log::error("Error code (", ntohs(pkt->code), ") not defined in OF version 1.3");
	}

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_3::error_bad_match(const ofp_error_msg_1_3* pkt, const std::string& message) 
{
    auto data_err = reinterpret_cast<const ofp_header_1_3*>(message.data() + sizeof(ofp_error_msg_1_3));

    switch (network_to_host_enum<ofp_bad_match_code_1_3>(pkt->code))
    {
    case ofp_bad_match_code_1_3::BAD_TYPE:
        tnt::Log::error("Error code: BAD_TYPE, Unsupported match type specified by the match.");
		break;
    case ofp_bad_match_code_1_3::BAD_LEN:
        tnt::Log::error("Error code: BAD_LEN, Length problem in match.");
		break;
    case ofp_bad_match_code_1_3::BAD_TAG:
        tnt::Log::error("Error code: BAD_TAG, Match uses an unsupported tag/encap.");
		break;
    case ofp_bad_match_code_1_3::BAD_DL_ADDR_MASK:
        tnt::Log::error("Error code: BAD_DL_ADDR_MASK, Unsupported datalink addr mask - switch does not support arbitrary datalink address mask.");
		break;
    case ofp_bad_match_code_1_3::BAD_NW_ADDR_MASK:
        tnt::Log::error("Error code: BAD_NW_ADDR_MASK, Unsupported network addr mask - switch does not support arbitrary network address mask.");
		break;
    case ofp_bad_match_code_1_3::BAD_WILDCARDS:
        tnt::Log::error("Error code: BAD_WILDCARDS, Unsupported combination of fields masked or omitted in the match.");
		break;
    case ofp_bad_match_code_1_3::BAD_FIELD:
        tnt::Log::error("Error code: BAD_FIELD, Unsupported field type in the match.");
		break;
    case ofp_bad_match_code_1_3::BAD_VALUE:
        tnt::Log::error("Error code: BAD_VALUE, Unsupported value in a match field.");
		break;
    case ofp_bad_match_code_1_3::BAD_MASK:
        tnt::Log::error("Error code: BAD_MASK, Unsupported mask specified in the match, field is not dl-address or nw-address.");
		break;
    case ofp_bad_match_code_1_3::BAD_PREREQ:
        tnt::Log::error("Error code: BAD_PREREQ, A prerequisite was not met.");
		break;
    case ofp_bad_match_code_1_3::DUP_FIELD:
        tnt::Log::error("Error code: DUP_FIELD, A field type was duplicated.");
		break;
    case ofp_bad_match_code_1_3::PERMISSION:
        tnt::Log::error("Error code: PERMISSION, Permissions error.");
		break;
    default:
		tnt::Log::error("Error code (", ntohs(pkt->code), ") not defined in OF version 1.3");
	}

	tnt::Log::error("Error data: type=", static_cast<unsigned int>(data_err->type), ", xid=", static_cast<unsigned int>(ntohl(data_err->xid)));
}

void Openflow_1_3::error_group_mod_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/) 
{
    tnt::Log::info(colors::magenta, "error_group_mod_failed");
}

void Openflow_1_3::error_table_mod_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/) 
{
    tnt::Log::info(colors::magenta, "error_table_mod_failed");
}

void Openflow_1_3::error_switch_config_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/) 
{
    tnt::Log::info(colors::magenta, "error_switch_config_failed");
}

void Openflow_1_3::error_role_request_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/) 
{
    tnt::Log::info(colors::magenta, "error_role_request_failed");
}

void Openflow_1_3::error_meter_mod_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/)
{
    tnt::Log::info(colors::magenta, "error_meter_mod_failed");
}

void Openflow_1_3::error_table_features_failed(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/) 
{
    tnt::Log::info(colors::magenta, "error_table_features_failed");
}

void Openflow_1_3::error_experimenter(const ofp_error_msg_1_3* /*pkt*/, const std::string& /*message*/) 
{
    tnt::Log::info(colors::magenta, "error_experimenter");
}

void Openflow_1_3::register_handlers()
{
    message_dispatcher_.register_listener(ofp_type_1_3::FEATURES_REPLY,    [this] (const std::string& message) { features(message); });
    message_dispatcher_.register_listener(ofp_type_1_3::ERROR,             [this] (const std::string& message) { error(message); });
    message_dispatcher_.register_listener(ofp_type_1_3::ECHO_REQUEST,      [this] (const std::string& message) { echo(message); });
    message_dispatcher_.register_listener(ofp_type_1_3::PACKET_IN,         [this] (const std::string& message) { packet_in(message); });
    message_dispatcher_.register_listener(ofp_type_1_3::GET_CONFIG_REPLY,  [this] (const std::string& message) { get_config(message); });
    message_dispatcher_.register_listener(ofp_type_1_3::FLOW_REMOVED,      [this] (const std::string& message) { flow_removed(message); });
    message_dispatcher_.register_listener(ofp_type_1_3::PORT_STATUS,       [this] (const std::string& message) { port_status(message); });
    //message_dispatcher_.register_listener(ofp_type_1_3::STATS_REPLY,       [this] (const std::string& message) { stats(message); });

    error_dispatcher_.register_listener(ofp_error_type_1_3::HELLO_FAILED,     [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_hello_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::BAD_REQUEST,      [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_bad_request(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::BAD_ACTION,       [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_bad_action(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::FLOW_MOD_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_flow_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::PORT_MOD_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_port_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::QUEUE_OP_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_queue_op_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::BAD_INSTRUCTION,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_bad_instruction(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::BAD_MATCH,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_bad_match(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::GROUP_MOD_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_group_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::TABLE_MOD_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_table_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::SWITCH_CONFIG_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_switch_config_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::ROLE_REQUEST_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_role_request_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::METER_MOD_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_meter_mod_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::TABLE_FEATURES_FAILED,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_table_features_failed(pkt, message); });
    error_dispatcher_.register_listener(ofp_error_type_1_3::EXPERIMENTER,  [this] (const ofp_error_msg_1_3* pkt, const std::string& message) { error_experimenter(pkt, message); });
}

} // namespace protocol
} // namespace drop
