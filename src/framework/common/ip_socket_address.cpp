
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

#include "ip_socket_address.hpp"

#include <cstring>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <Ws2tcpip.h>

#endif

#include "util/string.hpp"

#include "endianness.hpp"

namespace tnt {
namespace ip {

#if !defined(INET_ADDRSTRLEN)

#define INET_ADDRSTRLEN    16

#endif

SocketAddress::SocketAddress()
{
    memset(this, 0, sizeof(*this));
}
    
SocketAddress::SocketAddress(const std::string& address, short port) : SocketAddress()
{
    auto addr = Address::from_string(address);

    sin_family = AF_INET;
    sin_addr = addr;
    sin_port = htons(port);
}

SocketAddress::SocketAddress(const Address& address, short port) : SocketAddress()
{
    sin_family = AF_INET;
    sin_addr = address;
    sin_port = htons(port);
}

Address SocketAddress::address() const
{
    return Address(sin_addr.s_addr);
}

short SocketAddress::port() const
{
    return ntohs(sin_port);
}

std::string SocketAddress::to_string() const
{
    char str[INET_ADDRSTRLEN];

#if defined(TNT_PLATFORM_WIN32)

    inet_ntop(AF_INET, const_cast<IN_ADDR*>(&sin_addr), str, INET_ADDRSTRLEN);

#else

    inet_ntop(AF_INET, &sin_addr, str, INET_ADDRSTRLEN);

#endif

    return std::string(str).append(":").append(tnt::to_string(port()));
}

std::ostream& operator<<(std::ostream& os, const SocketAddress& address)
{
    os << address.to_string();

    return os;
}

} // namespace ip
} // namespace tnt
