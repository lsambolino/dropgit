
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

#ifndef DROP_PROTOCOL_OPENFLOW_STRUCTS_1_3_HPP_
#define DROP_PROTOCOL_OPENFLOW_STRUCTS_1_3_HPP_

#include <cstdint>

#include "protocol/openflow/v_1_3/values.hpp"

namespace drop {
namespace protocol {

struct ofp_header_1_3
{
    uint8_t version;    // OFP_VERSION.
    ofp_type_1_3 type;  // One of the  constants.
    uint16_t length;    // Length including this ofp_header_1_3.
    uint32_t xid;       // Transaction id associated with this packet. Replies use the same id as was in the request to facilitate pairing.
};

struct ofp_error_msg_1_3
{
	ofp_header_1_3 header;
	ofp_error_type_1_3 type;
	uint16_t code;
	//uint8_t data[0]; // Variable-length data. Interpreted based on the type and code. No padding.
};

// Description of a port 
struct ofp_port_1_3
{
	uint32_t port_no;
	uint8_t pad[4];
	uint8_t hw_addr[OFP_ETH_ALEN];
	uint8_t pad2[2];                  // Align to 64 bits. 
	char name[OFP_MAX_PORT_NAME_LEN]; // Null-terminated 
	ofp_port_config_1_3 config;       // Bitmap of ofp_port_config_1_3 flags. 
	ofp_port_state_1_3 state;         // Bitmap of OFPPS_* flags. 
	                        
    // Bitmaps of ofp_port_features_1_3 that describe features.  All bits zeroed if unsupported or unavailable. 
	ofp_port_features_1_3 curr;         // Current features. 
	ofp_port_features_1_3 advertised;   // Features being advertised by the port. 
	ofp_port_features_1_3 supported;    // Features supported by the port. 
	ofp_port_features_1_3 peer;         // Features advertised by peer. 
	ofp_port_features_1_3 curr_speed;   //Current port bitrate in kbps.
	ofp_port_features_1_3 max_speed;    // Max port bitrate in kbps 
};

struct ofp_switch_config_1_3
{
	ofp_header_1_3 header;
	ofp_config_flags flags; // Bitmap of ofp_config_flags flags.
	uint16_t miss_send_len; // Max bytes of packet that datapath should send to the controller. See ofp_controller_max_len for valid values.
};

// Switch features.
struct ofp_switch_features_1_3
{
	ofp_header_1_3 header;
	uint64_t datapath_id;   // Datapath unique ID.  The lower 48-bits are for a MAC address, while the upper 16-bits are implementer-defined. 
	uint32_t n_buffers;     // Max packets buffered at once. 
	uint8_t n_tables;       // Number of tables supported by datapath. 
	uint8_t auxiliary_id;   // Identify auxiliary connections 
	uint8_t pad[2];         // Align to 64-bits. 

	// Features. 
	ofp_capabilities_1_3 capabilities;  // Bitmap of support ofp_capabilities_1_3. 
	uint32_t reserved;
};

/// Fields to match against flows 
struct ofp_match_1_3
{
	ofp_match_type_1_3 type;
	uint16_t length; // Length of ofp_match (excluding padding) 
	// Followed by:
    // - Exactly (length - 4) (possibly 0) bytes containing OXM TLVs, then
    // - Exactly ((length + 7)/8*8 - length) (between 0 and 7) bytes of all-zero bytes
    // In summary, ofp_match is padded as needed, to make its overall size
    // a multiple of 8, to preserve alignement in structures using it.
	
	uint8_t oxm_fields[4]; // OXMs start here - Make compiler happy
};

struct ofp_packet_in_1_3
{
	ofp_header_1_3 header;
	uint32_t buffer_id;                 // ID assigned by datapath. 
	uint16_t total_len;                 // Full length of frame. 
	ofp_packet_in_reason_1_3 reason;    // Reason packet is being sent
	uint8_t table_id;                   // ID of the table that was looked up 
	uint64_t cookie;                    // Cookie of the flow entry that was looked up. 
	ofp_match_1_3 match;                // Packet metadata. Variable size. 

	// The variable size and padded match is always followed by:
	//- Exactly 2 all-zero padding bytes, then
	//- An Ethernet frame whose length is inferred from header.length.
	//The padding bytes preceding the Ethernet frame ensure that the IP
	//header (if any) following the Ethernet header is 32-bit aligned.

	//uint8_t pad[2];  // Align to 64 bit + 16 bit 
	//uint8_t data[0]; // Ethernet frame 
};

// Flow setup and teardown (controller -> datapath). 
struct ofp_flow_mod_1_3
{
	ofp_header_1_3 header;
	uint64_t cookie;        // Opaque controller-issued identifier. 
	uint64_t cookie_mask;   // Mask used to restrict the cookie bits that must match when the command is OFPFC_MODIFY* or OFPFC_DELETE*. A value of 0 indicates no restriction.

	// Flow actions. 
	uint8_t table_id;       // ID of the table to put the flow in. For OFPFC_DELETE_* commands, OFPTT_ALL can also be used to delete matching flows from all tables. 
	ofp_flow_mod_command_1_3 command;
	uint16_t idle_timeout;  // Idle time before discarding (seconds). 
	uint16_t hard_timeout;  // Max time before discarding (seconds). 
	uint16_t priority;      // Priority level of flow entry. 
	uint32_t buffer_id;     //Buffered packet to apply to, or OFP_NO_BUFFER. Not meaningful for OFPFC_DELETE*. 
	uint32_t out_port;      // For OFPFC_DELETE* commands, require matching entries to include this as an output port. A value of OFPP_ANY indicates no restriction. 
	uint32_t out_group;     // For OFPFC_DELETE* commands, require matching entries to include this as an output group. A value of OFPG_ANY indicates no restriction. 

	uint16_t flags; //Bitmap of OFPP_* flags.
	uint8_t pad[2];
	ofp_match_1_3 match; //Fields to match. Variabile size.

	// The variable size and padded match is always followed by instructions. 
	//struct ofp_instruction instructions[0]; // Instruction set - 0 or more.
	//The length of the instruction
	//set is inferred from the
	//length field in the header. 
};

struct ofp_port_status_1_3
{
	ofp_header_1_3 header;
	uint8_t reason;     // One of OFPPR_*.
	uint8_t pad[7];     // Align to 64-bits.
	ofp_port_1_3 desc;
};

struct ofp_multipart_request_1_3
{
	ofp_header_1_3 header;
	ofp_multipart_type_1_3 type;
	ofp_multipart_request_flags_1_3 flags; // ofp_multipart_request_flags_1_3 flags. 
	uint8_t pad[4];
	//uint8_t body[0];  // Body of the request. 0 or more bytes. 
};

struct ofp_multipart_reply_1_3
{
	ofp_header_1_3 header;
	ofp_multipart_type_1_3 type;
	ofp_multipart_reply_flags_1_3 flags; // ofp_multipart_reply_flags_1_3 flags. 
	uint8_t pad[4];
	//uint8_t body[0];  // Body of the reply. 0 or more bytes. 
};

// Body for ofp_multipart_request of type OFPMP_FLOW. 
struct ofp_flow_stats_request_1_3
{
	uint8_t table_id; // ID of table to read (from ofp_table_stats),
	//OFPTT_ALL for all tables. 
	uint8_t pad[3]; // Align to 32 bits. 
	uint32_t out_port; // Require matching entries to include this
	//as an output port.  A value of OFPP_ANY
	//indicates no restriction. 
	uint32_t out_group; // Require matching entries to include this
	//as an output group.  A value of OFPG_ANY
	//indicates no restriction. 
	uint8_t pad2[4]; // Align to 64 bits. 
	uint64_t cookie; // Require matching entries to contain this cookie value 
	uint64_t cookie_mask; // Mask used to restrict the cookie bits that
	//must match. A value of 0 indicates
	//no restriction. 
	ofp_match_1_3 match; // Fields to match. Variable size. 
};

struct ofp_desc_stats_1_3
{
	char mfr_desc[DESC_STR_LEN];        // Manufacturer description.
	char hw_desc[DESC_STR_LEN];         // Hardware description.
	char sw_desc[DESC_STR_LEN];         // Software description.
	char serial_num[SERIAL_NUM_LEN];    // Serial number.
};

// Body of reply to OFPMP_FLOW request. 
struct ofp_flow_stats_1_3
{
	uint16_t length; // Length of this entry. 
	uint8_t table_id; // ID of table flow came from. 
	uint8_t pad;
	uint32_t duration_sec; // Time flow has been alive in seconds. 
	uint32_t duration_nsec; // Time flow has been alive in nanoseconds beyond duration_sec. 
	uint16_t priority; // Priority of the entry. 
	uint16_t idle_timeout; // Number of seconds idle before expiration. 
	uint16_t hard_timeout; // Number of seconds before expiration. 
	ofp_flow_mod_flags_1_3 flags; // Bitmap of ofp_flow_mod_flags_1_3 flags. 
	uint8_t pad2[4]; // Align to 64-bits. 
	uint64_t cookie; // Opaque controller-issued identifier. 
	uint64_t packet_count; // Number of packets in flow. 
	uint64_t byte_count; // Number of bytes in flow. 
	ofp_match_1_3 match; // Description of fields. Variable size. 
	// The variable size and padded match is always followed by instructions. 
	//struct ofp_instruction instructions[0]; // Instruction set - 0 or more. 
};

struct ofp_aggregate_stats_request_1_3
{
	uint8_t table_id;// ID of table to read (from ofp_table_stats)
					 // OFPTT_ALL for all tables.
	uint8_t pad[3]; // Align to 32 bits.
	uint32_t out_port; // Require matching entries to include this
					   //as an output port.  A value of OFPP_ANY
					   //indicates no restriction.
	uint32_t out_group; // Require matching entries to include this
						//as an output group.  A value of OFPG_ANY
						//indicates no restriction.
	uint8_t pad2[4]; // Align to 64 bits.
	uint64_t cookie; // Require matching entries to contain this cookie value
	uint64_t cookie_mask; // Mask used to restrict the cookie bits that
						  //must match. A value of 0 indicates
						  //no restriction.
	ofp_match_1_3 match; //Fields to match. Variable size.
};

//Body of reply to OFPMP_TABLE request.
struct ofp_table_stats_1_3
{
	uint8_t table_id; // Identifier of table.  Lower numbered tables are consulted first.
	uint8_t pad[3]; // Align to 32-bits.
	uint32_t active_count; // Number of active entries.
	uint64_t lookup_count; // Number of packets looked up in table.
	uint64_t matched_count;  // Number of packets that hit table.
};

struct ofp_port_stats_1_3
{
	uint32_t port_no;
	uint8_t pad[4];          // Align to 64-bits. 
	uint64_t rx_packets; // Number of received packets. 
	uint64_t tx_packets; // Number of transmitted packets. 
	uint64_t rx_bytes; // Number of received bytes. 
	uint64_t tx_bytes; // Number of transmitted bytes. 
	uint64_t rx_dropped; // Number of packets dropped by RX. 
	uint64_t tx_dropped; // Number of packets dropped by TX. 
	uint64_t rx_errors; // Number of receive errors.  This is a super-set
						//of more specific receive errors and should be
						//greater than or equal to the sum of all
						//rx_*_err values. 
	uint64_t tx_errors; // Number of transmit errors.  This is a super-set
						//of more specific transmit errors and should be
						//greater than or equal to the sum of all
						//tx_*_err values (none currently defined.) 
	uint64_t rx_frame_err; // Number of frame alignment errors. 
	uint64_t rx_over_err; // Number of packets with RX overrun. 
	uint64_t rx_crc_err; // Number of CRC errors. 
	uint64_t collisions; // Number of collisions. 
	uint32_t duration_sec;  // Time port has been alive in seconds. 
	uint32_t duration_nsec;  // Time port has been alive in nanoseconds beyond duration_sec.                        
};

struct ofp_queue_stats_request_1_3
{
	uint16_t port_no;   // All ports if ALL.
	uint32_t queue_id;  // All queues if OFPQ_ALL.
};

struct ofp_queue_stats_1_3
{
	uint32_t port_no;
	uint32_t queue_id; // Queue i.d 
	uint64_t tx_bytes; // Number of transmitted bytes. 
	uint64_t tx_packets; // Number of transmitted packets. 
	uint64_t tx_errors; // Number of packets dropped due to overrun. 
	uint32_t duration_sec; // Time queue has been alive in seconds. 
	uint32_t duration_nsec;  // Time queue has been alive in nanoseconds beyond duration_sec.                            
};

struct ofp_action_header_1_3
{
    ofp_action_type_1_3 type;
    uint16_t len;   // Length of action, including this header. This is the length of action, including any padding to make it 64-bit aligned.
    uint8_t pad[4];
};

struct ofp_action_output_1_3
{
    ofp_action_type_1_3 type;
    uint16_t len;
    uint32_t port;
    uint16_t max_len;   // Max length to send to controller.
    uint8_t pad[6];     // Pad to 64 bits.
};


struct ofp_instruction_actions_1_3
{
    ofp_instruction_type_1_3 type;
    uint16_t len;   // Length of this struct in bytes.
    uint8_t pad[4]; // Align to 64-bits
    //ofp_action_header_1_3 actions[0]; // Actions associated with ofp_instruction_type::WRITE_ACTIONS and ofp_instruction_type::APPLY_ACTIONS
};

struct ofp_packet_out_1_3
{
	ofp_header_1_3 header;
	uint32_t buffer_id; // ID assigned by datapath (OFP_NO_BUFFER if none). 
	uint32_t in_port; // Packet’s input port or OFPP_CONTROLLER. 
	uint16_t actions_len; //Size of action array in bytes
	uint8_t pad[6];
	//ofp_action_header actions[0]; // Action list - 0 or more. 
	// The variable size action list is optionally followed by packet data.
	// This data is only present and meaningful if buffer_id == -1.
	// uint8_t data[0];         // Packet data.  The length is inferred rom the length field in the header. 
};

struct ofp_action_set_field_1_3
{
    ofp_action_type_1_3 type;   // OFPAT_SET_FIELD.
    uint16_t len;               // Length is padded to 64 bits.
                                // Followed by:
                                //  - Exactly oxm_len bytes containing a single OXM TLV, then
                                //  - Exactly ((oxm_len + 4) + 7)/8*8 - (oxm_len + 4) (between 0 and 7) bytes of all-zero bytes
                                
    uint8_t field[4];           // OXM TLV - Make compiler happy
};

} // namespace protocol
} // namespace drop

#endif
