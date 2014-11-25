
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

#ifndef ROUTE_INFO_HPP_
#define ROUTE_INFO_HPP_

#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <string>

#include <sys/socket.h>
#include <linux/rtnetlink.h>

namespace drop {

enum class RouteOrigin: uint8_t
{
    Unknown = RTPROT_UNSPEC,
    Kernel = RTPROT_KERNEL,
    Boot = RTPROT_BOOT,
    Static = RTPROT_STATIC,
    Zebra = RTPROT_ZEBRA
};

RouteOrigin route_origin(const std::string& value);
std::string route_origin(RouteOrigin origin);

struct RouteInfo
{
	RouteInfo();
    RouteInfo(const RouteInfo& ri) = default;

    uint32_t prefix;
    uint32_t destination;
    uint32_t gateway;
    uint32_t port_index;
    uint32_t metric;
    uint32_t source;

    RouteOrigin origin;
};

bool operator==(const RouteInfo& r1, const RouteInfo& r2);

std::ostream& operator<<(std::ostream& os, RouteOrigin origin);
std::ostream& operator<<(std::ostream& os, const RouteInfo& route);

} // namespace drop

#endif
