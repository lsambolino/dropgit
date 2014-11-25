
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

#include "routes_management.hpp"

#include <boost/asio.hpp>

#include "exception/drop_exception.hpp"

#include "router/de_control_element.hpp"

#include "message/network/management.hpp"

#include "util/configuration.hpp"
#include "util/pugixml.hpp"
#include "util/path.hpp"
#include "util/interfaces.hpp"

namespace drop {
namespace activity {

namespace {

void modify_interface_node(const std::string& iface, std::function<void(pugi::xml_node)> func)
{
    auto conf_path = util::full_path(tnt::Configuration::get("router.conf").as<std::string>());

    pugi::xml_document doc;
    auto result = doc.load_file(util::config_file_path("router").c_str());

    if (!result)
    {
        tnt::ConfigurationError(std::string("modify_interface_node: ") + result.description());
    }

    auto root = doc.child("router");
    assert(root);

    auto interfaces = root.child("interfaces");
    assert(interfaces);

    for (auto i : interfaces.children("iface"))
    {
        if (i.attribute("name").as_string() != iface)
        {
            continue;
        }

        func(i);
    }

    if (!doc.save_file(conf_path.c_str()))
    {
        tnt::Log::error("Unable to save the file ", conf_path);
    }
}

const char* lntoa(unsigned long value)
{
    in_addr addr = in_addr();
    addr.s_addr = value;

    return inet_ntoa(addr);
}

bool is_same_route(const RouteInfo& ri, const pugi::xml_node& route)
{
    return ri.destination == inet_addr(route.attribute("dst").as_string("0.0.0.0")) &&
           ri.source == inet_addr(route.attribute("src").as_string("0.0.0.0")) &&
           ri.gateway == inet_addr(route.attribute("gw").as_string("0.0.0.0")) &&
           ri.prefix == route.attribute("prefix").as_uint() &&
           ri.metric == route.attribute("metric").as_uint() &&
           ri.origin == route_origin(route.attribute("origin").as_string());
}

} // namespace

void RouteAdded::run()
{
    auto iface = tnt::index_to_name(route_.port_index);

    modify_interface_node(iface, [&] (pugi::xml_node iface)
    {
        auto routes = iface.child("routes");

        if (!routes)
        {
            tnt::Log::info(colors::blue, "Adding routes node");
            routes = iface.append_child("routes");
        }
        else
        {
            for (const auto& route : routes.children("route"))
            {
                if (is_same_route(route_, route))
                {
                    return;
                }
            }
        }

        tnt::Log::info(colors::blue, "Adding route node");
        auto route = routes.append_child("route");
        route.append_attribute("dst").set_value(lntoa(route_.destination));
        route.append_attribute("src").set_value(lntoa(route_.source));
        route.append_attribute("gw").set_value(lntoa(route_.gateway));
        route.append_attribute("prefix").set_value(route_.prefix);
        route.append_attribute("metric").set_value(route_.metric);
        route.append_attribute("origin").set_value(route_origin(route_.origin).c_str());
    });
}

void RouteDeleted::run()
{
    auto iface = tnt::index_to_name(route_.port_index);

    modify_interface_node(iface, [&] (pugi::xml_node iface)
    {
        auto routes = iface.child("routes");

        for (const auto& route : routes.children("route"))
        {
            if (is_same_route(route_, route))
            {
                routes.remove_child(route);

                break;
            }
        }
    });
}

void AddRouteFailed::run()
{
    try
    {
        // Close the connection with the CE.
        //fe::ForwardingElement::ce()->drop(nullptr);
	    //fe::ForwardingElement::ce(nullptr);
    }
    catch (NoCeAssociation&) {}
}

void DelRouteFailed::run()
{
    // Do nothing.
}

} // namespace activity
} // namespace drop
