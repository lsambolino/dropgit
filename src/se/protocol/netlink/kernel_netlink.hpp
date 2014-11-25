
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

#ifndef KERNEL_NETLINK_HPP_
#define KERNEL_NETLINK_HPP_

#include <memory>
#include <thread>
#include <functional>

#include <linux/rtnetlink.h>
#include <sys/socket.h> // for sa_family_t
#include <net/if_arp.h>

#include "protocol/netlink/netlink_protocol.hpp"
#include "protocol/netlink/values.hpp"

namespace drop {
namespace protocol {

class KernelNetlinkProtocol: public NetlinkProtocol
{
    enum class NetlinkDumpType: uint8_t
    {
        Links = RTM_GETLINK,
        Routes = RTM_GETROUTE,
        Addresses = RTM_GETADDR
    };

    enum class NetlinkPortOpType: uint8_t
    {
        SetPortDown,
        SetPortUp,
        DeletePort
    };
public:
    KernelNetlinkProtocol();
    virtual ~KernelNetlinkProtocol();
private:
    void register_message_handlers();
    void register_netlink_handlers();
    void setup_sockets();

    int send_and_parse(const std::string& data);

    template <typename T> std::vector<T> parse_multi(const std::vector<std::string>& messages, std::function<T(const nlmsghdr*, const bool)> data_parser);

    std::string build_dump(const NetlinkDumpType type, const NetlinkFamily family);
    std::string build_port(const NetlinkPortOpType type, const uint16_t port);
    std::string build_master(const std::string name, const std::string kind);
    std::string build_enslave(const uint16_t port, const uint16_t master);
    std::string build_vlan(const uint16_t vlan_id, const uint16_t link, const std::string& name);
    std::string build_address(const NetlinkAddrOpType type, const AddressInfo& addr);
    std::string build_route(const NetlinkRouteOpType type, const RouteInfo& route);
private:
    static int seq_num;

    int cmd_sock_;

    unsigned int cmd_tx_buff_size_;

    std::vector<char> cmd_rx_buffer_;

    sockaddr_nl cmd_local_addr_;
    sockaddr_nl cmd_peer_addr_;
};

} //namespace protocol
} // namespace drop

#endif
