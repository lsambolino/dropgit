
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

#ifndef DROP_MESSAGE_NETWORK_ARP_HPP_
#define DROP_MESSAGE_NETWORK_ARP_HPP_

#include <cstdint>

#include "message/message.hpp"

#include "mac_address.hpp"

namespace drop {
namespace message {

class ArpRequest: public virtual tnt::Message
{
public:
    explicit ArpRequest(uint32_t ip): ip_{ ip } { }

    uint32_t ip() const { return ip_; }
private:
    uint32_t ip_;
};

class ArpReply: public virtual tnt::Message
{
public:
    ArpReply(uint32_t ip, const tnt::MacAddress& mac): ip_{ ip }, mac_{ mac } { }

    uint32_t ip() const { return ip_; }
	const tnt::MacAddress& mac() const { return mac_; }
private:
    uint32_t ip_;
	tnt::MacAddress mac_;
};

} // namespace message
} // namespace drop

#endif
