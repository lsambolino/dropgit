
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

#ifndef DROP_LOOKUP_TABLE_HPP_
#define DROP_LOOKUP_TABLE_HPP_

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include <cstdint>
#include <cassert>

#include "mac_address_fwd.hpp"

namespace drop {

class LookupTable
{
    struct LookupNode
    {
		LookupNode();
		LookupNode(const std::string& d);

		bool def;

        char dst[6];
    };

	struct RouteNode
	{
		RouteNode(uint32_t p, const std::string& d);

		uint32_t prefix;

		char dst[6];
	};

    using TableType = std::unordered_map<uint32_t, LookupNode>;
	using RouteTableType = std::multimap<uint32_t, RouteNode>;
public:
	using RouteValue = RouteTableType::value_type;
	using LookupValue = TableType::value_type;

	explicit LookupTable(int n);

	const LookupNode& lookup(uint32_t address);

	void add(uint32_t address, uint32_t prefix, const tnt::MacAddress& dst);
	void del(uint32_t address, uint32_t prefix);

	void for_each(std::function<void(const RouteValue&)> func);
private:
	bool add_route(uint32_t address, uint32_t prefix, const std::string& dst);
	void add_net(uint32_t address, uint32_t prefix, const LookupNode& node);

	void del_address(uint32_t address, uint32_t prefix);
	void del_net(uint32_t address, uint32_t prefix);

	void update(std::function<void()> func);
	void swap();

	uint32_t route_prefix(uint32_t address);
	const RouteNode& match(uint32_t address, uint32_t prefix);
private:
	RouteTableType route_table_;

    std::unique_ptr<TableType> table_1_;
    std::unique_ptr<TableType> table_2_;

    std::atomic<TableType*> read_;

	TableType* write_;

    LookupNode default_;
};

} // namespace drop

#endif
