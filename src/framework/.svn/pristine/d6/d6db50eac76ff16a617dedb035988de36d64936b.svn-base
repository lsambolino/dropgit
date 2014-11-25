
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

#ifndef TNT_MESSAGE_FROM_DATA_HPP_
#define TNT_MESSAGE_FROM_DATA_HPP_

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

#include "message/message.hpp"

#include "demangle.hpp"
#include "unpack_tuple.hpp"
#include "log.hpp"

namespace tnt {

template <class T, class... Args> std::shared_ptr<T> create_message(Args... args)
{
    return std::make_shared<T>(args...);
}

struct UnregisteredMessageException: public std::invalid_argument
{
    explicit UnregisteredMessageException(const std::string& msg) : std::invalid_argument(msg) {}
};

struct MessageRegEntry;

using MessageFactoryMethod = std::function<std::shared_ptr<Message>(const std::string&)>;

class MessageFromData
{
    friend struct MessageRegEntry;
    using FactoryMethodMap = std::map<std::pair<std::type_index, uint32_t>, MessageFactoryMethod>;
public:
    template <class T> static std::shared_ptr<Message> create(T type, const std::string& data)
    {
        assert(factories_);
        auto it = factories_->find(std::make_pair(std::type_index(typeid(typename std::decay<T>::type)), static_cast<uint32_t>(type)));

        if (it == std::end(*factories_))
        {
            throw UnregisteredMessageException(std::string(R"(Unregistered message ")") + demangle(typeid(typename std::decay<T>::type).name()) + R"(" ()" + std::to_string(static_cast<uint32_t>(type)) + ")");
        }

        return (it->second)(data);
    }
private:
    MessageFromData();

    template <class T> static void register_event(T type, MessageFactoryMethod m)
    {
        init();
        factories_->emplace(std::make_pair(std::type_index(typeid(typename std::decay<T>::type)), static_cast<uint32_t>(type)), m);
    }

    static void init();
    static void init_map();
private:
    static std::unique_ptr<FactoryMethodMap> factories_;
};

struct MessageRegEntry 
{
    template <class T> MessageRegEntry(T type, MessageFactoryMethod m) 
    {
        MessageFromData::register_event<T>(type, m) ;
    }
};

template <class M, class T, T t, class F, class... Args> class RegisterMessageFromData
{
    friend class MessageFromData;
protected:
    RegisterMessageFromData()
    {
        const MessageRegEntry& dummy = r;
        (void)dummy;
    }
private:
    static std::shared_ptr<M> new_instance(const std::string& data) 
    {
        try
        {
            static_assert(std::is_base_of<Message, M>::value, "Invalid class type registered as Message: class must be derived from Message.");
            static_assert(std::is_constructible<M, Args...>::value, "Invalid class type registered as Message: invalid arguments passed to the constructor.");

            std::tuple<Args...> args;
			F()(data, args);
        
            return unpack(args, create_message<M, Args...>)();
        }
        catch (std::exception& ex)
        {
            Log::error("RegisterMessageFromData::new_instance (", tnt::demangle(typeid(M).name()), "): exception thrown: ", ex.what());
        }
        catch (...)
        {
            Log::error("RegisterMessageFromData::new_instance (", tnt::demangle(typeid(M).name()), "): error in serialization");
        }

        return nullptr;
    }
private:
    static const MessageRegEntry r;
};

template <class M, class T, T t, class F, class... Args> const MessageRegEntry RegisterMessageFromData<M, T, t, F, Args...>::r = MessageRegEntry(t, RegisterMessageFromData<M, T, t, F, Args...>::new_instance);

} // namespace tnt

#endif
