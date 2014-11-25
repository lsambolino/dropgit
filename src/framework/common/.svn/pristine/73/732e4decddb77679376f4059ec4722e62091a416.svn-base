
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

#ifndef TNT_UNPACK_TUPLE_HPP_
#define TNT_UNPACK_TUPLE_HPP_

#include <type_traits>
#include <functional>
#include <tuple>

namespace tnt {

template <int...> struct seq {};
template <int N, int... S> struct gens: gens<N-1, N-1, S...> {};
template <int... S> struct gens<0, S...> { using type = seq<S...>; };

template <class R, class... Args> struct UnpackTupleToFunction
{
    std::tuple<Args...> params;
    R (*func)(Args...);

    R operator()()
    {
        return callFunc(typename gens<sizeof...(Args)>::type());
    }
private:
    template <int... S> R callFunc(seq<S...>)
    {
        return func(std::get<S>(std::move(params))...);
    }
};

template <class... Args> struct UnpackTupleToProcedure
{
    std::tuple<Args...> params;
    void (*func)(Args...);

    void operator()()
    {
        callFunc(typename gens<sizeof...(Args)>::type());
    }
private:
    template <int... S> void callFunc(seq<S...>)
    {
        func(std::get<S>(params) ...);
    }
};

template <class R, class... Args> UnpackTupleToFunction<R, Args...> unpack(const std::tuple<Args...>& t, R (*func)(Args...))
{
    return { t, func };
}

template <class... Args> UnpackTupleToProcedure<Args...> unpack(const std::tuple<Args...>& t, void (*func)(Args...))
{
    return { t, func };
}

template <class R, class... Args> UnpackTupleToFunction<R, Args...> unpack(std::tuple<Args...>&& t, R (*func)(Args...))
{
    return { std::move(t), func };
}

template <class... Args> UnpackTupleToProcedure<Args...> unpack(std::tuple<Args...>&& t, void (*func)(Args...))
{
    return { std::move(t), func };
}

template <class ... Args> void unpack(const std::tuple<Args...>& t, Args&... args)
{
    std::tie(args...) = t;
}

} // namespace tnt

#endif
