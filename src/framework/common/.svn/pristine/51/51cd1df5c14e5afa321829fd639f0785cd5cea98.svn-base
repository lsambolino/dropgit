
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

#ifndef TNT_RANGE_HPP_
#define TNT_RANGE_HPP_

#include <utility>
#include <iterator>
#include <algorithm>
#include <functional>

#include "platform.hpp"

namespace tnt {

template <class T> class Range
{
public:
    Range(const T& f, const T& l) : first_(f), last_(l) {}

    T begin() const { return first_; }
    T end() const { return last_; }
private:
    T first_;
    T last_;
};

template <class T> Range<T> make_range(const T& f, const T& l)
{
    return Range<T>(f, l);
}

template <class T> Range<T> make_range(const std::pair<T, T>& p)
{
    return Range<T>(p.first, p.second);
}

template <class C> auto make_range(C& container) -> Range<decltype(std::begin(container))>
{
    return make_range(std::begin(container), std::end(container));
}

template <class C> auto make_range(const C& container) -> Range<decltype(std::begin(container))>
{
    return make_range(std::begin(container), std::end(container));
}

template <class C, class T> auto equal_range(const C& container, const T& key) -> Range<decltype(std::begin(container))>
{
    auto it = container.equal_range(key);

    return make_range(it.first, it.second);
}

template <class R, class T> bool in_range(const R& range, const T& value)
{
    return std::find(std::begin(range), std::end(range), value) != range.second;
}

template <class R, class P> bool in_range_if(const R& range, P pred)
{
    return std::find_if(std::begin(range), std::end(range), pred) != range.second;
}

template <class R, class F> auto for_range(const R& range, F func) -> decltype(std::for_each(range.first, range.second, func))
{
    return std::for_each(std::begin(range), std::end(range), func);
}

template <class R, class P> auto find_range_if(const R& range, P pred) -> decltype(std::find_if(range.first, range.second, pred))
{
    return std::find_if(std::begin(range), std::end(range), pred);
}

template <class R, class P> auto remove_range_if(const R& range, P pred) -> decltype(std::remove_if(range.first, range.second, pred))
{
    return std::remove_if(std::begin(range), std::end(range), pred);
}

template <class R> bool empty_range(const R& range)
{
    return std::begin(range) == std::end(range);
}

template <class T> bool empty_range(const std::pair<T, T>& pair)
{
    return pair.first == pair.second;
}

template <class R> size_t range_size(const R& range)
{
    return std::distance(std::begin(range), std::end(range));
}

} // namespace tnt

#endif
