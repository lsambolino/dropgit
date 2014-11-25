
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

#ifndef DROP_VIRTUAL_INTERFACES_MESSAGES_HPP_
#define DROP_VIRTUAL_INTERFACES_MESSAGES_HPP_

#include <string>
#include <cstdint>

#include "message/message.hpp"

namespace drop {
namespace message {

class DethInPacket: public virtual tnt::Message
{
public:
    DethInPacket(uint16_t iface, const std::string& pkt): iface_{ iface }, pkt_{ pkt } {}

    uint16_t interface() const { return iface_; }
    const std::string& packet() const { return pkt_; }
private:
    uint16_t iface_;
    std::string pkt_;
};

class DethLinkChange: public virtual tnt::Message
{
public:
    DethLinkChange(uint16_t iface, bool up): iface_{ iface }, up_{ up } {}

    uint16_t interface() const { return iface_; }
    bool up() const { return up_; }
private:
    uint16_t iface_;
    bool up_;
};

class DethLinkStats: public virtual tnt::Message
{
public:
    DethLinkStats(uint16_t iface, uint64_t rxpkts, uint64_t rxbytes, uint64_t txpkts, uint64_t txbytes): iface_{ iface }, 
																									 rxpkts_{ rxpkts }, 
																									 rxbytes_{ rxbytes },
																									 txpkts_{ txpkts }, 
																									 txbytes_{ txbytes } {}

    uint16_t interface() const { return iface_; }
	uint64_t rxpkts() const { return rxpkts_; }
	uint64_t rxbytes() const { return rxbytes_; }
	uint64_t txpkts() const { return txpkts_; }
	uint64_t txbytes() const { return txbytes_; }
private:
    uint16_t iface_;
	uint64_t rxpkts_;
	uint64_t rxbytes_;
	uint64_t txpkts_;
	uint64_t txbytes_;
};

} // namespace message
} // namespace drop

#endif
