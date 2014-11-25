
/*

Copyright (c) 2013, Giulio Scancarello (giulioscanca@hotmail.it), Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#ifndef DROP_PROTOCOL_OPENFLOW_VALUES_HPP_
#define DROP_PROTOCOL_OPENFLOW_VALUES_HPP_

#include <cstdint>

namespace drop {
namespace protocol {

static const auto OFP_ETH_ALEN = 6; // Bytes in an Ethernet address.
static const auto OFP_MAX_PORT_NAME_LEN = 16;
static const auto OFP_NO_BUFFER = 0xffffffff; // Special buffer-id to indicate "no buffer"
static const auto OFP_DEFAULT_PRIORITY = 32768; // Special buffer-id to indicate "no buffer"
static const auto OFP_MAX_TABLE_NAME_LEN = 32;
static const auto DESC_STR_LEN = 256;
static const auto SERIAL_NUM_LEN = 32;

enum ofp_hello_failed_code
{
	OFPHFC_INCOMPATIBLE = 0,    // No compatible version.
	OFPHFC_EPERM = 1,           // Permissions error.
};

enum class ofp_config_flags: uint16_t
{
	// Handling of IP fragments.
	FRAG_NORMAL = 0, // No special handling for fragments.
	FRAG_DROP = 1,   // Drop fragments.
	FRAG_REASM = 2,  // Reassemble (only if OFPC_IP_REASM set).
	FRAG_MASK = 3
};

enum class ofp_port_reason: uint8_t
{
    ADD,      //The port was added.
    DELETE,   // The port was removed.
    MODIFY    // Some attribute of the port has changed.
};

} // namespace protocol
} // namespace drop

#endif
