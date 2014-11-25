
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

#ifndef TNT_APPLICATION_HPP_
#define TNT_APPLICATION_HPP_

#include <memory>
#include <mutex>
#include <map>
#include <set>
#include <typeindex>
#include <functional>
#include <vector>
#include <atomic>
#include <tuple>
#include <condition_variable>
#include <future>
#include <cassert>

#include "event/network/connection_reset.hpp"

#include "lock.hpp"
#include "log.hpp"
#include "demangle.hpp"
#include "thread_safe_fifo.hpp"
#include "function_pointer_traits.hpp"

namespace tnt {

class Application
{
    struct SubscriberObject
    {
        virtual ~SubscriberObject() = default;
        virtual bool push(void* event, void* src) = 0;
    };

    template <class T> class Subscriber: public SubscriberObject
    {
    public:
        Subscriber(std::function<void(T)> func, void* src) : func_(func), src_(src) {}

        virtual bool push(void* event, void* src) override
        {
            auto source_valid = !(src_ && src) || (src_ == src);

            if (source_valid)
            {
                func_(*reinterpret_cast<T*>(event));
            }

            return source_valid;
        }
    private:
        std::function<void(T)> func_;
        void* src_;
    };

    struct EventObject
    {
        virtual ~EventObject() = default;
        virtual std::type_index type() = 0;
        virtual void* get() = 0;
    };

    template <class E> class Event: public EventObject
    {
    public:
        explicit Event(const E& event) : event_(event) {}

        virtual void* get() override
        {
            return &event_;
        }

        virtual std::type_index type() override
        {
            return std::type_index(typeid(event_));
        }
    private:
        E event_;
    };

    using SubscriberTable = std::multimap<std::type_index, std::unique_ptr<SubscriberObject>>;
public:
    using Token = SubscriberTable::iterator;

    template <class F> static auto subscribe(F&& func, void* src = nullptr)
    {
        return instance().do_subscribe(is_function(func), std::move(func), src);
    }

    template <class E> static void raise(E&& event, void* src = nullptr)
    {
        instance().do_raise(event, src);
    }

    static void unsubscribe(const Token& token);
    static void run();
    static void run_async();
private:
    Application();

    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    static Application& instance();

    template <class E> void do_raise(E&& event, void* src)
    {
        if (running_)
        {
            events_.push(std::make_pair(std::make_unique<Event<typename std::decay<E>::type>>(event), src));
        }
    }

    template <class F> auto do_subscribe(std::true_type, F&& func, void* src)
    {
        using E = typename FunctionParamTraits<F>::type;

        return tnt::lock_unique(subscribers_guard_, [&]()
        {
            return subscribers_.emplace(std::type_index(typeid(E)), std::make_unique<Subscriber<typename std::decay<E>::type>>(func, src));
        });
    }

    template <class F> auto do_subscribe(std::false_type, F&& func, void* src)
    {
        using E = typename MemberFunctionParamTraits<F>::type;

        return tnt::lock_unique(subscribers_guard_, [&]()
        {
            return subscribers_.emplace(std::type_index(typeid(E)), std::make_unique<Subscriber<typename std::decay<E>::type>>(func, src));
        });
    }

    void stop();

    void do_run();
    void do_unsubscribe(const Token& token);
private:
    static std::unique_ptr<Application> impl_;

    std::atomic_bool running_;

    std::mutex subscribers_guard_;
    SubscriberTable subscribers_;

    tnt::ThreadSafeFIFO<std::pair<std::unique_ptr<EventObject>, void*>> events_;
};

} // namespace tnt

#endif
