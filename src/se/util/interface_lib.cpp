
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

#include "interface_lib.hpp"

#include <algorithm>
#include <string>
#include <iostream>

#include "util/crc.hpp"
#include "util/lookup_table.hpp"
#include "util/likely_macro.hpp"
#include "util/configuration.hpp"

#include "mac_address.hpp"

namespace drop {

InterfaceLib::InterfaceLib(LookupTable& table) : table_(table)
{
	auto src_str = tnt::MacAddress(tnt::Configuration::get("forwarding.mac").as<std::string>()).raw();
	std::copy_n(src_str.c_str(), 6, src_);
}

bool InterfaceLib::forward_packet(uint8_t* pkt)
{
	auto offset = 12;

	if (LIKELY(pkt[offset] == 0x81 && pkt[offset + 1] == 0x00)) // The packet has a VLAN tag.
	{
		offset += 4;

		while (UNLIKELY(pkt[offset] == 0x81 && pkt[offset + 1] == 0x00)) // Check for stacked VLAN tags.
		{
			offset += 4;
		}
	}

	if (LIKELY(pkt[offset] == 0x08 && pkt[offset + 1] == 0x00)) // IP packet
	{
		const auto& node = table_.lookup(*reinterpret_cast<uint32_t*>(pkt + offset + 18));

		return LIKELY(!node.def && modify_packet(pkt, node.dst, offset));
	}

	return false;
}

bool InterfaceLib::modify_packet(uint8_t* pkt, const char* dst, int offset)
{
	std::copy_n(dst, 6, pkt);
	std::copy_n(src_, 6, pkt + 6);

	auto ttl = reinterpret_cast<uint8_t*>(pkt + offset + 10);

	--(*ttl); // Decrement TTL

	if (UNLIKELY(*ttl == 0))
	{
		return false;
	}

	auto cs = reinterpret_cast<uint16_t*>(pkt + offset + 12); // Checksum
	*cs = 0;

	auto ip_words = *reinterpret_cast<const uint8_t*>(pkt + offset + 2) & 0x0f; // Take the low order 4 bits.
	*cs = tnt::crc16(pkt + offset + 2, ip_words * 4);

	return true;
}

} // namespace drop
