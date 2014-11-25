
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

#include "sync_protocol.hpp"

#include <cassert>
#include <sstream>
#include <list>

#include "message/message.hpp"

#include "io/io.hpp"

#include "exception/exception.hpp"

#include "event/io_reset.hpp"
#include "event/quit.hpp"

#include "demangle.hpp"
#include "log.hpp"
#include "containers.hpp"
#include "application.hpp"
#include "dynamic_pointer_visitor.hpp"

namespace tnt {
namespace protocol {

/*SyncProtocol::SyncProtocol(const std::shared_ptr<IO>& io) : running_{ false }, io_(io) {}

void SyncProtocol::start()    
{
    register_messages();
    running_ = true;
}

std::future<void> SyncProtocol::send(std::unique_ptr<Message>&& message)
{
    std::promise<void> promise;
    auto future = promise.get_future();

    try
    {
        if (!running_)
        {
            throw ProtocolException(tnt::get_name(*this) + " is not running.");
        }

        if (!tx_dispatcher_.inject_object(std::move(message)))
        {
            throw ProtocolException(std::string("Unable to dispatch unknown message ") + get_name(message));
        }

        promise.set_value();
    }
    catch (...)
    {
        promise.set_exception(std::current_exception());
    }

    return future;
}

void SyncProtocol::write(const std::string& data)
{
    if (!running_)
    {
        Log::error(tnt::get_name(*this), "::write: Error writing a message (", data.size(), " bytes) while not running");

        return;
    }

    io_->write(data);
}

std::unique_ptr<Message> SyncProtocol::receive()
{
    try
    {
        static std::list<std::string> messages;

        std::string raw_input;

        while (running_)
        {
            if (!messages.empty())
            {
                auto message = messages.front();
                messages.pop_front();

                return invoke_message(message);
            }

            try
            {
                raw_input.append(io_->read());

                if (!running_)
                {
                    break;
                }
            }
            catch (IOReset&)
            {
                if (!running_)
                {
                    break;
                }

                Application::raise(tnt::event::ConnectionReset(this), this);

                break;
            }
            catch (...)
            {
                raw_input.clear();
            }

            if (!running_)
            {
                break;
            }

            if (raw_input.empty())
            {
                continue;
            }
            
            tnt::insert_back(messages, parse(raw_input));
        }
    }
    catch (...)
    {
        assert(false);
    }

    return nullptr;
}*/

} // namespace protocol
} // namespace tnt
