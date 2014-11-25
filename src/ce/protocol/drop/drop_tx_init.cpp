
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

#include "protocol/drop/drop.hpp"

#include "protocol/drop/packets.hpp"
#include "protocol/drop/values.hpp"

#include "message/network/management.hpp"
#include "message/network/interface_list.hpp"
#include "message/network/ack.hpp"
#include "message/network/arp.hpp"

#include "gal/power_state.hpp"
#include "gal/logical_resource.hpp"
#include "gal/optimal_config.hpp"

#include "util/serialization_structs.hpp"

namespace drop {
namespace protocol {

void Drop::register_messages()
{
    // Network Management

    register_message<message::Ack>([this] (auto message)
    {
        write(create_packet(DropMessage::Ack, message->type()));
    });

	register_message<message::InterfaceList>([this] (auto message)
	{
		write(create_packet(DropMessage::InterfaceList, message->ifaces()));
	});

	register_message<message::ArpReply>([this] (auto message)
	{
		write(create_packet(DropMessage::ArpReply, message->ip(), message->mac()));
	});
     
    register_message<message::AddKernelRoute>([this] (auto message)
    {
        write(create_packet(DropMessage::AddKernelRoute, message->route()));
    });
     
    register_message<message::AddUserspaceRoute>([this] (auto message)
    {
        write(create_packet(DropMessage::AddUserspaceRoute, message->route(), message->mac()));
    });

    register_message<message::DelRoute>([this] (auto message)
    {
        write(create_packet(DropMessage::DelRoute, message->route()));
    });
}

} // namespace protocol
} // namespace drop
