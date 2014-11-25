
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

#ifndef TNT_ENDIANNES_HPP_
#define TNT_ENDIANNES_HPP_

#include <limits>
#include <type_traits>
#include <cstdint>
#include <cassert>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN)

#include <Winsock2.h>

#else

#include <arpa/inet.h>

#endif

inline int64_t htonll(int64_t n)
{
	return htonl(1) == 1 ? n : ((int64_t) htonl(n) << 32) | htonl(n >> 32);
}

inline uint64_t htonll(uint64_t n)
{
	return htonl(1) == 1 ? n : ((uint64_t) htonl(n) << 32) | htonl(n >> 32);
}

inline int64_t ntohll(int64_t n)
{
	return htonl(1) == 1 ? n : ((int64_t) ntohl(n) << 32) | ntohl(n >> 32);
}

inline uint64_t ntohll(uint64_t n)
{
	return htonl(1) == 1 ? n : ((uint64_t) ntohl(n) << 32) | ntohl(n >> 32);
}

namespace private_ {

template <class T, class U> T network_to_host(std::true_type /*bit16*/, T value)
{
    return static_cast<T>(ntohs(static_cast<U>(value)));
}

template <class T, class U> T network_to_host(std::false_type /*bit32*/, T value)
{
    return static_cast<T>(ntohl(static_cast<U>(value)));
}

template <class T, class U> T host_to_network(std::true_type /*16 bit*/, T value)
{
    return static_cast<T>(htons(static_cast<U>(value)));
}

template <class T, class U> T host_to_network(std::false_type /*32 bit*/, T value)
{
    return static_cast<T>(htonl(static_cast<U>(value)));
}

template <class T> constexpr auto is_16_bit()
{
    return std::integral_constant<bool, sizeof(T) == 2>();
};

} // namespace private_

template <class T> T network_to_host(T value)
{
    static_assert(std::is_enum<T>::value, "Wrong type: Not an enum");
    using U = std::underlying_type_t<T>;

    return private_::network_to_host<T, U>(private_::is_16_bit<U>(), value);
}

template <class T> auto network_to_host_underlying(T value)
{
    static_assert(std::is_enum<T>::value, "Wrong type: Not an enum");
    using U = std::underlying_type_t<T>;

    return static_cast<U>(private_::network_to_host<T, U>(private_::is_16_bit<U>(), value));
}

template <class E, class T> E network_to_host_enum(T value)
{
    using U = std::underlying_type_t<E>;

    static_assert(std::is_enum<E>::value, "Wrong type: Not an enum");
    static_assert(std::is_same<U, T>::value, "Wrong type");

    return private_::network_to_host<E, U>(private_::is_16_bit<U>(), static_cast<E>(value));
}

template <class T> T host_to_network(T value)
{
    static_assert(std::is_enum<T>::value, "Wrong type: Not an enum");
    using U = std::underlying_type_t<T>;

    return private_::host_to_network<T, U>(private_::is_16_bit<U>(), value);
}

template <class T> auto host_to_network_underlying(T value)
{
    static_assert(std::is_enum<T>::value, "Wrong type: Not an enum");
    using U = std::underlying_type_t<T>;

    return static_cast<U>(private_::host_to_network<T, U>(private_::is_16_bit<U>(), value));
}

template <class E, class T> E host_to_network_enum(T value)
{
    using U = typename std::underlying_type_t<E>;

    static_assert(std::is_enum<E>::value, "Wrong type: Not an enum");
    static_assert(std::is_same<U, T>::value, "Wrong type");

    return private_::host_to_network<E, U>(private_::is_16_bit<U>(), static_cast<E>(value));
}

#endif
