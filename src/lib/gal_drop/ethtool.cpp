
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

#include "ethtool.hpp"

#include <unistd.h>

#include <linux/ethtool.h>
#include <linux/sockios.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "exception/drop_exception.hpp"

#include "socket.hpp"

namespace drop {
namespace {

void check_iface_name(const std::string& iface)
{
    if (iface.empty())
    {
        throw tnt::EmptyArgument("iface");
    }
}

template <class T> void send_request(T& eth_tool_data, const std::string& iface)
{
    tnt::ip::udp::Socket sock;

    ifreq ifr = ifreq();

    std::fill_n(ifr.ifr_name, IFNAMSIZ, 0);
    std::copy(std::begin(iface), std::end(iface), ifr.ifr_name);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&eth_tool_data);

    if (ioctl(sock.get(), SIOCETHTOOL, &ifr) == -1)
    {
        throw tnt::OperationNotSupported();
    }
}

} // namespace

namespace wol {

Capability capability(const std::string& iface)
{
    check_iface_name(iface);

    ethtool_wolinfo wolinfo = ethtool_wolinfo();
    wolinfo.cmd = ETHTOOL_GWOL;

    send_request(wolinfo, iface);

    Capability wc = Capability::None;

    if ((wolinfo.supported & WAKE_MAGIC) == WAKE_MAGIC)
    {
        wc = (wolinfo.wolopts & WAKE_MAGIC) == WAKE_MAGIC ? Capability::Enabled : Capability::Available;
    }

    return wc;
}

void enable(const std::string& iface)
{
    check_iface_name(iface);

    ethtool_wolinfo wolinfo = ethtool_wolinfo();
    wolinfo.cmd = ETHTOOL_GWOL;

    send_request(wolinfo, iface);	// it doesn't make sense, verify

    wolinfo.wolopts = WAKE_MAGIC;
    wolinfo.cmd = ETHTOOL_SWOL;

    send_request(wolinfo, iface);
}

} // namespace wol
} // namespace drop 
