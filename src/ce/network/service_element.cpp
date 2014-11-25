
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

#include "service_element.hpp"

#include "protocol/protocol.hpp"

#include "service/service.hpp"

#include "log.hpp"
#include "range.hpp"
#include "containers.hpp"

namespace drop {
namespace ce {

ServiceElement::ServiceElement(const std::string& name, const std::string& display_name, const std::vector<std::string>& services, bool force_connected, const std::string& address): 
    name_(name), display_name_(display_name), address_(address), force_connected_(force_connected)
{
    set_services(services);
}

const std::string& ServiceElement::name() const
{
    return name_;
}

const std::string& ServiceElement::display_name() const
{
    return display_name_;
}

const std::string& ServiceElement::address() const
{
    return address_;
}

const std::vector<std::shared_ptr<Service>> ServiceElement::services() const
{
    return services_;
}

void ServiceElement::connect(const std::string& address, const std::shared_ptr<tnt::Protocol>& proto, const std::vector<std::string>& services)
{
    if (!address.empty())
    {
        address_ = address;
    }
    
    proto_ = proto;
    set_services(services);
}

void ServiceElement::disconnect()
{
    proto_.reset();
}

bool ServiceElement::connected() const
{
    return force_connected_ || proto_;
}

void ServiceElement::send(std::unique_ptr<tnt::Message>&& message)
{
    if (proto_)
    {
        proto_->send(std::move(message));
    }
    else
    {
        tnt::Log::error("ServiceElement::send error: protocol is not connected."); // TODO: Throw?
    }
}

void ServiceElement::set_services(const std::vector<std::string>& services)
{
    services_.reserve(services.size());

    std::transform(std::begin(services), std::end(services), std::back_inserter(services_), [&] (const auto& id)
    {
        auto s = ServiceFromName::create(id)(this);
        assert(s);

        return s;
    });
}

std::ostream& operator<<(std::ostream& os, const ServiceElement& element)
{
    os << "{\n\t"
       << R"("Name": ")" << element.name() << R"(",)" << "\n\t"
       << R"("Connected": ")" << std::boolalpha << element.connected() << R"(",)" << "\n\t"
       << R"("Services":)" << "\n"
       << "\t[\n";

    for (const auto& s : element.services())
    {
        os << "\t\t" << *s << ",\n";
    }

    os << "\t]\n"
       << "}";

    return os;
}

} // namespace fe
} // namespace drop
