
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

#ifndef DROP_GAL_ACPI_TUNING_HPP_
#define DROP_GAL_ACPI_TUNING_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#include "gal_drop/acpi_state.hpp"

namespace drop {

using CoreId = int;

namespace acpi {

std::vector<int> get_cores();
std::vector<int> get_related_cores(int core);
std::vector<int> get_available_frequencies(int core);
std::vector<int> get_available_latencies(int core);

int current_frequency(int core);
void set_frequency(int core, int frequency);

int state_latency(int core, int state);

int c1_latency(int core);
int c3_latency(int core);

int current_latency(int core);
void set_latency(int core, int latency);

std::string current_governor(int core);
void set_governor(int core, const std::string& governor);

CoreId get_affinity(const std::string& iface, int queue);
void set_affinity(const std::string& iface, CoreId core);

uint64_t get_packets(const std::string& iface);

void set_dfs(int core, int dfs);
void set_voltage(int voltage);

void set_state(State state);

} // namespace acpi
} // namespace drop

#endif
