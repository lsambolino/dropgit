
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

#include "controller.hpp"

#include <mutex>
#include <cassert>

#include "activity/http/web_controller.hpp"

namespace tnt {
namespace activity {
namespace {

std::once_flag init_map_flag_;

} // namespace

Controller::~Controller() {}

std::unique_ptr<ControllerFromName::FactoryMethodMap> ControllerFromName::factories_;

ControllerFactoryMethod ControllerFromName::create(const std::string& name)
{
    auto default_method = [] (HttpConnection* connection, const std::string& base_path) -> std::unique_ptr<Controller> { return std::make_unique<WebController>(connection, base_path); };

	if (!factories_)
	{
        return default_method;
	}

    auto it = factories_->find(name);

    return it != factories_->end() ? it->second : default_method;
}

void ControllerFromName::Register(const std::string& name, ControllerFactoryMethod m)
{
    std::call_once(init_map_flag_, init_map);
    assert(factories_);

    factories_->emplace(name, m);
}

void ControllerFromName::init_map()
{
    factories_ = std::make_unique<FactoryMethodMap>();
}

} // namespace activity
} // namespace tnt
