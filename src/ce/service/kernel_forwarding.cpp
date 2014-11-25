
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

#include "kernel_forwarding.hpp"

#include "message/network/management.hpp"

#include "network/service_element.hpp"

namespace drop {
namespace service {
namespace {

RouteInfo on_link_route(const RouteInfo& route)
{
    RouteInfo ri;
    ri.origin = RouteOrigin::Kernel;
    ri.port_index = route.port_index;
    ri.prefix = route.prefix;
    ri.destination = route.destination;
    ri.gateway = 0;
    ri.source = 0;

    return ri;
}

} // namespace

KernelForwarding::KernelForwarding(ce::ServiceElement* parent) : parent_(parent) {}

std::ostream& KernelForwarding::print(std::ostream& os) const
{
    os << R"({ "Name": "KernelForwarding" })";

    return os;
}

void KernelForwarding::add(const RouteInfo& route)
{
    parent_->send(std::make_unique<message::AddKernelRoute>(on_link_route(route)));
}

void KernelForwarding::remove(const RouteInfo& route)
{
    parent_->send(std::make_unique<message::DelRoute>(route));
}

} // namespace service
} // namespace drop
