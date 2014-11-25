
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

#include "application.hpp"

#include <signal.h>

#include "activity/activity_from_event.hpp"

#include "event/quit.hpp"

#include "util/system.hpp"

#include "range.hpp"
#include "demangle.hpp"
#include "containers.hpp"
#include "async.hpp"

namespace tnt {
namespace {

std::once_flag init_instance_flag_;

}

std::unique_ptr<Application> Application::impl_;

Application::Application() : running_{ true } {}

Application& Application::instance()
{
    std::call_once(init_instance_flag_, [] ()
    {
        impl_.reset(new Application);
    });

    return *impl_;
}

void Application::run()
{
    instance().do_run();
}

void Application::run_async()
{
    std::thread th([&] ()
    {
        instance().do_run();
    });

    th.detach();
}

void Application::unsubscribe(const Token& token)
{
    instance().do_unsubscribe(token);
}

void Application::stop()
{
    running_ = false;
}

void Application::do_run()
{
    subscribe([&] (const event::Quit& /*event*/)
    {
        stop();
    });

    tnt::SignalHandle::ignore(SIGPIPE);

    tnt::SignalHandle::register_handler([&] ()
    {
        raise(event::Quit());
    });

    while (running_)
    {
        auto e = events_.pop();
        const auto& event = e.first;
        const auto src = e.second;

        auto delivered = false;

        tnt::lock_unique(subscribers_guard_, [&]()
        {
            for (const auto& it : tnt::equal_range(subscribers_, event->type()))
            {
                auto& p = it.second;
                assert(p);

                if (p->push(event->get(), src))
                {
                    delivered = true;
                }
            }
        });

        if (!delivered)
        {
            try
            {
                auto m = tnt::ActivityFromEvent::create(event->type(), event->get());

                tnt::async([m] ()
                {
                    m()->run();
                });
            }
            catch (std::exception& ex)
            {
                tnt::Log::error(ex.what());
            }
        }
    }
}

void Application::do_unsubscribe(const Token& token)
{
    if (running_)
    {
	    lock_unique(subscribers_guard_, [&] ()
        {
            subscribers_.erase(token);
        });
    }
}

} // namespace tnt
