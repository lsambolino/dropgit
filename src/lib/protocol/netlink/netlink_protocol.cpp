
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

#include "netlink_protocol.hpp"

#include <cstdint>
#include <sstream>

#include <linux/rtnetlink.h>
#include <sys/socket.h> // for sa_family_t
#include <net/if_arp.h>
#include <poll.h>

#include "exception/drop_exception.hpp"

#include "message/message.hpp"

#include "router/port_info.hpp"
#include "router/route_info.hpp"
#include "router/address_info.hpp"

#include "netlink_utils.hpp"

#include "util/configuration.hpp"

#include "log.hpp"
#include "application.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#endif

namespace drop {
namespace protocol {

bool NetlinkProtocol::netlink_debug = false;

NetlinkProtocol::NetlinkProtocol(): running_{ false }, rx_sock_(-1)
{
    netlink_debug = tnt::Configuration::get("netlink.debug", false);
}

NetlinkProtocol::~NetlinkProtocol()
{
    try
    {
        stop();

        if (rx_sock_ != -1)
        {
            close(rx_sock_);
        }
    }
    catch (...) {}
}

void NetlinkProtocol::start()
{
    if (!running_)
    {
        running_ = true;

        tx_thread_.start([this] () { tx_loop(); });
        rx_thread_.start([this] () { rx_loop(); });
    }
}

void NetlinkProtocol::stop()
{
    running_ = false;

    try
    {
        if (messages_.empty())
        {
            messages_.push(MessageNode(std::make_unique<tnt::message::StopMessage>()));
        }
    }
    catch (...) { }
}

std::future<void> NetlinkProtocol::send(std::unique_ptr<tnt::Message>&& message)
{
    std::promise<void> promise;
    auto future = promise.get_future();

    if (running_)
    {
        messages_.push(MessageNode(std::move(message), std::move(promise)));
    }
    else
    {
        promise.set_exception(std::make_exception_ptr(tnt::ProtocolException(tnt::get_name(*this) + " is not running.")));
    }

    return future;
}

void NetlinkProtocol::tx_loop()
{
    while (running_)
    {
        auto node = messages_.pop();
        auto& promise = node.promise;

        try
        {
            auto& message = node.message;
            assert(message);

            if (!running_)
            {
                promise.set_exception(std::make_exception_ptr(tnt::ProtocolException(tnt::get_name(*this) + " is not running.")));

                break;
            }

            if (!message_dispatcher_.inject_object(std::move(message)))
            {
                promise.set_exception(std::make_exception_ptr(tnt::ProtocolException(std::string("Unable to dispatch unknown message ") + get_name(message))));
            }
        }
        catch (...)
        {
            promise.set_exception(std::current_exception());
        }
    }
}

void NetlinkProtocol::rx_loop()
{
    try
    {
        const unsigned int timeout = 1000;	// millisec

        pollfd pf = pollfd();

        pf.fd = rx_sock_;
        pf.events = POLLIN;

        while (running_)
        {
            if (poll(&pf, 1, timeout) <= 0)
            {
                continue;
            }

            if (pf.revents & POLLIN)
            {
                parse_data();
            }
            else if (pf.revents & POLLNVAL)
            {
                break;
            }
            else
            {
                tnt::Log::warning(colors::red, "NetlinkProtocol: rx_sock_ socket state flags: ", pf.revents);
            }
        }
    }
    catch (...)
    {
        assert(false);
    }
}

void NetlinkProtocol::parse_data()
{
    int dim = recv(rx_sock_, rx_buffer_.data(), rx_buffer_.size(), 0);

    // Sanity checks
    if (dim <= 0 || static_cast<size_t>(dim) > rx_buffer_.size() || static_cast<size_t>(dim) > std::string().max_size())
    {
        if (dim != 0)
        {
            tnt::Log::error("NetlinkProtocol::parse_data: recv returned ", dim);
        }

        return;
    }

    if (netlink_debug)
    {
        tnt::Log::info(colors::blue, "\n==> NetlinkProtocol received new data (", dim, " bytes) from socket ", rx_sock_);
    }

	try
	{
        std::string raw_input(rx_buffer_.data(), dim);

        size_t len = raw_input.size();
        size_t pos = 0;

        for (auto nlh = reinterpret_cast<const nlmsghdr*>(raw_input.data()); NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len))
        {
            if (netlink_debug)
            {
                print_nlmsghdr_info(nlh);
            }

            pos += nlh->nlmsg_len;

            if (netlink_debug)
            {
                tnt::Log::info(raw_input.size() - pos, " of ", raw_input.size()," bytes left");
            }

            try
            {
                event_dispatcher_.inject_object(nlh->nlmsg_type, nlh);
            }
            catch (drop::IgnoredMessage& im)
            {
                if (netlink_debug)
                {
                    tnt::Log::info(im.what());
                }
            }
            catch (tnt::ListenerNotFound& lnf)
            {
                tnt::Log::info("NetlinkProtocol::parse_data(): parsing handler not found for message type ", type2string(nlh->nlmsg_type));
            }
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error("NetlinkProtocol::parse_data error: ", ex.what());
    }
}

} // namespace protocol
} // namespace drop

#ifdef __clang__
#pragma clang diagnostic pop
#endif
