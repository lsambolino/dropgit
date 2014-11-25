
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

#include "flow.hpp"

#include "containers.hpp"

namespace drop {
namespace protocol {

Flow::Flow(const Flow& other)
{
    tnt::for_all(other.flags_, [&] (const auto& flag)
    {
        flags_.emplace_back(flag->clone());
    });

    tnt::for_all(other.filters_, [&] (const auto& filter)
    {
        filters_.emplace_back(filter->clone());
    });

    tnt::for_all(other.actions_, [&] (const auto& action)
    {
        actions_.emplace_back(action->clone());
    });
}

Flow& Flow::operator=(const Flow& other)
{
    Flow tmp(other);
    swap(tmp);

    return *this;
}

Flow& Flow::add_flag(std::unique_ptr<FlowFlag>&& flag)
{
    flags_.push_back(std::move(flag));

    return *this;
}

Flow& Flow::add_filter(std::unique_ptr<FlowFilter>&& filter)
{
    filters_.push_back(std::move(filter));

    return *this;
}

Flow& Flow::add_action(std::unique_ptr<FlowAction>&& action)
{
    actions_.push_back(std::move(action));

    return *this;
}

void Flow::swap(Flow& other)
{
    using std::swap;
    swap(flags_, other.flags_);
    swap(filters_, other.filters_);
    swap(actions_, other.actions_);
}

bool operator==(const Flow& f, const Flow& s)
{
    return tnt::all_of(f.flags(), [&s] (const auto& ff)
    {
        return tnt::any_of(s.flags(), [&ff] (const auto& sf)
        {
            return *ff == *sf;
        });
    })
    && tnt::all_of(f.filters(), [&s] (const auto& ff)
    {
        return tnt::any_of(s.filters(), [&ff] (const auto& sf)
        {
            return *ff == *sf;
        });
    })
    && tnt::all_of(f.actions(), [&s] (const auto& fa)
    {
        return tnt::any_of(s.actions(), [&fa] (const auto& sa)
        {
            return *fa == *sa;
        });
    });
}

} // namespace protocol
} // namespace drop
