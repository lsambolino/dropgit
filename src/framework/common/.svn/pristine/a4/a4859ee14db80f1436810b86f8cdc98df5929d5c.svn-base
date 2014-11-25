
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

#include "scheduler.hpp"

namespace tnt {

Scheduler::Event::Event(std::function<void()> what, time_point when, duration repeat): what_{ what }, when_{ when }, repeat_{ repeat } {}

Scheduler::time_point Scheduler::Event::when() const
{ 
    return when_;
}

bool Scheduler::Event::repeat() const
{ 
    return repeat_ > std::chrono::system_clock::duration::zero();
}

void Scheduler::Event::operator()() const
{ 
    what_();
}

Scheduler::Event Scheduler::Event::next() const
{
    return Event(what_, when_ + repeat_, repeat_);
}

bool Scheduler::Event::operator<(const Event& other) const
{
    return other.when_ < when_; // Previous times has higher priority.
}

bool Scheduler::Event::operator==(const Event& other) const
{
    return when_ == other.when_;
}
    
Scheduler::Scheduler(Start mode): running_{ false }
{
    if (mode == Start::Automatic)
    {
        start();
    }
}

Scheduler::~Scheduler()
{
    try
    {
        stop();
    }
    catch (...) {}
}

void Scheduler::start()
{
    if (running_)
    {
        return;
    }

    running_ = true;
    run_thread_ = std::thread([this] () { run(); });
}

void Scheduler::stop()
{
    if (running_)
    {
        schedule(std::chrono::system_clock::now(), [this] () { running_ = false; });
    }

    if (run_thread_.joinable())
    {
        run_thread_.join();
    }
}

void Scheduler::run()
{
    while (running_)
    {
        std::unique_lock<std::mutex> lock(guard_);
        cv_.wait(lock, [=] { return !events_.empty(); });

        auto event = events_.top();
        auto when = event.when();

        while (true)
        {
            if (cv_.wait_until(lock, when) == std::cv_status::timeout) // Timeout expired
            {
                events_.pop();

                if (event.repeat())
                {
                    events_.push(event.next());
                }

                event();

                break;
            }
            else // New event arrived
            {
                if (events_.top() == event) // Same timeout
                {
                    continue;
                }

                // Update the timeout
                event =  events_.top();
                when = event.when();
            }
        }
    }
}

void Scheduler::schedule(time_point when, std::function<void()> what)
{
    schedule(when, std::chrono::system_clock::duration::zero(), what);
}

void Scheduler::schedule(time_point when, duration repeat, std::function<void()> what)
{
    std::unique_lock<std::mutex> lock(guard_);
    events_.push(Event(what, when, repeat));
    cv_.notify_one();
}

} // namespace tnt
