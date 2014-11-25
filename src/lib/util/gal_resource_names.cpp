
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

#include "gal_resource_names.hpp"

#include <algorithm>
#include <sstream>

#include "util/string.hpp"

#include "log.hpp"

namespace drop {

std::vector<std::string> remove_fragment(const std::vector<std::string>& ids, decltype(std::vector<std::string>().size()) index)
{
    std::vector<std::string> result;
    result.reserve(ids.size() - 1);

    for (decltype(index) i = 0; i < ids.size(); ++i)
    {
        if (i != index)
        {
            result.push_back(ids[i]);
        }
    }

    return result;
}

std::vector<std::string> remove_fragment(const std::vector<std::string>& ids, const std::string& name)
{
    std::vector<std::string> result;
    result.reserve(ids.size());

    std::copy_if(std::begin(ids), std::end(ids), std::back_inserter(result), [&] (const auto& id)
    {
        return id != name;
    });

    return result;
}

std::string compose(const std::vector<std::string>& ids)
{
    std::ostringstream ss;

    for (const auto& i : ids)
    {
        ss << i << ".";
    }

    auto result = ss.str();

    if (!result.empty() && result.back() == '.')
    {
        result = result.substr(0, result.size() - 1); // Remove the trailing '.'
    }

    return result;
}

} // namespace drop
