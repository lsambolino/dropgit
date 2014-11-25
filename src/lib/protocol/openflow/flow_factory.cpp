
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

#include "flow_factory.hpp"

#include <cassert>

#include "protocol/openflow/flow_flags.hpp"
#include "protocol/openflow/flow_filters.hpp"
#include "protocol/openflow/flow_actions.hpp"

#include "ip_address.hpp"
#include "endianness.hpp"

namespace drop {

using protocol::Flow;

// Flags
std::function<protocol::Flow&(protocol::Flow&)> priority(uint16_t value)
{
    return [value] (Flow& flow) -> Flow&
    {
        return flow.add_flag(std::make_unique<protocol::Priority>(value));
    };
}

std::function<protocol::Flow&(protocol::Flow&)> idle_timeout(uint16_t value)
{
    return [value] (Flow& flow) -> Flow&
    {
        return flow.add_flag(std::make_unique<protocol::IdleTimeout>(value));
    };
}

std::function<protocol::Flow&(protocol::Flow&)> hard_timeout(uint16_t value)
{
    return [value] (Flow& flow) -> Flow&
    {
        return flow.add_flag(std::make_unique<protocol::HardTimeout>(value));
    };
}

std::function<protocol::Flow&(protocol::Flow&)> out_port(uint16_t value)
{
    return [value] (Flow& flow) -> Flow&
    {
        return flow.add_flag(std::make_unique<protocol::OutPort>(value));
    };
}

std::function<protocol::Flow&(protocol::Flow&)> flags(uint16_t value)
{
    return [value] (Flow& flow) -> Flow&
    {
        return flow.add_flag(std::make_unique<protocol::Flags>(value));
    };
}

// Filters
std::function<Flow&(Flow&)> from_port(uint16_t src)
{
    return [src] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromPort>(src));
    };
}

std::function<Flow&(Flow&)> from_vlan(uint16_t tag)
{
    return [tag] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromVlan>(tag));
    };
}

std::function<Flow&(Flow&)> from_proto(L2Proto proto)
{
    return [proto] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromL2Proto>(proto));
    };
}

std::function<Flow&(Flow&)> from_proto(L3Proto proto)
{
    return [proto] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromL3Proto>(proto));
    };
}

std::function<Flow&(Flow&)> from_hw_src(const tnt::MacAddress& mac)
{
    return [&] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromHwSrc>(mac));
    };
}

std::function<Flow&(Flow&)> from_hw_dst(const tnt::MacAddress& mac)
{
    return [&] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromHwDst>(mac));
    };
}

std::function<Flow&(Flow&)> from_hw_bcast()
{
    return from_hw_dst(tnt::MacAddress::broadcast());
}

std::function<Flow&(Flow&)> from_ip_src(const tnt::ip::Address& ip, int prefix)
{
    return [&ip, prefix] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromIpSrc>(ip, prefix));
    };
}

std::function<Flow&(Flow&)> from_ip_dst(const tnt::ip::Address& ip, int prefix)
{
    return [&ip, prefix] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromIpDst>(ip, prefix));
    };
}

std::function<Flow&(Flow&)> from_ip_src(const std::string& ip, int prefix)
{
    return from_ip_src(tnt::ip::Address::from_string(ip), prefix);
}

std::function<Flow&(Flow&)> from_ip_dst(const std::string& ip, int prefix)
{
    return from_ip_dst(tnt::ip::Address::from_string(ip), prefix);
}

std::function<Flow&(Flow&)> from_transport_port_src(L3Proto proto, uint16_t port)
{
    return [=] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromTransportPortSrc>(proto, port));
    };
}

std::function<Flow&(Flow&)> from_transport_port_dst(L3Proto proto, uint16_t port)
{
    return [=] (Flow& flow) -> Flow&
    {
        return flow.add_filter(std::make_unique<protocol::FromTransportPortDst>(proto, port));
    };
}

// Actions
std::function<Flow&(Flow&)> to_controller(uint16_t size)
{
    return [size] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::ToController>(size));
    };
}

std::function<Flow&(Flow&)> loop()
{
    return [] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::Loop>());
    };
}

std::function<Flow&(Flow&)> to_port(uint16_t dst)
{
    return to_ports({ dst });
}

std::function<Flow&(Flow&)> to_ports(const std::vector<uint16_t>& dst)
{
    return [dst] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::ToPorts>(dst));
    };
}

std::function<Flow&(Flow&)> set_hw_src(const tnt::MacAddress& mac)
{
    return [mac] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::SetHwSrc>(mac));
    };
}

std::function<Flow&(Flow&)> set_hw_dst(const tnt::MacAddress& mac)
{
    return [&] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::SetHwDst>(mac));
    };
}

std::function<protocol::Flow&(protocol::Flow&)> set_ip_src(const tnt::ip::Address& ip)
{
    return [&] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::SetIpDst>(ip));
    };
}

std::function<protocol::Flow&(protocol::Flow&)> set_ip_dst(const tnt::ip::Address& ip)
{
    return [&] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::SetIpDst>(ip));
    };
}

std::function<Flow&(Flow&)> set_vlan(uint16_t tag)
{
	return [tag] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::SetVlan>(tag));
    };
}

std::function<Flow&(Flow&)> strip_vlan()
{
	return [] (Flow& flow) -> Flow&
    {
        return flow.add_action(std::make_unique<protocol::StripVlan>());
    };
}

} // namespace drop
