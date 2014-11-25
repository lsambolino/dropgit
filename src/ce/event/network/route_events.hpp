
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

#ifndef DROP_EVENT_ROUTE_EVENTS_HPP_
#define DROP_EVENT_ROUTE_EVENTS_HPP_

#include <memory>

#include "event/network/request.hpp"
#include "event/drop/drop_event.hpp"

#include "router/route_info.hpp"

namespace drop {
namespace router {

class Route;

} // namespace router

namespace event {

class RouteList
{
public:
    explicit RouteList(const std::vector<RouteInfo>& routes): routes_{ routes } { }
    const std::vector<RouteInfo>& routes() const { return routes_; }
private:
    std::vector<RouteInfo> routes_;
};

class AddRouteSuccess: public DropEvent<AddRouteSuccess, protocol::DropMessage::AddRouteSuccess, RouteInfo>
{
public:
    explicit AddRouteSuccess(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class DelRouteSuccess: public DropEvent<DelRouteSuccess, protocol::DropMessage::DelRouteSuccess, RouteInfo>
{
public:
    explicit DelRouteSuccess(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class AddRouteFailure: public DropEvent<AddRouteFailure, protocol::DropMessage::AddRouteFailure, unsigned int, RouteInfo>
{
public:
    AddRouteFailure(unsigned int error, const RouteInfo& route): route_{ route }, error_{ error } { }

    unsigned int error() const { return error_; }
    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
    unsigned int error_;
};

class DelRouteFailure: public DropEvent<DelRouteFailure, protocol::DropMessage::DelRouteFailure, unsigned int, RouteInfo>
{
public:
    DelRouteFailure(unsigned int error, const RouteInfo& route): route_{ route }, error_{ error } { }

    unsigned int error() const { return error_; }
    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
    unsigned int error_;
};

class LocalRouteAdded
{
public:
    explicit LocalRouteAdded(const RouteInfo& route): route_{ route } { }
    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class LocalRouteRemoved
{
public:
    explicit LocalRouteRemoved(const RouteInfo& route): route_{ route } { }
    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class AddRouteRequest: public Request
{
public:
    AddRouteRequest(const RouteInfo& route, const ReplyInfo& reply): Request(reply), route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class RemoveRouteRequest: public Request
{
public:
    RemoveRouteRequest(const RouteInfo& route, const ReplyInfo& reply): Request(reply), route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

} // namespace event
} // namespace drop

#endif
