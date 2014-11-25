
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

#include "ce_parser_activities.hpp"

#include <iostream>
#include <sstream>

#include "network/service_element.hpp"

#include "router/interface.hpp"
#include "router/address.hpp"
#include "router/route.hpp"

#include "exception/drop_exception.hpp"

#include "event/gal/traffic_profile_change.hpp"
#include "event/router_requests.hpp"
#include "event/service_request.hpp"
#include "event/network/port_stats_request.hpp"

#include "gal/green_standard_interface.hpp"

#include "util/wol.hpp"

#include "range.hpp"
#include "demangle.hpp"
#include "log.hpp"
#include "trie_map.hpp"
#include "dynamic_pointer_visitor.hpp"
#include "application.hpp"

namespace drop {
namespace activity {

ListServiceElements::ListServiceElements(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void ListServiceElements::operator()()
{
    int num = 0;

    auto e = std::make_shared<event::ServiceElementsRequest>([&] (const auto& se)
    {
        os_ << *se << std::endl;
        ++num;
    });

    tnt::Application::raise(e);
    e->wait();

    if (num == 0)
    {
        os_ << "No Service Elements connected." << std::endl;
    }
}

ListInterfaces::ListInterfaces(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void ListInterfaces::operator()()
{
    int num = 0;

    auto e = std::make_shared<event::InterfacesRequest>([&] (const auto& i)
    {
        os_ << *i << std::endl;
        ++num;
    });

    tnt::Application::raise(e);
    e->wait();

    if (num == 0)
    {
        os_ << "No Interfaces in the DROP router." << std::endl;
    }
}

ListRoutes::ListRoutes(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void ListRoutes::operator()()
{
    int num = 0;

    auto e = std::make_shared<event::RoutesRequest>([&] (const auto& r)
    {
        os_ << *r << std::endl;
        ++num;
    });

    tnt::Application::raise(e);
    e->wait();

    if (num == 0)
    {
        os_ << "No Routes configured in the DROP router." << std::endl;
    }
}

ListAddresses::ListAddresses(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void ListAddresses::operator()()
{
    auto num = 0;

    auto e = std::make_shared<event::AddressesRequest>([&] (const auto& a)
    {
        os_ << *a << std::endl;
        ++num;
    });

    tnt::Application::raise(e);
    e->wait();

    if (num == 0)
    {
        os_ << "No IP Addresses configured in the DROP router." << std::endl;
    }
}

TrafficProfileChange::TrafficProfileChange(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void TrafficProfileChange::operator()()
{
    std::stringstream ss(line_);
    std::string tmp;
    int num;
    
    ss >> tmp >> num;

    if (ss.fail())
    {
        os_ << "GAL command syntax error: missing parameters." << std::endl;

        return;
    }

    tnt::Application::raise(event::TrafficProfileChange(num));
}

WakeOnLanCommand::WakeOnLanCommand(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void WakeOnLanCommand::operator()()
{
    std::stringstream ss(line_);
    std::string tmp;
    std::string ip;
    std::string hw;

    ss >> tmp >> ip;

    if (ss.fail())
    {
        os_ << "WOL command syntax error: missing parameters." << std::endl;

        return;
    }

    ss >> hw;

    wol::send_magic_packets(1, std::chrono::milliseconds(0), ip, hw);
}

PortStatsRequest::PortStatsRequest(std::ostream& os, const std::string& line): os_(os), line_{ line } {}

void PortStatsRequest::operator()()
{
    std::stringstream ss(line_);
    std::string tmp;
    std::string iface;
    
    ss >> tmp >> iface;

    if (ss.fail())
    {
        os_ << "statistics command syntax error: missing parameters.\n"
            << "Parameters are:\n"
            << "\t<interface>" << std::endl;

        return;
    }

    tnt::Application::raise(event::PortStatsRequest(iface));
}

} // namespace activity
} // namespace drop
