
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

#include "lcp_sync.hpp"

#include <atomic>

#include "event/quit.hpp"
#include "event/gal/slot_change.hpp"

#include "ip_address.hpp"
#include "ip_socket_address.hpp"

#include "application.hpp"
#include "log.hpp"

namespace drop {
namespace activity {

LCPSync::LCPSync(const std::string& ip, int port, int num_slots): ip_(ip), port_(port), num_slots_(num_slots) {}

LCPSync::~LCPSync()
{
    tnt::Log::debug(colors::green, "LCPSync activity terminated.");
}

void LCPSync::operator()()
{
    auto running = true;

    register_handler([&] (const tnt::event::Quit& /*event*/)
    {
        running = false;
        sock_.close();
        tnt::Log::debug(colors::cyan, "LCPSync socket closed.");
    });

    try
    {
        tnt::ip::SocketAddress lep(ip_, port_);

        sock_.set_option(tnt::ip::Broadcast(true));
        sock_.bind(lep);

        tnt::ip::ReceiveBuffer opt;
        sock_.get_option(opt);

        auto size = opt.value();

        tnt::ip::SocketAddress rep;

        int prev = 1;

        while (running)
        {
            std::string buf;
            buf.resize(size);

            try
            {
                sock_.receive_from(buf, rep);
            }
            catch (std::exception& ex)
            {
                if (!running)
                {
                    break;
                }

                continue;
            }

            auto pos_s = buf.find("<slot>");
            auto pos_e = buf.find("</slot>", pos_s + 6);

            if (pos_s == std::string::npos || pos_e == std::string::npos)
            {
                tnt::Log::error("LCPSync: Wrong message received from sync server (invalid xml).");

                continue;
            }

            pos_s += 6;

            auto num = std::stoi(buf.substr(pos_s, pos_e - pos_s));

            if (num < 1 || num > num_slots_)
            {
                tnt::Log::error("LCPSync: Wrong message received from sync server (invalid slot ", num, ").");

                continue;
            }

            if (num == prev)
            {
                continue;
            }

            prev = num;

            tnt::Log::info(colors::cyan, "LCPSync: Sending slot ", num);
            tnt::Application::raise(event::SlotChange(num));
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error("LCPSync::run error: ", ex.what());
    }
}

} // namespace activity
} // namespace drop
