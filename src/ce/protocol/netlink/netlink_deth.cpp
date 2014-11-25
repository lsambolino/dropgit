
/*

Copyright (c) 2013, Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#include "netlink_deth.hpp"

#include <cassert>
#include <cstring>

#include <linux/rtnetlink.h>
#include <sys/socket.h> // for sa_family_t
#include <net/if_arp.h>

#include "event/virtual_interfaces_events.hpp"

#include "message/virtual_interfaces_messages.hpp"

#include "log.hpp"
#include "dump.hpp"
#include "application.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#endif

namespace drop {
namespace protocol {

enum message_type
{
    MULTICAST_ADDRESS_CHANGE,
    PACKET,
    LINK_CHANGE,
	LINK_STATS
};

NetlinkDethProtocol::NetlinkDethProtocol(const std::shared_ptr<tnt::IO>& io): AsyncProtocol(io) {}

void NetlinkDethProtocol::invoke_message(const std::string& message)
{
    size_t len = message.size();

	for (const nlmsghdr* nlh = reinterpret_cast<const nlmsghdr*>(message.data()); NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len))
    {
        auto ptr = reinterpret_cast<const char*>(NLMSG_DATA(nlh));

        auto index = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += sizeof(uint32_t);

        switch (nlh->nlmsg_type)
        {
        case MULTICAST_ADDRESS_CHANGE:
            tnt::Application::raise(event::OspfMulticastChange(index));
            break;
        case PACKET:
            assert(nlh->nlmsg_len - sizeof(uint32_t) > 0);
            tnt::Application::raise(event::DethOutPacket(index, std::string(ptr, nlh->nlmsg_len - sizeof(uint32_t))));
            break;
        default:
            assert(false); // TODO: Use an exception.
        }
    }
}

std::vector<std::string> NetlinkDethProtocol::parse(std::string& raw_input)
{
    std::vector<std::string> messages{ raw_input };
    raw_input.clear();

    return messages;
}

void NetlinkDethProtocol::register_messages()
{
    register_message<message::DethInPacket>([this] (auto message)
    {
		deth_in_packet(std::move(message));
    });

    register_message<message::DethLinkChange>([this] (auto message)
    {
		deth_link_change(std::move(message));
    });

	register_message<message::DethLinkStats>([this] (auto message)
    {
		deth_link_stats(std::move(message));
    });
}

void NetlinkDethProtocol::deth_in_packet(message::DethInPacket* message)
{
    const auto& pkt = message->packet();
    uint32_t idx = message->interface();

    const auto size = NLMSG_LENGTH(sizeof(idx) + pkt.size());
	auto buf = std::string(size, '0');

    auto ptr = reinterpret_cast<char*>(&buf[0]);
    auto req = reinterpret_cast<nlmsghdr*>(ptr);

    req->nlmsg_len = size;
    req->nlmsg_type = PACKET;
    req->nlmsg_flags = 0;
    req->nlmsg_pid = getpid();
    req->nlmsg_seq = 0;

    ptr = reinterpret_cast<char*>(NLMSG_DATA(req));

    memcpy(ptr, &idx, sizeof(idx));
    ptr += sizeof(idx);
    memcpy(ptr, pkt.data(), pkt.size());

    write(buf);
}

void NetlinkDethProtocol::deth_link_change(message::DethLinkChange* message)
{
    uint32_t idx = message->interface();
    uint32_t value = message->up() ? 1 : 0;

    const auto size = NLMSG_LENGTH(sizeof(idx) + sizeof(value));
    auto buf = std::string(size, '0');

    auto ptr = reinterpret_cast<char*>(&buf[0]);
    auto req = reinterpret_cast<nlmsghdr*>(ptr);

    req->nlmsg_len = size;
    req->nlmsg_type = LINK_CHANGE;
    req->nlmsg_flags = 0;
    req->nlmsg_pid = getpid();
    req->nlmsg_seq = 0;

    ptr = reinterpret_cast<char*>(NLMSG_DATA(req));

    memcpy(ptr, &idx, sizeof(idx));
    ptr += sizeof(idx);
    memcpy(ptr, &value, sizeof(value));

    write(buf);
}

void NetlinkDethProtocol::deth_link_stats(message::DethLinkStats* message)
{
    uint32_t idx = message->interface();
    uint64_t rx_packets = message->rxpkts();
	uint64_t rx_bytes = message->rxbytes();
	uint64_t tx_packets = message->txpkts();
	uint64_t tx_bytes = message->txbytes();

    const auto size = NLMSG_LENGTH(sizeof(idx) + 4 * sizeof(uint64_t));
    auto buf = std::string(size, '0');

    auto ptr = reinterpret_cast<char*>(&buf[0]);
    auto req = reinterpret_cast<nlmsghdr*>(ptr);

    req->nlmsg_len = size;
    req->nlmsg_type = LINK_STATS;
    req->nlmsg_flags = 0;
    req->nlmsg_pid = getpid();
    req->nlmsg_seq = 0;

    ptr = reinterpret_cast<char*>(NLMSG_DATA(req));

    memcpy(ptr, &idx, sizeof(idx));
    ptr += sizeof(idx);
		
	memcpy(ptr, &rx_packets, sizeof(rx_packets));
	ptr += sizeof(rx_packets);

	memcpy(ptr, &rx_bytes, sizeof(rx_bytes));
	ptr += sizeof(rx_bytes);

	memcpy(ptr, &tx_packets, sizeof(tx_packets));
	ptr += sizeof(tx_packets);

	memcpy(ptr, &tx_bytes, sizeof(tx_bytes));

    write(buf);
}

} // namespace protocol
} // namespace drop

#ifdef __clang__
#pragma clang diagnostic pop
#endif
