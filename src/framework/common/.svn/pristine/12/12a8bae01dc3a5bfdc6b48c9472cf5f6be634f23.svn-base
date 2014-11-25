
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

#ifndef TNT_PLATFORM_HPP_
#define TNT_PLATFORM_HPP_

#if defined(_HAS_CPP0X) && _HAS_CPP0X

#define TNT_CPP11

#endif

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 180021005

#define TNT_CPP14

#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#define TNT_PLATFORM_WIN
#define TNT_PLATFORM_WIN32
#define TNT_ARCH_32_BIT

#elif defined(_WIN64)

#define TNT_PLATFORM_WIN
#define TNT_PLATFORM_WIN64
#define TNT_ARCH_64_BIT

#elif defined(linux) || defined(__linux) || defined(__linux__)

#define TNT_PLATFORM_LINUX

#if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__

#define TNT_CPP11

#endif

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)

#define TNT_PLATFORM_BSD

#if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__

#define TNT_CPP11

#endif

#elif defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)

#define TNT_PLATFORM_UNIX

#endif

#if defined(__LP64__) && !defined(SIZEOF_VOID_P)

#define SIZEOF_VOID_P 8
#define TNT_ARCH_64_BIT

#elif !defined(SIZEOF_VOID_P)

#define SIZEOF_VOID_P 4
#define TNT_ARCH_32_BIT

#endif

#endif
