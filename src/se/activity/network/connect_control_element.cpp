
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

#include "connect_control_element.hpp"

#include <exception>
#include <memory>
#include <atomic>

#include "io/io.hpp"

#include "event/quit.hpp"

#include "util/io_factory.hpp"
#include "util/factory.hpp"
#include "util/configuration.hpp"

#include "activity/network/setup_control_element.hpp"

#include "application.hpp"
#include "async.hpp"
#include "log.hpp"

namespace drop {
namespace activity {

void ConnectControlElement::operator()()
{
    try
    {
        auto interval = tnt::Configuration::get("connection.interval").as<int>();
        auto attempts = tnt::Configuration::get("connection.attempts").as<int>();
        auto n = 0;

        auto ep = factory::create_io_end_point("drop");
        assert(ep);

        std::atomic_bool running{true};

        register_handler([&] (const tnt::event::Quit& /*event*/)
        {
            running = false;
            ep->reset();
        });

        while (attempts == 0 || n < attempts)
        {
            if (!running)
            {
                return;
            }

            tnt::Log::info("Trying to connect CE.");

            try
            {
                auto io = ep->get();

                tnt::async([io] ()
                {
                    SetupControlElement(factory::create_drop_protocol(io))();
                });

                break;
            }
            catch (tnt::IOConnectionError&) {}

            tnt::Log::info("Connection attempt #", n, " failed.");
            ++n;
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error("ConnectControlElement::run exception: ", ex.what());
    }
    catch (...)
    {
        tnt::Log::error("ConnectControlElement::run unexpected exception.");
    }
}

} // namespace activity
} // namespace drop
