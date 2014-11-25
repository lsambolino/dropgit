
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

#ifndef TNT_PROTOCOL_ASYNC_PROTOCOL_HPP_
#define TNT_PROTOCOL_ASYNC_PROTOCOL_HPP_

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

namespace tnt {

struct IO;
struct Message;

namespace protocol {

class AsyncProtocol: public virtual Protocol
{
    struct MessageNode
    {
        explicit MessageNode(std::unique_ptr<Message>&& m) : message(std::move(m)) {}
        MessageNode(std::unique_ptr<Message>&& m, std::promise<void>&& p) : message(std::move(m)), promise(std::move(p)) {}
        std::unique_ptr<Message> message;
        std::promise<void> promise;
    };
public:
    explicit AsyncProtocol(const std::shared_ptr<IO>& io);
    AsyncProtocol(const AsyncProtocol&) = delete;
    virtual ~AsyncProtocol();
    AsyncProtocol& operator=(const AsyncProtocol&) = delete;

    virtual void start() override;
    virtual std::future<void> send(std::unique_ptr<Message>&& message) override;
protected:
    virtual std::vector<std::string> parse(std::string& raw_input) = 0; //! raw_input is passed by non const reference, so parse function can modify it.
    virtual void invoke_message(const std::string& data) = 0;
    virtual void register_messages() = 0;

    template<class U, class F> void register_message(F func)
    {
        tx_dispatcher_.register_listener<U, F>(func);
    }

    template <class T> void invoke(T type, const std::string& data)
    {
        try
        {
            EventFromMessage::raise(type, data, this);
        }
        catch (std::bad_weak_ptr&) {}
        catch (std::exception& ex)
        {
            Log::error(tnt::get_name(*this), "::rx error: ", ex.what());
        }
        catch (...)
        {
            Log::error(tnt::get_name(*this), "::rx unknown error.");
        }
    }

    void write(const std::string& data);
private:
    void stop();

    // Rx members
    void rx_loop();
    
    // Tx members
    void tx_loop();
private:
    // Shared members
    std::atomic_bool running_;
    std::shared_ptr<IO> io_;

    // Rx members
    tnt::Thread rx_thread_;

    // Tx members
    ThreadSafeFIFO<MessageNode> messages_;

    tnt::Thread tx_thread_;
    TypeDispatch<void, std::unique_ptr<Message>> tx_dispatcher_;
};

} // namespace protocol
} // namespace tnt

#endif
