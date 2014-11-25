
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

#include "flow_actions.hpp"

#include "dynamic_pointer_visitor.hpp"

namespace drop {
namespace protocol {

ToController::ToController(uint16_t size): size_(size) {}

bool ToController::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const ToController>(&other, [&] (auto o)
    {
        res = (size_ == o->size_);
    });

    return res;
}

std::unique_ptr<FlowAction> ToController::clone() const
{
    return std::make_unique<ToController>(size_);
}

uint16_t ToController::size() const
{
    return size_;
}

bool Loop::operator==(const FlowAction& other) const
{
    return tnt::is<const Loop>(&other);
}

std::unique_ptr<FlowAction> Loop::clone() const
{
    return std::make_unique<Loop>();
}

bool Flood::operator==(const FlowAction& other) const
{
    return tnt::is<const Flood>(&other);
}

std::unique_ptr<FlowAction> Flood::clone() const
{
    return std::make_unique<Flood>();
}

ToPorts::ToPorts(const std::vector<uint16_t>& ports): ports_(ports) {}

bool ToPorts::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const ToPorts>(&other, [&] (auto o)
    {
        res = (ports_ == o->ports_);
    });

    return res;
}

std::unique_ptr<FlowAction> ToPorts::clone() const
{
    return std::make_unique<ToPorts>(ports_);
}

const std::vector<uint16_t>& ToPorts::ports() const
{
    return ports_;
}

SetHwSrc::SetHwSrc(const tnt::MacAddress& mac): mac_(mac) {}

bool SetHwSrc::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const SetHwSrc>(&other, [&] (auto o)
    {
        res = (mac_ == o->mac_);
    });

    return res;
}

std::unique_ptr<FlowAction> SetHwSrc::clone() const
{
    return std::make_unique<SetHwSrc>(mac_);
}

const tnt::MacAddress& SetHwSrc::mac() const
{
    return mac_;
}

SetHwDst::SetHwDst(const tnt::MacAddress& mac): mac_(mac) {}

bool SetHwDst::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const SetHwDst>(&other, [&] (auto o)
    {
        res = (mac_ == o->mac_);
    });

    return res;
}

std::unique_ptr<FlowAction> SetHwDst::clone() const
{
    return std::make_unique<SetHwDst>(mac_);
}

const tnt::MacAddress& SetHwDst::mac() const
{
    return mac_;
}

SetIpSrc::SetIpSrc(const tnt::ip::Address& ip): ip_(ip) {}

bool SetIpSrc::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const SetIpSrc>(&other, [&] (auto o)
    {
        res = (ip_ == o->ip_);
    });

    return res;
}

std::unique_ptr<FlowAction> SetIpSrc::clone() const
{
    return std::make_unique<SetIpSrc>(ip_);
}

const tnt::ip::Address& SetIpSrc::ip() const
{
    return ip_;
}

SetIpDst::SetIpDst(const tnt::ip::Address& ip): ip_(ip) {}

bool SetIpDst::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const SetIpDst>(&other, [&] (auto o)
    {
        res = (ip_ == o->ip_);
    });

    return res;
}

std::unique_ptr<FlowAction> SetIpDst::clone() const
{
    return std::make_unique<SetIpDst>(ip_);
}

const tnt::ip::Address& SetIpDst::ip() const
{
    return ip_;
}

SetVlan::SetVlan(uint16_t tag): tag_(tag) {}

bool SetVlan::operator==(const FlowAction& other) const
{
    bool res = false;

    tnt::visit<const SetVlan>(&other, [&] (auto o)
    {
        res = (tag_ == o->tag_);
    });

    return res;
}

std::unique_ptr<FlowAction> SetVlan::clone() const
{
    return std::make_unique<SetVlan>(tag_);
}

uint16_t SetVlan::tag() const
{
    return tag_;
}

bool StripVlan::operator==(const FlowAction& other) const
{
    return tnt::is<const StripVlan>(&other);
}

std::unique_ptr<FlowAction> StripVlan::clone() const
{
    return std::make_unique<StripVlan>();
}

} // namespace protocol
} // namespace drop
