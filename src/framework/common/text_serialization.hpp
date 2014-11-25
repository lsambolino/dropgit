
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

#ifndef TEXT_SERIALIZATION_HPP_
#define TEXT_SERIALIZATION_HPP_

#include <string>
#include <tuple>
#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "text_serialization_detail.hpp"

namespace ar = boost::archive;

namespace text {

inline std::string to_string()
{
    return "";
}

template <class ... T> std::string to_string(T... objs)
{
    std::stringstream ss;
    ar::text_oarchive oa(ss, ar::no_header);
    to_string_(oa, objs...);

    return ss.str();
}

template <class ... T> std::string to_string(std::tuple<T...>& objs)
{
    std::stringstream ss;
    ar::text_oarchive oa(ss, ar::no_header);
    to_string_<std::tuple_size<std::tuple<T...>>::value>(oa, objs);

    return ss.str();
}

template <class ... T> void from_string(const std::string& str, T&... objs)
{
    if (sizeof...(objs) > 0)
    {
        std::stringstream ss(str);
        ar::text_iarchive ia(ss, ar::no_header);
        from_string_(ia, objs...);
    }
}

template <class ... T> void from_string(const std::string& str, std::tuple<T...>& objs)
{
    if (std::tuple_size<std::tuple<T...>>::value > 0)
    {
        std::stringstream ss(str);
        ar::text_iarchive ia(ss, ar::no_header);
        from_string_(ia, objs);
    }
}

} // namespace text

#endif
