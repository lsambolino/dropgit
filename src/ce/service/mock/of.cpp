
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

#include "of.hpp"

#include <iostream>

#include "network/service_element.hpp"

#include "containers.hpp"
#include "memory.hpp"
#include "log.hpp"

namespace drop {
namespace service {

MockOpenflowManagement::MockOpenflowManagement(ce::ServiceElement* /*parent*/) {}

std::ostream& MockOpenflowManagement::print(std::ostream& os) const
{
    os << R"({ "Name": "MockOpenflowManagement" })";

    return os;
}

void MockOpenflowManagement::add(const protocol::Flow& flow)
{
    if (!tnt::contains(flows_, flow))
    {
        flows_.push_back(flow);
    }
}

void MockOpenflowManagement::remove(const protocol::Flow& flow)
{
    flows_.erase(std::remove(std::begin(flows_), std::end(flows_), flow), std::end(flows_));
}

const std::vector<protocol::Flow>& MockOpenflowManagement::flows() const
{
    return flows_;
}

} // namespace service
} // namespace drop
