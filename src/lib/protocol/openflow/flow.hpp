
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

#ifndef DROP_PROTOCOL_OPENFLOW_FLOW_HPP_
#define DROP_PROTOCOL_OPENFLOW_FLOW_HPP_

#include <vector>
#include <memory>

namespace drop {
namespace protocol {

struct FlowFlag
{ 
    virtual ~FlowFlag() = default;
    virtual bool operator==(const FlowFlag& other) const = 0;
    virtual std::unique_ptr<FlowFlag> clone() const = 0;
};

struct FlowFilter
{
    virtual ~FlowFilter() = default;
    virtual bool operator==(const FlowFilter& other) const = 0;
    virtual std::unique_ptr<FlowFilter> clone() const = 0;
};

struct FlowAction
{ 
    virtual ~FlowAction() = default;
    virtual bool operator==(const FlowAction& other) const = 0;
    virtual std::unique_ptr<FlowAction> clone() const = 0;
};

class Flow
{
public:
    Flow() = default;
    Flow(const Flow& other);
    Flow(Flow&& other) noexcept = default;
    ~Flow() = default;

    Flow& operator=(const Flow& other);
    Flow& operator=(Flow&& other) = default;

    template <class F> Flow& add(F func)
    {
        return func(*this);
    }

    void swap(Flow& other);

    Flow& add_flag(std::unique_ptr<FlowFlag>&& flag);
    Flow& add_filter(std::unique_ptr<FlowFilter>&& filter);
    Flow& add_action(std::unique_ptr<FlowAction>&& action);

    const std::vector<std::unique_ptr<FlowFlag>>& flags() const { return flags_; }
    const std::vector<std::unique_ptr<FlowFilter>>& filters() const { return filters_; }
    const std::vector<std::unique_ptr<FlowAction>>& actions() const { return actions_; }
private:
    std::vector<std::unique_ptr<FlowFlag>> flags_;
    std::vector<std::unique_ptr<FlowFilter>> filters_;
    std::vector<std::unique_ptr<FlowAction>> actions_;
};

bool operator==(const Flow& f, const Flow& s);

} // namespace protocol
} // namespace drop

#endif
