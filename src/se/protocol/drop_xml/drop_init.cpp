
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

#include "protocol/drop_xml/drop_xml.hpp"

// Messages
#include "message/network/service_element_data.hpp"
#include "message/network/openflow_messages.hpp"
#include "message/network/management.hpp"
#include "message/network/arp.hpp"

#include "log.hpp"
#include "dump.hpp"

namespace drop {
namespace protocol {

void DropXml::register_messages()
{
    register_out_message<message::ServiceElementData>();
    
    // Network Management

    register_out_message<message::PortAdminUp>();
    register_out_message<message::PortAdminDown>();
    register_out_message<message::AddRouteSuccess>();
    register_out_message<message::DelRouteSuccess>();
    register_out_message<message::AddRouteFailure>();
    register_out_message<message::DelRouteFailure>();
    register_out_message<message::ArpRequest>();
}

} // namespace protocol
} // namespace drop
