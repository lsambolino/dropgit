
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

#include "route_info.hpp"

#include <cassert>

#include "util/interfaces.hpp"

#include "ip_address.hpp"
#include "port_info.hpp"

namespace drop {

RouteOrigin route_origin(const std::string& value)
{
    if (value == "Kernel")
    {
        return RouteOrigin::Kernel;
    }

    if (value == "Static")
    {
        return RouteOrigin::Static;
    }

    if (value == "Zebra")
    {
        return RouteOrigin::Zebra;
    }

    if (value == "Boot")
    {
        return RouteOrigin::Boot;
    }

    return RouteOrigin::Unknown;
}

std::string route_origin(RouteOrigin origin)
{
    switch (origin)
    {
    case RouteOrigin::Kernel:
        return "Kernel";
    case RouteOrigin::Static:
        return "Static";
    case RouteOrigin::Zebra:
        return "Zebra";
    case RouteOrigin::Boot:
        return "Boot";
    case RouteOrigin::Unknown:
    default:
        return "Unknown";
    }
}

RouteInfo::RouteInfo(): prefix(0), destination(0), gateway(0), port_index(0), metric(0), source(0), origin(RouteOrigin::Unknown) {}

bool operator==(const RouteInfo& r1, const RouteInfo& r2)
{
    return r1.destination == r2.destination && r1.source == r2.source && r1.gateway == r2.gateway && ((r1.port_index == r2.port_index) || r1.port_index == 0 || r2.port_index == 0);
}

std::ostream& operator<<(std::ostream& os, RouteOrigin origin)
{
    switch (origin)
    {
    case RouteOrigin::Kernel:
        os << "kernel";
        break;
    case RouteOrigin::Boot:
        os << "boot";
        break;
    case RouteOrigin::Static:
        os << "static";
        break;
    case RouteOrigin::Zebra:
        os << "zebra";
        break;
    case RouteOrigin::Unknown:
    default:
        os << "unknown";
        break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const RouteInfo& route)
{
    auto gw = tnt::ip::Address::from_net_order_ulong(route.gateway);
    auto src = tnt::ip::Address::from_net_order_ulong(route.source);
    auto dst = tnt::ip::Address::from_net_order_ulong(route.destination);
    auto dev = tnt::index_to_name(route.port_index);

    if (gw == tnt::ip::Address::any())
    {
        os << dst << "/" << route.prefix << " dev " << dev << " proto " << route.origin << " scope link src " << src;
    }
    else if (dst == tnt::ip::Address::any())
    {
        os << "default via " << gw << " dev " << dev;
    }
    else
    {
        os << dst << "/" << route.prefix << " via " << gw << " dev " << dev;
    }

    return os;
}

} // namespace drop

