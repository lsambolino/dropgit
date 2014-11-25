
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

#ifndef TNT_EVENT_FROM_MESSAGE_HPP_
#define TNT_EVENT_FROM_MESSAGE_HPP_

#include <map>
#include <memory>
#include <cstdint>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <stdexcept>
#include <functional>
#include <iostream>

#include "application.hpp"
#include "demangle.hpp"
#include "unpack_tuple.hpp"
#include "log.hpp"

namespace tnt {

template <class T, class... Args> std::shared_ptr<T> create_event(Args... args)
{
    return std::make_shared<T>(args...);
}

struct UnregisteredEventException: public std::invalid_argument
{
    explicit UnregisteredEventException(const std::string& msg) : std::invalid_argument(msg) {}
};

struct EventRegEntry;

using EventFactoryMethod = std::function<void(const std::string&, void*)>;

class EventFromMessage
{
    friend struct EventRegEntry;
    using FactoryMethodMap = std::map<std::pair<std::type_index, uint32_t>, EventFactoryMethod>;
public:
    template <class T> static void raise(T type, const std::string& data, void* src)
    {
        assert(factories_);
        auto it = factories_->find(std::make_pair(std::type_index(typeid(typename std::decay<T>::type)), static_cast<uint32_t>(type)));

        if (it == std::end(*factories_))
        {
            throw UnregisteredEventException(std::string(R"(Unregistered message ")") + demangle(typeid(typename std::decay<T>::type).name()) + R"(" ()" + std::to_string(static_cast<uint32_t>(type)) + ")");
        }

        (it->second)(data, src);
    }
private:
    EventFromMessage();

    template <class T> static void register_event(T type, EventFactoryMethod m)
    {
        init();
        factories_->emplace(std::make_pair(std::type_index(typeid(typename std::decay<T>::type)), static_cast<uint32_t>(type)), m);
    }

    static void init();
    static void init_map();
private:
    static std::unique_ptr<FactoryMethodMap> factories_;
};

struct EventRegEntry 
{
    template <class T> EventRegEntry(T type, EventFactoryMethod m) 
    {
        EventFromMessage::register_event<T>(type, m) ;
    }
};

template <class E, class T, T t, class F, class... Args> class RegisterEventFromMessage
{
    friend class EventFromMessage;
protected:
    RegisterEventFromMessage()
    {
        const EventRegEntry& dummy = r;
        (void)dummy;
    }
private:
    static void raise(const std::string& data, void* src) 
    {
        static_assert(std::is_constructible<E, Args...>::value, "Invalid class type registered as Event: invalid arguments passed to the constructor.");

        try
        {
            std::tuple<Args...> args;
		    F()(data, args);
        
            tnt::Application::raise(unpack(args, create_event<E, Args...>)(), src);
        }
        catch (std::exception& ex)
        {
            Log::error("RegisterEventFromMessage::new_instance (", tnt::demangle(typeid(E).name()), "): exception thrown: ", ex.what());

            throw;
        }
        catch (...)
        {
            Log::error("RegisterEventFromMessage::new_instance (", tnt::demangle(typeid(E).name()), "): error in serialization");

            throw;
        }
    }
private:
    static const EventRegEntry r;
};

template <class E, class T, T t, class F, class... Args> const EventRegEntry RegisterEventFromMessage<E, T, t, F, Args...>::r = EventRegEntry(t, RegisterEventFromMessage<E, T, t, F, Args...>::raise);

} // namespace tnt

#endif
