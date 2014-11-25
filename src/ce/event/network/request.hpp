
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

#ifndef DROP_EVENT_NETWORK_REQUEST_HPP_
#define DROP_EVENT_NETWORK_REQUEST_HPP_

#include <cstdint>

#include "protocol/netlink/values.hpp"
#include "protocol/netlink/reply_info.hpp"

namespace drop {

enum class InetFamily: uint8_t
{
    Inet = static_cast<uint8_t>(protocol::NetlinkFamily::Inet),
    Inet6 = static_cast<uint8_t>(protocol::NetlinkFamily::Inet6)
};

namespace event {

using ReplyInfo = protocol::NetlinkReplyInfo;

class Request
{
protected:
    Request(const ReplyInfo& reply): reply_(reply) { }
public:
    const ReplyInfo& reply() const { return reply_; }
private:
    ReplyInfo reply_;
};

} // namespace event
} // namespace drop

#endif
