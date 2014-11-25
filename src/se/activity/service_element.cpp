
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

#include "service_element.hpp"

#include <atomic>
#include <cassert>

#include <boost/asio.hpp>

#include "event/quit.hpp"
#include "event/network/routes_management.hpp"
#include "event/network/ce_connected.hpp"

#include "message/network/management.hpp"

#include "protocol/protocol.hpp"

#include "router/route.hpp"

#include "util/configuration.hpp"
#include "util/pugixml.hpp"
#include "util/factory.hpp"
#include "util/path.hpp"
#include "util/interfaces.hpp"

#include "log.hpp"

namespace drop {
namespace activity {

ServiceElement::ServiceElement(): kernel_(factory::create_kernel_protocol())
{
    assert(kernel_);
    kernel_->start();

    configure();
}

ServiceElement::~ServiceElement()
{
    tnt::Log::debug(colors::green, "ServiceElement activity terminated.");
}

void ServiceElement::configure()
{
    /*pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("ServiceElement: ") + result.description());
    }

    auto root = doc.child("router");

    load_router(root.child("interfaces"));*/
}

void ServiceElement::load_router(const pugi::xml_node& root)
{
    for (const auto& iface : root.children("iface"))
    {
        auto name = iface.attribute("name").as_string();
        auto index = tnt::name_to_index(name);
        tnt::insert_back(interfaces_, index);

		if (!tnt::Configuration::get("", false))
		{
			auto routes = iface.child("routes");

			for (const auto& route : routes.children("route"))
			{
				RouteInfo ri;
				ri.destination = inet_addr(route.attribute("dst").as_string("0.0.0.0"));
				ri.source = inet_addr(route.attribute("src").as_string("0.0.0.0"));
				ri.gateway = inet_addr(route.attribute("gw").as_string("0.0.0.0"));
				ri.prefix = route.attribute("prefix").as_uint();
				ri.metric = route.attribute("metric").as_uint();
				ri.origin = route_origin(route.attribute("origin").as_string());
				ri.port_index = index;

				kernel_->send(std::make_unique<message::AddKernelRoute>(ri));

				tnt::insert_back(routes_, std::make_shared<router::Route>(ri));
			}
		}
    }
}

void ServiceElement::operator()()
{
    std::atomic_bool running{ true };

    register_handler([&] (const tnt::event::Quit& /*event*/)
    {
        running = false;
    });

    register_handler([&] (const event::CeConnected& event)
    {
        ce_ = event.ce();
    });

	if (!tnt::Configuration::get("use_netmap", false))
	{
		register_handler([this] (const event::AddKernelRoute& event)
		{
			auto ri = event.route();

			if (interfaces_.empty())
			{
				return;
			}

			for (const auto& r : routes_)
			{
				if (static_cast<const RouteInfo>(*r).destination == ri.destination)
				{
					return;
				}
			}

			// TODO: Select an external iface to which to add the route.
			ri.port_index = interfaces_.front();

			tnt::insert_back(routes_, std::make_shared<router::Route>(ri));

			// NOTE: before adding a route be sure the iface is admin UP, otherwise no route will be added
			tnt::Log::info(colors::blue, "Received from CE the request to add the route ", ri);
			kernel_->send(std::make_unique<message::AddKernelRoute>(ri));
		});
    
		register_handler([this] (const event::DelRoute& event)
		{
			auto ri = event.route();

			for (auto it = std::begin(routes_); it != std::end(routes_); ++it)
			{
				auto r = static_cast<const RouteInfo>(**it);

				if (r.destination == ri.destination)
				{
					routes_.erase(it);
					kernel_->send(std::make_unique<message::DelRoute>(r));

					break;
				}
			}
		});
	}

    while (running)
    {
        wait_event();
    }
}

} // namespace activity
} // namespace drop
