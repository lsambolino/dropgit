
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

#ifndef DROP_PROTOCOL_OPENFLOW_VALUES_1_3_HPP_
#define DROP_PROTOCOL_OPENFLOW_VALUES_1_3_HPP_

#include "protocol/openflow/values.hpp"

namespace drop {
namespace protocol {

enum class ofp_type_1_3 : uint8_t
{
	// Immutable messages. 
	HELLO= 0,  // Symmetric message 
	ERROR = 1,  // Symmetric message 
	ECHO_REQUEST = 2,  // Symmetric message 
	ECHO_REPLY = 3,  // Symmetric message 
	EXPERIMENTER = 4,  // Symmetric message 
	
	// Switch configuration messages.
	FEATURES_REQUEST = 5, // Controller/switch message  
	FEATURES_REPLY = 6, // Controller/switch message  
	GET_CONFIG_REQUEST = 7, // Controller/switch message  
	GET_CONFIG_REPLY = 8, // Controller/switch message  
	SET_CONFIG = 9, // Controller/switch message

	// Asynchronous messages.
	PACKET_IN = 10, // Async message 
	FLOW_REMOVED = 11, // Async message 
	PORT_STATUS = 12, // Async message 

	// Controller command messages. 
	PACKET_OUT = 13, // Controller/switch message 
	FLOW_MOD = 14, // Controller/switch message 
	GROUP_MOD = 15, // Controller/switch message 
	PORT_MOD = 16, // Controller/switch message 
	TABLE_MOD = 17, // Controller/switch message 

	// Multipart messages. 
	MULTIPART_REQUEST = 18, // Controller/switch message 
	MULTIPART_REPLY = 19, // Controller/switch message 

	// Barrier messages. 
	BARRIER_REQUEST = 20, // Controller/switch message 
	BARRIER_REPLY = 21, // Controller/switch message 

	// Queue Configuration messages. 
	QUEUE_GET_CONFIG_REQUEST = 22,  // Controller/switch message 
	QUEUE_GET_CONFIG_REPLY = 23,  // Controller/switch message 

	// Controller role change request messages. 
	ROLE_REQUEST = 24, // Controller/switch message 
	ROLE_REPLY = 25, // Controller/switch message

	// Asynchronous message configuration. 
	GET_ASYNC_REQUEST = 26, // Controller/switch message 
	GET_ASYNC_REPLY = 27, // Controller/switch message 
	SET_ASYNC = 28, // Controller/switch message 

	// Meters and rate limiters configuration messages. 
	METER_MOD = 29, // Controller/switch message 
};

// Modify Flow Entry Message
enum class ofp_flow_mod_command_1_3: uint8_t
{
	ADD = 0,            // New flow.
	MODIFY = 1,         // Modify all matching flows.
	MODIFY_STRICT = 2,  // Modify entry strictly matching wildcards
	DELETE = 3,         // Delete all matching flows.
	DELETE_STRICT = 4,  // Strictly match wildcards and priority.
};

enum class ofp_error_type_1_3: uint16_t
{
	HELLO_FAILED = 0,  // Hello protocol failed. 
	BAD_REQUEST = 1,  // Request was not understood. 
	BAD_ACTION = 2,  // Error in action description. 
	BAD_INSTRUCTION = 3,  // Error in instruction list. 
	BAD_MATCH = 4,  // Error in match. 
	FLOW_MOD_FAILED = 5,  // Problem modifying flow entry. 
	GROUP_MOD_FAILED = 6,  // Problem modifying group entry. 
	PORT_MOD_FAILED = 7,  // Port mod request failed. 
	TABLE_MOD_FAILED = 8,  // Table mod request failed. 
	QUEUE_OP_FAILED = 9,  // Queue operation failed. 
	SWITCH_CONFIG_FAILED = 10, // Switch config request failed. 
	ROLE_REQUEST_FAILED = 11, // Controller Role request failed. 
	METER_MOD_FAILED = 12, // Error in meter. 
	TABLE_FEATURES_FAILED = 13, // Setting table features failed. 
	EXPERIMENTER = 0xffff      // Experimenter error messages. 
};

enum class ofp_bad_request_code_1_3: uint16_t
{
	BAD_VERSION = 0,  // ofp_header.version not supported. 
	BAD_TYPE = 1,  // ofp_header.type not supported. 
	BAD_MULTIPART = 2,  // ofp_multipart_request.type not supported. 
	BAD_EXPERIMENTER = 3,  // Experimenter id not supported (in ofp_experimenter_header or ofp_multipart_request or ofp_multipart_reply). 
	BAD_EXP_TYPE = 4,  // Experimenter type not supported. 
	PERMISSION = 5,// Permissions error. 
	BAD_LEN = 6,// Wrong request length for type. 
	BUFFER_EMPTY = 7,// Specified buffer has already been used. 
	BUFFER_UNKNOWN = 8,// Specified buffer does not exist. 
	BAD_TABLE_ID = 9,// Specified table-id invalid or does not exist. 
	IS_SLAVE = 10,// Denied because controller is slave.  
	BAD_PORT = 11,// Invalid port. 
	BAD_PACKET = 12,// Invalid packet in packet-out. 
	MULTIPART_BUFFER_OVERFLOW = 13, // ofp_multipart_request overflowed the assigned buffer. 
};

enum class ofp_bad_action_code_1_3: uint16_t
{
	BAD_TYPE = 0,// Unknown or unsupported action type.  
	BAD_LEN = 1,// Length problem in actions. 
	BAD_EXPERIMENTER = 2,// Unknown experimenter id specified. 
	BAD_EXP_TYPE = 3,// Unknown action for experimenter id.  
	BAD_OUT_PORT = 4,// Problem validating output port.  
	BAD_ARGUMENT = 5,// Bad action argument. 
	PERMISSION = 6,// Permissions error. 
	TOO_MANY = 7,// Can’t handle this many actions. 
	BAD_QUEUE = 8,// Problem validating output queue. 
	BAD_OUT_GROUP = 9,// Invalid group id in forward action. 
	MATCH_INCONSISTENT = 10, // Action can’t apply for this match, or Set-Field missing prerequisite. 
	UNSUPPORTED_ORDER = 11, // Action order is unsupported for the action list in an Apply-Actions instruction 
	BAD_TAG = 12,// Actions uses an unsupported tag/encap. 
	BAD_SET_TYPE = 13,// Unsupported type in SET_FIELD action.  
	BAD_SET_LEN = 14,// Length problem in SET_FIELD action.  
	BAD_SET_ARGUMENT = 15,// Bad argument in SET_FIELD action. 
};

enum class ofp_flow_mod_failed_code_1_3: uint16_t
{
	UNKNOWN = 0, // Unspecified error. 
	TABLE_FULL = 1, // Flow not added because table was full. 
	BAD_TABLE_ID = 2, // Table does not exist 
	OVERLAP = 3, // Attempted to add overlapping flow with CHECK_OVERLAP flag set. 
	PERMISSION = 4, // Permissions error. 
	BAD_TIMEOUT = 5, // Flow not added because of unsupported idle/hard timeout. 
	BAD_COMMAND = 6, // Unsupported or unknown command. 
	BAD_FLAGS = 7, // Unsupported or unknown flags. 
};

enum class ofp_bad_instruction_code_1_3: uint16_t
{
    UNKNOWN_INST = 0, // Unknown instruction.
    UNSUP_INST = 1, // Switch or table does not support the instruction.
    BAD_TABLE_ID = 2, // Invalid Table-ID specified.
    UNSUP_METADATA = 3, // Metadata value unsupported by datapath.
    UNSUP_METADATA_MASK = 4, // Metadata mask value unsupported by datapath.
    BAD_EXPERIMENTER = 5, // Unknown experimenter id specified.
    BAD_EXP_TYPE = 6, // Unknown instruction for experimenter id.
    BAD_LEN = 7, // Length problem in instructions.
    PERMISSION = 8, // Permissions error.
};

enum class ofp_port_mod_failed_code_1_3: uint16_t
{
    BAD_PORT = 0, // Specified port number does not exist.
    BAD_HW_ADDR = 1, // Specified hardware address does not match the port number.
    BAD_CONFIG = 2, // Specified config is invalid.
    BAD_ADVERTISE = 3, // Specified advertise is invalid.
    PERMISSION = 4, // Permissions error.
};

enum class ofp_switch_config_failed_code_1_3: uint16_t
{
    BAD_FLAGS = 0, // Specified flags is invalid.
    BAD_LEN = 1, // Specified len is invalid.
    PERMISSION = 2, // Permissions error.
};

enum class ofp_meter_mod_failed_code_1_3: uint16_t
{
    UNKNOWN = 0, // Unspecified error.
    METER_EXISTS = 1, // Meter not added because a Meter ADD attempted to replace an existing Meter.
    INVALID_METER = 2, // Meter not added because Meter specified is invalid.
    UNKNOWN_METER = 3, // Meter not modified because a Meter MODIFY attempted to modify a non-existent Meter.
    BAD_COMMAND = 4, // Unsupported or unknown command.
    BAD_FLAGS = 5, // Flag configuration unsupported.
    BAD_RATE = 6, // Rate unsupported.
    BAD_BURST = 7, // Burst size unsupported.
    BAD_BAND = 8, // Band unsupported.
    BAD_BAND_VALUE = 9, // Band value unsupported.
    OUT_OF_METERS = 10, // No more meters available.
    OUT_OF_BANDS = 11, // The maximum number of properties for a meter has been exceeded.
};

enum class ofp_bad_match_code_1_3: uint16_t
{
    BAD_TYPE = 0, // Unsupported match type specified by the match
    BAD_LEN = 1, // Length problem in match.
    BAD_TAG = 2, // Match uses an unsupported tag/encap.
    BAD_DL_ADDR_MASK = 3, // Unsupported datalink addr mask - switch does not support arbitrary datalink address mask.
    BAD_NW_ADDR_MASK = 4, // Unsupported network addr mask - switch does not support arbitrary network address mask.
    BAD_WILDCARDS = 5, // Unsupported combination of fields masked or omitted in the match.
    BAD_FIELD = 6, // Unsupported field type in the match.
    BAD_VALUE = 7, // Unsupported value in a match field.
    BAD_MASK = 8, // Unsupported mask specified in the match, field is not dl-address or nw-address.
    BAD_PREREQ = 9, // A prerequisite was not met.
    DUP_FIELD = 10, // A field type was duplicated.
    PERMISSION = 11 // Permissions error.
};

enum class ofp_action_type_1_3: uint16_t 
{
	OUTPUT = 0,         // Output to switch port. 
	COPY_TTL_OUT = 11,  // Copy TTL "outwards" -- from next-to-outermost to outermost
	COPY_TTL_IN = 12,   // Copy TTL "inwards" from outermost to next-to-outermost
	SET_MPLS_TTL = 15,  // MPLS TTL
	DEC_MPLS_TTL = 16,  // Decrement MPLS TTL
	PUSH_VLAN = 17,     // Push a new VLAN tag
	POP_VLAN = 18,      // Pop the outer VLAN tag  
	PUSH_MPLS = 19,     // Push a new MPLS tag  
	POP_MPLS = 20,      // Pop the outer MPLS tag  
	SET_QUEUE = 21,     // Set queue id when outputting to a port  
	GROUP = 22,         // Apply group.  
	SET_NW_TTL = 23,    // IP TTL.  
	DEC_NW_TTL = 24,    // Decrement IP TTL.  
	SET_FIELD = 25,     // Set a header field using OXM TLV format.  
	PUSH_PBB = 26,      // Push a new PBB service tag (I-TAG)
	POP_PBB = 27,       // Pop the outer PBB service tag (I-TAG)  
	EXPERIMENTER = 0xffff
};

enum class ofp_port_config_1_3: uint32_t
{
	PORT_DOWN = 1 << 0,     // Port is administratively down.
	NO_RECV = 1 << 2,       // Drop all packets received by port.
	NO_FWD = 1 << 5,        // Drop packets forwarded to port.
	NO_PACKET_IN = 1 << 6   // Do not send packet-in msgs for port.
};

enum class ofp_port_state_1_3: uint32_t
{
	LINK_DOWN = 1 << 0, // No physical link present. 
	BLOCKED = 1 << 1,   // Port is blocked 
	LIVE = 1 << 2,      // Live for Fast Failover Group. 
};

enum class ofp_port_no_1_3: uint32_t
{
	MAX = 0xffffff00,           // Maximum number of physical and logical switch ports.

	// Reserved OpenFlow Port (fake output "ports"). 
	IN_PORT = 0xfffffff8,       // Send the packet out the input port.  This reserved port must be explicitly used in order to send back out of the input port. 
	TABLE = 0xfffffff9,         // Submit the packet to the first flow table NB: This destination port can only be used in packet-out messages. 
	NORMAL = 0xfffffffa,        //Forward using non-OpenFlow pipeline. 
	FLOOD = 0xfffffffb,         //Flood using non-OpenFlow pipeline. 
	ALL = 0xfffffffc,           //All standard ports except input port.
	CONTROLLER = 0xfffffffd,    // Send to controller. 
	LOCAL = 0xfffffffe,         // Local openflow "port". 
	ANY = 0xffffffff            // Special value used in some requests when no port is specified (i.e.wildcarded).
};

enum class ofp_group_no_1_3: uint32_t
{
	ANY = 0xffffffff // Special value used in some requests when no port is specified (i.e.wildcarded).
};

// Features of ports available in a datapath. 
enum class ofp_port_features_1_3: uint32_t
{
	SPEED_10MB_HD = 1 << 0, // 10 Mb half-duplex rate support.  
	SPEED_10MB_FD = 1 << 1, // 10 Mb full-duplex rate support.  
	SPEED_100MB_HD = 1 << 2, // 100 Mb half-duplex rate support.  
	SPEED_100MB_FD = 1 << 3, // 100 Mb full-duplex rate support.  
	SPEED_1GB_HD = 1 << 4, // 1 Gb half-duplex rate support. 
	SPEED_1GB_FD = 1 << 5, // 1 Gb full-duplex rate support.  
	SPEED_10GB_FD = 1 << 6, // 10 Gb full-duplex rate support. 
	SPEED_40GB_FD = 1 << 7, // 40 Gb full-duplex rate support.  
	SPEED_100GB_FD = 1 << 8, // 100 Gb full-duplex rate support.  
	SPEED_1TB_FD = 1 << 9, // 1 Tb full-duplex rate support. 
	OTHER = 1 << 10, // Other rate, not in the list. 

	COPPER = 1 << 11, // Copper medium.  
	FIBER = 1 << 12, // Fiber medium. 
	AUTONEG = 1 << 13, // Auto-negotiation. 
	PAUSE = 1 << 14, // Pause. 
	PAUSE_ASYM = 1 << 15  // Asymmetric pause. 
};

enum class ofp_capabilities_1_3: uint32_t
{
	FLOW_STATS = 1 << 0,    // Flow statistics.
	TABLE_STATS = 1 << 1,   // Table statistics.
	PORT_STATS = 1 << 2,    // Port statistics.
	GROUP_STATS = 1 << 3,   // Group statistics.
	IP_REASM = 1 << 5,      // Can reassemble IP fragments.
	QUEUE_STATS = 1 << 6,   // Queue statistics.
	PORT_BLOCKED = 1 << 8   // Switch will block looping ports.
};

// Flow wildcards.
enum class ofp_flow_wildcards_1_3: uint32_t
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
	// Wildcard all fields.
	ALL = ((1 << 20) - 1)
};

enum class ofp_flow_mod_flags_1_3: uint16_t
{
	SEND_FLOW_REM = 1 << 0,  // Send flow removed message when flow expires or is deleted.
	CHECK_OVERLAP = 1 << 1,  // Check for overlapping entries first.
	RESET_COUNTS = 1 << 2,  // Reset flow packet and byte counts. 
	NO_PKT_COUNTS = 1 << 3,  // Don’t keep track of packet count. 
	NO_BYT_COUNTS = 1 << 4,  // Don’t keep track of byte count. 
};

enum class ofp_multipart_request_flags_1_3: uint16_t
{
	REQ_MORE = 1 << 0  // More requests to follow.
};

enum class ofp_multipart_reply_flags_1_3: uint16_t
{
	REPLY_MORE = 1 << 0  // More replies to follow.
};

enum class ofp_multipart_type_1_3: uint16_t
{
	// Description of this OpenFlow switch.
	// The request body is empty.
	// The reply body is struct ofp_desc. 
	DESC = 0,
	// Individual flow statistics.
	// The request body is struct ofp_flow_stats_request.
	// The reply body is an array of struct ofp_flow_stats. 
	FLOW = 1,
	// Aggregate flow statistics.
	// The request body is struct ofp_aggregate_stats_request.
	// The reply body is struct ofp_aggregate_stats_reply. 
	AGGREGATE = 2,
	// Flow table statistics.
	// The request body is empty.
	// The reply body is an array of struct ofp_table_stats. 
	TABLE = 3,
	// Port statistics.
	// The request body is struct ofp_port_stats_request.
	// The reply body is an array of struct ofp_port_stats. 
	PORT_STATS = 4,
	// Queue statistics for a port
	// The request body is struct ofp_queue_stats_request.
	// The reply body is an array of struct ofp_queue_stats 
	QUEUE = 5,
	// Group counter statistics.
	// The request body is struct ofp_group_stats_request.
	// The reply is an array of struct ofp_group_stats. 
	GROUP = 6,
	// Group description.
	// The request body is empty.
	// The reply body is an array of struct ofp_group_desc. 
	GROUP_DESC = 7,
	// Group features.
	// The request body is empty.
	// The reply body is struct ofp_group_features. 
	GROUP_FEATURES = 8,
	// Meter statistics.
	// The request body is struct ofp_meter_multipart_requests.
	// The reply body is an array of struct ofp_meter_stats. 
	METER = 9,
	// Meter configuration.
	// The request body is struct ofp_meter_multipart_requests.
	// The reply body is an array of struct ofp_meter_config. 
	METER_CONFIG = 10,
	// Meter features.
	// The request body is empty.
	// The reply body is struct ofp_meter_features. 
	METER_FEATURES = 11,
	// Table features.
	// The request body is either empty or contains an array of
	// struct ofp_table_features containing the controller’s
	// desired view of the switch. If the switch is unable to
	// set the specified view an error is returned.
	// The reply body is an array of struct ofp_table_features. 
	TABLE_FEATURES = 12,
	// Port description.
	// The request body is empty.
	// The reply body is an array of struct ofp_port. 
	PORT_DESC = 13,
	// Experimenter extension.
	// The request and reply bodies begin with
	// struct ofp_experimenter_multipart_header.
	// The request and reply bodies are otherwise experimenter-defined. 
	EXPERIMENTER = 0xffff
};

enum class ofp_match_type_1_3: uint16_t
{
	STANDARD = 0,   // Deprecated. 
	OXM = 1,        // OpenFlow Extensible Match 
};

// OXM Class IDs.
// The high order bit differentiate reserved classes from member classes.
// Classes 0x0000 to 0x7FFF are member classes, allocated by ONF.
// Classes 0x8000 to 0xFFFE are reserved classes, reserved for standardisation.
enum class ofp_oxm_class_1_3: uint16_t
{
    NXM_0 = 0x0000,          // Backward compatibility with NXM
    NXM_1 = 0x0001,          // Backward compatibility with NXM
    OPENFLOW_BASIC = 0x8000, // Basic class for OpenFlow 
    EXPERIMENTER = 0xFFFF,   // Experimenter class
};

enum class ofp_packet_in_reason_1_3: uint8_t
{
	NO_MATCH =0,     // No matching flow.
	ACTION = 1,        // Action explicitly output to controller.
	INVALID_TTL = 2,	// Packet has invalid TTL
};

enum class ofp_table_1_3: uint8_t
{
    MAX = 0xfe,   // Last usable table number.

    // Fake tables.
    ALL = 0xff    // Wildcard table used for table config, flow stats and flow deletes. */
};

enum class ofp_instruction_type_1_3: uint16_t
{
    GOTO_TABLE = 1,       // Setup the next table in the lookup pipeline
    WRITE_METADATA = 2,   // Setup the metadata field for use later in pipeline
    WRITE_ACTIONS = 3,    // Write the action(s) onto the datapath action set
    APPLY_ACTIONS = 4,    // Applies the action(s) immediately
    CLEAR_ACTIONS = 5,    // Clears all actions from the datapath action set
    METER = 6,            // Apply meter (rate limiter)
    EXPERIMENTER = 0xFFFF // Experimenter instruction
};

enum class oxm_ofb_match_fields_1_3: uint8_t
{
    IN_PORT = 0, // Switch input port.
    IN_PHY_PORT = 1, // Switch physical input port.
    METADATA = 2, // Metadata passed between tables.
    ETH_DST = 3, // Ethernet destination address.
    ETH_SRC = 4, // Ethernet source address.
    ETH_TYPE = 5, // Ethernet frame type.
    VLAN_VID = 6, // VLAN id.
    VLAN_PCP = 7, // VLAN priority.
    IP_DSCP = 8, // IP DSCP (6 bits in ToS field).
    IP_ECN = 9, // IP ECN (2 bits in ToS field).
    IP_PROTO = 10, // IP protocol.
    IPV4_SRC = 11, // IPv4 source address.
    IPV4_DST = 12, // IPv4 destination address.
    TCP_SRC = 13, // TCP source port.
    TCP_DST = 14, // TCP destination port.
    UDP_SRC = 15, // UDP source port.
    UDP_DST = 16, // UDP destination port.
    SCTP_SRC = 17, // SCTP source port.
    SCTP_DST = 18, // SCTP destination port.
    ICMPV4_TYPE = 19, // ICMP type.
    ICMPV4_CODE = 20, // ICMP code.
    ARP_OP = 21, // ARP opcode.
    ARP_SPA = 22, // ARP source IPv4 address.
    ARP_TPA = 23, // ARP target IPv4 address.
    ARP_SHA = 24, // ARP source hardware address.
    ARP_THA = 25, // ARP target hardware address.
    IPV6_SRC = 26, // IPv6 source address.
    IPV6_DST = 27, // IPv6 destination address.
    IPV6_FLABEL = 28, // IPv6 Flow Label
    ICMPV6_TYPE = 29, // ICMPv6 type.
    ICMPV6_CODE = 30, // ICMPv6 code.
    IPV6_ND_TARGET = 31, // Target address for ND.
    IPV6_ND_SLL = 32, // Source link-layer for ND.
    IPV6_ND_TLL = 33, // Target link-layer for ND.
    MPLS_LABEL = 34, // MPLS label.
    MPLS_TC = 35, // MPLS TC.
    MPLS_BOS = 36, // MPLS BoS bit.
    PBB_ISID = 37, // PBB I-SID.
    TUNNEL_ID = 38, // Logical Port Metadata.
    IPV6_EXTHDR = 39, // IPv6 Extension Header pseudo-field
};

enum ofp_vlan_id_1_3: uint16_t
{
    PRESENT = 0x1000,    // Bit that indicate that a VLAN id is set
    NONE = 0x0000,       // No VLAN id was set.
};

} // namespace protocol
} // namespace drop

#endif
