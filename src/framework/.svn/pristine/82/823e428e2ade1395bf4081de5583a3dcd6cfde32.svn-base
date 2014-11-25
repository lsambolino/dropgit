
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

#include "async_protocol.hpp"

#include <thread>
#include <cassert>
#include <sstream>

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

AsyncProtocol::AsyncProtocol(const std::shared_ptr<IO>& io): running_{ false }, io_(io)
{
    assert(io_);
}

AsyncProtocol::~AsyncProtocol()
{
    stop();
}

void AsyncProtocol::start()    
{
    if (running_)
    {
        return;
    }

    running_ = true;

    Application::subscribe([&] (const event::Quit& /*event*/)
    {
        stop();
    });

    register_messages();

    rx_thread_.start([this] () { rx_loop(); });
    tx_thread_.start([this] () { tx_loop(); });
}

void AsyncProtocol::stop()
{
    try
    {
        running_ = false;

        if (messages_.empty())
        {
            messages_.push(MessageNode(std::make_unique<message::StopMessage>()));
        }

        if (io_)
        {
            io_->reset();
        }
    }
    catch (...) {}
}

std::future<void> AsyncProtocol::send(std::unique_ptr<Message>&& message)
{
    std::promise<void> promise;
    auto future = promise.get_future();

    if (running_)
    {
        messages_.push(MessageNode(std::move(message), std::move(promise)));
    }
    else
    {
        promise.set_exception(std::make_exception_ptr(ProtocolException(tnt::get_name(*this) + " is not running.")));
    }

    return future;
}

void AsyncProtocol::tx_loop()
{
    while (running_)
    {
        auto node = messages_.pop();
        auto& promise = node.promise;

        try
        {
            auto& message = node.message;
            assert(message);

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
    }
}

void AsyncProtocol::write(const std::string& data)
{
    if (!running_)
    {
        Log::error(tnt::get_name(*this), "::write: Error writing a message (", data.size(), " bytes) while not running");

        return;
    }

    io_->write(data);
}

void AsyncProtocol::rx_loop()
{
    try
    {
        std::string raw_input;

        while (running_)
        {
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

            const auto& messages = parse(raw_input);

            for_all(messages, [this] (const auto& message)
            {
                assert(!message.empty());

                try
                {
                    invoke_message(message);  
                }
                catch (std::exception& ex)
                {
                    tnt::Log::error(ex.what());
                }
            });
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error(ex.what());
        assert(false);
    }
    catch (...)
    {
        assert(false);
    }
}

} // namespace protocol
} // namespace tnt
