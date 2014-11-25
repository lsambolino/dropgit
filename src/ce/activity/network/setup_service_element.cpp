
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

#include "setup_service_element.hpp"

#include <algorithm>

#include "protocol/protocol.hpp"
#include "protocol/drop/values.hpp"

#include "message/network/ack.hpp"

#include "event/network/service_element_connected.hpp"
#include "event/network/service_element_data.hpp"

#include "util/configuration.hpp"

#include "log.hpp"
#include "application.hpp"

namespace drop {
namespace activity {

SetupServiceElement::SetupServiceElement(const std::shared_ptr<tnt::Protocol>& proto): proto_(proto) {}

void SetupServiceElement::operator()()
{
    proto_->start();

    register_handler(proto_.get(), [this] (const std::shared_ptr<event::ServiceElementData>& event)
    {
        tnt::Application::raise(event::ServiceElementConnected(event->name(), event->address(), event->services(), proto_));
    });

    auto timeout = tnt::Configuration::get("connection.timeout").as<int>();
    auto success = wait_event_for(std::chrono::milliseconds(timeout));

    if (success)
    {
        proto_->send(std::make_unique<message::Ack>(protocol::DropMessage::ServiceElementData));

        return;
    }

    tnt::Log::warning("Service Element setup timeout expired.");
}

} // namespace activity
} // namespace drop
