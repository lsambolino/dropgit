
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

#include "init_sockets.hpp"

#include <mutex>
#include <cstring>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <WinSock2.h>

#endif

namespace tnt {
namespace {

#if defined(TNT_PLATFORM_WIN32)

#pragma comment(lib, "ws2_32.lib")

std::once_flag init_instance_flag_;

#endif

void init_sockets()
{
#if defined(TNT_PLATFORM_WIN32)
    std::call_once(init_instance_flag_, [] ()
    {
        auto version_requested = MAKEWORD(2, 2);
        WSADATA data;

        if (WSAStartup(version_requested, &data) != 0)
        {
            std::terminate();
        }
    });
#endif
}

} // namespace

int socket_error_number()
{
#if defined(TNT_PLATFORM_WIN32)
    return ::WSAGetLastError();
#else
    return errno;
#endif
}

std::error_code socket_error_code()
{
    return std::error_code(socket_error_number(), std::system_category());
}

InitSockets::InitSockets()
{
    init_sockets();
}

} // namespace tnt
