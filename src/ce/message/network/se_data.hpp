
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

#ifndef DROP_MESSAGE_SERVICE_ELEMENT_DATA_HPP_
#define DROP_MESSAGE_SERVICE_ELEMENT_DATA_HPP_

#include <string>
#include <memory>
#include <vector>

#include "message/message_from_data.hpp"

#include "protocol/drop/values.hpp"

#include "util/serialization.hpp"

namespace drop {
namespace message {

struct DropCreateMessage
{
	template <class... Args> void operator()(const std::string& data, std::tuple<Args...>& args)
	{
		from_string(data, args);
	}
};

template <class E, protocol::DropMessage t, class... Args> using RegisterDropMessage = tnt::RegisterMessageFromData<E, protocol::DropMessage, t, DropCreateMessage, Args...>;
template <class E, protocol::DropMessage t, class... Args> struct DropMessage: public virtual tnt::Message, private RegisterDropMessage<E, t, Args...> {};

class SEData: public DropMessage<SEData, protocol::DropMessage::ServiceElementData, std::string, std::string, std::vector<std::string>>
{
public:
    SEData(const std::string& name, const std::string& address, const std::vector<std::string>& services): name_(name), address_(address), services_(services) {}
    SEData(const std::string& name, const std::string& address, std::vector<std::string>&& services): name_(name), address_(address), services_(std::move(services)) {}

    const std::string& name() const { return name_; }
    const std::string& address() const { return address_; }
    const std::vector<std::string>& services() const { return services_; }
private:
    std::string name_;
    std::string address_;
    std::vector<std::string> services_;
};

} // namespace message
} // namespace drop

#endif
