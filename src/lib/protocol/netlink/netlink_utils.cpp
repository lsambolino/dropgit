
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

#include "netlink_utils.hpp"

#include <boost/asio.hpp>

#include <sstream>

#include <linux/rtnetlink.h>
#include <sys/socket.h> // for sa_family_t
#include <net/if_arp.h>

#include "protocol/netlink/reply_info.hpp"

#include "exception/drop_exception.hpp"

#include "router/ports.hpp"
#include "router/port_info.hpp"
#include "router/route_info.hpp"
#include "router/address_info.hpp"

#include "log.hpp"

const int IFF_LOWER_UP = 0x10000; // can't include <linux/if.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#endif

namespace drop {
namespace protocol {

int send_to(const int sock, const std::string& data, unsigned int max_size, const sockaddr* peer)
{
    if (data.size() > max_size)
    {
        // TODO: split dump messages depending on the socket buffer size (usually not needed)
        tnt::Log::error("NetlinkProtocol::send_data: data.size() > buff_size");
    }

    int ret = sendto(sock, data.data(), data.size(), 0, peer, sizeof(*peer));

    if (ret < 0)
    {
        tnt::Log::warning("NetlinkProtocol::send_data: sendto error ", errno);
    }
    else if (static_cast<unsigned int>(ret) < data.size())
    {
        tnt::Log::warning("Sent only ", ret, " bytes of ", data.size());
    }

    return ret;
}

std::shared_ptr<NetworkPort> link_parser(const nlmsghdr* nlh, bool filter, bool netlink_debug)
{
    static std::string iflas[] = { "IFLA_UNSPEC", "IFLA_ADDRESS", "IFLA_BROADCAST", "IFLA_IFNAME", "IFLA_MTU", "IFLA_LINK", "IFLA_QDISC", "IFLA_STATS",
                                   "IFLA_COST", "IFLA_PRIORITY", "IFLA_MASTER", "IFLA_WIRELESS", "IFLA_PROTINFO", "IFLA_TXQLEN", "IFLA_MAP", "IFLA_WEIGHT",
                                   "IFLA_OPERSTATE", "IFLA_LINKMODE", "IFLA_LINKINFO", "IFLA_NET_NS_PID", "IFLA_IFALIAS", "IFLA_NUM_VF", "IFLA_VFINFO_LIST",
                                   "IFLA_STATS64", "IFLA_VF_PORTS", "IFLA_PORT_SELF", "IFLA_AF_SPEC", "IFLA_GROUP", "IFLA_NET_NS_FD", "IFLA_EXT_MASK" };

    auto ifi = reinterpret_cast<ifinfomsg*>(NLMSG_DATA(nlh));

    if (netlink_debug)
    {
        tnt::Log::info("\nport ",ifi->ifi_index, "\t  flags: ", (ifi->ifi_flags & IFF_UP) ? "UP ": "   ",
            (ifi->ifi_flags & IFF_LOWER_UP) ? "PHY " : "    ", (ifi->ifi_flags & IFF_RUNNING) ? "RUNNING " : "        ",
            (ifi->ifi_flags & IFF_SLAVE) ? "SLAVE " : "      ", (ifi->ifi_flags & IFF_MASTER) ? "MASTER " : "       ");
    }

    if (netlink_debug)
    {
        tnt::Log::info("\tchanges: ", (ifi->ifi_change & IFF_UP) ? "UP ": "   ", (ifi->ifi_change & IFF_LOWER_UP) ? "PHY " : "    ",
            (ifi->ifi_change & IFF_RUNNING) ? "RUNNING " : "        ", (ifi->ifi_flags & IFF_SLAVE) ? "SLAVE " : "      ",
            (ifi->ifi_flags & IFF_MASTER) ? "MASTER " : "       ");
    }

    //			FILTERS			//
    //	NOTE: ifi->ifi_family should always be 0x00 / AF_UNSPEC or AF_BRIDGE (no need to check though)
    //	NOTE: ifi->ifi_type should always be 0x01 / ARPHRD_ETHER
    //	NOTE: look for: IFF_RUNNING, IFF_UP, IFF_LOWER_UP, ~IFF_NOARP (redundant with type check),
    //		~IFF_LOOPBACK (redundant with type check), IFF_BROADCAST (redundant with type check)
    if (filter && (ifi->ifi_type != ARPHRD_ETHER))
    {
        throw_filtered(nlh);
    }

    enum InterfaceKind { PHYSICAL, VLAN, BOND, BRIDGE, DETH, VETH };
    auto pi = PortInfo();
    InterfaceKind kind = PHYSICAL;
    uint16_t vlan_tag = 0;
    uint16_t lower_link = 0;
    uint16_t master = 0;

    pi.index = ifi->ifi_index;
    pi.flags = ifi->ifi_flags;

    int len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));

    for (rtattr* ifi_attr = IFLA_RTA(ifi); RTA_OK(ifi_attr, len); ifi_attr = RTA_NEXT(ifi_attr, len))
    {
        switch (ifi_attr->rta_type)
        {
        case IFLA_IFNAME:
            pi.name = std::string(reinterpret_cast<char*>(RTA_DATA(ifi_attr)));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], " = ", pi.name);
            break;

        case IFLA_ADDRESS:
            pi.hw_address = std::string(reinterpret_cast<char*>(RTA_DATA(ifi_attr)), RTA_PAYLOAD(ifi_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], " = ", pi.hw_address);
            break;

        case IFLA_OPERSTATE:
            // see RFC 2863	--> may be used in place of flag IFF_RUNNING for more accuracy
            // doc: kernel/Documentation/networking/operstates.txt
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], " = ", *reinterpret_cast<unsigned int*>(RTA_DATA(ifi_attr)));
            break;

        /*case IFLA_STATS:
            pi.stats = *reinterpret_cast<rtnl_link_stats*>(RTA_DATA(ifi_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], ": ", "rx_packets = ", pi.stats.rx_packets, ", tx_packets = ", pi.stats.tx_packets,
                                    ", rx_bytes = ", pi.stats.rx_bytes, ", tx_bytes = ", pi.stats.tx_bytes, ", rx_errors = ", pi.stats.rx_errors, ", tx_errors = ", pi.stats.tx_errors);
            break;*/
        case IFLA_MTU:
            pi.mtu = *reinterpret_cast<unsigned int*>(RTA_DATA(ifi_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], ": ", pi.mtu);
            break;

        case IFLA_LINK:
            /* IFLA_LINK.
                For usual devices it is equal ifi_index. If it is a "virtual interface" (f.e. tunnel), ifi_link can point to real physical
                interface (f.e. for bandwidth calculations), or maybe 0, what means, that real media is unknown (usual for IPIP tunnels,
                when route to endpoint is allowed to change)
            */
            lower_link = *reinterpret_cast<unsigned int*>(RTA_DATA(ifi_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], ": ", lower_link);
            if (lower_link == ifi->ifi_index) lower_link = 0;
            break;

        case IFLA_MASTER:
            master = *reinterpret_cast<unsigned int*>(RTA_DATA(ifi_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], ": ", master);
            break;
/*
        case IFLA_IFALIAS:
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], ": ", std::string(reinterpret_cast<char*>(RTA_DATA(ifi_attr))));
            break;

*/
        case IFLA_LINKINFO:		// IFLA_LINKINFO attribute contains nested attributes in its payload, for virtual links
        {
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], ".");

            unsigned int linkinfo_len = RTA_PAYLOAD(ifi_attr);

            for (rtattr* linkinfo_attr = reinterpret_cast<rtattr*>(RTA_DATA(ifi_attr)); RTA_OK(linkinfo_attr, linkinfo_len); linkinfo_attr = RTA_NEXT(linkinfo_attr, linkinfo_len))
            {
                if (linkinfo_attr->rta_type == IFLA_INFO_KIND)
                {
                    // info_kind contains a terminated string, trip the terminator decreasing the length size
                    std::string info_kind(reinterpret_cast<char*>(RTA_DATA(linkinfo_attr)), RTA_PAYLOAD(linkinfo_attr));
                    if (info_kind[info_kind.size() - 1] == '\0')
                    {
                        info_kind = info_kind.substr(0, info_kind.size() - 1);
                    }

                    if (netlink_debug) tnt::Log::info("  Found RTA attribute: IFLA_INFO_KIND: ", info_kind);

                    if (info_kind == "vlan")
                    {
                        kind = VLAN;
                    }
                    else if (info_kind == "bond")
                    {
                        kind = BOND;
                    }
                    else if (info_kind == "bridge")
                    {
                        kind = BRIDGE;
                    }
                    else if (info_kind == "deth")
                    {
                        kind = DETH;
                    }
                    else if (info_kind == "veth")
                    {
                        kind = VETH;
                    }
                }
                else if (linkinfo_attr->rta_type == IFLA_INFO_DATA)
                {
                    if (netlink_debug) tnt::Log::info("  Found RTA attribute: IFLA_INFO_DATA.");
                    // IFLA_INFO_DATA contains more nested attributes with type depending on IFLA_INFO_KIND

                    unsigned int infodata_len = RTA_PAYLOAD(linkinfo_attr);

                    for (rtattr* infodata_attr = reinterpret_cast<rtattr*>(RTA_DATA(linkinfo_attr)); RTA_OK(infodata_attr, infodata_len); infodata_attr = RTA_NEXT(infodata_attr, infodata_len))
                    {
                        if (infodata_attr->rta_type == IFLA_VLAN_ID && kind == VLAN)
                        {
                            vlan_tag = *reinterpret_cast<uint16_t*>(RTA_DATA(infodata_attr));
                            if (netlink_debug) tnt::Log::info("    Found RTA attribute: IFLA_VLAN_ID: ", vlan_tag);
                        }
                    }
                }
                else if (linkinfo_attr->rta_type == IFLA_INFO_UNSPEC)
                {
                    if (netlink_debug) tnt::Log::info("  Found RTA attribute: IFLA_INFO_UNSPEC.");
                }
            }
            break;
        }

#if 0
        case IFLA_BROADCAST:
            link_attr->hw_broadcast = std::string(reinterpret_cast<char*>(RTA_DATA(ifi_attr)), RTA_PAYLOAD(ifi_attr));
            break;

        case IFLA_LINKMODE:		// IF_LINK_MODE_DEFAULT or IF_LINK_MODE_DORMANT
            link_attr->link_mode = *reinterpret_cast<int*>(RTA_DATA(ifi_attr));
            break;

        case IFLA_AF_SPEC:
            /*
             * IFLA_AF_SPEC
             *   Contains nested attributes for address family specific attributes.
             *   Each address family may create a attribute with the address family
             *   number as type and create its own attribute structure in it.
             *
             *   Example:
             *   [IFLA_AF_SPEC] = {
             *       [AF_INET] = {
             *           [IFLA_INET_CONF] = ...,
             *       },
             *       [AF_INET6] = {
             *           [IFLA_INET6_FLAGS] = ...,
             *           [IFLA_INET6_CONF] = ...,
             *       }
             *   }
             *
             *   Basically, the same values are available from /proc/sys/net/ipv4 and /proc/sys/net/ipv6
             *   They can also be sent to the kernel.
             */

            break;
#endif

        default:
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", iflas[ifi_attr->rta_type], " (value not parsed, ", RTA_PAYLOAD(ifi_attr), " byte(s) long)");
            break;
        }
    }

    std::shared_ptr<NetworkPort> ni;

    switch (kind)
    {
    default:
    case PHYSICAL:
        ni = std::make_shared<PhysicalPort>(pi, master);
        break;

    case VLAN:
        ni = std::make_shared<VlanPort>(pi, master, vlan_tag, lower_link);
        break;

    case BOND:
        ni = std::make_shared<BondPort>(pi, master);
        break;

    case BRIDGE:
        ni = std::make_shared<BridgePort>(pi, master);
        break;
    case DETH:
        ni = std::make_shared<DethPort>(pi, master);
        break;
    case VETH:
        ni = std::make_shared<VethPort>(pi, master);
        break;
    }

    return ni;
}

AddressInfo address_parser(const nlmsghdr* nlh, bool filter, bool netlink_debug)
{
    static std::string ifas[] = { "IFA_UNSPEC", "IFA_ADDRESS", "IFA_LOCAL", "IFA_LABEL", "IFA_BROADCAST", "IFA_ANYCAST", "IFA_CACHEINFO", "IFA_MULTICAST" };

    AddressInfo na = AddressInfo();
    ifaddrmsg* ifa = reinterpret_cast<ifaddrmsg*>(NLMSG_DATA(nlh));

    if (netlink_debug)
    {
        tnt::Log::info("ifa_prefixlen ", (int) ifa->ifa_prefixlen, ", ifa_flags ", (int) ifa->ifa_flags, ", ifa_scope ", (int) ifa->ifa_scope, ", ifa_index ", ifa->ifa_index);
    }

    //			FILTERS			//
    // NOTE: ifa->ifa_flags should always be 0x80 / IFA_F_PERMANENT
    // NOTE: ifa->ifa_scope should always be 0x00 / RT_SCOPE_UNIVERSE (at least ~RT_SCOPE_HOST)
    //		(e.g. 127.0.0.1 comes with RT_SCOPE_HOST)

    if (filter && (ifa->ifa_scope != RT_SCOPE_UNIVERSE))
    {
        throw_filtered(nlh);
    }

    na.prefix = ifa->ifa_prefixlen;
    na.port_index = ifa->ifa_index;

    int len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa));

    for (rtattr* ifa_attr = IFA_RTA(ifa); RTA_OK(ifa_attr, len); ifa_attr = RTA_NEXT(ifa_attr, len))
    {
        // Quote from libnl/lib/route/addr.c:
        // IPv6 sends the local address as IFA_ADDRESS with
        // no IFA_LOCAL, IPv4 sends both IFA_LOCAL and IFA_ADDRESS
        // with IFA_ADDRESS being the peer address if they differ
        if (ifa_attr->rta_type == IFA_LOCAL)
        {
            na.address = *reinterpret_cast<uint32_t*>(RTA_DATA(ifa_attr));

            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", ifas[ifa_attr->rta_type], " = ", addr2string(na.address));
        }
        else if (ifa_attr->rta_type == IFA_LABEL)	// null terminated string
        {
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", ifas[ifa_attr->rta_type], " = ", reinterpret_cast<char*>(RTA_DATA(ifa_attr)));
        }
        else
        {
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", ifas[ifa_attr->rta_type], " (value not parsed)");
        }
    }

    if (netlink_debug) tnt::Log::info("\taddress ", addr2string(na.address), "/", na.prefix, " on port ", na.port_index);

    return na;
}

RouteInfo route_parser(const nlmsghdr* nlh, bool filter, bool netlink_debug)
{
    static std::string rtas[] = { "RTA_UNSPEC", "RTA_DST", "RTA_SRC", "RTA_IIF", "RTA_OIF", "RTA_GATEWAY", "RTA_PRIORITY", "RTA_PREFSRC",
                                "RTA_METRICS", "RTA_MULTIPATH", "NONE", "RTA_FLOW", "RTA_CACHEINFO", "NONE", "NONE", "RTA_TABLE", "RTA_MARK" };

    RouteInfo nr = RouteInfo();
    rtmsg* rtm = reinterpret_cast<rtmsg*>(NLMSG_DATA(nlh));

    if (netlink_debug)
    {
        tnt::Log::info("rtm_dst_len ", (int) rtm->rtm_dst_len, ", rtm_type ", (int) rtm->rtm_type, ", rtm_flags ", rtm->rtm_flags,
                ", rtm_protocol ", (int) rtm->rtm_protocol, ", rtm_scope ", (int) rtm->rtm_scope, ", rtm_table ", (int) rtm->rtm_table);
    }

    //			FILTERS			//
    //	NOTE: do not care about rtm->rtm_table == RT_TABLE_LOCAL (local routes)
    //	NOTE: avoid non unicast global routes ==> rtm->rtm_type == RTN_UNICAST
    //	NOTE: rtm->rtm_protocol must not be RTPROT_REDIRECT (ICMP driven route)
    //	NOTE: rtm->rtm_src_len must be 0 (no source routing)
    //	NOTE: rtm->rtm_flags must not be RTM_F_CLONED

    if (filter && (rtm->rtm_table == RT_TABLE_LOCAL			// most common first
                || rtm->rtm_type != RTN_UNICAST				// Quagga may use RTN_UNREACHABLE or RTN_BLACKHOLE instead (unlikely...)
                || rtm->rtm_protocol == RTPROT_REDIRECT
                || rtm->rtm_src_len != 0
                || rtm->rtm_flags == RTM_F_CLONED))
    {
        throw_filtered(nlh);
    }

    nr.prefix = rtm->rtm_dst_len;
    nr.origin = static_cast<RouteOrigin>(rtm->rtm_protocol);
    int len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*rtm));

    for (rtattr* rtm_attr = RTM_RTA(rtm); RTA_OK(rtm_attr, len); rtm_attr = RTA_NEXT(rtm_attr, len))
    {
        switch (rtm_attr->rta_type)
        {
        case RTA_OIF:
            nr.port_index = *reinterpret_cast<uint32_t*>(RTA_DATA(rtm_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " = ", nr.port_index);
            break;

        case RTA_DST:
            nr.destination = *reinterpret_cast<uint32_t*>(RTA_DATA(rtm_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " = ", addr2string(nr.destination));
            break;

        case RTA_GATEWAY:
            nr.gateway = *reinterpret_cast<uint32_t*>(RTA_DATA(rtm_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " = ", addr2string(nr.gateway));
            break;

        case RTA_PRIORITY:
            nr.metric = *reinterpret_cast<uint32_t*>(RTA_DATA(rtm_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " = ", nr.metric);
            break;

        case RTA_TABLE:
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " = ", *reinterpret_cast<uint32_t*>(RTA_DATA(rtm_attr)));
            break;

        case RTA_PREFSRC:
            nr.source = *reinterpret_cast<uint32_t*>(RTA_DATA(rtm_attr));
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " = ", addr2string(nr.source));
            break;

        default:
            if (netlink_debug) tnt::Log::info("Found RTA attribute: ", rtas[rtm_attr->rta_type], " (value not parsed, ", rtm_attr->rta_len, " bytes long)");
            break;
        }
    }

    if (netlink_debug) tnt::Log::info("\troute to ", addr2string(nr.destination), "/", nr.prefix, " via ", addr2string(nr.gateway), " by port ", nr.port_index, " with metric ", nr.metric , " in table ", (int) rtm->rtm_table, " src ", addr2string(nr.source));

    return nr;
}


void addattr_l(nlmsghdr *n, unsigned int max_len, int type, const void *data, int alen)
{
    unsigned int len = RTA_LENGTH(alen);

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > max_len)
    {
        throw 1;
    }

    rtattr* rta = reinterpret_cast<rtattr*>(reinterpret_cast<char*>(n) + NLMSG_ALIGN (n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;

    memcpy(RTA_DATA(rta), data, alen);

    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
}

void addattr_32(nlmsghdr* n, unsigned int max_len, int type, uint32_t data)
{
    unsigned int len = RTA_LENGTH(sizeof(data));

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > max_len)
    {
        throw 1;
    }

    rtattr* rta = reinterpret_cast<rtattr*>(reinterpret_cast<char*>(n) + NLMSG_ALIGN (n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;

    memcpy(RTA_DATA(rta), &data, sizeof(data));

    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
}

void throw_filtered(const nlmsghdr* nlh)
{
    std::stringstream ss;
    ss << "NetlinkProtocol: dropping netlink message type [" << type2string(nlh->nlmsg_type) << "], sequence [" << nlh->nlmsg_seq << "], port id [" << nlh->nlmsg_pid << "]";

    throw drop::IgnoredMessage(ss.str());
}

void print_nlmsghdr_info(const nlmsghdr* nlh)
{
    tnt::Log::info(colors::yellow, "\n-----------------------------------------------------------------");
    tnt::Log::info(colors::yellow, "| type\t\t| pid\t| seq\t\t| len\t| mul\t| req\t|");
    tnt::Log::info(colors::yellow, "-----------------------------------------------------------------");
    tnt::Log::info(colors::yellow, "| ", type2string(nlh->nlmsg_type), "\t| ", nlh->nlmsg_pid, "\t| ", nlh->nlmsg_seq, nlh->nlmsg_seq ? "\t| " : "\t\t| ", nlh->nlmsg_len, "\t| ", (nlh->nlmsg_flags & NLM_F_MULTI) ? "Yes" : "No", "\t| ", (nlh->nlmsg_flags & NLM_F_REQUEST) ? "Yes" : "No", "\t|");
    tnt::Log::info(colors::yellow, "-----------------------------------------------------------------");
}

std::string type2string(uint16_t type)
{
    static const std::string nlmsg[] = { "NLMSG_NOOP", "NLMSG_ERROR", "NLMSG_DONE", "NLMSG_OVERRUN" };
    static const std::string rtm[] = { "RTM_NEWLINK", "RTM_DELLINK", "RTM_GETLINK", "RTM_SETLINK",
                                    "RTM_NEWADDR", "RTM_DELADDR", "RTM_GETADDR", "UNKNOWN",
                                    "RTM_NEWROUTE", "RTM_DELROUTE", "RTM_GETROUTE", "UNKNOWN" };

    if (type < NLMSG_MIN_TYPE)
    {
        return nlmsg[type - 1];
    }
    //else if (RTM_BASE <= type && type < RTM_MAX)	// the rtm vector is not complete
    else if (RTM_BASE <= type && type < 28)
    {
        return rtm[type - RTM_BASE];
    }
    else
    {
        return std::string("UNKNOWN");
    }
}

std::string addr2string(uint32_t addr)
{
    std::stringstream ss;
    ss << (addr & 0xff) << "." << ((addr >> 0x08) & 0xff) << "." << ((addr >> 0x10) & 0xff) << "." << (addr >> 0x18);

    return ss.str();
}

NetlinkReplyInfo parse_reply_info(const nlmsghdr* nlh)
{
    NetlinkReplyInfo nri = NetlinkReplyInfo();

    if (nlh->nlmsg_flags & NLM_F_ACK)
    {
        nri.ack = true;
        nri.request = std::move(std::string(reinterpret_cast<const char*>(nlh), sizeof(*nlh)));
    }

    nri.pid = nlh->nlmsg_pid;
    nri.sequence = nlh->nlmsg_seq;

    return nri;
}

void print_request_flags(const nlmsghdr* nlh)
{
    /*
     *    NLM_F_REPLACE   Replace existing matching object.
     *    NLM_F_EXCL      Don't replace if the object already exists.
     *    NLM_F_CREATE    Create object if it doesn't already exist.
     *    NLM_F_APPEND    Add to the end of the object list.
     *
     *	4.4BSD ADD           NLM_F_CREATE|NLM_F_EXCL
     *	4.4BSD CHANGE        NLM_F_REPLACE
     *
     *		True CHANGE          NLM_F_CREATE|NLM_F_REPLACE
     *		Append               NLM_F_CREATE
     *		Check                NLM_F_EXCL
     */

    tnt::Log::info(colors::yellow, "NLF_F_REQUEST flag ", nlh->nlmsg_flags & NLM_F_REQUEST ? "" : "not ", "set");
    tnt::Log::info(colors::yellow, "NLM_F_REPLACE flag ", nlh->nlmsg_flags & NLM_F_REPLACE ? "" : " not ", "set");
    tnt::Log::info(colors::yellow, "NLM_F_EXCL flag ", nlh->nlmsg_flags & NLM_F_EXCL ? "" : " not ", "set");
    tnt::Log::info(colors::yellow, "NLM_F_CREATE flag ", nlh->nlmsg_flags & NLM_F_CREATE ? "" : " not ", "set");
    tnt::Log::info(colors::yellow, "NLM_F_APPEND flag ", nlh->nlmsg_flags & NLM_F_APPEND ? "" : " not ", "set");
}

bool replace_flag(const nlmsghdr* nlh)
{ 
    return nlh->nlmsg_flags & NLM_F_REPLACE;
}

bool create_flag(const nlmsghdr* nlh)
{ 
    return nlh->nlmsg_flags & NLM_F_CREATE;
}

} // namespace protocol
} // namespace drop

#ifdef __clang__
#pragma clang diagnostic pop
#endif
