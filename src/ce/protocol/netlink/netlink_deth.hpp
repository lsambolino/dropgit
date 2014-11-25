
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

#ifndef NETLINK_DETH_PROTOCOL_HPP_
#define NETLINK_DETH_PROTOCOL_HPP_

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "protocol/async_protocol.hpp"

namespace drop {
namespace message {

class DethInPacket;
class DethLinkChange;
class DethLinkStats;

} // namespace message

namespace protocol {

class NetlinkDethProtocol: public tnt::protocol::AsyncProtocol
{
public:
    explicit NetlinkDethProtocol(const std::shared_ptr<tnt::IO>& io);
private:
    virtual std::vector<std::string> parse(std::string& raw_input) override; //! raw_input is passed by non const reference, so parse function can modify it.
    virtual void invoke_message(const std::string& data) override;
    virtual void register_messages() override;

	// Messages handlers
	void deth_in_packet(message::DethInPacket* message);
	void deth_link_change(message::DethLinkChange* message);
	void deth_link_stats(message::DethLinkStats* message);
};

} // namespace protocol
} // namespace drop

#endif
