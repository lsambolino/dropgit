
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

#include "weekly_scheduler.hpp"

#include <ctime>

namespace tnt { 

void WeeklyScheduler::schedule(DayOfWeek d, hours h, minutes m, seconds s, std::function<void()> func)
{
    schedule(d, h, m, s, std::chrono::system_clock::duration::zero(), func);
}

void WeeklyScheduler::schedule(DayOfWeek d, hours h, minutes m, seconds s, Repeat repeat, std::function<void()> func)
{
    Scheduler::duration r = std::chrono::system_clock::duration::zero();

    switch (repeat)
    {
    case Repeat::Hourly:
        r = std::chrono::hours(1);
        break;
    case Repeat::Daily:
        r = std::chrono::hours(24);
        break;
    case Repeat::Weekly:
        r = std::chrono::hours(168);
        break;
    default:
        break;
    }

    schedule(d, h, m, s, r, func);
}

void WeeklyScheduler::schedule(DayOfWeek d, hours h, minutes m, seconds s, Scheduler::duration repeat, std::function<void()> func)
{
    scheduler_.schedule(next_time_point(std::chrono::system_clock::now(), d, h, m, s, repeat), repeat, func);
}

std::tm WeeklyScheduler::to_tm(const Scheduler::time_point& tp)
{
    auto t = std::chrono::system_clock::to_time_t(tp);

    // Use a lock because the localtime function may not be thread-safe.
    std::unique_lock<std::mutex> lock(guard_);
    
    return *std::localtime(&t);
}

Scheduler::time_point WeeklyScheduler::next_time_point(const Scheduler::time_point& /*now*/, DayOfWeek /*d*/, WeeklyScheduler::hours /*h*/, WeeklyScheduler::minutes /*m*/, WeeklyScheduler::seconds /*s*/, Scheduler::duration /*repeat*/)
{
    return Scheduler::time_point(); // TODO: add implementation
}

} // namespace tnt
