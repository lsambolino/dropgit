
/*

Copyright (c) 2013, Marco Chiappero (marco@absence.it)
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

#ifndef DROP_PROTOCOL_NETLINK_UTILS_HPP_
#define DROP_PROTOCOL_NETLINK_UTILS_HPP_

#include <string>
#include <memory>
#include <cstdint>

struct sockaddr;
struct nlmsghdr;
struct rtattr;

// placed here as can be seen by both derived classes
#define NLMSG_TAIL(nmsg) ((struct rtattr *) (((char *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

namespace drop {

class NetworkPort;

struct PortInfo;
struct RouteInfo;
struct AddressInfo;

namespace protocol {

struct NetlinkReplyInfo;

// helper method for sending data to a socket
int send_to(int sock, const std::string& data, unsigned int max_size, const sockaddr* peer);

// utility functions for parsing netlink messages
std::shared_ptr<NetworkPort> link_parser(const nlmsghdr* nlh, bool filter, bool netlink_debug);
RouteInfo route_parser(const nlmsghdr* nlh, bool filter, bool netlink_debug);
AddressInfo address_parser(const nlmsghdr* nlh, bool filter, bool netlink_debug);

// utility functions for adding attributes when building netlink messages
void addattr_l(nlmsghdr* n, unsigned int maxlen, int type, const void* data, int alen);
void addattr_32(nlmsghdr* n, unsigned int maxlen, int type, uint32_t data);

void throw_filtered(const nlmsghdr* nlh);
void print_nlmsghdr_info(const nlmsghdr* nlh);

std::string type2string(uint16_t type);
std::string addr2string(uint32_t addr);

NetlinkReplyInfo parse_reply_info(const nlmsghdr* nlh);
void print_request_flags(const nlmsghdr*);
bool replace_flag(const nlmsghdr* nlh);
bool create_flag(const nlmsghdr* nlh);

} // namespace protocol
} // namespace drop

#endif
