
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

#include "arp_cache.hpp"

#include <fstream>

#include "exception/drop_exception.hpp"

#include "mac_address.hpp"
#include "log.hpp"

namespace tnt {

const char* arp_cache = "/proc/net/arp";

tnt::MacAddress hw_from_arp(const std::string& ip)
{
    std::ifstream file(arp_cache);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string line;

    while (std::getline(file, line))
    {
        if (line.find(ip) == std::string::npos)
        {
            continue;
        }

        std::stringstream ss(line);
        std::string tmp0;
        std::string tmp1;
        std::string tmp2;
        std::string hw;

        ss >> tmp0 >> tmp1 >> tmp2 >> hw;

        return tnt::MacAddress(hw);
    }

    throw std::invalid_argument("Error: The ip address does not exists.");
}

} // namespace tnt 

