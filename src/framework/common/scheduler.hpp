
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

#ifndef TNT_SCHEDULER_HPP_
#define TNT_SCHEDULER_HPP_

#include <queue>
#include <thread>
#include <functional>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

namespace tnt {

enum class Start
{
    Automatic,
    Manual
};

class Scheduler
{
public:
    using time_point = std::chrono::system_clock::time_point;
    using duration = std::chrono::system_clock::duration;
private:
    class Event
    {
    public:
        Event(std::function<void()> what, time_point when, duration repeat);

        time_point when() const;
        bool repeat() const;
        void operator()() const;
        Event next() const;

        bool operator<(const Event& other) const;
        bool operator==(const Event& other) const;
    private:
        std::function<void()> what_;
        time_point when_;
        duration repeat_;
    };
public:
    explicit Scheduler(Start mode = Start::Automatic);
    Scheduler(const Scheduler&) = delete;
    ~Scheduler();

    Scheduler& operator=(const Scheduler&) = delete;

    void start();
    void stop();

    void schedule(time_point when, std::function<void()> what);
    void schedule(time_point when, duration repeat, std::function<void()> what);
private:
    void run();
private:
    std::atomic_bool running_;
    std::thread run_thread_;

    std::priority_queue<Event> events_;
    std::mutex guard_;
    std::condition_variable cv_;
};

} // namespace tnt

#endif
