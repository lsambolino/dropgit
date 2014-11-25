
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

#ifndef XLP_FE

#include "netmap_interface_lib.hpp"

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

#include <fcntl.h>

#include "util/netmap.h"

#include "platform.hpp"

#if defined(TNT_PLATFORM_LINUX)

#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC

#include <linux/ethtool.h>
#include <linux/sockios.h>

#endif

#include "util/configuration.hpp"
#include "util/likely_macro.hpp"

#include "file_descriptor.hpp"
#include "log.hpp"

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
	return r->num_slots == i + 1 ? 0 : i + 1;
}

char* netmap_buf(netmap_ring* r, uint32_t i)
{
	return reinterpret_cast<char*>(r) + r->buf_ofs + i * r->nr_buf_size;
}

int nm_do_ioctl(const std::string& ifname, uint32_t& if_flags, int what, int subcmd)
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0)
	{
		tnt::Log::error("Error: cannot get device control socket.\n");

		return -1;
	}

	ifreq ifr = ifreq();
	strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name));

	switch (what)
	{
	case SIOCSIFFLAGS:
		ifr.ifr_flagshigh = if_flags >> 16;
		ifr.ifr_flags = if_flags & 0xffff;

		break;

	case SIOCETHTOOL:
		{
			ethtool_value eval;
			eval.cmd = subcmd;
			eval.data = 0;
			ifr.ifr_data = (caddr_t)&eval;
		}
		
		break;
	default:
		break;
	}

	int error = ioctl(fd, what, &ifr);
    close(fd);

	if (error)
	{
		return error;
	}

	switch (what)
	{
	case SIOCGIFFLAGS:
		if_flags = (ifr.ifr_flagshigh << 16) | (0xffff & ifr.ifr_flags);

		break;
	default:
		break;
	}

	return error;
}

} // namespace

NetmapInterfaceLib::NetmapInterfaceLib(LookupTable& table) : InterfaceLib(table), if_flags_(0), num_tasks_(0)
{
	acquire();
}

NetmapInterfaceLib::~NetmapInterfaceLib()
{
	release();
}

int NetmapInterfaceLib::num_tasks() const
{
	return num_tasks_;
}

void NetmapInterfaceLib::acquire()
{
	tnt::FileDescriptor fd("/dev/netmap", O_RDWR);

	if (*fd < 0)
	{
		tnt::Log::error("NetmapInterfaceLib::run: ", std::strerror(errno));

		throw NetmapException(std::strerror(errno));
	}

	auto req = nmreq();
	req.nr_version = NETMAP_API;
	auto name = tnt::Configuration::get("netmap.interface").as<std::string>();

	std::fill_n(req.nr_name, IFNAMSIZ, 0);
	strncpy(req.nr_name, name.c_str(), sizeof(req.nr_name));

	req.nr_ringid = 0;
	
	int err = ioctl(*fd, NIOCGINFO, &req);

	if (err != 0)
	{
		tnt::Log::error("NetmapInterfaceLib::run: Cannot get info on ", name, " (", std::strerror(errno), ")");

		throw NetmapException(std::strerror(errno));
	}

	auto memsize = req.nr_memsize;
	mem_ = std::shared_ptr<char>(reinterpret_cast<char*>(mmap(0, memsize, PROT_WRITE | PROT_READ, MAP_SHARED, *fd, 0)), [=](char* ptr) { if (ptr && ptr != MAP_FAILED) munmap(ptr, memsize); });

	if (mem_.get() == MAP_FAILED)
	{
		tnt::Log::error("NetmapInterfaceLib::run: Unable to mmap");
		mem_.reset();

		throw NetmapException(std::strerror(errno));
	}

	if_flags_ = 0;

	nm_do_ioctl(name, if_flags_, SIOCGIFFLAGS, 0);

	if ((if_flags_ & IFF_UP) == 0)
	{
		if_flags_ |= IFF_UP;
	}

	if_flags_ |= IFF_PPROMISC;
	nm_do_ioctl(name, if_flags_, SIOCSIFFLAGS, 0);

	nm_do_ioctl(name, if_flags_, SIOCETHTOOL, ETHTOOL_SGSO);
	nm_do_ioctl(name, if_flags_, SIOCETHTOOL, ETHTOOL_STSO);
	nm_do_ioctl(name, if_flags_, SIOCETHTOOL, ETHTOOL_SRXCSUM);
	nm_do_ioctl(name, if_flags_, SIOCETHTOOL, ETHTOOL_STXCSUM);

	num_tasks_ = req.nr_rx_rings;
}

void NetmapInterfaceLib::release()
{
	if_flags_ &= ~IFF_PPROMISC;
	nm_do_ioctl(tnt::Configuration::get("netmap.interface").as<std::string>(), if_flags_, SIOCSIFFLAGS, 0);
}

void NetmapInterfaceLib::forward_loop(std::atomic_bool& running, int ringid, std::atomic_uint_fast64_t& rx, std::atomic_uint_fast64_t& tx, std::atomic_uint_fast64_t& drop)
{
	tnt::FileDescriptor fd("/dev/netmap", O_RDWR);

	if (*fd < 0)
	{
		tnt::Log::error("NetmapInterfaceLib::forward_loop: ", std::strerror(errno));

		return;
	}

	auto req = nmreq();
	req.nr_version = NETMAP_API;
	auto name = tnt::Configuration::get("netmap.interface").as<std::string>();

	std::fill_n(req.nr_name, IFNAMSIZ, 0);
	strncpy(req.nr_name, name.c_str(), sizeof(req.nr_name));

	req.nr_ringid = NETMAP_HW_RING + ringid;

	int err = ioctl(*fd, NIOCREGIF, &req);

	if (err != 0)
	{
		tnt::Log::error("NetmapInterfaceLib::forward_loop (ring id = ", ringid, "): Unable to register ", name, " (", std::strerror(errno), ")");

		return;
	}

	auto nifp = reinterpret_cast<netmap_if*>(mem_.get() + req.nr_offset);

    std::this_thread::sleep_for(std::chrono::seconds(tnt::Configuration::get("netmap.init_wait").as<unsigned int>()));

	auto burst = tnt::Configuration::get("netmap.burst").as<unsigned int>();

	auto rxring = netmap_rxring(nifp, ringid);
	auto txring = netmap_txring(nifp, ringid);

    tnt::Log::info(colors::green, "Ring #", ringid, " up.");
	
	while (running)
	{
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

void NetmapInterfaceLib::process_rings(netmap_ring* rxring, netmap_ring* txring, std::atomic_uint_fast64_t& rx, std::atomic_uint_fast64_t& tx, std::atomic_uint_fast64_t& drop)
{
	auto j = rxring->cur;
	auto k = txring->cur;

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

		if (LIKELY(rs.len > 34 && rs.len < 2048)) // Not an IP packet if rs.len < 34.
		{
			auto pkt = reinterpret_cast<uint8_t*>(netmap_buf(rxring, rs.buf_idx));

			if (LIKELY(forward_packet(pkt)))
			{
				std::swap(ts.buf_idx, rs.buf_idx);

				ts.len = rs.len;

				ts.flags |= NS_BUF_CHANGED;
				rs.flags |= NS_BUF_CHANGED;

				++tx;
				k = netmap_ring_next_slot(txring, k);
				txring->cur = k;
				txring->avail--;
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
	}

	rxring->avail -= m;
	rxring->cur = j;
}

} // namespace drop

#endif
