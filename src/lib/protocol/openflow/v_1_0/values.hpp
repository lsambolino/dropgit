
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

#ifndef DROP_PROTOCOL_OPENFLOW_VALUES_1_0_HPP_
#define DROP_PROTOCOL_OPENFLOW_VALUES_1_0_HPP_

#include "protocol/openflow/values.hpp"

#include <boost/asio.hpp>

namespace drop {
namespace protocol {

enum class ofp_type_1_0: uint8_t
{
	// Immutable messages.
	HELLO, // Symmetric message
	ERROR, // Symmetric message
	ECHO_REQUEST, // Symmetric message
	ECHO_REPLY, // Symmetric message
	VENDOR, // Symmetric message

	// Switch configuration messages.
	FEATURES_REQUEST,    // Controller/switch message
	FEATURES_REPLY,      // Controller/switch message
	GET_CONFIG_REQUEST,  // Controller/switch message
	GET_CONFIG_REPLY,    // Controller/switch message
	SET_CONFIG,          // Controller/switch message

	// Asynchronous messages.
	PACKET_IN, // Async message
	FLOW_REMOVED, // Async message
	PORT_STATUS, // Async message

	// Controller command messages.
	PACKET_OUT, // Controller/switch message
	FLOW_MOD, // Controller/switch message
	PORT_MOD, // Controller/switch message

	// Statistics messages.
	STATS_REQUEST, // Controller/switch message
	STATS_REPLY, // Controller/switch message

	// Barrier messages.
	BARRIER_REQUEST, // Controller/switch message
	BARRIER_REPLY, // Controller/switch message

	// Queue Configuration messages.
	QUEUE_GET_CONFIG_REQUEST,  // Controller/switch message
	QUEUE_GET_CONFIG_REPLY     // Controller/switch message
};

// Modify Flow Entry Message
enum class ofp_flow_mod_command_1_0: uint16_t
{
	ADD = 0,            // New flow.
	MODIFY = 1,         // Modify all matching flows.
	MODIFY_STRICT = 2,  // Modify entry strictly matching wildcards
	DELETE = 3,         // Delete all matching flows.
	DELETE_STRICT = 4,  // Strictly match wildcards and priority.
};

enum class ofp_error_type_1_0: uint16_t
{
	HELLO_FAILED,     // Hello protocol failed.
	BAD_REQUEST,      // Request was not understood.
	BAD_ACTION,       // Error in action description.
	FLOW_MOD_FAILED,  // Problem modifying flow entry.
	PORT_MOD_FAILED,  // Port mod request failed.
	QUEUE_OP_FAILED   // Queue operation failed.
};

enum class ofp_bad_request_code_1_0: uint16_t
{
	BAD_VERSION,     // ofp_header.version not supported.
	BAD_TYPE,        // ofp_header.type not supported.
	BAD_STAT,        // ofp_stats_request.type not supported.
	BAD_VENDOR,      // Vendor not supported (in ofp_vendor_header or ofp_stats_request or ofp_stats_reply).
	BAD_SUBTYPE,     // Vendor subtype not supported.
	PERMISSION,      // Permissions error.
	BAD_LEN,         // Wrong request length for type.
	BUFFER_EMPTY,    // Specified buffer has already been used.
	BUFFER_UNKNOWN   // Specified buffer does not exist.
};

enum class ofp_bad_action_code_1_0: uint16_t
{
	BAD_TYPE,        // Unknown action type.
	BAD_LEN,         // Length problem in actions.
	BAD_VENDOR,      // Unknown vendor id specified.
	BAD_VENDOR_TYPE, // Unknown action type for vendor id.
	BAD_OUT_PORT,    // Problem validating output action.
	BAD_ARGUMENT,    // Bad action argument.
	PERMISSION,      // Permissions error.
	TOO_MANY,        // Can’t handle this many actions.
	BAD_QUEUE        // Problem validating output queue.
};

enum class ofp_flow_mod_failed_code_1_0: uint16_t
{
	ALL_TABLES_FULL,    // Flow not added because of full tables.
	OVERLAP,            // Attempted to add overlapping flow with CHECK_OVERLAP flag set.
	PERMISSION,         // Permissions error.
	BAD_EMERG_TIMEOUT,  // Flow not added because of non-zero idle/hard timeout.
	BAD_COMMAND,        // Unknown command.
	UNSUPPORTED         // Unsupported action list - cannot process in the order specified.
};

enum class ofp_stats_types_1_0: uint16_t
{
    DESC,           // Description of this OpenFlow switch. The request body is empty. The reply body is struct ofp_desc_stats_1_0.
    FLOW,           // Individual flow statistics. The request body is struct ofp_flow_stats_request_1_0. The reply body is an array of struct ofp_flow_stats_1_0.
    AGGREGATE,      //Aggregate flow statistics. The request body is struct ofp_aggregate_stats_request_1_0. The reply body is struct ofp_aggregate_stats_reply_1_0.
    TABLE,          // Flow table statistics. The request body is empty. The reply body is an array of struct ofp_table_stats_1_0.
    PORT,           // Physical port statistics. The request body is struct ofp_port_stats_request_1_0. The reply body is an array of struct ofp_port_stats_1_0.
    QUEUE,          // Queue statistics for a port The request body defines the port The reply body is an array of struct ofp_queue_stats_1_0
    VENDOR = 0xffff // Vendor extension. The request and reply bodies begin with a 32-bit vendor ID, which takes the same form as in "struct ofp_vendor_header".
                    // The request and reply bodies are otherwise vendor-defined.
};

enum class ofp_stats_flag_1_0: uint16_t
{
    // None yet defined.
};

// Flow wildcards.
enum class ofp_flow_wildcards_1_0: uint32_t
{
	IN_PORT = 1 << 0,  // Switch input port.
	DL_VLAN = 1 << 1,  // VLAN id.
	DL_SRC = 1 << 2,  // Ethernet source address.
	DL_DST = 1 << 3,  // Ethernet destination address.
	DL_TYPE = 1 << 4,  // Ethernet frame type.
	NW_PROTO = 1 << 5,  // IP protocol.
	TP_SRC = 1 << 6,  // TCP/UDP source port.
	TP_DST = 1 << 7,  // TCP/UDP destination port.

	// IP source address wildcard bit count.  0 is exact match, 1 ignores the
	// LSB, 2 ignores the 2 least-significant bits, ..., 32 and higher wildcard
	// the entire field.  This is the *opposite* of the usual convention where
	// e.g. /24 indicates that 8 bits (not 24 bits) are wildcarded.
	NW_SRC_SHIFT = 8,
	NW_SRC_BITS = 6,
	NW_SRC_MASK = ((1 << NW_SRC_BITS) - 1) << NW_SRC_SHIFT,
	NW_SRC_ALL = 32 << NW_SRC_SHIFT,
	// IP destination address wildcard bit count.  Same format as source.
	NW_DST_SHIFT = 14,
	NW_DST_BITS = 6,
	NW_DST_MASK = ((1 << NW_DST_BITS) - 1) << NW_DST_SHIFT,
	NW_DST_ALL = 32 << NW_DST_SHIFT,
	DL_VLAN_PCP = 1 << 20,  // VLAN priority.
	NW_TOS = 1 << 21,  // IP ToS (DSCP field, 6 bits).
	// Wildcard all fields.
	ALL = ((1 << 22) - 1)
};

enum class ofp_action_type_1_0: uint16_t
{
    OUTPUT,       // Output to switch port.
    SET_VLAN_VID, // Set the 802.1q VLAN id.
    SET_VLAN_PCP, // Set the 802.1q priority.
    STRIP_VLAN,   // Strip the 802.1q header.
    SET_DL_SRC,   // Ethernet source address.
    SET_DL_DST,   // Ethernet destination address.
    SET_NW_SRC,   // IP source address.
    SET_NW_DST,   // IP destination address.
    SET_NW_TOS,   // IP ToS (DSCP field, 6 bits).
    SET_TP_SRC,   // TCP/UDP source port.
    SET_TP_DST,   // TCP/UDP destination port.
    ENQUEUE,      // Output to queue.
    VENDOR = 0xffff
};

enum class ofp_port_1_0: uint16_t
{
	// Maximum number of physical switch ports.
	MAX = 0xff00,

	// Fake output "ports".
	IN_PORT = 0xfff8,  // Send the packet out the input port.  This virtual port must be explicitly used in order to send back out of the input port.
	TABLE = 0xfff9,  // Perform actions in flow table. NB: This can only be the destination port for packet-out messages.
	NORMAL = 0xfffa,  // Process with normal L2/L3 switching.
	FLOOD = 0xfffb,  // All physical ports except input port and those disabled by STP.
	ALL = 0xfffc,  // All physical ports except input port.
	CONTROLLER = 0xfffd,  // Send to controller.
	LOCAL = 0xfffe,  // Local openflow "port".
	NONE = 0xffff   // Not associated with a physical port.
};

enum class ofp_flow_mod_flags_1_0: uint16_t
{
	SEND_FLOW_REM = 1 << 0,  // Send flow removed message when flow expires or is deleted.
	CHECK_OVERLAP = 1 << 1,  // Check for overlapping entries first.
	EMERG = 1 << 2   // Remark this is for emergency.
};

enum class ofp_port_state_1_0: uint32_t
{
	LINK_DOWN = 1 << 0, // No physical link present.
	// The OFPPS_STP_// bits have no effect on switch operation.  The
	// controller must adjust OFPPC_NO_RECV, OFPPC_NO_FWD, and
	// OFPPC_NO_PACKET_IN appropriately to fully implement an 802.1D spanning tree.
	STP_LISTEN = 0 << 8, // Not learning or relaying frames.
	STP_LEARN = 1 << 8, // Learning but not relaying frames.
	STP_FORWARD = 2 << 8, // Learning and relaying frames.
	STP_BLOCK = 3 << 8, // Not part of spanning tree.
	STP_MASK = 3 << 8  // Bit mask for OFPPS_STP_// values.
};

enum class ofp_packet_in_reason_1_0: uint8_t
{
	NO_MATCH =0,   // No matching flow.
	ACTION = 1,    // Action explicitly output to controller.
};

} // namespace protocol
} // namespace drop

#endif
