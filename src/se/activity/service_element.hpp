
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

#ifndef DROP_ACTIVITY_SERVICE_ELEMENT_HPP_
#define DROP_ACTIVITY_SERVICE_ELEMENT_HPP_

#include <memory>
#include <vector>
#include <utility>
#include <cstdint>
#include <unordered_map>
#include <string>

#include "activity/concurrent_activity.hpp"

#include "dispatch.hpp"

namespace pugi {

class xml_node;

} // namespace pugi

namespace tnt {

struct Protocol;

} // namespace tnt

namespace drop {
namespace router {

class Route;
class Interface;

} // namespace router

namespace de {

class ControlElement;

} // namespace de

namespace activity {

class ServiceElement: private tnt::ConcurrentActivity
{
public:
    ServiceElement();
    ~ServiceElement();

    void operator()();
private:
    void configure();
    void load_router(const pugi::xml_node& root);
private:
    std::vector<unsigned int> interfaces_;
    std::vector<std::shared_ptr<router::Route>> routes_;
    std::shared_ptr<tnt::Protocol> kernel_;
    std::shared_ptr<de::ControlElement> ce_;
};

} // namespace activity
} // namespace drop

#endif
