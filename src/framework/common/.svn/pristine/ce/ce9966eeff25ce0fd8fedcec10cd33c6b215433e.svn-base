
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

#ifndef TNT_CONTAINERS_HPP_
#define TNT_CONTAINERS_HPP_

#include <algorithm>
#include <functional>
#include <map>

namespace tnt {

template <class C, class F> auto for_all(const C& container, F func)
{
    return std::for_each(std::begin(container), std::end(container), func);
}

template <class C, class T> auto erase(C& container, const T& element)
{
    return container.erase(std::remove(std::begin(container), std::end(container), element), std::end(container));
}

template <class C, class P> auto erase_if(C& container, P pred)
{
    return container.erase(std::remove_if(std::begin(container), std::end(container), pred), std::end(container));
}

template <class C, class P> auto find_if(const C& container, P pred)
{
    return std::find_if(std::begin(container), std::end(container), pred);
}

template <template <class, class> class C, class T, class A> auto insert_back(C<T, A>& container, const T& element)
{
    return container.insert(std::end(container), element);
}

template <class C, class I> auto insert_back(C& container, const I& first, const I& last)
{
    return container.insert(std::end(container), first, last);
}

template <class C1, class C2> auto insert_back(C1& c1, const C2& c2)
{
    return c1.insert(std::end(c1), std::begin(c2), std::end(c2));
}

template <class C, class T> bool contains(const C& container, const T& value)
{
    return std::find(std::begin(container), std::end(container), value) != std::end(container);
}

template <class C, class P> bool contains_if(const C& container, P pred)
{
    return std::find_if(std::begin(container), std::end(container), pred) != std::end(container);
}

template <class M, class K> bool contains_key(const M& map, const K& key)
{
    return map.find(key) != std::end(map);
}

template <class C, class P> bool all_of(const C& container, const P& pred)
{
    return std::all_of(std::begin(container), std::end(container), pred);
}

template <class C, class P> bool any_of(const C& container, const P& pred)
{
    return std::any_of(std::begin(container), std::end(container), pred);
}

template <class C, class P> bool none_of(const C& container, const P& pred)
{
    return std::none_of(std::begin(container), std::end(container), pred);
}

} // namespace tnt

#endif
