
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

#ifndef DROP_PROTOCOL_OPENFLOW_FLOW_FACTORY_HPP_
#define DROP_PROTOCOL_OPENFLOW_FLOW_FACTORY_HPP_

#include <functional>
#include <string>
#include <limits>
#include <cstdint>

#include "protocol/values.hpp"
#include "protocol/openflow/flow.hpp"

#include "mac_address.hpp"

namespace tnt {
namespace ip {

class Address;

} // namespace ip
} // namespace tnt

namespace drop {
    
// Flags
std::function<protocol::Flow&(protocol::Flow&)> priority(uint16_t value);
std::function<protocol::Flow&(protocol::Flow&)> idle_timeout(uint16_t value);
std::function<protocol::Flow&(protocol::Flow&)> hard_timeout(uint16_t value);
std::function<protocol::Flow&(protocol::Flow&)> out_port(uint16_t value);
std::function<protocol::Flow&(protocol::Flow&)> flags(uint16_t value);

// Filters
std::function<protocol::Flow&(protocol::Flow&)> from_port(uint16_t src);

std::function<protocol::Flow&(protocol::Flow&)> from_vlan(uint16_t tag);

std::function<protocol::Flow&(protocol::Flow&)> from_proto(L2Proto proto);
std::function<protocol::Flow&(protocol::Flow&)> from_proto(L3Proto proto);

std::function<protocol::Flow&(protocol::Flow&)> from_hw_src(const tnt::MacAddress& mac);
std::function<protocol::Flow&(protocol::Flow&)> from_hw_dst(const tnt::MacAddress& mac);

std::function<protocol::Flow&(protocol::Flow&)> from_hw_bcast();

std::function<protocol::Flow&(protocol::Flow&)> from_ip_src(const tnt::ip::Address& ip, int prefix = 32);
std::function<protocol::Flow&(protocol::Flow&)> from_ip_dst(const tnt::ip::Address& ip, int prefix = 32);
std::function<protocol::Flow&(protocol::Flow&)> from_ip_src(const std::string& ip, int prefix = 32);
std::function<protocol::Flow&(protocol::Flow&)> from_ip_dst(const std::string& ip, int prefix = 32);

std::function<protocol::Flow&(protocol::Flow&)> from_transport_port_src(L3Proto proto, uint16_t port);
std::function<protocol::Flow&(protocol::Flow&)> from_transport_port_dst(L3Proto proto, uint16_t port);

// Actions
std::function<protocol::Flow&(protocol::Flow&)> to_controller(uint16_t size = std::numeric_limits<uint16_t>::max());
std::function<protocol::Flow&(protocol::Flow&)> loop();

std::function<protocol::Flow&(protocol::Flow&)> to_port(uint16_t dst);
std::function<protocol::Flow&(protocol::Flow&)> to_ports(const std::vector<uint16_t>& dst);

std::function<protocol::Flow&(protocol::Flow&)> set_hw_src(const tnt::MacAddress& mac);
std::function<protocol::Flow&(protocol::Flow&)> set_hw_dst(const tnt::MacAddress& mac);

std::function<protocol::Flow&(protocol::Flow&)> set_ip_src(const tnt::ip::Address& ip);
std::function<protocol::Flow&(protocol::Flow&)> set_ip_dst(const tnt::ip::Address& ip);

std::function<protocol::Flow&(protocol::Flow&)> set_vlan(uint16_t tag);
std::function<protocol::Flow&(protocol::Flow&)> strip_vlan();

} //namespace drop

#endif
