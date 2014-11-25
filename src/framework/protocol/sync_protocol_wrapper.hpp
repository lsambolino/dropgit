
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

#ifndef TNT_PROTOCOL_SYNC_PROTOCOL_WRAPPER_HPP_
#define TNT_PROTOCOL_SYNC_PROTOCOL_WRAPPER_HPP_

#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>
#include <string>
#include <atomic>
#include <stdexcept>
#include <cstdint>

#include "protocol/protocol.hpp"

#include "event/event_from_message.hpp"

#include "thread.hpp"
#include "log.hpp"
#include "dispatch.hpp"
#include "thread_safe_fifo.hpp"
#include "unpack_tuple.hpp"
#include "application.hpp"

namespace tnt {

struct IO;
struct Message;

namespace protocol {

class SyncProtocolWrapper: public tnt::Protocol
{
public:
    template <class P> explicit SyncProtocolWrapper(const std::shared_ptr<IO>& io) : running_{ false }, io_(io), proto_(std::make_shared<P>(io)) {}

    SyncProtocolWrapper(const SyncProtocolWrapper&) = delete;
    ~SyncProtocolWrapper();
    SyncProtocolWrapper& operator=(const SyncProtocolWrapper&) = delete;

    const std::shared_ptr<Protocol>& get() const;

    void start();
    std::future<void> send(std::unique_ptr<Message>&& message);
private:
    void stop();
    void rx_loop();
private:
    std::atomic_bool running_;
    std::shared_ptr<IO> io_;
    std::shared_ptr<Protocol> proto_;

    tnt::Thread rx_thread_;
};

} // namespace protocol
} // namespace tnt

#endif
