
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

#ifndef TNT_THREAD_HPP_
#define TNT_THREAD_HPP_

#include <thread>
#include <cassert>

namespace tnt {

class Thread
{
public:
    Thread() = default;

    template <class... Args> Thread(Args&&... args) : th_(std::forward<Args>(args)...) {}
    
    Thread(const Thread&) = delete;

    Thread(Thread&& other): th_(std::move(other.th_)) {}

    ~Thread()
    {
        try
        {
            if (th_.joinable())
            {
                th_.join();
            }
        }
        catch (...) {}
    }

    Thread& operator=(const Thread&) = delete;

    Thread& operator=(Thread&& other)
    {
        assert(!th_.joinable());
        th_ = std::move(other.th_);

        return *this;
    }

    template <class... Args> void start(Args&&... args)
    {
        th_ = std::thread(std::forward<Args>(args)...);
    }

    bool joinable()
    {
        return th_.joinable();
    }

    void join()
    {
        th_.join();
    }
 
    void detach()
    {
        th_.detach();
    }

    auto get_id() const
    {
        return th_.get_id();
    }

    auto native_handle()
    {
        return th_.native_handle();
    }
private:
    std::thread th_;
};

} // namespace tnt

#endif
