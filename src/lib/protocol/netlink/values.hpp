
/*

Copyright (c) 2013, Marco Chiappero (marco@absence.it)
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

#ifndef DROP_PROTOCOL_NETLINK_VALUES_HPP_
#define DROP_PROTOCOL_NETLINK_VALUES_HPP_

#include <cstdint>

#include <sys/socket.h>
#include <linux/rtnetlink.h>

namespace drop {
namespace protocol {

enum class NetlinkFamily: uint8_t
{
	None = AF_UNSPEC,
	Inet = AF_INET,
	Inet6 = AF_INET6,
	Link = AF_PACKET
};

enum class NetlinkAddrOpType: uint8_t
{
	AddAddress = RTM_NEWADDR,
	DelAddress = RTM_DELADDR
};

enum class NetlinkRouteOpType: uint8_t
{
	AddRoute = RTM_NEWROUTE,
	DelRoute = RTM_DELROUTE
};

}
}

#endif
