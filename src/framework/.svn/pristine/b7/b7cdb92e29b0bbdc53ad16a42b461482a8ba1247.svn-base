
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

#ifndef TNT_CONCURRENT_ACTIVITY_HPP_
#define TNT_CONCURRENT_ACTIVITY_HPP_

#include <memory>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <type_traits>
#include <typeinfo>
#include <map>
#include <cassert>

#include "active_object.hpp"
#include "application.hpp"
#include "log.hpp"
#include "function_pointer_traits.hpp"
#include "thread_safe_fifo.hpp"

namespace tnt {

class ConcurrentActivity
{
public:
    ConcurrentActivity() = default;
    ~ConcurrentActivity();
protected:
    void wait_event()
    {
        auto handler = handlers_.pop();
        handler();
    }

    template <class D> bool wait_event_for(D duration)
    {
        std::function<void()> handler;

        if (handlers_.pop_for(handler, duration))
        {
            handler();

            return true;
        }

        return false;
    }

    template <class F> void register_handler(F&& func)
    {
        do_register_handler(is_function(func), std::move(func), nullptr);
    }

    template <class F> void register_handler(void* src, F&& func)
    {
        do_register_handler(is_function(func), std::move(func), src);
    }

    template <class E> void push_event(E&& event)
    {
        tnt::Application::raise(std::forward<E>(event));
    }
private:
    template <class F> void do_register_handler(std::true_type, F&& func, void* src)
    {
        do_register_handler<typename FunctionParamTraits<F>::type>(func, src);
    }

    template <class F> void do_register_handler(std::false_type, F&& func, void* src)
    {
        do_register_handler<typename MemberFunctionParamTraits<F>::type>(func, src);
    }

    template <class E, class F> void do_register_handler(F&& func, void* src)
    {
        tokens_.push_back(tnt::Application::subscribe([=] (E event)
        {
            handlers_.push([=] ()
            {
                func(event);
            });
        },
        src));
    }
private:
    std::vector<tnt::Application::Token> tokens_;
    tnt::ThreadSafeFIFO<std::function<void()>> handlers_;
};

} // namespace tnt

#endif
