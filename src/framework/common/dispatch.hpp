
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

#ifndef TNT_DISPATCHER_HPP_
#define TNT_DISPATCHER_HPP_

#include <functional>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <stdexcept>
#include <string>

#include "demangle.hpp"
#include "range.hpp"

namespace tnt {

struct ListenerNotFound: public std::runtime_error
{
    ListenerNotFound(const std::string& msg): std::runtime_error(msg) {}
};

template <class R, class T, class... A> class TypeDispatch
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<R(const U&, A ...)>>::value, "Invalid callback for this type");
        static_assert(std::is_convertible<T, U>::value, "Invalid listener type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](const T& obj, A... args) mutable
            {
                return func(dynamic_cast<U>(obj), args ...);
            });
    }

    R inject_object(const T& obj, A... args)
    { 
        auto it = listeners_.find(std::type_index(typeid(*obj)));

        if (it != std::end(listeners_))
        {
            return (it->second)(obj, args ...);
        }

        throw tnt::ListenerNotFound(std::string("Unknown object type: ") + get_name(*obj));
    }
private:
    std::unordered_map<std::type_index, std::function<R(const T&, A...)>> listeners_;
};

template <class R, class T, class... A> class TypeDispatch<R, std::shared_ptr<T>, A...>
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<R(const std::shared_ptr<U>&, A ...)>>::value, "Invalid callback for this type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](const std::shared_ptr<T>& obj, A... args) mutable
            {
                return func(std::dynamic_pointer_cast<U>(obj), args ...);
            });
    }

    R inject_object(const std::shared_ptr<T>& obj, A... args)
    { 
        auto it = listeners_.find(std::type_index(typeid(*obj)));

        if (it != std::end(listeners_))
        {
            return (it->second)(obj, args ...);
        }

        throw tnt::ListenerNotFound(std::string("Unknown object type: ") + get_name(*obj));
    }
private:
    std::unordered_map<std::type_index, std::function<R(const std::shared_ptr<T>&, A...)>> listeners_;
};

template <class R, class T, class... A> class TypeDispatch<R, std::unique_ptr<T>, A...>
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<R(U*, A ...)>>::value, "Invalid callback for this type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](std::unique_ptr<T>&& obj, A... args) mutable
            {
                return func(dynamic_cast<U*>(obj.get()), args ...);
            });
    }

    R inject_object(std::unique_ptr<T>&& obj, A... args)
    { 
        auto it = listeners_.find(std::type_index(typeid(*obj)));

        if (it != std::end(listeners_))
        {
            return (it->second)(std::move(obj), args ...);
        }

        throw tnt::ListenerNotFound(std::string("Unknown object type: ") + get_name(*obj));
    }
private:
    std::unordered_map<std::type_index, std::function<R(std::unique_ptr<T>&&, A...)>> listeners_;
};

template <class T, class... A> class TypeDispatch <void, T, A...>
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<void(const U&, A...)>>::value, "Invalid callback for this type");
        static_assert(std::is_convertible<T, U>::value, "Invalid listener type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](const T& obj, A... args) mutable
            {
                func(dynamic_cast<U>(obj), args...);
            });
    }

    bool inject_object(const T& obj, A... args)
    {
        auto it = listeners_.find(std::type_index(typeid(obj)));

        if (it != std::end(listeners_))
        {
            (it->second)(obj, args...);

            return true;
        }

        return false;
    }
private:
    std::unordered_map<std::type_index, std::function<void(const T&, A...)>> listeners_;
};

template <class T, class... A> class TypeDispatch <void, std::shared_ptr<T>, A...>
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<void(const std::shared_ptr<U>&, A...)>>::value, "Invalid callback for this type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](const std::shared_ptr<T>& obj, A... args) mutable
            {
                func(std::dynamic_pointer_cast<U>(obj), args...);
            });
    }

    bool inject_object(const std::shared_ptr<T>& obj, A... args)
    {
        auto it = listeners_.find(std::type_index(typeid(*obj)));

        if (it != std::end(listeners_))
        {
            (it->second)(obj, args...);

            return true;
        }

        return false;
    }
private:
    std::unordered_map<std::type_index, std::function<void(const std::shared_ptr<T>&, A...)>> listeners_;
};

template <class T, class... A> class TypeDispatch <void, std::unique_ptr<T>, A...>
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<void(U*, A...)>>::value, "Invalid callback for this type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](std::unique_ptr<T>&& obj, A... args) mutable
            {
                func(dynamic_cast<U*>(obj.get()), args...);
            });
    }

    bool inject_object(std::unique_ptr<T>&& obj, A... args)
    {
        auto it = listeners_.find(std::type_index(typeid(*obj)));

        if (it != std::end(listeners_))
        {
            (it->second)(std::move(obj), args...);

            return true;
        }

        return false;
    }
private:
    std::unordered_map<std::type_index, std::function<void(std::unique_ptr<T>&&, A...)>> listeners_;
};

template <class T, class... A> class TypeMultiDispatch
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<void(const U&, A ...)>>::value, "Invalid callback for this type");
        static_assert(std::is_convertible<T, U>::value, "Invalid listener type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](const T& obj, A... args) mutable
            {
                func(dynamic_cast<U>(obj), args...);
            });
    }

    size_t inject_object(const T& obj, A... args)
    {
        size_t num = 0;

        for (const auto& it : tnt::equal_range(listeners_, std::type_index(typeid(*obj))))
        {
            (it.second)(obj, args...);
            ++num;
        }

        if (num == 0)
        {
            throw tnt::ListenerNotFound(std::string("Unknown object type: ") + get_name(*obj));
        }

        return num;
    }
private:
    std::unordered_multimap<std::type_index, std::function<void(const T&, A...)>> listeners_;
};

template <class T, class... A> class TypeMultiDispatch<std::shared_ptr<T>, A...>
{
public:
    template <class U, class F> void register_listener(F func)
    {
        static_assert(std::is_convertible<F, std::function<void(const std::shared_ptr<U>&, A ...)>>::value, "Invalid callback for this type");

        listeners_.emplace(std::type_index(typeid(typename std::decay<U>::type)),
            [=](const std::shared_ptr<T>& obj, A... args) mutable
            {
                func(std::dynamic_pointer_cast<U>(obj), args ...);
            });
    }

    size_t inject_object(const std::shared_ptr<T>& obj, A... args)
    { 
        size_t num = 0;

        for (const auto& it : tnt::equal_range(listeners_, std::type_index(typeid(*obj))))
        {
            (it.second)(obj, args...);
            ++num;
        }

        if (num == 0)
        {
            throw tnt::ListenerNotFound(std::string("Unknown object type: ") + get_name(*obj));
        }

        return num;
    }
private:
    std::unordered_multimap<std::type_index, std::function<void(const std::shared_ptr<T>&, A...)>> listeners_;
};

template <class K, class R, class... A> class KeyDispatch
{
public:
    void register_listener(const K& key, const std::function<R(A ...)>& func)
    {
        listeners_.emplace(key, func);
    }

    R inject_object(const K& key, A... args)
    {
        auto it = listeners_.find(key);

        if (it != std::end(listeners_))
        {
            return (it->second)(args...);
        }

        throw tnt::ListenerNotFound("Unknown key value");
    }
private:
    std::unordered_map<K, std::function<R(A ...)>> listeners_;
};

} // namespace tnt

#endif
