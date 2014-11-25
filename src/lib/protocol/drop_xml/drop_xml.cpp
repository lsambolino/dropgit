
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

#include "drop_xml.hpp"

#include <cassert>

#include "event/network/connection_reset.hpp"

#include "exception/drop_exception.hpp"

#include "message/message.hpp"

namespace drop {
namespace protocol {

DropXml::DropXml(const std::shared_ptr<tnt::IO>& io): AsyncProtocol(io) {}

void DropXml::invoke_message(const std::string& message)
{
    pugi::xml_document doc;
	auto result = doc.load_buffer(message.data(), message.size());

	if (!result)
	{
        throw InvalidMessage(result.description());
	}

    auto root = doc.root();

    rx_dispatcher_.inject_object(root.first_child().name(), root);
}

std::vector<std::string> DropXml::parse(std::string& raw_input)
{
    std::vector<std::string> messages;

    while (true)
    {
        const auto end_tag_len = 7;
        auto len = raw_input.size();

        auto pos_s = raw_input.find("<drop>");
        auto pos_e = raw_input.find("</drop>");

        if (pos_s != 0)
        {
            throw InvalidMessage("Start tag missing");
        }

        if (pos_e == std::string::npos)
        {
            // Got only a part of a message, wait for other data from IO class.
            break;
        }

        pos_e += end_tag_len;

        if (pos_e == len)
        {
            // Got one single message.
            messages.push_back(std::move(raw_input)); // TODO: Check if the move is valid.
            raw_input.clear();

            break;
        }

        // Got more than one message.
        messages.push_back(raw_input.substr(0, pos_e));
        raw_input = raw_input.substr(pos_e);
    }

    return messages;
}

} // namespace protocol
} // namespace drop
