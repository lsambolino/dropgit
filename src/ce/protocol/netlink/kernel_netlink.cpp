
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

#include "kernel_netlink.hpp"

#include <cstring>

#include <fcntl.h>
#include <poll.h>

#include <linux/rtnetlink.h>
#include <sys/socket.h> // for sa_family_t
#include <net/if_arp.h>

#include "protocol/netlink/netlink_utils.hpp"

#include "router/port_info.hpp"

#include "event/network/address_events.hpp"
#include "event/network/port_events.hpp"
#include "event/network/route_events.hpp"

#include "demangle.hpp"
#include "mac_address.hpp"
#include "log.hpp"
#include "application.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#endif

namespace drop {
namespace protocol {

int KernelNetlinkProtocol::seq_num = time(0) & 0x00FF00FF;	// randomize

KernelNetlinkProtocol::KernelNetlinkProtocol()
{
    register_netlink_handlers();
	register_message_handlers();

    setup_sockets();
}

KernelNetlinkProtocol::~KernelNetlinkProtocol()
{
    try
    {
        close(cmd_sock_);
    }
    catch (...) {}
}

//send data to command socket and wait - blocking - for a reply (an error message or a data dump)
int KernelNetlinkProtocol::send_and_parse(const std::string& data)
{
    int ret = send_to(cmd_sock_, data, cmd_tx_buff_size_, reinterpret_cast<sockaddr*>(&cmd_peer_addr_));

    if (ret <= 0)
    {
        tnt::Log::warning("KernelNetlinkProtocol::send_and_parse: send_to returned ", ret);

        return ret;
    }

    //tnt::Log::info(colors::green, "\n==> KernelNetlinkProtocol sent new data (", ret, " bytes) to socket ", cmd_sock_);

    static std::vector<std::string> messages;
    static uint16_t multi_type;

    int error = 5;
    bool all = false;
    int dim = 0;

    while (!all)
    {
        dim = recv(cmd_sock_, cmd_rx_buffer_.data(), cmd_rx_buffer_.size(), 0);

        // sanity checks
        if (dim <= 0)
        {
            if (dim < -1)
            {
                tnt::Log::error("KernelNetlinkProtocol::send_and_parse: recv returned ", dim);
            }

            return dim;
        }

        //tnt::Log::info(colors::blue, "\n==> received new data from socket ", cmd_sock_, " (command socket)");

        std::string raw_input(cmd_rx_buffer_.data(), dim);
        size_t len = raw_input.size();
        size_t pos = 0;

        for (const nlmsghdr* nlh = reinterpret_cast<const nlmsghdr*>(raw_input.data()); NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len))
        {
            if (netlink_debug) print_nlmsghdr_info(nlh);

            pos += nlh->nlmsg_len;

            //tnt::Log::info(raw_input.size() - pos, " of ", raw_input.size()," bytes left");

            if (nlh->nlmsg_flags & NLM_F_MULTI)	// Multipart message
            {
                if (nlh->nlmsg_type == NLMSG_DONE)	// Multipart message ended, we can start parsing all the previous messages all together
                {
                    //tnt::Log::info(colors::green, "\n----> multipart ended, now parsing");

                    switch (multi_type)
                    {
                    case RTM_NEWLINK:
                        tnt::Application::raise(event::PortList(parse_multi<std::shared_ptr<NetworkPort>>(messages, link_parser)), this);
                        break;

                    case RTM_NEWADDR:
                        tnt::Application::raise(event::AddressList(parse_multi<AddressInfo>(messages, address_parser)), this);
                        break;

                    case RTM_NEWROUTE:
                        tnt::Application::raise(event::RouteList(parse_multi<RouteInfo>(messages, route_parser)), this);
                        break;

                    default:
                        break;
                    }

                    messages.clear();
                    error = 0;
                }
                else
                {
                    multi_type = nlh->nlmsg_type;
                    messages.push_back(raw_input.substr(pos - nlh->nlmsg_len, pos));

                    continue;	// do not parse yet, thus continue;
                }
            }
            else	// single message
            {
                //tnt::Log::info(colors::green, "\n----> single message, now parsing");

                if (nlh->nlmsg_type == NLMSG_ERROR)
                {
                    nlmsgerr* nl_err = reinterpret_cast<nlmsgerr*>(NLMSG_DATA(nlh));

                    if (nl_err->error)
                    {
                        tnt::Log::warning("error message, code: ", nl_err->error, "\tin reply to message ", type2string(nl_err->msg.nlmsg_type), ", sequence ", nl_err->msg.nlmsg_seq);
                    }
                    else
                    {
                        //tnt::Log::info("ACK message\tin reply to message ", type2string(nl_err->msg.nlmsg_type), ", sequence ", nl_err->msg.nlmsg_seq);
                    }

                    error = -(nl_err->error);
                }
            }

            all = true;
        }

        // sanity checks
        if (raw_input.size() - pos > 0)
        {
            tnt::Log::warning(colors::red, "unable to parse everything (", len, " bytes remaining)");
            raw_input = raw_input.substr(pos);
        }
        else
        {
            raw_input.clear();
        }
    }

    return error;
}

template <typename T> std::vector<T> KernelNetlinkProtocol::parse_multi(const std::vector<std::string>& messages, std::function<T(const nlmsghdr*, bool, bool)> data_parser)
{
    std::vector<T> elements;

    for (const auto& mess : messages)
    {
        auto nlh = reinterpret_cast<const nlmsghdr*>(mess.data());

        try
        {
            elements.push_back(data_parser(nlh, true, netlink_debug));
        }
        catch (drop::IgnoredMessage& /*im*/)
        {
            // tnt::Log::info(im.what());
        }
    }

    return elements;
}

std::string KernelNetlinkProtocol::build_dump(NetlinkDumpType type, NetlinkFamily family)
{
    struct
    {
        nlmsghdr nl_hdr;
        rtgenmsg g_msg;
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.g_msg));
    req.nl_hdr.nlmsg_type = static_cast<uint16_t>(type);
    req.nl_hdr.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

    req.g_msg.rtgen_family = static_cast<unsigned int>(family);
    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

std::string KernelNetlinkProtocol::build_port(NetlinkPortOpType type, uint16_t port)
{
    struct
    {
        nlmsghdr nl_hdr;
        ifinfomsg if_msg;
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.if_msg));
    req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

    req.if_msg.ifi_index = port;

    switch (type)
    {
    case NetlinkPortOpType::SetPortUp:
        req.if_msg.ifi_flags |= IFF_UP;
        // no break

    case NetlinkPortOpType::SetPortDown:
        req.nl_hdr.nlmsg_type = RTM_NEWLINK;
        req.if_msg.ifi_change |= IFF_UP;

        break;

    case NetlinkPortOpType::DeletePort:
        req.nl_hdr.nlmsg_type = RTM_DELLINK;
        break;

    default:
        break;
    }
    
    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

std::string KernelNetlinkProtocol::build_add_link(const std::string& kind, const std::string& name, const std::string& mac, const std::string& mtu)
{
    struct
    {
        nlmsghdr nl_hdr;
        ifinfomsg if_msg;
        char buf[128];
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.if_msg));
    req.nl_hdr.nlmsg_type = RTM_NEWLINK;
    req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

	if (!name.empty())
	{
		addattr_l(&req.nl_hdr, sizeof(req), IFLA_IFNAME, name.c_str(), name.size() + 1);
	}

	if (!mac.empty())
	{
		addattr_l(&req.nl_hdr, sizeof(req), IFLA_ADDRESS, tnt::MacAddress(mac).raw().data(), 6);
	}

	if (!mtu.empty())
	{
		int m = std::stoi(mtu);
		addattr_l(&req.nl_hdr, sizeof(req), IFLA_MTU, &m, 4);
	}

	rtattr* linkinfo = NLMSG_TAIL(&req.nl_hdr);

    addattr_l(&req.nl_hdr, sizeof(req), IFLA_LINKINFO, NULL, 0);
    addattr_l(&req.nl_hdr, sizeof(req), IFLA_INFO_KIND, kind.data(), kind.size());

	linkinfo->rta_len = reinterpret_cast<char*>(NLMSG_TAIL(&req.nl_hdr)) - reinterpret_cast<char*>(linkinfo);
    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

std::string KernelNetlinkProtocol::build_enslave(uint16_t port, uint16_t master)
{
    struct
    {
        nlmsghdr nl_hdr;
        ifinfomsg if_msg;
        char buf[32];
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.if_msg));
    req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nl_hdr.nlmsg_type = RTM_NEWLINK;	// older kernels may require RTM_SETLINK instead
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

    req.if_msg.ifi_index = port;

    addattr_32(&req.nl_hdr, sizeof(req), IFLA_MASTER, master);	// master == 0 release, else enslave to iface "master"
    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

std::string KernelNetlinkProtocol::build_vlan(uint16_t vlan_id, uint16_t link, const std::string& name)
{
    struct
    {
        nlmsghdr nl_hdr;
        ifinfomsg if_msg;
        char buf[1024];
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.if_msg));
    req.nl_hdr.nlmsg_type = RTM_NEWLINK;
    req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

    // physical device id, required
    addattr_32(&req.nl_hdr, sizeof(req), IFLA_LINK, link);


    if (!name.empty())
    {
        addattr_l(&req.nl_hdr, sizeof(req), IFLA_IFNAME, name.c_str(), name.size() + 1);
    }

//	if (!address.empty()) // optional vlan MAC address setting
//	{
//		addattr_l(&req.nl_hdr, sizeof(req), IFLA_ADDRESS, address.data(), address.size());
//	}

    rtattr* linkinfo = NLMSG_TAIL(&req.nl_hdr);
    addattr_l(&req.nl_hdr, sizeof(req), IFLA_LINKINFO, NULL, 0);
    addattr_l(&req.nl_hdr, sizeof(req), IFLA_INFO_KIND, "vlan", strlen("vlan"));

    rtattr* data = NLMSG_TAIL(&req.nl_hdr);
    addattr_l(&req.nl_hdr, sizeof(req), IFLA_INFO_DATA, NULL, 0);
    addattr_l(&req.nl_hdr, sizeof(req), IFLA_VLAN_ID, &vlan_id, 2);
    // not adding vlan flags

    data->rta_len = (char *) NLMSG_TAIL(&req.nl_hdr) - (char *) data;
    linkinfo->rta_len = (char *) NLMSG_TAIL(&req.nl_hdr) - (char *) linkinfo;
    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

std::string KernelNetlinkProtocol::build_address(NetlinkAddrOpType type, const AddressInfo& addr)
{
    struct
    {
        nlmsghdr nl_hdr;
        ifaddrmsg ifaddr_msg;
        char buf[128];
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.ifaddr_msg));
    req.nl_hdr.nlmsg_type = static_cast<uint16_t>(type);
    req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

    req.ifaddr_msg.ifa_family = AF_INET;
    req.ifaddr_msg.ifa_flags = IFA_F_PERMANENT;
    req.ifaddr_msg.ifa_index = addr.port_index;
    req.ifaddr_msg.ifa_prefixlen = addr.prefix ;
    req.ifaddr_msg.ifa_scope = RT_SCOPE_UNIVERSE;

    // Quote from libnl/lib/route/addr.c:
    /* IPv6 sends the local address as IFA_ADDRESS with
     * no IFA_LOCAL, IPv4 sends both IFA_LOCAL and IFA_ADDRESS
     * with IFA_ADDRESS being the peer address if they differ */
    if (type == NetlinkAddrOpType::AddAddress)
    {
        req.nl_hdr.nlmsg_flags |= NLM_F_CREATE | NLM_F_EXCL;
    }

    addattr_32(&req.nl_hdr, sizeof(req), IFA_LOCAL, addr.address);
    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

std::string KernelNetlinkProtocol::build_route(NetlinkRouteOpType type, const RouteInfo& route)
{
    struct
    {
        nlmsghdr nl_hdr;
        rtmsg rt_msg;
        char buf[1024];
    } req;

    memset(&req, 0, sizeof(req));

    req.nl_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(req.rt_msg));
    req.nl_hdr.nlmsg_type = static_cast<uint16_t>(type);
    req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nl_hdr.nlmsg_pid = cmd_local_addr_.nl_pid;
    req.nl_hdr.nlmsg_seq = ++seq_num;

    req.rt_msg.rtm_family = AF_INET;
    req.rt_msg.rtm_table = RT_TABLE_MAIN;
    req.rt_msg.rtm_dst_len = route.prefix;

    addattr_32(&req.nl_hdr, sizeof(req), RTA_DST, route.destination);

    if (type == NetlinkRouteOpType::DelRoute)
    {
        req.rt_msg.rtm_type = RTN_UNSPEC;
        req.rt_msg.rtm_protocol = RTPROT_UNSPEC;
        req.rt_msg.rtm_scope = RT_SCOPE_NOWHERE;

        if (route.port_index != 0)
        {
            // NOT necessary, if the gateway does NOT belong to this port the route WON'T be deleted
            addattr_32(&req.nl_hdr, sizeof(req), RTA_OIF, route.port_index);
        }
    }
    else // NetlinkRouteOpType::AddRoute
    {
        req.nl_hdr.nlmsg_flags |= NLM_F_CREATE | NLM_F_EXCL;

        req.rt_msg.rtm_type = RTN_UNICAST;
        req.rt_msg.rtm_protocol = static_cast<unsigned int>(route.origin);

        if (route.gateway)
        {
            addattr_32(&req.nl_hdr, sizeof(req), RTA_GATEWAY, route.gateway);
            req.rt_msg.rtm_scope = RT_SCOPE_UNIVERSE;

            if (route.port_index)
            {
                // NOT necessary, if the gateway does NOT belong to this port the route WON'T be added
                addattr_32(&req.nl_hdr, sizeof(req), RTA_OIF, route.port_index);
            }
        }
        else
        {
            addattr_32(&req.nl_hdr, sizeof(req), RTA_OIF, route.port_index);
            req.rt_msg.rtm_scope = RT_SCOPE_LINK;
        }

        if (route.source)
        {
            addattr_32(&req.nl_hdr, sizeof(req), RTA_PREFSRC, route.source);
        }

        if (route.metric)
        {
            addattr_32(&req.nl_hdr, sizeof(req), RTA_PRIORITY, route.metric);
        }
    }

    assert(req.nl_hdr.nlmsg_len > 0);

    return std::string(reinterpret_cast<char*>(&req), req.nl_hdr.nlmsg_len);
}

} // namespace protocol
} // namespace drop

#ifdef __clang__
#pragma clang diagnostic pop
#endif
