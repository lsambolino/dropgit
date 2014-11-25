
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

#include "configuration_persistence.hpp"

#include <string>
#include <fstream>
#include <atomic>

#include "event/quit.hpp"
#include "event/network/address_events.hpp"
#include "event/network/route_events.hpp"

#include "router/address.hpp"
#include "router/route.hpp"
#include "router/route_info.hpp"

#include "util/configuration.hpp"
#include "util/pugixml.hpp"
#include "util/path.hpp"

#include "log.hpp"
#include "demangle.hpp"

namespace drop {
namespace activity {
namespace {

/*void modify_interface_node(const std::string& path, const std::string& iface, std::function<void(pugi::xml_node&)> func)
{
    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());

    if (!result)
    {
        tnt::ConfigurationError(std::string("ConfigurationPersistence: ") + result.description());
    }

    auto root = doc.child("router");
    assert(root);

    auto interfaces = root.child("interfaces");
    assert(interfaces);

    for (auto& interface : interfaces.children("interface"))
    {
        if (interface.attribute("name").as_string() != iface)
        {
            continue;
        }

        func(interface);
    }

    if (!doc.save_file(path.c_str()))
    {
        tnt::Log::error("Unable to save the file ", path);
    }
}

const char* lntoa(unsigned long value)
{
    in_addr addr = in_addr();
    addr.s_addr = value;

    return inet_ntoa(addr);
}*/

} // namespace

ConfigurationPersistence::ConfigurationPersistence() {}

void ConfigurationPersistence::operator()()
{
    //auto conf_path = util::full_path(tnt::Configuration::get("router.conf").as<std::string>());

    std::atomic_bool running{ true };

    register_handler([&] (const tnt::event::Quit& /*event*/)
    {
        running = false;
    });

    /*register_handler([&] (const event::RouteAdded& event)
    {
        const auto& ri = static_cast<const RouteInfo>(*event->route());

        if (ri.origin == RouteOrigin::Zebra)
        {
            return;
        }

        modify_interface_node(conf_path, event->interface(), [&] (pugi::xml_node& interface)
        {
            auto routes = interface.child("routes");

            if (!routes)
            {
                routes = interface.append_child("routes");
            }
            else
            {
                for (auto& route : routes.children("route"))
                {
                    if (ri.destination == inet_addr(route.attribute("dest").as_string("0.0.0.0")) &&
                        ri.source == inet_addr(route.attribute("src").as_string("0.0.0.0")) &&
                        ri.gateway == inet_addr(route.attribute("gw").as_string("0.0.0.0")) &&
                        ri.prefix == route.attribute("prefix").as_uint())
                    {
                        if (ri.metric == route.attribute("metric").as_uint())
                        {
                            return;
                        }

                        route.attribute("metric").set_value(ri.metric);

                        return;
                    }
                }
            }

            auto route = routes.append_child("route");

            route.append_attribute("dst").set_value(lntoa(ri.destination));
            route.append_attribute("src").set_value(lntoa(ri.source));
            route.append_attribute("gw").set_value(lntoa(ri.gateway));
            route.append_attribute("prefix").set_value(ri.prefix);
            route.append_attribute("metric").set_value(ri.metric);
            route.append_attribute("origin").set_value(route_origin(ri.origin).c_str());
        });
    });

    register_handler([&] (const event::RouteRemoved& event)
    {
        const auto& ri = static_cast<const RouteInfo>(*event->route());

        modify_interface_node(conf_path, event->interface(), [&] (pugi::xml_node& interface)
        {
            auto routes = interface.child("routes");

            for (const auto& route : routes.children("route"))
            {
                if (ri.destination == inet_addr(route.attribute("dest").as_string("0.0.0.0")) &&
                    ri.source == inet_addr(route.attribute("src").as_string("0.0.0.0")) &&
                    ri.gateway == inet_addr(route.attribute("gw").as_string("0.0.0.0")) &&
                    ri.prefix == route.attribute("prefix").as_uint() &&
                    ri.metric == route.attribute("metric").as_uint())
                {
                    routes.remove_child(route);

                    break;
                }
            }
        });
    });

    register_handler([&] (const event::AddressAdded& event)
    {
        auto ip_addr = event->address()->str();

        modify_interface_node(conf_path, event->interface(), [&] (pugi::xml_node& interface)
        {
            auto addresses = interface.child("addresses");

            if (!addresses)
            {
                addresses = interface.append_child("addresses");
            }
            else
            {
                for (const auto& address : addresses.children("address"))
                {
                    if (address.attribute("value").as_string() == ip_addr)
                    {
                        return;
                    }
                }
            }

            auto address = addresses.append_child("address");
            address.append_attribute("value").set_value(ip_addr.c_str());
            address.append_attribute("prefix").set_value(event->address()->prefix());
        });
    });

    register_handler([&] (const event::AddressRemoved& event)
    {
        auto ip_addr = event->address()->str();

        modify_interface_node(conf_path, event->interface(), [&] (pugi::xml_node& interface)
        {
            auto addresses = interface.child("addresses");
            
            for (const auto& address : addresses.children("address"))
            {
                if (address.attribute("value").as_string() == ip_addr)
                {
                    addresses.remove_child(address);

                    break;
                }
            }
        });
    });*/

    while (running)
    {
        wait_event();
    }
}

} // namespace activity
} // namespace drop
