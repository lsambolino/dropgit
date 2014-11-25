
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

#ifndef TNT_BINARY_SERIALIZATION_DETAIL_HPP_
#define TNT_BINARY_SERIALIZATION_DETAIL_HPP_

#include <string>
#include <tuple>
#include <typeinfo>

#include "portable_binary_archive.hpp"

namespace binary {

template <class T> void to_string_(ar::portable_binary_oarchive& oa, const T& obj)
{
    oa << obj;
}

template <class T, class... R> void to_string_(ar::portable_binary_oarchive& oa, const T& obj, R ... rest)
{
    oa << obj;
    to_string_(oa, rest...);
}

template <class T> void from_string_(ar::portable_binary_iarchive& ia, T& obj)
{
    ia >> obj;
}

template <class T, class... R> void from_string_(ar::portable_binary_iarchive& ia, T& obj, R& ... rest)
{
    ia >> obj;
    from_string_(ia, rest...);
}

template <size_t I, class... T> void to_string_(ar::portable_binary_oarchive& oa, std::tuple<T...>& objs);

template <size_t I, class... T> struct ToString
{
    void exec(ar::portable_binary_oarchive& oa, std::tuple<T...>& objs) const
    {
        oa << std::get<std::tuple_size<std::tuple<T...>>::value - I>(objs);
        to_string_<I-1>(oa, objs);
    }
};

template <class... T> struct ToString<0, T...>
{
    void exec(ar::portable_binary_oarchive& /*oa*/, std::tuple<T...>& /*objs*/) const {}
};

template <size_t I, class... T> void to_string_(ar::portable_binary_oarchive& oa, std::tuple<T...>& objs)
{
    ToString<I, T...>().exec(oa, objs);
}

template <size_t I, class... T> void from_string_(ar::portable_binary_iarchive& ia, std::tuple<T...>& objs);
template <size_t I, class... T> struct FromString;

template <class... T> struct FromString<0, T...>
{
    void exec(ar::portable_binary_iarchive& /*ia*/, std::tuple<T...>& /*objs*/) const {}
};

template <size_t I, class... T> struct FromString
{
    void exec(ar::portable_binary_iarchive& ia, std::tuple<T...>& objs) const
    {
        ia >> std::get<std::tuple_size<std::tuple<T...>>::value - I>(objs);
        from_string_<I-1>(ia, objs);
    }
};

template <size_t I, class... T> void from_string_(ar::portable_binary_iarchive& ia, std::tuple<T...>& objs)
{
    FromString<I, T...>().exec(ia, objs);
}

template <class... T> void from_string_(ar::portable_binary_iarchive& ia, std::tuple<T...>& objs)
{
    FromString<std::tuple_size<std::tuple<T...>>::value, T...>().exec(ia, objs);
}

} // namespace binary

#endif
