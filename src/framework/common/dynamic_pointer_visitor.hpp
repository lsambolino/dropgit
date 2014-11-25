
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

#ifndef TNT_DYNAMIC_POINTER_VISITOR_HPP_
#define TNT_DYNAMIC_POINTER_VISITOR_HPP_

#include <memory>
#include <functional>
#include <algorithm>

namespace tnt {

template <class P, class T> bool is(T* ptr)
{
    return !!dynamic_cast<P*>(ptr);
}

template <class P, class T> bool is(const std::shared_ptr<T>& ptr)
{
    return !!std::dynamic_pointer_cast<P>(ptr);
}

template <class P, class C> bool any_of_is(const C& container)
{
    return std::any_of(std::begin(container), std::end(container), [] (typename C::const_reference v)
    {
        return is<P>(v);
    });
}

template <class P, class C> bool all_of_is(const C& container)
{
    return std::all_of(std::begin(container), std::end(container), [] (typename C::const_reference v)
    {
        return is<P>(v);
    });
}

template <class P, class C> bool none_of_is(const C& container)
{
    return std::none_of(std::begin(container), std::end(container), [] (typename C::const_reference v)
    {
        return is<P>(v);
    });
}

template <class P, class T> bool visit(T* ptr, std::function<void(P*)> func)
{
    if (auto p = dynamic_cast<P*>(ptr))
    {
        func(p);

        return true;
    }

    return false;
}

template <class P, class T, class R> bool visit(T* ptr, std::function<R(P*)> func, R& result)
{
    if (auto p = dynamic_cast<P*>(ptr))
    {
        result = func(p);

        return true;
    }

    return false;
}

template <class P, class T> bool visit(const std::shared_ptr<T>& ptr, std::function<void(const std::shared_ptr<P>&)> func)
{
    if (auto p = std::dynamic_pointer_cast<P>(ptr))
    {
        func(p);

        return true;
    }

    return false;
}

template <class P, class T, class R> bool visit(const std::shared_ptr<T>& ptr, std::function<R(const std::shared_ptr<P>&)> func, R& result)
{
    if (auto p = std::dynamic_pointer_cast<P>(ptr))
    {
        result = func(p);

        return true;
    }

    return false;
}

template <class P, class C> bool visit_any_of(const C& container, std::function<void(const std::shared_ptr<P>&)> func)
{
    return std::any_of(std::begin(container), std::end(container), [&] (typename C::const_reference v)
    {
        return visit<P>(v, func);
    });
}

} // namespace tnt

#endif
