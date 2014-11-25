
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

#include "event/network/address_events.hpp"
#include "event/network/port_events.hpp"
#include "event/network/route_events.hpp"

#include "message/network/management.hpp"

#include "protocol/netlink/netlink_utils.hpp"

#include "log.hpp"
#include "application.hpp"

namespace drop {
namespace protocol {

void KernelNetlinkProtocol::setup_sockets()
{
    cmd_peer_addr_ = sockaddr_nl();
    cmd_peer_addr_.nl_family = AF_NETLINK;
    cmd_peer_addr_.nl_pid = 0;	// port id 0 stands for kernel

    //==================== command socket ========================//

    cmd_sock_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

    if (cmd_sock_ < 0)
    {
        throw tnt::IODataError();
    }

    unsigned int len = sizeof(cmd_tx_buff_size_);

    if (getsockopt(cmd_sock_, SOL_SOCKET, SO_SNDBUF, &cmd_tx_buff_size_, &len) < 0)
    {
        throw tnt::IODataError();
    }

    unsigned int cmd_rx_buffer_size;

    if (getsockopt(cmd_sock_, SOL_SOCKET, SO_RCVBUF, &cmd_rx_buffer_size, &len) < 0)
    {
        throw tnt::IODataError();
    }

    cmd_rx_buffer_.resize(cmd_rx_buffer_size);

    cmd_local_addr_ = sockaddr_nl();
    cmd_local_addr_.nl_pid = getpid();
    cmd_local_addr_.nl_family = AF_NETLINK;

    if (bind(cmd_sock_, reinterpret_cast<sockaddr*>(&cmd_local_addr_), sizeof(cmd_local_addr_)) < 0)
    {
        throw tnt::IODataError();
    }

    //===========================================================//

    rx_sock_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

    if (rx_sock_ < 0)
    {
        throw tnt::IODataError();
    }

    if (fcntl(rx_sock_, F_SETFL, O_NONBLOCK) < 0)
    {
        throw tnt::IODataError();
    }

    unsigned int rx_buffer_size;

    if (getsockopt(rx_sock_, SOL_SOCKET, SO_RCVBUF, &rx_buffer_size, &len) < 0)
    {
        throw tnt::IODataError();
    }

    rx_buffer_.resize(rx_buffer_size);

    sockaddr_nl addr = sockaddr_nl();
    addr.nl_pid = getpid() + 1;
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR; // | RTMGRP_NOTIFY;

    if (bind(rx_sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        throw tnt::IODataError();
    }
}

//	Rx handlers: netlink packet -> Event
void KernelNetlinkProtocol::register_netlink_handlers()
{
    event_dispatcher_.register_listener(RTM_NEWLINK, [this] (const nlmsghdr* nlh)
    {
        auto ifi = reinterpret_cast<ifinfomsg*>(NLMSG_DATA(nlh));

        if (ifi->ifi_change == 0xffffffff || ifi->ifi_change == 0x0)
        {
            tnt::Application::raise(event::LocalPortAdded(link_parser(nlh, true, netlink_debug)), this);
        }
        else if (ifi->ifi_change & IFF_UP)	// !IFF_DORMANT
        {
            if (!(ifi->ifi_flags & IFF_UP))
            {
                tnt::Application::raise(event::LocalPortAdminDown(link_parser(nlh, true, netlink_debug)), this);
            }
            else
            {
                tnt::Application::raise(event::LocalPortAdminUp(link_parser(nlh, true, netlink_debug)), this);
            }
        }
        else
        {
            tnt::Application::raise(event::UndecodedPortEvent(link_parser(nlh, true, netlink_debug)), this);
        }
    });

    event_dispatcher_.register_listener(RTM_DELLINK, [this] (const nlmsghdr* nlh)
    {
        tnt::Application::raise(event::LocalPortDeleted(link_parser(nlh, true, netlink_debug)), this);
    });

    event_dispatcher_.register_listener(RTM_NEWADDR, [this] (const nlmsghdr* nlh)
    {
        if (nlh->nlmsg_pid == cmd_local_addr_.nl_pid)
        {
            if (netlink_debug) tnt::Log::info(colors::red, "reply message to our request");
        }

        tnt::Application::raise(event::LocalAddressAdded(address_parser(nlh, true, netlink_debug)), this);
    });

    event_dispatcher_.register_listener(RTM_DELADDR, [this] (const nlmsghdr* nlh)
    {
        if (nlh->nlmsg_pid == cmd_local_addr_.nl_pid)
        {
            if (netlink_debug) tnt::Log::info(colors::red, "reply message to our request");
        }

        tnt::Application::raise(event::LocalAddressRemoved(address_parser(nlh, true, netlink_debug)), this);
    });

    event_dispatcher_.register_listener(RTM_NEWROUTE, [this] (const nlmsghdr* nlh)
    {
        if (nlh->nlmsg_pid == cmd_local_addr_.nl_pid)
        {
            if (netlink_debug) tnt::Log::info(colors::red, "reply message to our request");
        }

        tnt::Application::raise(event::LocalRouteAdded(route_parser(nlh, true, netlink_debug)), this);
    });

    event_dispatcher_.register_listener(RTM_DELROUTE, [this] (const nlmsghdr* nlh)
    {
        if (nlh->nlmsg_pid == cmd_local_addr_.nl_pid)
        {
            if (netlink_debug) tnt::Log::info(colors::red, "reply message to our request");
        }

        tnt::Application::raise(event::LocalRouteRemoved(route_parser(nlh, true, netlink_debug)), this);
    });
}

void KernelNetlinkProtocol::register_message_handlers()
{
	message_dispatcher_.register_listener<message::AddLink>([this] (message::AddLink* message)
	{
		if (send_and_parse(build_add_link(message->type(), message->name(), message->mac(), message->mtu())))
		{
			tnt::Log::error("AddLink Failed");
		}
	});

    message_dispatcher_.register_listener<message::GetRoutes>([this] (message::GetRoutes* /*message*/)
    {
        if (send_and_parse(build_dump(NetlinkDumpType::Routes, protocol::NetlinkFamily::Inet)))
        {
            tnt::Log::error("GetRoutes Failed");
        }
    });
}

} // namespace protocol
} // namespace drop
