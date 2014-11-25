
#include "forward.hpp"

#include <thread>
#include <atomic>
#include <vector>

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdint>

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <sched.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <net/if.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>

#include "netmap.h"

#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC

#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "file_descriptor.hpp"
#include "log.hpp"
#include "likely_macro.hpp"

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

/*char* netmap_buf(netmap_ring* r, uint32_t i)
{
	return reinterpret_cast<char*>(r) + r->buf_ofs + i * r->nr_buf_size;
}*/

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

int acquire(uint32_t& if_flags, const std::string& name, std::shared_ptr<char>& mem);
void forward_loop(std::atomic_bool& running, int ringid, std::shared_ptr<char> mem, const std::string& name);
void release(uint32_t& if_flags, const std::string& name);
void process_rings(netmap_ring* rxring, netmap_ring* txring);
void start(const std::string& name, int num_cores);

void start(const std::vector<std::string>& interfaces, int start_core, int num_cores, std::chrono::seconds delay)
{
    auto n_interfaces = interfaces.size();

    std::vector<uint32_t> flags;
    flags.resize(n_interfaces);

    std::vector<std::shared_ptr<char>> memory;
    memory.resize(n_interfaces);

    int n_tasks;

    for (decltype(n_interfaces) i = 0; i < n_interfaces; ++i)
    {
        n_tasks = acquire(flags.at(i), interfaces.at(i), memory.at(i));

        if (n_tasks == 0)
        {
            return;
        }
    }

    tnt::Log::info(colors::cyan, "Starting ", n_tasks * n_interfaces, " threads (", n_tasks, " * ", n_interfaces, ") distributed over ", num_cores, " cores.");

    std::vector<std::thread> forward_threads;
    forward_threads.reserve(n_tasks * n_interfaces);

    std::atomic_bool running{ true };

    for (decltype(n_interfaces) i = 0; i < n_interfaces; ++i)
    {
        const auto& mem = memory.at(i);
        const auto& name = interfaces.at(i);

        for (decltype(n_tasks) t = 0; t < n_tasks; ++t)
	    {
            forward_threads.emplace_back([&running, t, mem, &name] ()
            {
                forward_loop(running, t, mem, name);
            });

            if (delay.count() != 0)
            {
                std::this_thread::sleep_for(delay);
            }
	    }
    }
    
    for (decltype(n_interfaces) i = 0; i < n_interfaces; ++i)
    {
        for (decltype(n_tasks) t = 0; t < n_tasks; ++t)
	    {
            auto cpu = start_core + (t % num_cores);
            tnt::Log::info(colors::cyan, "Ring #", t, " ==> cpu #", cpu);
            set_affinity(forward_threads[i * n_tasks + t].native_handle(), cpu);
	    }
    }

    for (const auto& iface : interfaces)
    {
        std::system((std::string("/root/set_affinity.sh ") + iface + " " + std::to_string(start_core) + " " + std::to_string(num_cores)).c_str());
    }

    while (true)
    {
        std::string str;
        std::cin >> str;

        if (str == "q")
        {
            break;
        }

        try
        {
            auto n = std::stoi(str);

            for (decltype(n_interfaces) i = 0; i < n_interfaces; ++i)
            {
                for (decltype(n_tasks) t = 0; t < n_tasks; ++t)
	            {
                    auto cpu = start_core + (t % n);
                    tnt::Log::info(colors::cyan, "Ring #", t, " ==> cpu #", cpu);
                    set_affinity(forward_threads[i * n_tasks + t].native_handle(), cpu);
	            }
            }

            for (const auto& iface : interfaces)
            {
                std::system((std::string("/root/set_affinity.sh ") + iface + " " + std::to_string(start_core) + " " + std::to_string(n)).c_str());
            }
        }
        catch (...) {}
    }
    
    running = false;
    
    for (auto& t : forward_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    for (decltype(n_interfaces) i = 0; i < n_interfaces; ++i)
    {
        release(flags.at(i), interfaces.at(i));
    }
}

int acquire(uint32_t& if_flags, const std::string& name, std::shared_ptr<char>& mem)
{
	tnt::FileDescriptor fd("/dev/netmap", O_RDWR);

	if (*fd < 0)
	{
		tnt::Log::error("NetmapInterfaceLib::run: ", std::strerror(errno));

        return 0;
	}

	auto req = nmreq();
	req.nr_version = NETMAP_API;

	std::fill_n(req.nr_name, IFNAMSIZ, 0);
	strncpy(req.nr_name, name.c_str(), sizeof(req.nr_name));

	req.nr_ringid = 0;
	
	int err = ioctl(*fd, NIOCGINFO, &req);

	if (err != 0)
	{
		tnt::Log::error("NetmapInterfaceLib::run: Cannot get info on ", name, " (", std::strerror(errno), ")");

		return 0;
	}

	auto memsize = req.nr_memsize;
	mem = std::shared_ptr<char>(reinterpret_cast<char*>(mmap(0, memsize, PROT_WRITE | PROT_READ, MAP_SHARED, *fd, 0)), [=](char* ptr) { if (ptr && ptr != MAP_FAILED) munmap(ptr, memsize); });

	if (mem.get() == MAP_FAILED)
	{
		tnt::Log::error("NetmapInterfaceLib::run: Unable to mmap");
		mem.reset();

		return 0;
	}

	if_flags = 0;

	nm_do_ioctl(name, if_flags, SIOCGIFFLAGS, 0);

	if ((if_flags & IFF_UP) == 0)
	{
		if_flags |= IFF_UP;
	}

	if_flags |= IFF_PPROMISC;
	nm_do_ioctl(name, if_flags, SIOCSIFFLAGS, 0);

	nm_do_ioctl(name, if_flags, SIOCETHTOOL, ETHTOOL_SGSO);
	nm_do_ioctl(name, if_flags, SIOCETHTOOL, ETHTOOL_STSO);
	nm_do_ioctl(name, if_flags, SIOCETHTOOL, ETHTOOL_SRXCSUM);
	nm_do_ioctl(name, if_flags, SIOCETHTOOL, ETHTOOL_STXCSUM);

	return req.nr_rx_rings;
}

void release(uint32_t& if_flags, const std::string& name)
{
	if_flags &= ~IFF_PPROMISC;
	nm_do_ioctl(name, if_flags, SIOCSIFFLAGS, 0);
}

void forward_loop(std::atomic_bool& running, int ringid, std::shared_ptr<char> mem, const std::string& name)
{
	tnt::FileDescriptor fd("/dev/netmap", O_RDWR);

	if (*fd < 0)
	{
		tnt::Log::error("NetmapInterfaceLib::forward_loop: ", std::strerror(errno));

		return;
	}

	auto req = nmreq();
	req.nr_version = NETMAP_API;

	std::fill_n(req.nr_name, IFNAMSIZ, 0);
	strncpy(req.nr_name, name.c_str(), sizeof(req.nr_name));

	req.nr_ringid = NETMAP_HW_RING + ringid;

	int err = ioctl(*fd, NIOCREGIF, &req);

	if (err != 0)
	{
		tnt::Log::error("NetmapInterfaceLib::forward_loop (ring id = ", ringid, "): Unable to register ", name, " (", std::strerror(errno), ")");

		return;
	}

	auto nifp = reinterpret_cast<netmap_if*>(mem.get() + req.nr_offset);

    std::this_thread::sleep_for(std::chrono::seconds(5));

	uint32_t burst = 32;

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

		process_rings(rxring, txring);
		ioctl(*fd, NIOCTXSYNC, nullptr);
	}

	ioctl(*fd, NIOCUNREGIF, nullptr);

	tnt::Log::info(colors::blue, "Interface up: exiting forwarding loop on ring #", ringid);
}

void process_rings(netmap_ring* rxring, netmap_ring* txring)
{
    auto j = rxring->cur;
	auto k = txring->cur;

	auto limit = rxring->avail;

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
			std::swap(ts.buf_idx, rs.buf_idx);

			ts.len = rs.len;

			ts.flags |= NS_BUF_CHANGED;
			rs.flags |= NS_BUF_CHANGED;

			k = netmap_ring_next_slot(txring, k);
			txring->cur = k;
			txring->avail--;
		}

		j = netmap_ring_next_slot(rxring, j);
	}

	rxring->avail -= m;
	rxring->cur = j;
}
