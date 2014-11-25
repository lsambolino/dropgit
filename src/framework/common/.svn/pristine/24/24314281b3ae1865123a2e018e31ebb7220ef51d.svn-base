
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

#include "ip_address.hpp"

#include <vector>

#include <cstring>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <Ws2tcpip.h>

#endif

#include "endianness.hpp"

namespace tnt {
namespace ip {

Address::Address()
{
    s_addr = 0;
}

Address::Address(unsigned long net_order_value)
{
    s_addr = net_order_value;
}

Address Address::from_string(const std::string& address)
{
    Address tmp;
    inet_pton(AF_INET, address.c_str(), &tmp);

    return tmp;
}

Address Address::from_net_order_ulong(uint32_t address)
{
    return Address(address);
}

Address Address::from_host_order_ulong(uint32_t address)
{
    return Address(htonl(address));
}

Address Address::from_bytes(std::initializer_list<uint8_t>&& list)
{
    assert(list.size() == 4);

    return Address(*reinterpret_cast<uint32_t*>(std::vector<uint8_t>(std::move(list)).data()));
}

Address Address::broadcast()
{
    return Address(INADDR_BROADCAST);
}

Address Address::broadcast(const Address& address, const Address& mask)
{
    return Address(htonl(ntohl(address.s_addr) | (ntohl(mask.s_addr) ^ 0xFFFFFFFF)));
}

Address Address::any()
{
    return Address(INADDR_ANY);
}

size_t Address::size() const
{
    return sizeof(in_addr);
}

bool Address::is_loopback() const
{
    return (ntohl(s_addr) & 0xFF000000) == 0x7F000000;
}

bool Address::is_broadcast() const
{
    return s_addr == INADDR_BROADCAST;
}

bool Address::is_multicast() const
{
    return (ntohl(s_addr) & 0xF0000000) == 0xE0000000;
}

bool Address::is_any() const
{
    return s_addr == INADDR_ANY;
}

std::string Address::to_string() const
{
    char str[INET_ADDRSTRLEN];

#if defined(TNT_PLATFORM_WIN32)

    inet_ntop(AF_INET, const_cast<Address*>(this), str, INET_ADDRSTRLEN);

#else

    inet_ntop(AF_INET, this, str, INET_ADDRSTRLEN);

#endif

    return str;
}

unsigned long Address::to_net_order_ulong() const
{
    return s_addr;
}

bool Address::operator==(const Address& other) const
{
    return s_addr == other.s_addr;
}

bool Address::operator!=(const Address& other) const
{
    return !(*this == other);
}

bool Address::operator<(const Address& other) const
{
    return s_addr < other.s_addr;
}

std::ostream& operator<<(std::ostream& os, const Address& address)
{
    os << address.to_string();

    return os;
}

} // namespace ip
} // namespace tnt
