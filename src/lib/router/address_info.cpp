
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

#include "address_info.hpp"

#define NOMINMAX // Needed on Windows to avoid the min and max macro definitions.

#include <iostream>
#include <limits>

#include "util/interfaces.hpp"

#include "ip_address.hpp"
#include "endianness.hpp"

namespace drop {

bool operator==(const AddressInfo& a1, const AddressInfo& a2)
{
    return a1.address == a2.address && a1.prefix == a2.prefix && a1.port_index == a2.port_index;
}

bool same_network(const AddressInfo& a1, const AddressInfo& a2)
{
    auto mask1 = std::numeric_limits<uint32_t>::max() << (32 - a1.prefix);
    auto mask2 = std::numeric_limits<uint32_t>::max() << (32 - a2.prefix);

    return (ntohl(a1.address) & mask1) == (ntohl(a2.address) & mask2);
}

bool same_network(const AddressInfo& a1, const tnt::ip::Address& a2)
{
    uint32_t mask = std::numeric_limits<uint32_t>::max() << (32 - a1.prefix);

    return (ntohl(a1.address) & mask) == (ntohl(a2.to_net_order_ulong()) & mask);
}

std::ostream& operator<<(std::ostream& os, const AddressInfo& address)
{
    os << tnt::ip::Address::from_net_order_ulong(address.address) << "/" << address.prefix << " dev " << tnt::index_to_name(address.port_index);

    return os;
}

} // namespace drop
