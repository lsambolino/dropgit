
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

#include "exec.hpp"

#include <string>
#include <memory>
#include <cstdio>

#include "platform.hpp"

namespace tnt {

std::string exec(const std::string& path)
{
#if defined(TNT_PLATFORM_WIN)
    std::unique_ptr<FILE, int(*)(FILE*)> f(_popen(path.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, int(*)(FILE*)> f(popen(path.c_str(), "r"), pclose);
#endif

    if (!f)
    {
        throw ExecutionError(std::string("Executable file \"") + path + "\" not found.");
    }

    std::string buffer(1024, '\0');
    auto res = fgets(&buffer[0], buffer.size(), f.get());

    if (!res)
    {
        throw ExecutionError(std::string("Error executing file \"") + path + "\" not found.");
    }

    return res;
}

} // namespace tnt
