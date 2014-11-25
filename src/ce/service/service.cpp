
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

#include "service.hpp"

#include <mutex>
#include <iostream>
#include <cassert>

namespace drop {
namespace {

std::once_flag init_map_flag_;

} // namespace

std::unique_ptr<ServiceFromName::FactoryMethodMap> ServiceFromName::factories_;

ServiceFactoryMethod ServiceFromName::create(const std::string& name)
{
    auto default_method = [] (ce::ServiceElement* /*parent*/) -> std::shared_ptr<Service> { return nullptr; };

	if (!factories_)
	{
        return default_method;
	}

    auto it = factories_->find(name);

    return it != factories_->end() ? it->second : default_method;
}

void ServiceFromName::Register(const std::string& name, ServiceFactoryMethod m)
{
    std::call_once(init_map_flag_, init_map);
    assert(factories_);

    factories_->emplace(name, m);
}

void ServiceFromName::init_map()
{
    factories_ = std::make_unique<FactoryMethodMap>();
}

std::ostream& operator<<(std::ostream& os, const Service& service)
{
    return service.print(os);
}

} // namespace drop
