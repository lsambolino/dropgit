
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

#ifndef TNT_SYSTEM_HPP_
#define TNT_SYSTEM_HPP_

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include "platform.hpp"

namespace tnt {

struct SignalHandleException: public std::runtime_error
{
    explicit SignalHandleException(const std::string& msg): std::runtime_error(msg) { }
};

class SignalHandle
{
public:
    static void register_handler(std::function<void()> handler);
    static void ignore(int sig);
private:
    SignalHandle() = default;
    SignalHandle(const SignalHandle&) = delete;
    SignalHandle(SignalHandle&&) = delete;
    SignalHandle& operator=(const SignalHandle&) = delete;
    SignalHandle& operator=(SignalHandle&&) = delete;

    static SignalHandle& instance();

#if defined(TNT_PLATFORM_WIN)
    static int __stdcall sig_handler(unsigned long sig);
#else
    static void sig_handler(int sig);
#endif

private:
    static std::unique_ptr<SignalHandle> impl_;

    std::function<void()> handler_;
};

} // namespace tnt

#endif
