
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

#ifndef TNT_NETLINK_SOCKET_HPP_
#define TNT_NETLINK_SOCKET_HPP_

#include "basic_socket.hpp"
#include "platform.hpp"

#if defined(TNT_PLATFORM_LINUX)

namespace tnt {
namespace netlink {

struct NetlinkAddress: public sockaddr_nl
{
    explicit NetlinkAddress(int pid = 0, int groups = 0): nl_family(AF_NETLINK), nl_pid(pid), nl_groups(groups) {}

    static NetlinkAddress from_process(int groups = 0)
    {
        return NetlinkAddress(getpid(), groups);
    }
};

namespace route {

using Socket = BasicSocket<AddressFamily::Inet, SocketType::Raw, SocketProtocol::NetlinkRoute>;

} // namespace route
} // namespace netlink
} // namespace tnt

#endif

#endif
