
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

#include "eth_factory.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "protocol/values.hpp"

#include "util/crc.hpp"

#include "log.hpp"
#include "endianness.hpp"
#include "mac_address.hpp"

namespace drop {
namespace {

struct	ether_header
{
	uint8_t	ether_dhost[6];
	uint8_t	ether_shost[6];
	uint16_t ether_type;
};

} // namespace

std::string create_eth_packet(const std::string& buffer, const tnt::MacAddress& src, const tnt::MacAddress& dst, L2Proto proto)
{
	const decltype(buffer.size()) eth_min_size = 60; // 64 bytes minus the 4 bytes of FCS.

    auto size = std::max(buffer.size() + 14, eth_min_size + 4);
    std::string sendbuf(size, '\0');

    auto eh = reinterpret_cast<ether_header*>(&sendbuf[0]);

    const auto src_str = src.raw();
    const auto dst_str = dst.raw();

	// Ethernet header
    memcpy(eh->ether_dhost, dst_str.data(), 6);
    memcpy(eh->ether_shost, src_str.data(), 6);

	// Ethertype field
	eh->ether_type = htons(static_cast<uint16_t>(proto));

	auto tx_len = sizeof(ether_header);

	memcpy(&sendbuf[0] + tx_len, buffer.data(), buffer.size());

    if (proto == L2Proto::IPv4)
    {
        // Check if the IP checksum is set.
        auto cs = reinterpret_cast<uint16_t*>(&sendbuf[0] + tx_len + 10);

        if (*cs == 0)
        {
            auto ip_words = *reinterpret_cast<const uint8_t*>(buffer.data()) & 0x0f; // Take the low order 4 bits.

            *cs = tnt::crc16(reinterpret_cast<const uint8_t*>(buffer.data()), ip_words * 4);
        }
    }

    tx_len += buffer.size();
    tx_len = std::max(tx_len, eth_min_size);

    if (tx_len == eth_min_size)
    {
        auto crc = tnt::crc32(reinterpret_cast<const uint8_t*>(sendbuf.data()), tx_len);

        memcpy(&sendbuf[0] + tx_len, &crc, sizeof(crc));
        tx_len += sizeof(crc);
    }

	return sendbuf.substr(0, tx_len);
}

} // namespace drop
