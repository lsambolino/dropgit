
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

#ifndef DROP_HYPERAPP_INTERFACE_LIB_HPP_
#define DROP_HYPERAPP_INTERFACE_LIB_HPP_

#include <cstdint>
#include <memory>

#include <stdexcept>
#include <system_error>

#include "interface_lib.hpp"

struct netmap_if;
struct netmap_ring;

namespace drop {

class LookupTable;

struct HyperAppException: public std::runtime_error
{
    explicit HyperAppException(const std::string& msg): std::runtime_error(msg) { }
};

class HyperAppInterfaceLib: public InterfaceLib
{
public:
	HyperAppInterfaceLib(LookupTable& table, std::atomic_bool& running);
	virtual ~HyperAppInterfaceLib() override;

	virtual int num_tasks() const override;
	virtual void forward_loop(, std::atomic_bool& running, int ringid, std::atomic_uint_fast64_t& rx, std::atomic_uint_fast64_t& tx, std::atomic_uint_fast64_t& drop, std::atomic_bool& sleep) override;
private:
	void acquire();
	void release();

	void process_rings(netmap_ring* rxring, netmap_ring* txring, std::atomic_uint_fast64_t& rx, std::atomic_uint_fast64_t& tx, std::atomic_uint_fast64_t& drop);
private:
	std::atomic_bool& running_;
	std::shared_ptr<char> mem_;
	int num_tasks_;
};

} // namespace drop

#endif

#endif	// XLP_FE

#endif
