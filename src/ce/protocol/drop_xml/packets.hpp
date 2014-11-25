
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

#ifndef DROP_PROTOCOL_DROP_XML_PACKETS_HPP_
#define DROP_PROTOCOL_DROP_XML_PACKETS_HPP_

#include <string>

namespace drop {
namespace message {
    
class AddFlow;
class RemoveFlow;
class ModifyPort;
class PacketOut;
class Ack;
class InterfaceList;
class ArpReply;
class AddKernelRoute;
class AddUserspaceRoute;
class DelRoute;
class PortStatsRequest;

} // namespace message

namespace protocol {

std::string create_packet(const message::AddFlow* message);
std::string create_packet(const message::RemoveFlow* message);
std::string create_packet(const message::ModifyPort* message);
std::string create_packet(const message::PacketOut* message);
std::string create_packet(const message::Ack* message);
std::string create_packet(const message::InterfaceList* message);
std::string create_packet(const message::ArpReply* message);
std::string create_packet(const message::AddKernelRoute* message);
std::string create_packet(const message::AddUserspaceRoute* message);
std::string create_packet(const message::DelRoute* message);
std::string create_packet(const message::PortStatsRequest* message);

} // namespace protocol 
} // namespace drop

#endif
