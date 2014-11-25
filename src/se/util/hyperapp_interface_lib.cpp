
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

#if 0

#ifdef XLP_FE

#include "hyperapp_interface_lib.hpp"

#include <algorithm>
#include <thread>

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <net/if.h>
#include <arpa/inet.h>

// XLP specific includes
#include "usr/include/sys/nlm_sched.h"

#include <fcntl.h>

#include "platform.hpp"

#if defined(TNT_PLATFORM_LINUX)

#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC

#include <linux/ethtool.h>
#include <linux/sockios.h>

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <sched.h>

#endif

#include "util/configuration.hpp"

#include "file_descriptor.hpp"
#include "log.hpp"

#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

#define CLONE_BUDDY_FLGS           	(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND)

namespace drop {
namespace {

netmap_ring* netmap_txring(netmap_if* nifp, int index)
{
	return reinterpret_cast<netmap_ring*>(reinterpret_cast<char*>(nifp) + nifp->ring_ofs[index]);
}

netmap_ring* netmap_rxring(netmap_if* nifp, int index)
{
	return reinterpret_cast<netmap_ring*>(reinterpret_cast<char*>(nifp) + nifp->ring_ofs[index + nifp->ni_tx_rings + 1]);
}

int netmap_ring_next_slot(netmap_ring* r, uint32_t i)
{
	return i + 1 == r->num_slots ? 0 : i + 1;
}

char* netmap_buf(netmap_ring* r, uint32_t i)
{
	return reinterpret_cast<char*>(r) + r->buf_ofs + i * r->nr_buf_size;
}

int child_function(void *arg)
{
//	int i=0, msize = (1<<20); /* 1MB */
//	int nblocks = msize / sizeof(MEM_TYPE);
//	MEM_TYPE *mem = 0;

	printf("Child process running pid=%d arg=%lx\n", getpid(), (long)arg);

//	/* malloc */
//	printf("Child: allocating memory (size=%d bytes)...\n", msize);
//	mem = malloc(msize);
//	if (!mem) {
//		printf("Child: unable to allocate memory (size=%d bytes)\n", msize);
//		printf("pid = %d exiting (failure)\n", getpid());
//		exit(EXIT_FAILURE);
//	}
//
//	printf("Child:Writing to memory...@ 0x%lx\n", (long)mem);
//	for(i=0;i<nblocks;i++) {
//		mem[i] = MEM_PATTERN;
//	}
//
//	printf("Child:Reading from memory @ 0x%lx\n", (long)mem);
//	for(i=0;i<nblocks;i++) {
//		if (mem[i] != MEM_PATTERN) {
//			printf("Child:Memory read at byte[%x] failed!\n", i);
//			printf("pid = %d exiting (failure)\n", getpid());
//			exit(EXIT_FAILURE);
//		}
//	}
//	free(mem);
//
//	if((long)arg == 1)
//		mem_test_child();
//
//	printf("pid = %d exiting (success)\n", getpid());
//	exit(EXIT_SUCCESS);

	return 0;
}

void launch_children(void)
{
	long instance = 1;
	char *child_stack;
	int  childid = 0, size = 32 * 1024;

	for(;;) {
		/* parent will clone the child till no cpus are left
		 */
		if((child_stack = (char *) malloc(size)) == NULL) {
			printf("[%s] Mem alloc failed \n", __FUNCTION__);
			break;
		}

		/* stack grows from top to bottom
		 */
		child_stack = child_stack + size - 64;

		childid = clone(child_function, child_stack, CLONE_BUDDY_FLGS, (void *)instance);
		if (childid < 0) {
			printf("[Parent] : Failed to clone!\n");
			break;
		}

		instance++;
		printf("[Parent] : Child process (%d) created\n", childid);
	}
}

} // namespace

HyperAppInterfaceLib::HyperAppInterfaceLib(LookupTable& table, std::atomic_bool& running) : InterfaceLib(table), running_{ running }, num_tasks_{ 0 }
{
	acquire();
}

HyperAppInterfaceLib::~HyperAppInterfaceLib()
{
	release();
}

int HyperAppInterfaceLib::num_tasks() const
{
	return num_tasks_;
}

void HyperAppInterfaceLib::start_task(int num_task, std::atomic_uint_fast64_t& rx_packets, std::atomic_uint_fast64_t& tx_packets, std::atomic_uint_fast64_t& drop_packets, std::atomic_bool& flag)
{
	std::thread th([&, num_task]()
	{
		launch_children();
		forward_loop(mem_, num_task, rx_packets, tx_packets, drop_packets, flag);
	});

	th.detach();
}

void HyperAppInterfaceLib::acquire()
{
	// perform init
}

void HyperAppInterfaceLib::release()
{
	// perform exit
}

void HyperAppInterfaceLib::forward_loop(std::shared_ptr<char> mem, int ringid, std::atomic_uint_fast64_t& rx, std::atomic_uint_fast64_t& tx, std::atomic_uint_fast64_t& drop, std::atomic_bool& sleep)
{
	cpu_set_t cpumask;

	CPU_ZERO(&cpumask);
	CPU_SET(ringid, &cpumask);

	if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpumask) != 0)
	{
		tnt::Log::error("Unable to set affinity: ", std::strerror(errno));
	}

	tnt::FileDescriptor fd("/dev/netmap", O_RDWR);

	if (*fd < 0)
	{
		tnt::Log::error("HyperAppInterfaceLib::forward_loop: ", std::strerror(errno));

		return;
	}

	nmreq req = nmreq();
	req.nr_version = NETMAP_API;
	auto name = tnt::Configuration::get("netmap.interface").as<std::string>();

	std::fill_n(req.nr_name, IFNAMSIZ, 0);
	strncpy(req.nr_name, name.c_str(), sizeof(req.nr_name));

	req.nr_ringid = NETMAP_HW_RING + ringid;

	int err = ioctl(*fd, NIOCREGIF, &req);

	if (err != 0)
	{
		tnt::Log::error("HyperAppInterfaceLib::run: Unable to register ", name, " (", std::strerror(errno), ")");

		return;
	}

	auto nifp = reinterpret_cast<netmap_if*>(mem.get() + req.nr_offset);

	std::this_thread::sleep_for(std::chrono::seconds(5));

	auto burst = tnt::Configuration::get("netmap.burst").as<unsigned int>();

	auto rxring = netmap_rxring(nifp, ringid);
	auto txring = netmap_txring(nifp, ringid);

	tnt::Log::info(colors::blue, "Ring #", ringid, " up.\n", colors::green, "Starting forwarding on ring #", ringid, " burst size: ", burst);
	
	auto sleep_time = tnt::Configuration::get("netmap.sleep").as<int>();

	while (running_)
	{
		if (unlikely(sleep))
		{
			if (burst > 1 && rxring->avail > 0) // Empty the queues.
			{
				process_rings(rxring, txring, rx, tx, drop);
				ioctl(*fd, NIOCTXSYNC, nullptr);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		}

		ioctl(*fd, NIOCRXSYNC, nullptr);

		if (rxring->avail < burst)
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));

			continue;
		}

		process_rings(rxring, txring, rx, tx, drop);
		ioctl(*fd, NIOCTXSYNC, nullptr);
	}

	ioctl(*fd, NIOCUNREGIF, nullptr);
	tnt::Log::info(colors::blue, "Interface up: exiting forwarding loop on ring #", ringid);
}

void HyperAppInterfaceLib::process_rings(netmap_ring* rxring, netmap_ring* txring, std::atomic_uint_fast64_t& rx, std::atomic_uint_fast64_t& tx, std::atomic_uint_fast64_t& drop)
{
	u_int j = rxring->cur;
	u_int k = txring->cur;

	auto limit = rxring->avail;
	rx += limit;

	if (txring->avail < limit)
	{
		limit = txring->avail;
	}

	auto m = limit;

	while (limit-- > 0)
	{
		auto& rs = rxring->slot[j];
		auto& ts = txring->slot[k];

		if (ts.buf_idx < 2 || rs.buf_idx < 2)
		{
			tnt::Log::info(colors::red, "Wrong index rx[", j, "] = ", rs.buf_idx, " -> tx[", k, "] = ", ts.buf_idx);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		if (likely(rs.len > 34 && rs.len < 2048)) // Not an IP packet if rs.len < 34.
		{
			auto pkt = netmap_buf(rxring, rs.buf_idx);

			if (likely(pkt[12] == 0x08 && pkt[13] == 0x00)) // IP packet
			{
				if (likely(forward_packet(pkt)))	// InterfaceLib method
				{
					std::swap(ts.buf_idx, rs.buf_idx);

					ts.len = rs.len;

					ts.flags |= NS_BUF_CHANGED;
					rs.flags |= NS_BUF_CHANGED;

					++tx;
				}
				else
				{
					++drop;
				}
			}
			else
			{
				++drop;
			}
		}
		else
		{
			++drop;
		}

		j = netmap_ring_next_slot(rxring, j);
		k = netmap_ring_next_slot(txring, k);
	}

	rxring->avail -= m;
	txring->avail -= m;
	rxring->cur = j;
	txring->cur = k;
}

} // namespace drop

#endif

#endif
