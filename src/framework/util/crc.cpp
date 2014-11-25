
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

#include "crc.hpp"

namespace tnt {

uint32_t crc32(const uint8_t* data, size_t len)
{
    unsigned int crc_table[] =
    {
        0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0,
        0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320,
        0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190,
        0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000
    };

    uint32_t crc = 0;

    for (auto n=0u; n<len; ++n)
    {
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[n] >> 0)) & 0x0F];  // Lower nibble
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[n] >> 4)) & 0x0F];  // Upper nibble
    }

    return crc;
}

uint32_t crc32(const std::string& str)
{
    return crc32(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

uint16_t crc16(const uint8_t* packet, size_t len)
{
	const uint16_t* ptr;

#if defined(__i386__) || defined(__x86_64__) || defined(_WIN32)
	ptr = reinterpret_cast<const uint16_t*>(packet);
#elif defined(__mips__)
	// This is like saying: "Yes, I know what I am doing. Do as I say."
    ptr = reinterpret_cast<const uint16_t*>(reinterpret_cast<const void*>(packet));
#endif

	uint32_t sum = 0;

	while (len > 1)
	{
	    sum += *ptr++;
	    len -= 2;
	}

	if (len == 1)
    {
	    sum += *reinterpret_cast<const uint8_t*>(ptr);
    }

	while (sum >> 16)
    {
	    sum = (sum & 0xFFFF) + (sum >> 16);
    }

	return static_cast<uint16_t>(~sum);
}

uint16_t crc16(const std::string& str)
{
    return crc16(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

} // namespace tnt
