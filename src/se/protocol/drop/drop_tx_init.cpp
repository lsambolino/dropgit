
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

// Messages
#include "message/network/service_element_data.hpp"
#include "message/network/management.hpp"
#include "message/network/arp.hpp"

#include "log.hpp"

#include "dump.hpp"

namespace drop {
namespace protocol {

void Drop::register_messages()
{
    register_message<message::ServiceElementData>([this] (auto message)
    {
        write(create_packet(DropMessage::ServiceElementData, message->name(), message->address(), message->services()));
    });

    // Network Management

    register_message<message::PortAdminUp>([this] (auto message)
    {
        write(create_packet(DropMessage::PortAdminUp, message->port()));
    });

    register_message<message::PortAdminDown>([this] (auto message)
    {
        write(create_packet(DropMessage::PortAdminDown, message->port()));
    });

    register_message<message::AddRouteSuccess>([this] (auto message)
    {
        write(create_packet(DropMessage::AddRouteSuccess, message->route()));
    });

    register_message<message::DelRouteSuccess>([this] (auto message)
    {
        write(create_packet(DropMessage::DelRouteSuccess, message->route()));
    });

    register_message<message::AddRouteFailure>([this] (auto message)
    {
        write(create_packet(DropMessage::AddRouteFailure, message->error(), message->route()));
    });

    register_message<message::DelRouteFailure>([this] (auto message)
    {
        write(create_packet(DropMessage::DelRouteFailure, message->error(), message->route()));
    });

    register_message<message::ArpRequest>([this] (auto message)
    {
        write(create_packet(DropMessage::ArpRequest, message->ip()));
    });
}

} // namespace protocol
} // namespace drop
