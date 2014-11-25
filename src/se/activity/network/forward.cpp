
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

#include "forward.hpp"

#include <thread>
#include <memory>
#include <iterator>
#include <algorithm>
#include <atomic>
#include <vector>
#include <iostream>

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <sched.h>

#include "message/network/arp.hpp"

#include "router/de_control_element.hpp"

#include "event/quit.hpp"
#include "event/network/arp.hpp"
#include "event/network/toggle_rate.hpp"
#include "event/network/interface_list.hpp"
#include "event/network/routes_management.hpp"
#include "event/network/forwarding_started.hpp"
#include "event/gal/active_cores_change.hpp"
#include "event/gal/profile_change.hpp"
#include "event/gal/statistics_update.hpp"

#include "util/crc.hpp"
#include "util/configuration.hpp"
#include "util/interface_lib.hpp"
#include "util/hyperapp_interface_lib.hpp"
#include "util/netmap_interface_lib.hpp"
#include "util/interfaces.hpp"

#include "ip_address.hpp"
#include "file_descriptor.hpp"
#include "log.hpp"
#include "dump.hpp"
#include "memory.hpp"
#include "application.hpp"
#include "thread.hpp"

namespace drop {
namespace activity {
namespace {

void set_affinity(std::thread::native_handle_type id, int cpu)
{
    cpu_set_t cpumask;

	CPU_ZERO(&cpumask);
	CPU_SET(cpu, &cpumask);

	if (pthread_setaffinity_np(id, sizeof(cpu_set_t), &cpumask) != 0)
	{
		tnt::Log::error("Unable to set affinity: ", std::strerror(errno));
	}
}

} // namespace

Forward::Forward(): table_{ tnt::Configuration::get("netmap.entries").as<int>() } {}

Forward::~Forward()
{
    tnt::Log::debug(colors::green, "Forward activity terminated.");
}

void Forward::operator()()
{
    std::atomic_bool running{ true };

	register_handler([&] (const tnt::event::Quit& /*event*/)
	{
		running = false;
	});

	register_handler([this] (const event::RouteListRequest& event)
	{
		int num = 0;

		table_.for_each([&] (const auto& p)
		{
			event.os() << tnt::ip::Address::from_net_order_ulong(p.first) << "/" << p.second.prefix << " " << tnt::MacAddress(std::string(p.second.dst, 6)) << std::endl;
			++num;
		});

		if (num == 0)
		{
			event.os() << "No routes" << std::endl;
		}
	});

	register_handler([this] (const event::AddUserspaceRoute& event)
	{
		auto ri = event.route();
        const auto& dst = event.mac();

        ri.port_index = tnt::name_to_index(tnt::Configuration::get("netmap.interface").as<std::string>());

		tnt::Log::info(colors::blue, "event::AddUserspaceRoute: ", ri);

        add(ri.destination, ri.prefix, dst);
	});

	register_handler([this] (const event::DelRoute& event)
	{
		const auto& ri = event.route();
		tnt::Log::info(colors::blue, "event::DelRoute", ri);

		del(ri.destination, ri.prefix);
	});

#ifdef XLP_FE
	HyperAppInterfaceLib lib(table_);
#else
	NetmapInterfaceLib lib(table_);
#endif

	auto n_tasks = lib.num_tasks();

	std::atomic_uint_fast64_t rx_packets{ 0 };
	std::atomic_uint_fast64_t tx_packets{ 0 };
	std::atomic_uint_fast64_t drop_packets{ 0 };

    std::vector<tnt::Thread> forward_threads;
    forward_threads.reserve(n_tasks);

	tnt::Log::info(colors::blue, "Starting ", n_tasks, " threads.");

	for (auto i = 0; i < n_tasks; ++i)
	{
        forward_threads.emplace_back([&lib, &running, i, &rx_packets, &tx_packets, &drop_packets] ()
        {
            lib.forward_loop(running, i, rx_packets, tx_packets, drop_packets);
        });
	}

    for (auto i = 0; i < n_tasks; ++i)
    {
        set_affinity(forward_threads[i].native_handle(), i);
    }

    register_handler([&] (const event::ActiveCoresChange& event)
	{
        for (int i = 0; i < n_tasks; ++i)
        {
            auto cpu = i % event.num();
            tnt::Log::info(colors::cyan, "Ring #", i, " ==> cpu #", cpu);
            set_affinity(forward_threads[i].native_handle(), cpu);
        }

        std::system(std::string("/root/set_affinity.sh ").append(std::to_string(event.num())).c_str());
    });

    std::atomic_bool show_rate{ false };

	register_handler([&] (const event::ToggleRate& /*event*/)
	{
		show_rate = !show_rate;
	});

    tnt::Thread stats_job;

	if (tnt::Configuration::get("forwarding.stats.enable", false))
	{
		stats_job.start([&] ()
		{
			auto stat_interval = tnt::Configuration::get("forwarding.stats.interval", 500);

			std::this_thread::sleep_for(std::chrono::seconds(7));

			auto prev = std::chrono::steady_clock::now();
			uint_fast64_t prev_rx = 0;
			uint_fast64_t prev_tx = 0;
			auto prev_show_rate = false;

			while (running)
			{
                auto diff_rx =  rx_packets - prev_rx;
				auto diff_tx =  tx_packets - prev_tx;

				auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - prev);

				auto rx_rate = static_cast<double>(diff_rx) / static_cast<double>(diff.count()) * static_cast<double>(1000);
				auto tx_rate = static_cast<double>(diff_tx) / static_cast<double>(diff.count()) * static_cast<double>(1000);

                tnt::Application::raise(event::StatisticsUpdate(rx_packets, tx_packets, rx_rate, tx_rate));

				if (show_rate)
				{
                    if (!prev_show_rate)
				    {
					    tnt::Log::output(manip::lf, manip::lf, manip::lf);
				    }

					prev_show_rate = true;

					tnt::Log::output(manip::clear, manip::cr, "Rx: ", rx_rate, " pkts/s", manip::tab, "Tx: ", tx_rate, " pkts/s", manip::tab, "Dropped: ", drop_packets.load(), " pkts");
				}

                prev_rx = rx_packets;
				prev_tx = tx_packets;

				prev = std::chrono::steady_clock::now();

				std::this_thread::sleep_for(std::chrono::milliseconds(stat_interval));
			}
		});
	}

    tnt::Application::raise(event::ForwardingStarted());

    while (running)
	{
		wait_event();
	}
}

void Forward::add(uint32_t address, uint32_t prefix, const tnt::MacAddress& dst)
{
	table_.add(address, prefix, dst);
}

void Forward::del(uint32_t address, uint32_t prefix)
{
	table_.del(address, prefix);
}

} // namespace activity
} // namespace drop
