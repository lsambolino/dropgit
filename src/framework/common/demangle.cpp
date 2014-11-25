
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

#include "demangle.hpp"

#include <memory>
#include <cstdlib>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <Windows.h>
#include <Dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

#else

#ifdef __GNUC__

#include <cxxabi.h>

#endif

#endif

namespace tnt {

#ifdef __GNUC__

std::string demangle(const char* name)
{
    int status;
    std::unique_ptr<char, void (*)(void*)> tmp(abi::__cxa_demangle(name, 0, 0, &status), std::free);

    return !tmp ? name : tmp.get();
}

#elif defined(TNT_PLATFORM_WIN32)

std::string demangle(const char* name)
{
    std::string result(512, '\0');
    auto len = UnDecorateSymbolName(name, &result[0], result.size(), UNDNAME_COMPLETE);

    return len == 0 ? name : result.substr(0, len);
}

#else

std::string demangle(const char* name)
{
    return name;
}

#endif

} // namespace tnt
