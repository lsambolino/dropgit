
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

#include "packets.hpp"

#include <string>
#include <sstream>

#include "message/network/service_element_data.hpp"
#include "message/network/arp.hpp"

#include "util/pugixml.hpp"

using namespace std::literals;

namespace drop {
namespace protocol {
namespace {
    
} // namespace

std::string create_packet(const message::ServiceElementData* message)
{
    pugi::xml_document doc; 
    auto root = doc.append_child("drop");
    auto node = root.append_child("service_element_data");

    node.append_attribute("name").set_value(message->name().c_str());
    node.append_attribute("address").set_value(message->address().c_str());
    
    auto services = node.append_child("services");

    for (const auto& s : message->services())
    {
        services.append_child("service").append_attribute("name").set_value(s.c_str());
    }

    std::ostringstream os;
    doc.save(os);

    return os.str();
}

std::string create_packet(const message::PortAdminUp* /*message*/)
{
    return "";
}

std::string create_packet(const message::PortAdminDown* /*message*/)
{
    return "";
}

std::string create_packet(const message::AddRouteSuccess* /*message*/)
{
    return "";
}

std::string create_packet(const message::DelRouteSuccess* /*message*/)
{
    return "";
}

std::string create_packet(const message::AddRouteFailure* /*message*/)
{
    return "";
}

std::string create_packet(const message::DelRouteFailure* /*message*/)
{
    return "";
}

std::string create_packet(const message::ArpRequest* /*message*/)
{
    return "";
}

} // namespace protocol 
} // namespace drop
