
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

#include "interface.hpp"

#include <algorithm>

#include <boost/asio.hpp>

#include "router/port_info.hpp"

#include "containers.hpp"
#include "range.hpp"

namespace drop {
namespace router {

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000 // driver signals L1 up
#endif

Interface::Interface(uint32_t index, const std::string& name, const tnt::MacAddress& hw): index_{ index }, flags_(IFF_MULTICAST | IFF_BROADCAST | IFF_RUNNING | IFF_UP | IFF_LOWER_UP), name_{ name }, hw_address_{ hw } {}

uint32_t Interface::index() const
{
    return index_;
}

uint32_t Interface::flags() const
{
    return flags_;
}

std::string Interface::name() const
{
    return name_;
}

const tnt::MacAddress& Interface::hw_address() const
{
    return hw_address_;
}

bool Interface::running() const
{
    return (flags_ & IFF_RUNNING) == IFF_RUNNING;
}

std::ostream& operator<<(std::ostream& os, const Interface& interface)
{
    os << interface.index() << ": " << interface.name() << ": link/ether " << interface.hw_address();

    return os;
}

} // namespace logical
} // namespace drop
