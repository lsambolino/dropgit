
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

#ifndef DROP_MESSAGE_NETWORK_REQUEST_HPP_
#define DROP_MESSAGE_NETWORK_REQUEST_HPP_

#include <cstdint>
#include <vector>
#include <memory>

#include "message/message.hpp"

#include "protocol/netlink/values.hpp"
#include "protocol/netlink/reply_info.hpp"

namespace drop {

class NetworkPort;

namespace router {

class Interface;
class Address;
class Route;

} // namespace router

namespace message {

using ReplyInfo = protocol::NetlinkReplyInfo;

class Reply: public virtual tnt::Message
{
protected:
    Reply() {}
    explicit Reply(const ReplyInfo& reply): reply_(reply) { }
public:
    const ReplyInfo& reply() const { return reply_; }
private:
    ReplyInfo reply_;
};

class GetPorts: public Reply
{
public:
    explicit GetPorts(const std::vector<std::shared_ptr<router::Interface>>& ports): Reply(), ports_{ ports } { }
    GetPorts(const std::vector<std::shared_ptr<router::Interface>>& ports, const ReplyInfo& reply): Reply(reply), ports_{ ports } { }

    const std::vector<std::shared_ptr<router::Interface>>& ports() const { return ports_; }
private:
    std::vector<std::shared_ptr<router::Interface>> ports_;
};

class GetAddresses: public Reply
{
public:
    explicit GetAddresses(const std::vector<std::shared_ptr<router::Address>>& addresses): Reply(), addresses_{ addresses } { }
    GetAddresses(const std::vector<std::shared_ptr<router::Address>>& addresses, const ReplyInfo& reply): Reply(reply), addresses_{ addresses } { }

    const std::vector<std::shared_ptr<router::Address>>& addresses() const { return addresses_; }
private:
    std::vector<std::shared_ptr<router::Address>> addresses_;
};

class GetRoutes: public Reply
{
public:
    explicit GetRoutes(const std::vector<std::shared_ptr<router::Route>>& routes): Reply(), routes_{ routes } { }
    GetRoutes(const std::vector<std::shared_ptr<router::Route>>& routes, const ReplyInfo& reply): Reply(reply), routes_{ routes } { }

    const std::vector<std::shared_ptr<router::Route>>& routes() const { return routes_; }
private:
    std::vector<std::shared_ptr<router::Route>> routes_;
};

class PortChanged: public Reply
{
public:
    explicit PortChanged(const std::shared_ptr<NetworkPort>& port): Reply(), port_(port) { }
    PortChanged(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Reply(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class Error: public Reply
{
public:
    explicit Error(int code): Reply(), code_{ code } { }
    Error(int code, const ReplyInfo& reply): Reply(reply), code_{ code } { }

    int code() const { return code_; }
private:
    int code_;
};

class PortDown: public Reply
{
public:
    explicit PortDown(const std::shared_ptr<NetworkPort>& port): Reply(), port_(port) { }
    PortDown(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Reply(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class PortUp: public Reply
{
public:
    explicit PortUp(const std::shared_ptr<NetworkPort>& port): Reply(), port_(port) { }
    PortUp(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Reply(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class PortAdded: public Reply
{
public:
    explicit PortAdded(const std::shared_ptr<NetworkPort>& port): Reply(), port_(port) { }
    PortAdded(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Reply(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class PortRemoved: public Reply
{
public:
    explicit PortRemoved(const std::shared_ptr<NetworkPort>& port): Reply(), port_(port) { }
    PortRemoved(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Reply(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class AddressAdded: public Reply
{
public:
    explicit AddressAdded(const std::shared_ptr<router::Address>& address): Reply(), address_(address) { }
    AddressAdded(const std::shared_ptr<router::Address>& address, const ReplyInfo& reply): Reply(reply), address_(address) { }

    const std::shared_ptr<router::Address>& address() const { return address_; }
private:
    std::shared_ptr<router::Address> address_;
};

class AddressRemoved: public Reply
{
public:
    explicit AddressRemoved(const std::shared_ptr<router::Address>& address): Reply(), address_(address) { }
    AddressRemoved(const std::shared_ptr<router::Address>& address, const ReplyInfo& reply): Reply(reply), address_(address) { }

    const std::shared_ptr<router::Address>& address() const { return address_; }
private:
    std::shared_ptr<router::Address> address_;
};

class RouteAdded: public Reply
{
public:
    explicit RouteAdded(const std::shared_ptr<router::Route>& route): Reply(), route_{ route } { }
    RouteAdded(const std::shared_ptr<router::Route>& route, const ReplyInfo& reply): Reply(reply), route_{ route } { }

    const std::shared_ptr<router::Route>& route() const { return route_; }
private:
    std::shared_ptr<router::Route> route_;
};

class RouteRemoved: public Reply
{
public:
    explicit RouteRemoved(const std::shared_ptr<router::Route>& route): Reply(), route_{ route } { }
    RouteRemoved(const std::shared_ptr<router::Route>& route, const ReplyInfo& reply): Reply(reply), route_{ route } { }

    const std::shared_ptr<router::Route>& route() const { return route_; }
private:
    std::shared_ptr<router::Route> route_;
};

} // namespace message
} // namespace drop

#endif
