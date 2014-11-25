
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

#include "netlink_io.hpp"

#include <algorithm>
#include <atomic>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/rtnetlink.h>

#include "exception/drop_exception.hpp"

#include "log.hpp"

namespace drop {
namespace io {

NetlinkIO::NetlinkIO(NetlinkSocketProtocol proto): sock_fd_(socket(AF_NETLINK, SOCK_RAW, static_cast<int>(proto)))
{
    if (sock_fd_ < 0)
    {
        throw tnt::IODataError();
    }

    if (fcntl(sock_fd_, F_SETFL, O_NONBLOCK) < 0)
    {
        throw tnt::IODataError();
    }

    const size_t len = buff_len;

    if (setsockopt(sock_fd_, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len)) != 0)
    {
        throw tnt::IODataError();
    }

    auto laddr = sockaddr_nl();
    laddr.nl_pid = getpid();
    laddr.nl_family = AF_NETLINK;
    laddr.nl_groups = 1;

    if (bind(sock_fd_, reinterpret_cast<sockaddr*>(&laddr), sizeof(laddr)) < 0)
    {
        throw tnt::IODataError();
    }
}

void NetlinkIO::reset()
{
    if (sock_fd_ != -1)
    {
        shutdown(sock_fd_, SHUT_RDWR);
        close(sock_fd_);
        sock_fd_ = -1;
    }
}

void NetlinkIO::write(const std::string& data)
{
    sockaddr_nl addr_k = sockaddr_nl(); 

    addr_k.nl_family = AF_NETLINK;
    addr_k.nl_pid = 0;
    addr_k.nl_groups = 1;

    sendto(sock_fd_, data.data(), data.size(), 0, reinterpret_cast<sockaddr*>(&addr_k), sizeof(addr_k));
}

bool NetlinkIO::try_read(std::string& data)
{
    const unsigned int timeout = 10;	// millisec

    pollfd pf = pollfd();

    pf.fd = sock_fd_;
    pf.events = POLLIN;

    if (poll(&pf, 1, timeout) <= 0)
    {
        return false;
    }

    if (pf.revents & POLLIN)
    {
        data = do_read();

        return true;
    }
        
    return false;
}

std::string NetlinkIO::read()
{
    const unsigned int timeout = 1000;	// millisec

    pollfd pf = pollfd();

    pf.fd = sock_fd_;
    pf.events = POLLIN;

    while (true)
    {
        if (poll(&pf, 1, timeout) <= 0)
        {
            continue;
        }

        if (pf.revents & POLLIN)
        {
            return do_read();
        }
        
        return "";
    }

    tnt::Log::info(colors::green, "==> Exiting NetlinkIO::read loop <==");
}

std::string NetlinkIO::do_read()
{
    int dim = recv(sock_fd_, buff_.data(), buff_len, 0);

    std::string data(buff_.data(), dim);
    std::fill(std::begin(buff_), std::begin(buff_) + dim, 0);

    return data;
}

} // namespace io
} // namespace drop
