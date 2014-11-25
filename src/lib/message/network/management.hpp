
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

#ifndef DROP_MESSAGE_NETWORK_MANAGEMENT_HPP_
#define DROP_MESSAGE_NETWORK_MANAGEMENT_HPP_

#include <string>
#include <vector>

#include "message/message.hpp"

#include "router/port_info.hpp"
#include "router/route_info.hpp"
#include "router/address_info.hpp"

#include "mac_address.hpp"

namespace drop {
namespace message {

struct GetRoutes: public virtual tnt::Message {};

class SetPortUp: public virtual tnt::Message
{
public:
    explicit SetPortUp(unsigned int index): index_{ index } { }

    unsigned int index() const { return index_; }
private:
    unsigned int index_;
};

class SetPortDown: public virtual tnt::Message
{
public:
    explicit SetPortDown(unsigned int index): index_{ index } { }

    unsigned int index() const { return index_; }
private:
    unsigned int index_;
};

class AddLink: public virtual tnt::Message
{
public:
	AddLink(const std::string& type, const std::string& name, const std::string& mac, const std::string& mtu) : type_{ type }, name_{ name }, mac_{ mac }, mtu_{ mtu } {}

	const std::string& type() const { return type_; }
	const std::string& name() const { return name_; }
	const std::string& mac() const { return mac_; }
	const std::string& mtu() const { return mtu_; }
private:
	std::string type_;
	std::string name_;
	std::string mac_;
	std::string mtu_;
};

// NOTE: before adding a route be sure the interface is admin UP, otherwise no route will be added
class AddKernelRoute: public virtual tnt::Message
{
public:
    explicit AddKernelRoute(const RouteInfo& route): route_(route) { }
    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class AddUserspaceRoute: public virtual tnt::Message
{
public:
    AddUserspaceRoute(const RouteInfo& route, const tnt::MacAddress& mac): route_(route), mac_(mac) {}
    const RouteInfo& route() const { return route_; }
    const tnt::MacAddress& mac() const  { return mac_; }
private:
    RouteInfo route_;
    tnt::MacAddress mac_;
};

class DelRoute: public virtual tnt::Message
{
public:
    explicit DelRoute(const RouteInfo& route): route_{ route } {}

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class NewTrafficStats: public virtual tnt::Message
{
public:
    NewTrafficStats(const std::vector<std::pair<uint32_t, uint64_t>>& stats, uint64_t timestamp): stats_{ stats }, timestamp_{ timestamp } {}

    const std::vector<std::pair<uint32_t, uint64_t>>& stats() const { return stats_; }
    uint64_t timestamp() const { return timestamp_; }
private:
    std::vector<std::pair<uint32_t, uint64_t>> stats_;
    uint64_t timestamp_;
};

class PortAdminUp: public virtual tnt::Message
{
public:
    explicit PortAdminUp(const PortInfo& port): port_(port) {}

    const PortInfo& port() const { return port_; }
private:
    PortInfo port_;
};

class PortAdminDown: public virtual tnt::Message
{
public:
    explicit PortAdminDown(const PortInfo& port): port_(port) { }

    const PortInfo& port() const { return port_; }
private:
    PortInfo port_;
};

class AddRouteSuccess: public virtual tnt::Message
{
public:
    explicit AddRouteSuccess(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class DelRouteSuccess: public virtual tnt::Message
{
public:
    explicit DelRouteSuccess(const RouteInfo& route): route_{ route } { }

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class NetworkManagementErrorMessage: public virtual tnt::Message
{
protected:
    NetworkManagementErrorMessage(const unsigned int error): error_{ error } {}
public:
    unsigned int error() const { return error_; }
private:
    const unsigned int error_;
};

class AddAddressFailed: public NetworkManagementErrorMessage
{
public:
    AddAddressFailed(unsigned int error, const AddressInfo& address): NetworkManagementErrorMessage(error), address_(address) { }

    const AddressInfo& address() const { return address_; }
private:
    AddressInfo address_;
};

class DelAddressFailed: public NetworkManagementErrorMessage
{
public:
    DelAddressFailed(unsigned int error, const AddressInfo& address): NetworkManagementErrorMessage(error), address_(address) { }

    const AddressInfo& address() const { return address_; }
private:
    AddressInfo address_;
};

class AddRouteFailure: public NetworkManagementErrorMessage
{
public:
    AddRouteFailure(unsigned int error, const RouteInfo& route): NetworkManagementErrorMessage(error), route_{ route } {}

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

class DelRouteFailure: public NetworkManagementErrorMessage
{
public:
    DelRouteFailure(unsigned int error, const RouteInfo& route): NetworkManagementErrorMessage(error), route_{ route } {}

    const RouteInfo& route() const { return route_; }
private:
    RouteInfo route_;
};

} // namespace message
} // namespace drop

#endif
