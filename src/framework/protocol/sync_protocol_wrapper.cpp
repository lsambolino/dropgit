
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

#include "sync_protocol_wrapper.hpp"

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

SyncProtocolWrapper::~SyncProtocolWrapper()
{
    stop();
}

const std::shared_ptr<Protocol>& SyncProtocolWrapper::get() const
{
    return proto_;
}

void SyncProtocolWrapper::start()    
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

    rx_thread_.start([this] () { rx_loop(); });
}

void SyncProtocolWrapper::stop()
{
    try
    {
        running_ = false;
        io_->reset();
    }
    catch (...) {}
}

std::future<void> SyncProtocolWrapper::send(std::unique_ptr<Message>&& message)
{
    std::promise<void> promise;

    if (!running_)
    {
        promise.set_exception(std::make_exception_ptr(ProtocolException(tnt::get_name(*this) + " is not running.")));
    }

    proto_->send(std::move(message));

    promise.set_value();

    return promise.get_future();
}

void SyncProtocolWrapper::rx_loop()
{
    while (running_)
    {
        try
        {
            /*auto message = proto_->receive();
                
            if (!running_)
            {
                break;
            }*/

            // TODO:
        }
        catch (...)
        {
            break;
        }
    }
}

} // namespace protocol
} // namespace tnt
