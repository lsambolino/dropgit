
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

#include "drop.hpp"

#include <cassert>

#include "protocol/drop/structs.hpp"

#include "event/network/connection_reset.hpp"

namespace drop {
namespace protocol {

Drop::Drop(const std::shared_ptr<tnt::IO>& io): AsyncProtocol(io) {}

void Drop::invoke_message(const std::string& message)
{
    assert(message.size() >= MessageHeader::hdr_len);
    auto hdr = reinterpret_cast<const MessageHeader*>(message.data());
    assert(message.size() == hdr->total_length());

    invoke(hdr->type(), message.substr(MessageHeader::hdr_len));
}

std::vector<std::string> Drop::parse(std::string& raw_input)
{
    std::vector<std::string> messages;

    while (true)
    {
        auto len = raw_input.size();

        if (len < MessageHeader::hdr_len)
        {
            // Got only a partial header of a message, wait for other data from IO class.
            break;
        }

        auto hdr = reinterpret_cast<const MessageHeader*>(raw_input.data());
        auto packet_len = hdr->total_length();

        if (len < packet_len)
        {
            // Got only a part of a message, wait for other data from IO class.
            break;
        }

        if (len == packet_len)
        {
            // Got one single message.
            messages.push_back(raw_input);
            raw_input.clear();

            break;
        }

        // Got more than one message.
        messages.push_back(raw_input.substr(0, packet_len));
        raw_input = raw_input.substr(packet_len);
    }

    return messages;
}

} // namespace protocol
} // namespace drop
