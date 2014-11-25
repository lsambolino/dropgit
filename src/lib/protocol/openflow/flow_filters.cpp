
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

#include "flow_filters.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "log.hpp"

namespace drop {
namespace protocol {

FromPort::FromPort(uint16_t port): port_(port) {}

bool FromPort::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromPort>(&other, [&] (auto o)
    {
        res = (port_ == o->port_);
    });

    return res;
}

std::unique_ptr<FlowFilter> FromPort::clone() const
{
    return std::make_unique<FromPort>(port_);
}

uint16_t FromPort::port() const
{
    return port_; 
}

FromVlan::FromVlan(uint16_t tag): tag_(tag) {}

bool FromVlan::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromVlan>(&other, [&] (auto o)
    {
        res = (tag_ == o->tag_);
    });

    return res;
}
 
std::unique_ptr<FlowFilter> FromVlan::clone() const
{
    return std::make_unique<FromVlan>(tag_);
}

uint16_t FromVlan::tag() const
{
    return tag_;
}

FromL2Proto::FromL2Proto(L2Proto proto): proto_(proto) {}

bool FromL2Proto::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromL2Proto>(&other, [&] (auto o)
    {
        res = (proto_ == o->proto_);
    });

    return res;
}

std::unique_ptr<FlowFilter> FromL2Proto::clone() const
{
    return std::make_unique<FromL2Proto>(proto_);
}

L2Proto FromL2Proto::proto() const
{
    return proto_;
}

FromL3Proto::FromL3Proto(L3Proto proto): proto_(proto) {}

bool FromL3Proto::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromL3Proto>(&other, [&] (auto o)
    {
        res = (proto_ == o->proto_);
    });

    return res;
}

std::unique_ptr<FlowFilter> FromL3Proto::clone() const
{
    return std::make_unique<FromL3Proto>(proto_);
}

L3Proto FromL3Proto::proto() const
{
    return proto_;
}

FromHwSrc::FromHwSrc(const tnt::MacAddress& mac): mac_(mac) {}

bool FromHwSrc::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromHwSrc>(&other, [&] (auto o)
    {
        res = (mac_ == o->mac_);
    });

    return res;
}

std::unique_ptr<FlowFilter> FromHwSrc::clone() const
{
    return std::make_unique<FromHwSrc>(mac_);
}

const tnt::MacAddress& FromHwSrc::mac() const
{
    return mac_;
}

FromHwDst::FromHwDst(const tnt::MacAddress& mac): mac_(mac) {}

bool FromHwDst::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromHwDst>(&other, [&] (auto o)
    {
        res = (mac_ == o->mac_);
    });

    return res;
}
 
std::unique_ptr<FlowFilter> FromHwDst::clone() const
{
    return std::make_unique<FromHwDst>(mac_);
}

const tnt::MacAddress& FromHwDst::mac() const
{
    return mac_;
}

FromIpSrc::FromIpSrc(const tnt::ip::Address& ip, int prefix): ip_(ip), prefix_(prefix) {}

bool FromIpSrc::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromIpSrc>(&other, [&] (auto o)
    {
        res = (ip_ == o->ip_ && prefix_ == o->prefix_);
    });

    return res;
}
 
std::unique_ptr<FlowFilter> FromIpSrc::clone() const
{
    return std::make_unique<FromIpSrc>(ip_, prefix_);
}

const tnt::ip::Address& FromIpSrc::ip() const
{
    return ip_;
}

int FromIpSrc::prefix() const
{
    return prefix_;
}

FromIpDst::FromIpDst(const tnt::ip::Address& ip, int prefix) : ip_(ip), prefix_(prefix) {}

bool FromIpDst::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromIpDst>(&other, [&] (auto o)
    {
        res = (ip_ == o->ip_ && prefix_ == o->prefix_);
    });

    return res;
}
 
std::unique_ptr<FlowFilter> FromIpDst::clone() const
{
    return std::make_unique<FromIpDst>(ip_, prefix_);
}

const tnt::ip::Address& FromIpDst::ip() const
{
    return ip_;
}

int FromIpDst::prefix() const
{
    return prefix_;
}

FromTransportPortSrc::FromTransportPortSrc(L3Proto proto, uint16_t port): proto_(proto), port_(port) {}

bool FromTransportPortSrc::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromTransportPortSrc>(&other, [&] (auto o)
    {
        res = (proto_ == o->proto_ && port_ == o->port_);
    });

    return res;
}
 
std::unique_ptr<FlowFilter> FromTransportPortSrc::clone() const
{
    return std::make_unique<FromTransportPortSrc>(proto_, port_);
}

L3Proto FromTransportPortSrc::proto() const
{
    return proto_;
}

uint16_t FromTransportPortSrc::port() const
{
    return port_;
}

FromTransportPortDst::FromTransportPortDst(L3Proto proto, uint16_t port): proto_(proto), port_(port) {}

bool FromTransportPortDst::operator==(const FlowFilter& other) const
{
    bool res = false;

    tnt::visit<const FromTransportPortDst>(&other, [&] (auto o)
    {
        res = (proto_ == o->proto_ && port_ == o->port_);
    });

    return res;
}
 
std::unique_ptr<FlowFilter> FromTransportPortDst::clone() const
{
    return std::make_unique<FromTransportPortDst>(proto_, port_);
}

L3Proto FromTransportPortDst::proto() const
{
    return proto_;
}

uint16_t FromTransportPortDst::port() const
{
    return port_;
}

} // namespace protocol
} // namespace drop
