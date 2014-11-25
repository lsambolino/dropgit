
/*

Copyright (c) 2013, Marco Chiappero (marco@absence.it)
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

#ifndef DROP_PROTOCOL_NETLINK_PROTOCOL_HPP_
#define DROP_PROTOCOL_NETLINK_PROTOCOL_HPP_

#include <utility>
#include <memory>
#include <vector>
#include <atomic>

#include "protocol/protocol.hpp"

#include "exception/drop_exception.hpp"

#include "dispatch.hpp"
#include "thread_safe_fifo.hpp"
#include "demangle.hpp"
#include "thread.hpp"

struct nlmsghdr;

namespace drop {

struct PortInfo;
struct RouteInfo;
struct AddressInfo;

namespace protocol {

class NetlinkProtocol: public virtual tnt::Protocol, public std::enable_shared_from_this<tnt::Protocol>
{
    struct MessageNode
    {
        explicit MessageNode(std::unique_ptr<tnt::Message>&& m) : message(std::move(m)) {}
        MessageNode(std::unique_ptr<tnt::Message>&& m, std::promise<void>&& p) : message(std::move(m)), promise(std::move(p)) {}
        std::unique_ptr<tnt::Message> message;
        std::promise<void> promise;
    };
public:
    virtual ~NetlinkProtocol();

    virtual void start() override;
    virtual std::future<void> send(std::unique_ptr<tnt::Message>&& message) override;

    void stop();
protected:
    NetlinkProtocol();
private:
    void tx_loop();
    void rx_loop();

    void parse_data();
protected:
    std::atomic_bool running_;

    tnt::ThreadSafeFIFO<MessageNode> messages_;

    tnt::TypeDispatch<void, std::unique_ptr<tnt::Message>> message_dispatcher_;
    tnt::KeyDispatch<uint32_t, void, const nlmsghdr*> event_dispatcher_;

    int rx_sock_;
    std::vector<char> rx_buffer_;

    static bool netlink_debug;
private:
    tnt::Thread rx_thread_;
    tnt::Thread tx_thread_;
};

} // namespace protocol
} // namespace drop

#endif
