
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

#ifndef DROP_EVENT_SERVICE_ELEMENT_DATA_HPP_
#define DROP_EVENT_SERVICE_ELEMENT_DATA_HPP_

#include <string>
#include <vector>
#include <cassert>

#include "event/drop/drop_event.hpp"

namespace drop{
namespace event {

class ServiceElementData: public DropEvent<ServiceElementData, protocol::DropMessage::ServiceElementData, std::string, std::string, std::vector<std::string>>
{
public:
    ServiceElementData(const std::string& name, const std::string& address, const std::vector<std::string>& services): name_(name), address_(address), services_(services) {}

    const std::string& name() const { return name_; }
    const std::string& address() const { return address_; }
    const std::vector<std::string>& services() const { return services_; }
private:
    std::string name_;
    std::string address_;
    std::vector<std::string> services_;
};

} // namespace event
} // namespace drop

#endif
