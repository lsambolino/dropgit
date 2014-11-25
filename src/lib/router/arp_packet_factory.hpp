
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

#ifndef DROP_ROUTER_ARP_PACKET_FACTORY_HPP_
#define DROP_ROUTER_ARP_PACKET_FACTORY_HPP_

#include <string>
#include <cstring>
#include <iosfwd>

#include "mac_address.hpp"

namespace tnt { 
namespace ip {

class Address;

} // namespace ip
} // namespace tnt

namespace drop {

struct ArpPacket;

enum class ArpPacketOperation
{
    Request = 1,
    Reply = 2
};

ArpPacket arp_packet(const char* buffer);
ArpPacket arp_packet(const std::string& buffer);
ArpPacket arp_packet(ArpPacketOperation oper, const tnt::MacAddress& hs, const tnt::MacAddress& ht, const tnt::ip::Address& ps, const tnt::ip::Address& pt);

ArpPacketOperation arp_operation(const ArpPacket& packet);
tnt::MacAddress arp_hw_source(const ArpPacket& packet);
tnt::MacAddress arp_hw_target(const ArpPacket& packet);
tnt::ip::Address arp_proto_source(const ArpPacket& packet);
tnt::ip::Address arp_proto_target(const ArpPacket& packet);

std::ostream& operator<<(std::ostream& os, const ArpPacket& pkt);

} // namespace drop

#endif
