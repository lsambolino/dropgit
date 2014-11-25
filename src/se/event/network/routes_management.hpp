
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

#ifndef DROP_EVENT_ROUTES_MANAGEMENT_HPP_
#define DROP_EVENT_ROUTES_MANAGEMENT_HPP_

#include <cstdint>
#include <string>
#include <iosfwd>

#include "event/drop/drop_event.hpp"
#include "event/network/operation_failed.hpp"

#include "router/address_info.hpp"
#include "router/route_info.hpp"

#include "mac_address.hpp"

namespace drop {
namespace event {

class AddKernelRoute: public DropEvent<AddKernelRoute, protocol::DropMessage::AddKernelRoute, RouteInfo>
{
public:
    explicit AddKernelRoute(const RouteInfo& route): route_{ route } { }
    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class AddUserspaceRoute: public DropEvent<AddUserspaceRoute, protocol::DropMessage::AddUserspaceRoute, RouteInfo, tnt::MacAddress>
{
public:
    AddUserspaceRoute(const RouteInfo& route, const tnt::MacAddress& mac): route_(route), mac_(mac) {}
    const RouteInfo& route() const { return route_; }
    const tnt::MacAddress& mac() const  { return mac_; }
private:
    RouteInfo route_;
    tnt::MacAddress mac_;
};

class DelRoute: public DropEvent<DelRoute, protocol::DropMessage::DelRoute, RouteInfo>
{
public:
    explicit DelRoute(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class RouteAdded
{
public:
    explicit RouteAdded(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class RouteRemoved
{
public:
    explicit RouteRemoved(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class AddRouteFailed: public OperationFailed
{
public:
    AddRouteFailed(int error, const RouteInfo& route): OperationFailed(error), route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class DelRouteFailed: public OperationFailed
{
public:
    DelRouteFailed(int error, const RouteInfo& route): OperationFailed(error), route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class RouteListRequest
{
public:
    explicit RouteListRequest(std::ostream& os): os_( os ) { }

    std::ostream& os() const { return os_; }
private:
    std::ostream& os_;
};

} // namespace event
} // namespace drop

#endif
