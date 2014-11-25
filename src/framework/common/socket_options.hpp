
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

#ifndef TNT_SOCKET_OPTIONS_HPP_
#define TNT_SOCKET_OPTIONS_HPP_

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <WinSock2.h>

using SocketLen = int;

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using SocketLen = socklen_t;

#endif

namespace tnt {
namespace ip {

template <int Level, int Name> class BooleanOption
{
public:
    BooleanOption(): size_(sizeof(value_)) {}
    explicit BooleanOption(bool value): value_(value ? 1 : 0), size_(sizeof(value_)) {}

    BooleanOption& operator=(bool value)
    {
        value_ = value ? 1 : 0;

        return *this;
    }

    bool value() const
    {
        return !!value_;
    }

    operator bool() const
    {
        return value();
    }

    bool operator!() const
    {
        return !value();
    }

    int level() const
    {
        return Level;
    }

    int name() const
    {
        return Name;
    }

#if defined(TNT_PLATFORM_WIN32)

    const char* data() const
    {
        return reinterpret_cast<const char*>(&value_);
    }

    char* data()
    {
        return reinterpret_cast<char*>(&value_);
    }

#else

    const int* data() const
    {
        return &value_;
    }

    int* data()
    {
        return &value_;
    }

#endif

    const SocketLen* len() const
    {
        return &size_;
    }

    SocketLen* len()
    {
        return &size_;
    }
private:
    int value_;
    SocketLen size_;
};

template <int Level, int Name> class IntOption
{
public:
    IntOption(): size_(sizeof(value_)) {}
    explicit IntOption(int value): value_{ value }, size_(sizeof(value_)) {}

    IntOption& operator=(int value)
    {
        value_ = value;

        return *this;
    }

    int value() const
    {
        return value_;
    }

    operator int() const
    {
        return value();
    }

    int level() const
    {
        return Level;
    }

    int name() const
    {
        return Name;
    }

#if defined(TNT_PLATFORM_WIN32)

    const char* data() const
    {
        return reinterpret_cast<const char*>(&value_);
    }

    char* data()
    {
        return reinterpret_cast<char*>(&value_);
    }

#else

    const int* data() const
    {
        return &value_;
    }

    int* data()
    {
        return &value_;
    }

#endif

    const SocketLen* len() const
    {
        return &size_;
    }

    SocketLen* len()
    {
        return &size_;
    }
private:
    int value_;
    SocketLen size_;
};

using ReuseAddress = BooleanOption<SOL_SOCKET, SO_REUSEADDR>;
using Broadcast = BooleanOption<SOL_SOCKET, SO_BROADCAST>;

using SendBuffer = IntOption<SOL_SOCKET, SO_SNDBUF>;
using ReceiveBuffer = IntOption<SOL_SOCKET, SO_RCVBUF>;

} // namespace ip
} // namespace tnt

#endif
