
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

#include "flow_flags.hpp"

#include "dynamic_pointer_visitor.hpp"

namespace drop {
namespace protocol {
namespace {


} // namespace

Priority::Priority(uint16_t value) : value_(value) {}

bool Priority::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const Priority>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> Priority::clone() const
{
    return std::make_unique<Priority>(value_);
}

uint16_t Priority::value() const
{
    return value_;
}

Buffer::Buffer(uint32_t value) : value_(value) {}

bool Buffer::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const Buffer>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> Buffer::clone() const
{
    return std::make_unique<Buffer>(value_);
}

uint32_t Buffer::value() const
{
    return value_;
}

Cookie::Cookie(uint64_t value) : value_(value) {}

bool Cookie::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const Cookie>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> Cookie::clone() const
{
    return std::make_unique<Cookie>(value_);
}

uint64_t Cookie::value() const
{
    return value_;
}

IdleTimeout::IdleTimeout(uint16_t value) : value_(value) {}

bool IdleTimeout::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const IdleTimeout>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> IdleTimeout::clone() const
{
    return std::make_unique<IdleTimeout>(value_);
}

uint16_t IdleTimeout::value() const
{
    return value_;
}

HardTimeout::HardTimeout(uint16_t value) : value_(value) {}

bool HardTimeout::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const HardTimeout>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> HardTimeout::clone() const
{
    return std::make_unique<HardTimeout>(value_);
}

uint16_t HardTimeout::value() const
{
    return value_;
}

OutPort::OutPort(uint16_t value) : value_(value) {}

bool OutPort::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const OutPort>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> OutPort::clone() const
{
    return std::make_unique<OutPort>(value_);
}

uint16_t OutPort::value() const
{
    return value_;
}

Flags::Flags(uint16_t value) : value_(value) {}

bool Flags::operator==(const FlowFlag& other) const
{
    bool res = false;

    tnt::visit<const Flags>(&other, [&] (auto o)
    {
        res = (value_ == o->value_);
    });

    return res;
}

std::unique_ptr<FlowFlag> Flags::clone() const
{
    return std::make_unique<Flags>(value_);
}

uint16_t Flags::value() const
{
    return value_;
}

} // namespace protocol
} // namespace drop
