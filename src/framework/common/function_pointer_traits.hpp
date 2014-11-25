
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

#ifndef TNT_FUNCTION_POINTER_TRAITS_HPP_
#define TNT_FUNCTION_POINTER_TRAITS_HPP_

#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/at.hpp>

namespace tnt {

template <class T> struct FunctionPointerTraits;

template <class R, class... Args> struct FunctionPointerTraits<R(Args...)>
{
    using type = R(*)(Args...);
};

template <class F> struct MemberFunctionParamTraits
{
    using type = typename boost::mpl::at_c<boost::function_types::parameter_types<decltype(&F::operator())>, 1>::type;
};

template <class F> struct FunctionParamTraits
{
    using type = typename boost::mpl::at_c<boost::function_types::parameter_types<F>, 0>::type;
};

template <class F> constexpr auto is_function(F&& func)
{
    return std::integral_constant<bool, std::is_function<typename std::decay<decltype(func)>::type>::value>();
};

} // namespace tnt

#endif
