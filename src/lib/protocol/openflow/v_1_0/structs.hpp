
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

#ifndef DROP_PROTOCOL_OPENFLOW_STRUCTS_1_0_HPP_
#define DROP_PROTOCOL_OPENFLOW_STRUCTS_1_0_HPP_

#include <cstdint>

#include "protocol/openflow/v_1_0/values.hpp"

namespace drop {
namespace protocol {

struct ofp_header_1_0
{
    uint8_t version;    // OFP_VERSION.
    ofp_type_1_0 type;  // One of the  constants.
    uint16_t length;    // Length including this ofp_header.
    uint32_t xid;       // Transaction id associated with this packet. Replies use the same id as was in the request to facilitate pairing.
};

struct ofp_error_msg_1_0
{
	ofp_header_1_0 header;
	ofp_error_type_1_0 type;
	uint16_t code;
	//uint8_t data[0]; // Variable-length data. Interpreted based on the type and code. No padding.
};

struct ofp_action_header_1_0
{
	ofp_action_type_1_0 type; // One of ofp_action_type_1_0.
	uint16_t len;  // Length of action, including this header. This is the length of action, including any padding to make it 64-bit aligned.
	uint8_t pad[4];
};

// Description of a physical port
struct ofp_phy_port_1_0
{
	uint16_t port_no;
	uint8_t hw_addr[OFP_ETH_ALEN];
	char name[OFP_MAX_PORT_NAME_LEN]; // Null-terminated
	uint32_t config;                  // Bitmap of OFPPC_* flags.
	ofp_port_state_1_0 state;         // Bitmap of ofp_port_state_1_0 flags.

	// Bitmaps of OFPPF_* that describe features. All bits zeroed if unsupported or unavailable.
	uint32_t curr;       // Current features.
	uint32_t advertised; // Features being advertised by the port.
	uint32_t supported;  // Features supported by the port.
	uint32_t peer;       // Features advertised by peer.
};

struct ofp_switch_features_1_0
{
	ofp_header_1_0 header;
	uint64_t datapath_id;   // Datapath unique ID. The lower 48-bits are for a MAC address, while the upper 16-bits are implementer-defined.
	uint32_t n_buffers;     // Max packets buffered at once.
	uint8_t n_tables;       // Number of tables supported by datapath.
	uint8_t auxiliary_id;   // Identify auxiliary connections
	uint8_t pad[3];         // Align to 64-bits.

	// Features.
	uint32_t capabilities;  // Bitmap of support "ofp_capabilities".
	uint32_t actions;       // Bitmap of supported "ofp_action_type"s.

	// Port info.
	//ofp_phy_port_1_0 ports[0];  // Port definitions. The number of ports is inferred from the length field in the header.
};

struct ofp_switch_config_1_0
{
	ofp_header_1_0 header;
	ofp_config_flags flags; // Bitmap of ofp_config_flags flags.
	uint16_t miss_send_len; // Max bytes of packet that datapath should send to the controller. See ofp_controller_max_len for valid values.
};

struct ofp_match_1_0
{
	ofp_flow_wildcards_1_0 wildcards;   // Wildcard fields.
	uint16_t in_port;                   // Input switch port.
	uint8_t dl_src[OFP_ETH_ALEN];       // Ethernet source address.
	uint8_t dl_dst[OFP_ETH_ALEN];       // Ethernet destination address.
	uint16_t dl_vlan;                   // Input VLAN id.
	uint8_t dl_vlan_pcp;                // Input VLAN priority.
	uint8_t pad1[1];                    // Align to 64-bits
	uint16_t dl_type;                   // Ethernet frame type.
	uint8_t nw_tos;                     // IP ToS (actually DSCP field, 6 bits).
	uint8_t nw_proto;                   // IP protocol or lower 8 bits of ARP opcode.
	uint8_t pad2[2];                    // Align to 64-bits
	uint32_t nw_src;                    // IP source address.
	uint32_t nw_dst;                    // IP destination address.
	uint16_t tp_src;                    // TCP/UDP source port.
	uint16_t tp_dst;                    // TCP/UDP destination port.
};

// Packet received on port (datapath -> controller).
struct ofp_packet_in_1_0
{
	ofp_header_1_0 header;
	uint32_t buffer_id;                 // ID assigned by datapath.
	uint16_t total_len;                 // Full length of frame.
	uint16_t in_port;                   // Port on which frame was received.
	ofp_packet_in_reason_1_0 reason;    // Reason packet is being sent
	uint8_t pad;         
	//uint8_t data[0];                  // Ethernet frame, halfway through 32-bit word, so the IP header is 32-bit aligned.
                                        // The amount of data is inferred from the length field in the header.
                                        // Because of padding, offsetof(ofp_packet_in_1_0, data) == sizeof(ofp_packet_in_1_0) - 2.
};

struct ofp_flow_mod_1_0
{
	ofp_header_1_0 header;
	ofp_match_1_0 match;            // Fields to match
	uint64_t cookie;                // Opaque controller-issued identifier.

	// Flow actions.
	ofp_flow_mod_command_1_0 command;
	uint16_t idle_timeout;          // Idle time before discarding (seconds).
	uint16_t hard_timeout;          // Max time before discarding (seconds).
	uint16_t priority;              // Priority level of flow entry.
	uint32_t buffer_id;             // Buffered packet to apply to (or -1). Not meaningful for OFPFC_DELETE*.
	uint16_t out_port;              // For OFPFC_DELETE* commands, require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction.
	uint16_t flags;                 // One of OFPFF
	//ofp_action_header_1_0 actions[0];   // The action length is inferred from the length field in the header.
};

struct ofp_port_status_1_0
{
    ofp_header_1_0 header;
    ofp_port_reason reason;
    uint8_t pad[7];         // Align to 64-bits.
    ofp_phy_port_1_0 desc;
};

struct ofp_stats_request_1_0
{
    ofp_header_1_0 header;
    ofp_stats_types_1_0 type;
    ofp_stats_flag_1_0 flags;
    //uint8_t body[0];    // Body of the request.
};

struct ofp_flow_stats_request_1_0
{
    ofp_match_1_0 match;// Fields to match.
    uint8_t table_id;   // ID of table to read (from ofp_table_stats_1_0), 0xff for all tables or 0xfe for emergency.
    uint8_t pad;        // Align to 32 bits. */
    uint16_t out_port;  // Require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction.
};

struct ofp_desc_stats_1_0
{
    char mfr_desc[DESC_STR_LEN];        // Manufacturer description.
    char hw_desc[DESC_STR_LEN];         // Hardware description.
    char sw_desc[DESC_STR_LEN];         // Software description.
    char serial_num[SERIAL_NUM_LEN];    // Serial number.
    char dp_desc[DESC_STR_LEN];         // Human readable description of datapath.
};

struct ofp_flow_stats_1_0
{
    uint16_t length;                    // Length of this entry.
    uint8_t table_id;                   // ID of table flow came from.
    uint8_t pad;
    ofp_match_1_0 match;                // Description of fields.
    uint32_t duration_sec;              // Time flow has been alive in seconds.
    uint32_t duration_nsec;             // Time flow has been alive in nanoseconds beyond duration_sec.
    uint16_t priority;                  // Priority of the entry. Only meaningful when this is not an exact-match entry.
    uint16_t idle_timeout;              // Number of seconds idle before expiration.
    uint16_t hard_timeout;              // Number of seconds before expiration.
    uint8_t pad2[6];                    // Align to 64-bits.
    uint64_t cookie;                    // Opaque controller-issued identifier.
    uint64_t packet_count;              // Number of packets in flow.
    uint64_t byte_count;                // Number of bytes in flow.
    //ofp_action_header_1_0 actions[0]; // Actions.
};

struct ofp_aggregate_stats_request_1_0
{
    ofp_match_1_0 match;    // Fields to match.
    uint8_t table_id;       // ID of table to read (from ofp_table_stats_1_0) 0xff for all tables or 0xfe for emergency.
    uint8_t pad;            // Align to 32 bits.
    uint16_t out_port;      // Require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction.
};

struct ofp_aggregate_stats_reply_1_0
{
    uint64_t packet_count;  // Number of packets in flows.
    uint64_t byte_count;    // Number of bytes in flows.
    uint32_t flow_count;    // Number of flows.
    uint8_t pad[4];         // Align to 64 bits.
};

struct ofp_table_stats_1_0
{
    uint8_t table_id;                   // Identifier of table. Lower numbered tables are consulted first.
    uint8_t pad[3];                     // Align to 32-bits.
    char name[OFP_MAX_TABLE_NAME_LEN];
    ofp_flow_wildcards_1_0 wildcards;   // Bitmap of wildcards that are supported by the table.
    uint32_t max_entries;               // Max number of entries supported.
    uint32_t active_count;              // Number of active entries.
    uint64_t lookup_count;              // Number of packets looked up in table.
    uint64_t matched_count;             // Number of packets that hit table.
};

struct ofp_port_stats_request_1_0
{
    uint16_t port_no; // OFPST_PORT message must request statistics either for a single port (specified in port_no) or for all ports (if port_no == OFPP_NONE).
    uint8_t pad[6];
};

struct ofp_port_stats_1_0
{
    uint16_t port_no;
    uint8_t pad[6];         // Align to 64-bits.
    uint64_t rx_packets;    // Number of received packets.
    uint64_t tx_packets;    // Number of transmitted packets.
    uint64_t rx_bytes;      // Number of received bytes.
    uint64_t tx_bytes;      // Number of transmitted bytes.
    uint64_t rx_dropped;    // Number of packets dropped by RX.
    uint64_t tx_dropped;    // Number of packets dropped by TX.
    uint64_t rx_errors;     // Number of receive errors. This is a super-set of more specific receive errors and should be greater than or equal to the sum of all rx_*_err values.
    uint64_t tx_errors;     // Number of transmit errors. This is a super-set of more specific transmit errors and should be greater than or equal to the sum of all tx_*_err values (none currently defined.)
    uint64_t rx_frame_err;  // Number of frame alignment errors.
    uint64_t rx_over_err;   // Number of packets with RX overrun.
    uint64_t rx_crc_err;    // Number of CRC errors.
    uint64_t collisions;    // Number of collisions.
};

struct ofp_queue_stats_request_1_0
{
    uint16_t port_no;   // All ports if ALL.
    uint8_t pad[2];     // Align to 32-bits.
    uint32_t queue_id;  // All queues if OFPQ_ALL.
};

struct ofp_queue_stats_1_0
{
    uint16_t port_no;
    uint8_t pad[2];         // Align to 32-bits.
    uint32_t queue_id;      // Queue i.d
    uint64_t tx_bytes;      // Number of transmitted bytes.
    uint64_t tx_packets;    // Number of transmitted packets.
    uint64_t tx_errors;     // Number of packets dropped due to overrun.
};

struct ofp_packet_out_1_0
{
    ofp_header_1_0 header;
    uint32_t buffer_id;             // ID assigned by datapath (-1 if none).
    uint16_t in_port;               // Packet’s input port (OFPP_NONE if none).
    uint16_t actions_len;           // Size of action array in bytes.
    //ofp_action_header_1_0 actions[0]; // Actions.
    // uint8_t data[0];             // Packet data. The length is inferred from the length field in the header. (Only meaningful if buffer_id == -1.)
};

} // namespace protocol
} // namespace drop

#endif
