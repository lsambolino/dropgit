
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

#ifndef DROP_PROTOCOL_OPENFLOW_FLOW_FLAGS_HPP_
#define DROP_PROTOCOL_OPENFLOW_FLOW_FLAGS_HPP_

#include <cstdint>

#include "protocol/openflow/flow.hpp"

namespace drop {
namespace protocol {

class Priority: public virtual FlowFlag
{
public:
    explicit Priority(uint16_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint16_t value() const;
private:
    uint16_t value_;
};

class Buffer: public virtual FlowFlag
{
public:
    explicit Buffer(uint32_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint32_t value() const;
private:
    uint32_t value_;
};

class Cookie: public virtual FlowFlag
{
public:
    explicit Cookie(uint64_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint64_t value() const;
private:
    uint64_t value_;
};

class IdleTimeout: public virtual FlowFlag
{
public:
    explicit IdleTimeout(uint16_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint16_t value() const;
private:
    uint16_t value_;
};

class HardTimeout: public virtual FlowFlag
{
public:
    explicit HardTimeout(uint16_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint16_t value() const;
private:
    uint16_t value_;
};

class OutPort: public virtual FlowFlag
{
public:
    explicit OutPort(uint16_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint16_t value() const;
private:
    uint16_t value_;
};

class Flags: public virtual FlowFlag
{
public:
    explicit Flags(uint16_t value);
    virtual bool operator==(const FlowFlag& other) const override;
    virtual std::unique_ptr<FlowFlag> clone() const override;

    uint16_t value() const;
private:
    uint16_t value_;
};

} // namespace protocol
} // namespace drop

#endif
