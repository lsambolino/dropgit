
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

#include "lookup_table.hpp"

#include <algorithm>
#include <thread>
#include <cmath>

#include "range.hpp"
#include "endianness.hpp"
#include "mac_address.hpp"

namespace drop {

LookupTable::LookupNode::LookupNode(): def(true) {}

LookupTable::LookupNode::LookupNode(const std::string& d) : def(false)
{
	assert(d.size() == 6);

	std::copy_n(std::begin(d), 6, std::begin(dst));
}

LookupTable::RouteNode::RouteNode(uint32_t p, const std::string& d) : prefix(p)
{
	assert(d.size() == 6);

	std::copy_n(std::begin(d), 6, std::begin(dst));
}

LookupTable::LookupTable(int n): table_1_(std::make_unique<TableType>(n)), table_2_(std::make_unique<TableType>(n)), read_(table_1_.get()), write_(table_2_.get()) {}

const LookupTable::LookupNode& LookupTable::lookup(uint32_t address)
{
    const auto ptr = read_.load(); // Atomic operation
    auto it = ptr->find(address);

    return it != std::end(*ptr) ? it->second : default_;
}

void LookupTable::add(uint32_t address, uint32_t prefix, const tnt::MacAddress& dst)
{
	assert(prefix > 1 && prefix <= 32);

	const auto d = std::move(dst.raw());
	const auto node = LookupNode(d);

    if (add_route(address, prefix, d))
    {
	    if (prefix == 32)
	    {
		    update([&] ()
		    {
			    (*write_)[address] = node;
		    });
	    }
	    else
	    {
		    add_net(address, prefix, node);
	    }
    }
}

void LookupTable::del(uint32_t address, uint32_t prefix)
{
	auto p = route_table_.equal_range(address);

	if (p.first == p.second)
	{
		// No route for the address/subnet. Nothing to do.
		return;
	}

	for (auto it = p.first; it != p.second; ++it)
	{
		if (it->second.prefix == prefix)
		{
			route_table_.erase(it);

			break;
		}
	}

	if (prefix == 32)
	{
		del_address(address, prefix);
	}
	else
	{
		del_net(address, prefix);
	}
}

void LookupTable::for_each(std::function<void(const RouteValue&)> func)
{
	for (const auto& r : route_table_)
	{
		func(r);
	}
}

bool LookupTable::add_route(uint32_t address, uint32_t prefix, const std::string& dst)
{
	auto p = route_table_.equal_range(address);

	if (p.first != p.second)
	{
		for (auto it = p.first; it != p.second; ++it)
		{
			if (it->second.prefix == prefix)
			{
                const auto& rn = it->second;

                if (rn.dst[0] == dst[0] && rn.dst[1] == dst[1] && rn.dst[2] == dst[2] && rn.dst[3] == dst[3] && rn.dst[4] == dst[4] && rn.dst[5] == dst[5])
                {
                    return false;
                }

				it->second = RouteNode(prefix, dst);
				
				return true;
			}
		}
	}

	route_table_.insert(std::make_pair(address, RouteNode(prefix, dst)));

    return true;
}

void LookupTable::add_net(uint32_t address, uint32_t prefix, const LookupNode& node)
{
	address = ntohl(address);

	auto net_len = 32 - prefix;

	auto num = 1 << net_len;
	auto mask = std::numeric_limits<uint32_t>::max() << net_len;

	std::vector<uint32_t> addresses;
	addresses.reserve(num);

	for (int i = 0; i < num; ++i)
	{
		auto dst_address = htonl((address & mask) + i);

		if (prefix > route_prefix(dst_address))
		{
			addresses.push_back(dst_address);
		}
	}

	update([&] ()
	{
		for (auto a : addresses)
		{
			(*write_)[a] = node;
		}
	});
}

void LookupTable::del_address(uint32_t address, uint32_t prefix)
{
	auto old_prefix = route_prefix(address);

	update([&] ()
	{
		if (old_prefix == 0)
		{
			write_->erase(address);
		}
		else if (old_prefix < prefix)
		{
			const auto& node = match(address, old_prefix);

			(*write_)[address] = LookupNode(std::string(node.dst, 6));
		}
	});
}

void LookupTable::del_net(uint32_t address, uint32_t prefix)
{
	auto net_len = 32 - prefix;

	auto n = static_cast<int>(std::pow(2, net_len));
	auto mask = std::numeric_limits<uint32_t>::max() << net_len;

	std::vector<std::pair<uint32_t, LookupNode>> addresses;
	addresses.reserve(n);

	for (int i = 0; i < n; ++i)
	{
		auto dst_address = htonl((ntohl(address) & mask) + i);
		auto old_prefix = route_prefix(dst_address);

		if (old_prefix == 0)
		{
			addresses.push_back(std::make_pair(dst_address, LookupNode()));
		}
		else if (old_prefix < prefix)
		{
			const auto& node = match(dst_address, old_prefix);
			addresses.push_back(std::make_pair(dst_address, LookupNode(std::string(node.dst, 6))));
		}
	}

	update([&] ()
	{
		for (auto p : addresses)
		{
			if (p.second.def)
			{
				write_->erase(p.first);
			}
			else
			{
				(*write_)[p.first] = p.second;
			}
		}
	});
}

void LookupTable::update(std::function<void()> func)
{
	func();
	swap();

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	func();
}

void LookupTable::swap()
{
    TableType* tmp = read_;
    read_ = write_;  // Atomic operation
    write_ = tmp;
}

uint32_t LookupTable::route_prefix(uint32_t address)
{
	auto prefix = 0u;

	for (const auto& p : route_table_)
	{
		auto net_len = 32 - p.second.prefix;
		auto mask = std::numeric_limits<uint32_t>::max() << net_len;

		if ((ntohl(p.first) & mask) == (ntohl(address) & mask))
		{
			prefix = std::max(prefix, p.second.prefix);
		}
	}

	return prefix;
}

const LookupTable::RouteNode& LookupTable::match(uint32_t address, uint32_t prefix)
{
	for (const auto& p : route_table_)
	{
		if (p.second.prefix == prefix)
		{
			auto net_len = 32 - prefix;
			auto mask = std::numeric_limits<uint32_t>::max() << net_len;

			if ((ntohl(p.first) & mask) == (ntohl(address) & mask))
			{
				return p.second;
			}
		}
	}

	assert(false);
}

} // namespace drop
