
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

#include "system.hpp"

#include <mutex>
#include <iostream>
#include <cassert>
#include <cstring>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN)

#include <Windows.h>

#else

#include <signal.h>
#include <errno.h>

#endif

namespace tnt {
namespace {

std::once_flag init_instance_flag_;

} // namespace

std::unique_ptr<SignalHandle> SignalHandle::impl_;

SignalHandle& SignalHandle::instance()
{
    std::call_once(init_instance_flag_, [] ()
    {
        impl_.reset(new SignalHandle);
    });

    assert(impl_);

    return *impl_.get();
}

void SignalHandle::register_handler(std::function<void()> handler)
{
    instance().handler_ = handler;

#if defined(TNT_PLATFORM_WIN)

    if (!SetConsoleCtrlHandler(sig_handler, true))
    {
        throw SignalHandleException(std::strerror(GetLastError()));
    }
#else
    struct sigaction act;
    std::memset(&act, 0, sizeof(act));

    act.sa_handler = sig_handler;
    act.sa_flags = SA_RESETHAND;

    if (sigaction(SIGINT, &act, nullptr) == -1)
    {
        throw SignalHandleException(std::strerror(errno));
    }
#endif
}

void SignalHandle::ignore(int sig)
{
#if defined(TNT_PLATFORM_WIN)

#else
    signal(sig, SIG_IGN);
#endif
}

#if defined(TNT_PLATFORM_WIN)

int SignalHandle::sig_handler(unsigned long sig)
{
    if (sig == CTRL_C_EVENT)
    {
        instance().handler_();
    }

    return true;
}

#else

void SignalHandle::sig_handler(int sig)
{
    if (sig == SIGINT)
    {
        instance().handler_();
    }
}

#endif

} // namespace tnt
