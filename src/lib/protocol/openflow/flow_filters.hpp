
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

#ifndef DROP_PROTOCOL_OPENFLOW_FLOW_FILTERS_HPP_
#define DROP_PROTOCOL_OPENFLOW_FLOW_FILTERS_HPP_

#include <cstdint>

#include "protocol/values.hpp"
#include "protocol/openflow/flow.hpp"

#include "mac_address.hpp"
#include "ip_address.hpp"

namespace drop {
namespace protocol {

class FromPort: public virtual FlowFilter
{
public:
    explicit FromPort(uint16_t port);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    uint16_t port() const;
private:
    uint16_t port_;
};

class FromVlan: public virtual FlowFilter
{
public:
    explicit FromVlan(uint16_t tag);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    uint16_t tag() const;
private:
    uint16_t tag_;
};

class FromL2Proto: public virtual FlowFilter
{
public:
    explicit FromL2Proto(L2Proto proto);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    L2Proto proto() const;
private:
    L2Proto proto_;
};

class FromL3Proto: public virtual FlowFilter
{
public:
    explicit FromL3Proto(L3Proto proto);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    L3Proto proto() const;
private:
    L3Proto proto_;
};

class FromHwSrc: public virtual FlowFilter
{
public:
    explicit FromHwSrc(const tnt::MacAddress& mac);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    const tnt::MacAddress& mac() const;
private:
    tnt::MacAddress mac_;
};

class FromHwDst: public virtual FlowFilter
{
public:
    explicit FromHwDst(const tnt::MacAddress& mac);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    const tnt::MacAddress& mac() const;
private:
    tnt::MacAddress mac_;
};

class FromIpSrc: public virtual FlowFilter
{
public:
    explicit FromIpSrc(const tnt::ip::Address& ip, int prefix = 32);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    const tnt::ip::Address& ip() const;
    int prefix() const;
private:
    tnt::ip::Address ip_;
    int prefix_;
};

class FromIpDst: public virtual FlowFilter
{
public:
    explicit FromIpDst(const tnt::ip::Address& ip, int prefix = 32);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    const tnt::ip::Address& ip() const;
    int prefix() const;
private:
    tnt::ip::Address ip_;
    int prefix_;
};

class FromTransportPortSrc: public virtual FlowFilter
{
public:
    FromTransportPortSrc(L3Proto proto, uint16_t port);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    L3Proto proto() const;
    uint16_t port() const;
private:
    L3Proto proto_;
    uint16_t port_;
};

class FromTransportPortDst: public virtual FlowFilter
{
public:
    FromTransportPortDst(L3Proto proto, uint16_t port);
    virtual bool operator==(const FlowFilter& other) const override;
    virtual std::unique_ptr<FlowFilter> clone() const override;

    L3Proto proto() const;
    uint16_t port() const;
private:
    L3Proto proto_;
    uint16_t port_;
};

} // namespace protocol
} // namespace drop

#endif
