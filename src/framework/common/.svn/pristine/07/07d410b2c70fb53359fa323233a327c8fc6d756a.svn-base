
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

#ifndef TNT_INET_SOCKET_ADDRESS_HPP_
#define TNT_INET_SOCKET_ADDRESS_HPP_

#include <string>
#include <iosfwd>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <WinSock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#include "init_sockets.hpp"
#include "ip_address.hpp"

namespace tnt {
namespace ip {

struct SocketAddress: public sockaddr_in, private InitSockets
{
    SocketAddress();
    SocketAddress(const std::string& address, short port);
    SocketAddress(const Address& address, short port);

    Address address() const;
    short port() const;

    std::string to_string() const;

    operator const sockaddr*() const
    {
        return reinterpret_cast<const sockaddr*>(this);
    }

    operator sockaddr*()
    {
        return reinterpret_cast<sockaddr*>(this);
    }

    size_t size() const
    {
        return sizeof(*this);
    }
};

std::ostream& operator<<(std::ostream& os, const SocketAddress& address);

} // namespace ip
} // namespace tnt

#endif
