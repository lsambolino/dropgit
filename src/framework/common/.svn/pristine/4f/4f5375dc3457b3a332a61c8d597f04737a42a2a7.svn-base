
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

#ifndef TNT_DUMP_HPP_
#define TNT_DUMP_HPP_

#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>

namespace tnt {

inline void print(std::ostream& os, int16_t val)
{
    os << std::hex << std::setw(2) << std::setfill('0') << val; 
}

template <class T> std::string dump(const T& obj)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&obj);
    std::stringstream ss;
    auto len = sizeof(T);

    for (decltype(len) i = 0; i < len; ++i)
    {
        print(ss, static_cast<const int16_t>(ptr[i]));
        ss << " ";
    }

    return ss.str();
}

inline std::string dump(const std::string& str)
{
    std::stringstream ss;
    auto len = str.size();

    for (decltype(len) i = 0; i < len; ++i)
    {
        print(ss, static_cast<const int16_t>(static_cast<const uint8_t>(str[i])));
        ss << " ";
    }

    return ss.str();
}

} // namespace tnt

#endif
