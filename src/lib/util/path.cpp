
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

#include "path.hpp"

#include <boost/asio.hpp>

#include "exception/exception.hpp"

#include "log.hpp"

namespace drop {
namespace util {
namespace detail_ {

std::string combine_path(const std::string& root, const std::string& first)
{
    auto path = fs::path(root) / first;

    return path.string();
}

std::string combine_path(const std::string& p)
{
    return p;
}

std::string combine_file_name(const std::string& n)
{
    return n;
}

} // namespace detail_

namespace fs = boost::filesystem;

std::string full_path(const std::string& file)
{
    fs::path path = file;

    if (!path.is_complete())
    {
        path = fs::initial_path<fs::path>() / file;
    }

    if (!fs::exists(path))
    {
        tnt::Log::error("drop::util::full_path(", file, ") error: file non existent.");

        throw tnt::NonExistentFile();
    }

    return path.string();
}

std::string config_file_path(const std::string& name)
{
    return full_path(combine_path("config", boost::asio::ip::host_name(), combine_file_name(name, "xml")));
}

} // namespace util
} // namespace drop
