
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

/*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Message Type                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           Length                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                            Data                                |
   ~                                                                ~
   ~                                                                ~
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    0                   1                   2                   3
                            Data Header
*/

#ifndef DROP_PROTOCOL_DROP_STRUCTS_HPP_
#define DROP_PROTOCOL_DROP_STRUCTS_HPP_

#include <cstdint>

#include <arpa/inet.h>

#include "protocol/drop/values.hpp"

#include "endianness.hpp"

namespace drop {
namespace protocol {

class MessageHeader
{
private:
    struct drop_message_hdr_t
    {
        drop_message_hdr_t(uint32_t type, uint32_t len): type(type), length(len) {}

        uint32_t type;
        uint32_t length;        //! Message lenght in bytes, including the header.
    };
public:
    static const uint32_t hdr_len = sizeof(drop_message_hdr_t);

    MessageHeader(DropMessage type, uint32_t data_len): hdr_(htonl(static_cast<uint32_t>(type)), htonl(data_len + hdr_len)) {}

    DropMessage type() const
    {
        return static_cast<DropMessage>(ntohl(hdr_.type));
    }

    uint32_t total_length() const
    {
        return ntohl(hdr_.length);
    }

    uint32_t data_length() const
    {
        return total_length() - hdr_len;
    }

    std::string str() const
    {
        const char* hdr = reinterpret_cast<const char*>(&hdr_);

        return std::string(hdr, hdr_len);
    }
private:
    drop_message_hdr_t hdr_;
};

} // namespace protocol
} // namespace drop

#endif

