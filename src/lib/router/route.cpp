
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

#include "route.hpp"

#include <cassert>

#include "util/interfaces.hpp"

#include "ip_address.hpp"

namespace drop {
namespace router {

Route::Route(const RouteInfo& route): route_{ route } {}

Route::operator RouteInfo() const
{
    return route_;
}

bool Route::is_default() const
{
    return destination() == tnt::ip::Address::any();
}

tnt::ip::Address Route::source() const
{
    return tnt::ip::Address::from_net_order_ulong(route_.source);
}

tnt::ip::Address Route::destination() const
{
    return tnt::ip::Address::from_net_order_ulong(route_.destination);
}

tnt::ip::Address Route::gateway() const
{
    return tnt::ip::Address::from_net_order_ulong(route_.gateway);
}

uint32_t Route::prefix() const
{
    return route_.prefix;
}

uint32_t Route::metric() const
{
    return route_.metric;
}

uint32_t Route::if_index() const
{
    return route_.port_index;
}

std::string Route::interface() const
{
    return tnt::index_to_name(route_.port_index);
}

std::ostream& operator<<(std::ostream& os, const Route& route)
{
    os << static_cast<const RouteInfo>(route);

    return os;
}

bool operator==(const Route& route, const RouteInfo& ri)
{
    return static_cast<const RouteInfo>(route) == ri;
}

} // namespace router
} // namespace drop
