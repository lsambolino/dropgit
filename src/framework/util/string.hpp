
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

#ifndef TNT_STRING_HPP_
#define TNT_STRING_HPP_

#include <string>
#include <vector>
#include <algorithm>

#if defined(WITH_BOOST) || defined(__CYGWIN__)
#include <boost/algorithm/string.hpp>
#endif

#if !defined(WITH_BOOST) || defined(__CYGWIN__)
#include <sstream>
#endif

namespace tnt {

#if !defined(__CYGWIN__)

inline int stoi(const std::string& str)
{
    return std::stoi(str);
}

template <class T> inline std::string to_string(T value)
{
	return std::to_string(value);
}

#else

int stoi(const std::string& str);

#endif

inline std::string to_lower(std::string data)
{
#if defined(WITH_BOOST)
	boost::algorithm::to_lower(data);
#else
	std::transform(std::begin(data), std::end(data), std::begin(data), ::tolower);
#endif

    return data;
}

#if defined(WITH_BOOST)

inline std::vector<std::string> split(const std::string& str, const std::string& sep)
{
    std::vector<std::string> parts;
	boost::algorithm::split(parts, str, boost::algorithm::is_any_of(sep));

    return parts;
}

#else

std::vector<std::string> split(const std::string& str, const std::string& sep);

#endif

} // namespace tnt

#endif
