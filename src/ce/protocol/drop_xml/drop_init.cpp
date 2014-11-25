
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

#include "message/network/openflow_messages.hpp"
#include "message/network/management.hpp"
#include "message/network/interface_list.hpp"
#include "message/network/ack.hpp"
#include "message/network/arp.hpp"
#include "message/network/port_stats_request.hpp"

#include "gal/power_state.hpp"
#include "gal/logical_resource.hpp"
#include "gal/optimal_config.hpp"

#include "util/serialization_structs.hpp"
#include "util/pugixml.hpp"

namespace drop {
namespace protocol {

void DropXml::register_messages()
{
    register_out_message<message::AddFlow>();
    register_out_message<message::RemoveFlow>();
    register_out_message<message::ModifyPort>();
    register_out_message<message::PacketOut>();

    // Network Management
    register_out_message<message::Ack>();
	register_out_message<message::InterfaceList>();
	register_out_message<message::ArpReply>();
    register_out_message<message::AddKernelRoute>();
    register_out_message<message::AddUserspaceRoute>();
    register_out_message<message::DelRoute>();
    register_out_message<message::PortStatsRequest>();
}

void DropXml::register_in_messages()
{
    register_in_message("", [this] (pugi::xml_node /*node*/)
    {
        return nullptr;
    });
}

} // namespace protocol
} // namespace drop
