
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

#include "discover_service_elements.hpp"

#include <atomic>

#include "activity/network/setup_service_element.hpp"

#include "event/quit.hpp"

#include "io/io.hpp"

#include "util/factory.hpp"
#include "util/io_factory.hpp"

#include "application.hpp"
#include "log.hpp"
#include "async.hpp"
#include "demangle.hpp"

namespace drop {
namespace activity {

DiscoverServiceElements::~DiscoverServiceElements()
{
    tnt::Log::debug(colors::green, "DiscoverServiceElements activity terminated.");
}

void DiscoverServiceElements::operator()()
{
    std::atomic_bool running{true};
    auto ep = factory::create_io_end_point("drop");

    tnt::Application::subscribe([&] (const tnt::event::Quit& /*event*/)
    {
        running = false;
        ep->reset();
    });

    while (running)
    {
        try
        {
            auto io = ep->get();

            if (!running)
            {
                break;
            }

            tnt::async([io] ()
            {
                SetupServiceElement(factory::create_drop_protocol(io))();
            });
        }
        catch (std::exception& ex)
        {
            if (running) tnt::Log::error("DiscoverServiceElements exception: ", ex.what());
        }
        catch (...)
        {
            if (running) tnt::Log::error("DiscoverServiceElements unexpected exception.");
        }
    }
}

} // namespace activity
} // namespace drop
