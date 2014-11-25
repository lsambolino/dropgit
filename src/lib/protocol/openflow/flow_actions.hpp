
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

#ifndef DROP_PROTOCOL_OPENFLOW_FLOW_ACTIONS_HPP_
#define DROP_PROTOCOL_OPENFLOW_FLOW_ACTIONS_HPP_

#include <cstdint>

#include "protocol/openflow/flow.hpp"

#include "ip_address.hpp"
#include "mac_address.hpp"

namespace drop {
namespace protocol {

class ToController: public virtual FlowAction
{
public:
    explicit ToController(uint16_t size);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    uint16_t size() const;
private:
    uint16_t size_;
};

struct Loop: public virtual FlowAction
{
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;
};

struct Flood: public virtual FlowAction
{
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;
};

class ToPorts: public virtual FlowAction
{
public:
    explicit ToPorts(const std::vector<uint16_t>& ports);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    const std::vector<uint16_t>& ports() const;
private:
    std::vector<uint16_t> ports_;
};

class SetHwSrc: public virtual FlowAction
{
public:
    explicit SetHwSrc(const tnt::MacAddress& mac);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    const tnt::MacAddress& mac() const;
private:
    tnt::MacAddress mac_;
};

class SetHwDst: public virtual FlowAction
{
public:
    explicit SetHwDst(const tnt::MacAddress& mac);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    const tnt::MacAddress& mac() const;
private:
    tnt::MacAddress mac_;
};

class SetIpSrc: public virtual FlowAction
{
public:
    explicit SetIpSrc(const tnt::ip::Address& ip);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    const tnt::ip::Address& ip() const;
private:
    tnt::ip::Address ip_;
};

class SetIpDst: public virtual FlowAction
{
public:
    explicit SetIpDst(const tnt::ip::Address& ip);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    const tnt::ip::Address& ip() const;
private:
    tnt::ip::Address ip_;
};

class SetVlan: public virtual FlowAction
{
public:
    explicit SetVlan(uint16_t tag);
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;

    uint16_t tag() const;
private:
    uint16_t tag_;
};

class StripVlan: public virtual FlowAction
{
public:
    virtual bool operator==(const FlowAction& other) const override;
    virtual std::unique_ptr<FlowAction> clone() const override;
};

} // namespace protocol
} // namespace drop

#endif
