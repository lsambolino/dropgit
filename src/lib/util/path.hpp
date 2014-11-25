
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

#ifndef DROP_UTIL_PATH_HPP_
#define DROP_UTIL_PATH_HPP_

#include <string>

#include <boost/filesystem.hpp>

namespace drop {
namespace util {
namespace detail_ {

namespace fs = boost::filesystem;

std::string combine_file_name(const std::string& n);

template <class... T> std::string combine_file_name(const std::string& n0, T&&... args)
{
    return n0 + "." + combine_file_name(args...);
}

std::string combine_path(const std::string& root);

template <class... T> std::string combine_path(const std::string& p0, T&&... args)
{
    auto path = fs::path(p0) / combine_path(args...);

    return path.string();
}

} // namespace detail_

std::string full_path(const std::string& path);

template <class... T> std::string combine_path(T&&... args)
{
    return detail_::combine_path(args...);
}

template <class... T> std::string combine_file_name(T&&... args)
{
    return detail_::combine_file_name(args...);
}

std::string config_file_path(const std::string& name);

} // namespace util
} // namespace drop

#endif
