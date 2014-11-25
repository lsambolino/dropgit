
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

#ifndef TNT_IP_ADDRESS_HPP_
#define TNT_IP_ADDRESS_HPP_

#include <string>
#include <iosfwd>
#include <initializer_list>

#include <cstdint>

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

namespace tnt {
namespace ip {

class Address: public in_addr, private InitSockets
{
    friend struct SocketAddress;
public:
    Address();

    static Address from_string(const std::string& address);
    static Address from_net_order_ulong(uint32_t address);
    static Address from_host_order_ulong(uint32_t address);
    static Address from_bytes(std::initializer_list<uint8_t>&& list);
    static Address broadcast();
    static Address broadcast(const Address& address, const Address& mask);
    static Address any();

    size_t size() const;

    bool is_loopback() const;
    bool is_broadcast() const;
    bool is_multicast() const;
    bool is_any() const;

    std::string to_string() const;
    unsigned long to_net_order_ulong() const;

    bool operator==(const Address& other) const;
    bool operator!=(const Address& other) const;
    bool operator<(const Address& other) const;
private:
    explicit Address(unsigned long net_order_value);
};

std::ostream& operator<<(std::ostream& os, const Address& address);

} // namespace ip
} // namespace tnt

#endif
