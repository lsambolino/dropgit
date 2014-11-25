
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

#include "setup_control_element.hpp"

#include <boost/asio.hpp>

#include "router/de_control_element.hpp"

#include "message/network/service_element_data.hpp"

#include "event/network/ce_connected.hpp"
#include "event/network/ack.hpp"

#include "protocol/protocol.hpp"

#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/pugixml.hpp"

#include "application.hpp"
#include "log.hpp"

namespace drop {
namespace activity {

SetupControlElement::SetupControlElement(const std::shared_ptr<tnt::Protocol>& proto): proto_(proto)
{
    proto_->start();
}

void SetupControlElement::operator()()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(tnt::Configuration::get("connection.delay").as<int>()));

    register_handler(proto_.get(), [] (const event::Ack& /*event*/) {});

    int timeout = 0;

	// In a block to close the file ASAP.
    {
        pugi::xml_document doc;
        auto result = doc.load_file(util::config_file_path("drop").c_str());

        if (!result)
        {
            throw tnt::ConfigurationError(std::string("SetupControlElement error: ") + result.description());
        }

        auto root = doc.first_child();
        auto address = root.child("local").child("address").text().as_string();
        timeout = root.child("connection").child("timeout").text().as_int();

        auto fwd = tnt::Configuration::get("use_userspace_forward", false) ? "UserspaceForwarding" : "KernelForwarding";

        proto_->send(std::make_unique<message::ServiceElementData>(boost::asio::ip::host_name(), address, std::vector<std::string>{ fwd, "GalRest" }));
    }

    if (wait_event_for(std::chrono::milliseconds(timeout)))
    {
        tnt::Log::info("Control Element Set Up");
        tnt::Application::raise(event::CeConnected(std::make_shared<de::ControlElement>(proto_)));
    }
    else
    {
        tnt::Log::error("Timeout expired while setting up the Control Element");
    }
}

} // namespace activity
} // namespace drop
