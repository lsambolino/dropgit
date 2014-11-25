
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

#include "wol.hpp"

#include <thread>
#include <fstream>
#include <regex>
#include <cerrno>
#include <cctype>
#include <cstring>

#include "exception/exception.hpp"

#include "log.hpp"
#include "ip_address.hpp"
#include "mac_address.hpp"
#include "ip_socket_address.hpp"
#include "socket.hpp"

namespace drop {
namespace wol {
namespace {

const int magic_packet_size = 102;

const char* arp_cache = "/proc/net/arp";

tnt::MacAddress hw_from_arp(const std::string& ip)
{
    std::ifstream file(arp_cache);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string line;

    while (std::getline(file, line))
    {
        if (line.find(ip) == std::string::npos)
        {
            continue;
        }

        std::stringstream ss(line);
        std::string tmp0;
        int tmp1;
        int tmp2;
        std::string hw;

        ss >> tmp0 >> tmp1 >> tmp2 >> hw;

        return tnt::MacAddress(hw);
    }

    // If the address is not in the cache there isn't any possibility to send an UDP WOL packet.
    throw std::invalid_argument("Error: The ip address does not exists.");
}

std::string create_magic_packet(const tnt::MacAddress& mac)
{
    char buf[magic_packet_size];

    // The magic packet is a broadcast frame containing anywhere within its payload 6 bytes
    // of all 255 (FF FF FF FF FF FF in hexadecimal), followed by sixteen repetitions of
    // the target computer's 48-bit MAC address, for a total of 102 bytes. (packet_size = 102)
    std::fill_n(std::begin(buf), 6, 0xff);
    char* ptr = buf + 6;
    std::string hw = mac.raw();

    for (int count = 0; count < 16; ++count, ptr += 6)
    {
        std::copy(std::begin(hw), std::end(hw), ptr);
    }

    return std::string(buf, magic_packet_size);
}

void send_magic_packets(const tnt::ip::SocketAddress& remote, const tnt::MacAddress& mac, int num, const std::chrono::microseconds& interval)
{
    tnt::ip::udp::Socket sock;

    if (remote.address().is_broadcast())
    {
        sock.set_option(tnt::ip::Broadcast(true));
    }

    std::string packet = create_magic_packet(mac.empty() ? hw_from_arp(remote.address().to_string()) : mac);

    for (int i=0; i<num; ++i)
    {
        tnt::Log::info(colors::blue, "Sending WOL magic packet to ", remote, " - ", mac);

        int ret = sock.send_to(packet, remote);

        if (ret < magic_packet_size)
        {
            throw tnt::SocketException();
        }

        std::this_thread::sleep_for(interval);
    }
}

} // namespace

void send_magic_packets(int num, const std::chrono::milliseconds& interval, const std::string& ip_addr, const tnt::MacAddress& mac)
{
    send_magic_packets(tnt::ip::SocketAddress(ip_addr, 9), mac, num, interval);
}

void send_bcast_magic_packets(int num, const std::chrono::milliseconds& interval, const tnt::MacAddress& mac)
{
    send_magic_packets(tnt::ip::SocketAddress(tnt::ip::Address::broadcast(), 9), mac, num, interval);
}

} // namespace wol
} // namespace drop
