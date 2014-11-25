
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

#ifndef DROP_ACTIVITY_ROUTES_MANAGEMENT_HPP_
#define DROP_ACTIVITY_ROUTES_MANAGEMENT_HPP_

#include "activity/concurrent_activity.hpp"
#include "activity/activity_from_event.hpp"

#include "protocol/netlink/values.hpp"
#include "protocol/netlink/kernel_netlink.hpp"

#include "event/network/routes_management.hpp"

#include "log.hpp"

namespace drop {
namespace activity {

// Requests coming from Control Element

// Locally generated replies / events	(notifications)

class RouteAdded: public tnt::RegisterActivityFromEvent<RouteAdded, event::RouteAdded>
{
public:
    explicit RouteAdded(const event::RouteAdded& event): route_(event.route()) { }
    virtual void run() override;
private:
    RouteInfo route_;
};

class RouteDeleted: public tnt::RegisterActivityFromEvent<RouteDeleted, event::RouteRemoved>
{
public:
    explicit RouteDeleted(const event::RouteRemoved& event): route_(event.route()) { }
    virtual void run() override;
private:
    RouteInfo route_;
};

// Failure (operation) replies

class AddRouteFailed: public tnt::RegisterActivityFromEvent<AddRouteFailed, event::AddRouteFailed>
{
public:
    explicit AddRouteFailed(const event::AddRouteFailed& event): error_(event.error()), route_(event.route()) { }
    virtual void run() override;
private:
    unsigned int error_;
    RouteInfo route_;
};

class DelRouteFailed: public tnt::RegisterActivityFromEvent<DelRouteFailed, event::DelRouteFailed>
{
public:
    explicit DelRouteFailed(const event::DelRouteFailed& event): error_(event.error()), route_(event.route()) { }
    virtual void run() override;
private:
    unsigned int error_;
    RouteInfo route_;
};

} // namespace activity
} // namespace drop

#endif
