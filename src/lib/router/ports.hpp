
/*

Copyright (c) 2013, Marco Chiappero (marco@absence.it)
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

#ifndef DROP_ROUTER_PORTS_HPP_
#define DROP_ROUTER_PORTS_HPP_

#include <cstdint>

#include "router/port_info.hpp"

namespace drop {

class NetworkPort
{
public:
    virtual ~NetworkPort() = 0;

    const PortInfo& info() const { return pi_; }
    uint16_t master() const { return master_; }
protected:
	NetworkPort(const PortInfo& info, uint16_t master) : pi_(info), master_{ master } {}
private:
    PortInfo pi_;
    uint16_t master_;
};

inline NetworkPort::~NetworkPort() {}

class PhysicalPort: public NetworkPort
{
public:
    explicit PhysicalPort(const PortInfo& info): NetworkPort(info, 0) { }
    PhysicalPort(const PortInfo& info, const uint16_t master): NetworkPort(info, master) { }
};

class VlanPort: public NetworkPort
{
public:
    VlanPort(const PortInfo& info, const uint16_t id, const uint16_t link): NetworkPort(info, 0), tag_{ id }, link_{ link } { }
    VlanPort(const PortInfo& info, const uint16_t master, const uint16_t id, const uint16_t link): NetworkPort(info, master), tag_{ id }, link_{ link } { }

    uint16_t tag() const { return tag_; }
    uint16_t link() const { return link_; }
private:
    uint16_t tag_;
    uint16_t link_;
};

class BridgePort: public NetworkPort
{
public:
    explicit BridgePort(const PortInfo& info): NetworkPort(info, 0) { }
    BridgePort(const PortInfo& info, const uint16_t master): NetworkPort(info, master) { }
};

struct BondPort: public NetworkPort
{
public:
    explicit BondPort(const PortInfo& info): NetworkPort(info, 0) { }
    BondPort(const PortInfo& info, const uint16_t master): NetworkPort(info, master) { }
};

class DethPort: public NetworkPort
{
public:
    explicit DethPort(const PortInfo& info): NetworkPort(info, 0) { }
    DethPort(const PortInfo& info, const uint16_t master): NetworkPort(info, master) { }
};

class VethPort: public NetworkPort
{
public:
    explicit VethPort(const PortInfo& info): NetworkPort(info, 0) { }
    VethPort(const PortInfo& info, const uint16_t master): NetworkPort(info, master) { }
};

} // namespace drop

#endif
