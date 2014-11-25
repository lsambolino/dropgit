
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

#ifndef DROP_PROTOCOL_DROP_XML_HPP_
#define DROP_PROTOCOL_DROP_XML_HPP_

#include <memory>
#include <string>

#include "protocol/async_protocol.hpp"
#include "protocol/drop_xml/packets.hpp"

#include "util/pugixml.hpp"

namespace drop {
namespace protocol {

class DropXml: public tnt::protocol::AsyncProtocol
{
public:
    explicit DropXml(const std::shared_ptr<tnt::IO>& io);
private:
    virtual std::vector<std::string> parse(std::string& raw_input) override; //! raw_input is passed by non const reference, so parse function can modify it.
    virtual void invoke_message(const std::string& data) override;

    virtual void register_messages() override;

    void register_in_messages();

    template <class F> void register_in_message(const std::string& key, F func)
    {
        rx_dispatcher_.register_listener(key, func);
    }

    template <class M> void register_out_message()
    {
        register_message<M>([this] (auto message)
        {
            write(create_packet(message));
        });
    }
private:
    tnt::KeyDispatch<std::string, std::unique_ptr<tnt::Message>, pugi::xml_node> rx_dispatcher_;
};

} // namespace protocol
} // namespace drop

#endif
