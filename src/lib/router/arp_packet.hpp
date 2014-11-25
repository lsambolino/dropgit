
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

#ifndef DROP_ROUTER_ARP_HEADER_HPP_
#define DROP_ROUTER_ARP_HEADER_HPP_

#include <cstdint>

#include <boost/asio.hpp>

#include "endianness.hpp"

namespace drop {

struct ArpPacket
{
    ArpPacket() : htype(htons(1)), ptype(htons(0x0800)) {}

    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen = 6;
    uint8_t plen = 4;
    uint16_t operation = 0;

    uint16_t sha0 = 0;
    uint16_t sha1 = 0;
    uint16_t sha2 = 0;

    uint16_t spa0 = 0;
    uint16_t spa1 = 0;

    uint16_t tha0 = 0;
    uint16_t tha1 = 0;
    uint16_t tha2 = 0;

    uint16_t tpa0 = 0;
    uint16_t tpa1 = 0;
};

} // namespace drop

#endif
