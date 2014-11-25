
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

#include "message/network/openflow_messages.hpp"
#include "message/network/management.hpp"
#include "message/network/interface_list.hpp"
#include "message/network/ack.hpp"
#include "message/network/arp.hpp"
#include "message/network/port_stats_request.hpp"

#include "util/pugixml.hpp"

namespace drop {
namespace protocol {
namespace {
    
} // namespace

std::string create_packet(const message::AddFlow*  /*message*/)
{
    return "";
}

std::string create_packet(const message::RemoveFlow*  /*message*/)
{
    return "";
}

std::string create_packet(const message::ModifyPort*  /*message*/)
{
    return "";
}

std::string create_packet(const message::PacketOut*  /*message*/)
{
    return "";
}

std::string create_packet(const message::Ack*  /*message*/)
{
    return "";
}

std::string create_packet(const message::InterfaceList*  /*message*/)
{
	return "";
}

std::string create_packet(const message::ArpReply*  /*message*/)
{
	return "";
}
     
std::string create_packet(const message::AddKernelRoute*  /*message*/)
{
    return "";
}
     
std::string create_packet(const message::AddUserspaceRoute*  /*message*/)
{
    return "";
}

std::string create_packet(const message::DelRoute*  /*message*/)
{
    return "";
}

std::string create_packet(const message::PortStatsRequest*  /*message*/)
{
    return "";
}

} // namespace protocol 
} // namespace drop
