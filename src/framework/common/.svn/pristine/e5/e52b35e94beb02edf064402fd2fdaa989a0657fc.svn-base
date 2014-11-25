
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

#ifndef TNT_SOCKET_EXCEPTION_HPP_
#define TNT_SOCKET_EXCEPTION_HPP_

#include <system_error>

#include "init_sockets.hpp"

namespace tnt {

struct SocketException: public std::system_error, private InitSockets
{
    explicit SocketException(std::error_code ec = socket_error_code()): std::system_error(ec) {}
};

struct OpenSocketException: public SocketException
{
    explicit OpenSocketException() {}
};

struct BindSocketException: public SocketException
{
    explicit BindSocketException() {}
};

struct ConnectSocketException: public SocketException
{
    explicit ConnectSocketException() {}
};

struct ConnectRefusedException: public SocketException
{
    explicit ConnectRefusedException() {}
};

struct ListenSocketException: public SocketException
{
    explicit ListenSocketException() {}
};

struct AcceptSocketException: public SocketException
{
    explicit AcceptSocketException() {}
};

struct GetSocketOptionException: public SocketException
{
    explicit GetSocketOptionException() {}
};

struct SetSocketOptionException: public SocketException
{
    explicit SetSocketOptionException() {}
};

struct SendToException: public SocketException
{
    explicit SendToException() {}
};

struct SendException: public SocketException
{
    explicit SendException() {}
};

struct ReceiveFromException: public SocketException
{
    explicit ReceiveFromException() {}
};

struct ReceiveException: public SocketException
{
    explicit ReceiveException() {}
};

struct ShutdownSocketException: public SocketException
{
    explicit ShutdownSocketException() {}
};

} // namespace tnt

#endif
