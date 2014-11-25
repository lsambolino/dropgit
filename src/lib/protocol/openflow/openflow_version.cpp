
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

#include "openflow_version.hpp"

#include <mutex>
#include <cassert>

#include "activity/http/web_controller.hpp"

#include "log.hpp"

namespace drop {
namespace protocol {
namespace {

std::once_flag init_map_flag_;

} // namespace

std::unique_ptr<OpenflowVersion::FactoryMethodMap> OpenflowVersion::factories_;

std::unique_ptr<OpenflowProtocol> OpenflowVersion::create(int version, Openflow* ptr)
{
	if (!factories_)
	{
        tnt::Log::error("OpenflowVersion error: null map");

        return nullptr;
	}

    auto it = factories_->find(version);

    if (it == factories_->end())
    {
        tnt::Log::debug(colors::red, "OpenflowVersion error: version (", version, ") not found");
    }

    return it != factories_->end() ? it->second(ptr) : nullptr;
}

void OpenflowVersion::init()
{
    std::call_once(init_map_flag_, init_map);
    assert(factories_);
}

void OpenflowVersion::init_map()
{
    factories_ = std::make_unique<FactoryMethodMap>();
}

} // namespace protocol
} // namespace drop
