
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

#ifndef DROP_ACTIVITY_FORWARD_HPP_
#define DROP_ACTIVITY_FORWARD_HPP_

#include <cstdint>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <atomic>

#include "activity/concurrent_activity.hpp"

#include "util/lookup_table.hpp"

#include "mac_address.hpp"

namespace drop {
namespace activity {

class Forward: private tnt::ConcurrentActivity
{
	using InterfaceTable = std::unordered_map<uint32_t, tnt::MacAddress>;
public:
	Forward();
    ~Forward();
	void operator()();
private:
	void add(uint32_t address, uint32_t prefix, const tnt::MacAddress& dst);
	void del(uint32_t address, uint32_t prefix);
private:
	LookupTable table_;
	InterfaceTable interfaces_;
};

} // namespace activity
} // namespace drop

#endif
