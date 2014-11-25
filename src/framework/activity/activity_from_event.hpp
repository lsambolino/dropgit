
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

#ifndef TNT_ACTIVITY_FROM_EVENT_HPP_
#define TNT_ACTIVITY_FROM_EVENT_HPP_

#include <unordered_map>
#include <mutex>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <stdexcept>
#include <string>

#include "demangle.hpp"

namespace tnt {

struct EventNotRegistered: public std::runtime_error
{
    explicit EventNotRegistered(const std::string& msg) : std::runtime_error(msg) {}
};

struct AutoActivity
{
    virtual ~AutoActivity() = default;
    virtual void run() = 0;
};

struct ActivityRegEntry;

using ActivityFactoryMethod = std::function<std::unique_ptr<AutoActivity>(void*)>;

class ActivityFromEvent
{
    friend struct ActivityRegEntry;
    using FactoryMethodMap = std::unordered_map<std::type_index, ActivityFactoryMethod>;
public:
    ActivityFromEvent() = delete;

    static auto create(std::type_index ti, void* event)
    {
        if (!factories_)
        {
            throw EventNotRegistered(std::string("ActivityFromEvent, no Event registered: ") + demangle(ti.name()));
        }

        auto it = factories_->find(ti);

        if (it == std::end(*factories_))
        {
            throw EventNotRegistered(std::string("ActivityFromEvent, received an unregistered Event: ") + demangle(ti.name()));
        }

        auto func = it->second;

        return [func, event] () { return func(event); };
    }
private:
    static void Register(const std::type_index& ti, ActivityFactoryMethod m);
    static void init_map();
private:
    static std::unique_ptr<FactoryMethodMap> factories_;
};

struct ActivityRegEntry
{
    ActivityRegEntry(const std::type_index& ti, ActivityFactoryMethod m)
    {
        ActivityFromEvent::Register(ti, m);
    }
};

template <class T, class E> class RegisterActivityFromEvent: public virtual AutoActivity
{
protected:
    RegisterActivityFromEvent()
    {
        const ActivityRegEntry& dummy = r;
        (void) dummy;
    }
private:
    static std::unique_ptr<T> new_instance(void* event)
    {
        static_assert(std::is_base_of<AutoActivity, T>::value, "Invalid class type registered as AutoActivity.");
        static_assert(std::is_constructible<T, E>::value, "Invalid constructor of the class registered as AutoActivity.");

        return std::make_unique<T>(*reinterpret_cast<E*>(event));
    }
private:
    static const ActivityRegEntry r;
};

template <class T, class E> const ActivityRegEntry RegisterActivityFromEvent<T, E>::r = ActivityRegEntry(std::type_index(typeid(typename std::decay<E>::type)), RegisterActivityFromEvent<T, E>::new_instance);

} // namespace tnt

#endif
