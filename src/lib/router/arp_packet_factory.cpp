
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

#include "arp_packet_factory.hpp"

#include <algorithm>
#include <cassert>

#include "router/arp_packet.hpp"

#include "ip_address.hpp"
#include "endianness.hpp"

namespace drop {

ArpPacket arp_packet(const char* buffer)
{
    return *reinterpret_cast<const ArpPacket*>(buffer);
}

ArpPacket arp_packet(const std::string& buffer)
{
    return arp_packet(buffer.data());
}

ArpPacket arp_packet(ArpPacketOperation oper, const tnt::MacAddress& hs, const tnt::MacAddress& ht, const tnt::ip::Address& ps, const tnt::ip::Address& pt)
{
    ArpPacket p;
    p.operation = htons(static_cast<uint16_t>(oper));

	std::copy_n(hs.raw().data(), p.hlen, reinterpret_cast<char*>(&p.sha0));
	std::copy_n(ht.raw().data(), p.hlen, reinterpret_cast<char*>(&p.tha0));

    auto s = static_cast<uint32_t>(ps.to_net_order_ulong());
    auto t = static_cast<uint32_t>(pt.to_net_order_ulong());

	std::copy_n(reinterpret_cast<char*>(&s), p.plen, reinterpret_cast<char*>(&p.spa0));
	std::copy_n(reinterpret_cast<char*>(&t), p.plen, reinterpret_cast<char*>(&p.tpa0));

    return p;
}

ArpPacketOperation arp_operation(const ArpPacket& packet)
{
    return static_cast<ArpPacketOperation>(ntohs(packet.operation));
}

tnt::MacAddress arp_hw_source(const ArpPacket& packet)
{
    assert(packet.hlen == 6);

    return tnt::MacAddress(std::string(reinterpret_cast<const char*>(&packet.sha0), packet.hlen));
}

tnt::MacAddress arp_hw_target(const ArpPacket& packet)
{
    assert(packet.hlen == 6);

    return tnt::MacAddress(std::string(reinterpret_cast<const char*>(&packet.tha0), packet.hlen));
}

tnt::ip::Address arp_proto_source(const ArpPacket& packet)
{
    assert(packet.plen == 4);
	auto ptr = reinterpret_cast<const char*>(&packet.spa0);

    return tnt::ip::Address::from_net_order_ulong(*reinterpret_cast<const uint32_t*>(ptr));
}

tnt::ip::Address arp_proto_target(const ArpPacket& packet)
{
    assert(packet.plen == 4);
	auto ptr = reinterpret_cast<const char*>(&packet.tpa0);

    return tnt::ip::Address::from_net_order_ulong(*reinterpret_cast<const uint32_t*>(ptr));
}

std::ostream& operator<<(std::ostream& os, const ArpPacket& pkt)
{
    if (arp_operation(pkt) == ArpPacketOperation::Request)
    {
        os << "ARP request: Who has " << arp_proto_target(pkt) << "? Tell " << arp_proto_source(pkt) << std::endl;
    }
    else
    {
        os << "ARP Reply: " << arp_proto_source(pkt) << " is " << arp_hw_source(pkt) << std::endl;
    }

    return os;
}

} // namespace drop
