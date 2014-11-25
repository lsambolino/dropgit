
/*

Copyright (c) 2013, Luca D'Agostino (lucadago@gmail.com)
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

#ifndef DROP_MESSAGE_OPENFLOW_MESSAGES_HPP_
#define DROP_MESSAGE_OPENFLOW_MESSAGES_HPP_

#include "message/message.hpp"

#include <string>
#include <memory>
#include <vector>

#include "protocol/openflow/flow.hpp"

namespace tnt {

struct Protocol;

} // namespace tnt

namespace drop {
namespace message {
    
class AddFlow: public virtual tnt::Message
{
public:
    explicit AddFlow(const protocol::Flow& flow): flow_(flow) {}
    const protocol::Flow& flow() const { return flow_; }
private:
    protocol::Flow flow_;
};

class RemoveFlow: public virtual tnt::Message
{
public:
    explicit RemoveFlow(const protocol::Flow& flow): flow_(flow) {}
    const protocol::Flow& flow() const { return flow_; }
private:
    protocol::Flow flow_;
};

class ModifyPort: public virtual tnt::Message
{
public:
    explicit ModifyPort(int port_index): port_index_(port_index) {}
    int port_index() const { return port_index_; }
private:
    int port_index_;
};

class DatapathLeave: public virtual tnt::Message
{
public:
    explicit DatapathLeave(const std::shared_ptr<tnt::Protocol>& proto): proto_(proto) {}
    const std::shared_ptr<tnt::Protocol>& proto() const { return proto_; }
private:
    std::shared_ptr<tnt::Protocol> proto_;
};

class PacketIn: public virtual tnt::Message
{
public:
    PacketIn(const protocol::Flow& flow, const std::string& buffer): flow_(flow), buffer_(buffer) {}
    const protocol::Flow& flow() const { return flow_; }
    const std::string& buffer() const { return buffer_; }
private:
    protocol::Flow flow_;
    std::string buffer_;
};

class PacketOut: public virtual tnt::Message
{
public:
    PacketOut(const std::string& buffer, uint16_t port): buffer_(buffer), port_(port) {}
    PacketOut(std::string&& buffer, uint16_t port): buffer_(std::move(buffer)), port_(port) {}

    const std::string& buffer() const { return buffer_; }
    uint16_t port() const { return port_; }
private:
    std::string buffer_;
    uint16_t port_;
};

} // namespace message
} // namespace drop

#endif
