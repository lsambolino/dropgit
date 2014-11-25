
/*

Copyright (c) 2013, Marco Chiappero (marco@absence.it), Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#ifndef DROP_EVENT_PORT_EVENTS_HPP_
#define DROP_EVENT_PORT_EVENTS_HPP_

#include <memory>
#include <cstdint>
#include <string>

#include "event/network/request.hpp"
#include "event/drop/drop_event.hpp"

#include "router/port_info.hpp"

namespace drop {

class NetworkPort;
class VlanPort;

namespace physical {

class Interface;

} // namespace physical

namespace event {

class PortList
{
public:
    explicit PortList(const std::vector<std::shared_ptr<NetworkPort>>& ports): ports_{ ports } { }
    const std::vector<std::shared_ptr<NetworkPort>>& ports() const { return ports_; }
private:
    std::vector<std::shared_ptr<NetworkPort>> ports_;
};

class LocalPortAdded
{
public:
    explicit LocalPortAdded(const std::shared_ptr<NetworkPort>& port): port_(port) { }
    std::shared_ptr<NetworkPort> port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class LocalPortDeleted
{
public:
    explicit LocalPortDeleted(const std::shared_ptr<NetworkPort>& port): port_(port) { }
    std::shared_ptr<NetworkPort> port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class UndecodedPortEvent
{
public:
    explicit UndecodedPortEvent(const std::shared_ptr<NetworkPort>& port): port_(port) { }
    std::shared_ptr<NetworkPort> port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class SetPortUpRequest: public Request
{
public:
    SetPortUpRequest(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Request(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class SetPortDownRequest: public Request
{
public:
    SetPortDownRequest(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Request(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class AddVlanPort: public Request
{
public:
    AddVlanPort(const std::shared_ptr<VlanPort>& port, const ReplyInfo& reply): Request(reply), port_(port) { }

    const std::shared_ptr<VlanPort>& port() const { return port_; }
private:
    std::shared_ptr<VlanPort> port_;
};

class RemovePortRequest: public Request
{
public:
    RemovePortRequest(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply): Request(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class PortUndecodedRequest: public Request
{
public:
    PortUndecodedRequest(const std::shared_ptr<NetworkPort>& port, const ReplyInfo& reply):  Request(reply), port_(port) { }

    const std::shared_ptr<NetworkPort>& port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class LocalPortAdminUp
{
public:
    explicit LocalPortAdminUp(const std::shared_ptr<NetworkPort>& port): port_(port) { }

    std::shared_ptr<NetworkPort> port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class LocalPortAdminDown
{
public:
    explicit LocalPortAdminDown(const std::shared_ptr<NetworkPort>& port): port_(port) { }

    std::shared_ptr<NetworkPort> port() const { return port_; }
private:
    std::shared_ptr<NetworkPort> port_;
};

class PortAdminUp: public DropEvent<PortAdminUp, protocol::DropMessage::PortAdminUp, PortInfo>
{
public:
    explicit PortAdminUp(const PortInfo& port): port_(port) { }

    const PortInfo& port() const { return port_; }
private:
    PortInfo port_;
};

class PortAdminDown: public DropEvent<PortAdminDown, protocol::DropMessage::PortAdminDown, PortInfo>
{
public:
    explicit PortAdminDown(const PortInfo& port): port_(port) { }

    const PortInfo& port() const { return port_; }
private:
    PortInfo port_;
};

} // namespace event
} // namespace drop

#endif
