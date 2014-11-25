
/*

Copyright (c) 2012, Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#ifndef GAL_POWER_STATE_HPP_
#define GAL_POWER_STATE_HPP_

#include <vector>
#include <cstdint>
#include <iosfwd>

namespace gal {

struct CurveState
{
    uint16_t offered_load;
    uint16_t maximum_consumption;

    double maximum_packet_service_time;
};

struct PowerState
{
    using PowerStateId = int;

    PowerStateId id;

    uint16_t minimum_power_gain;
    uint16_t power_gain;
    uint16_t lpi_transition_power;
    uint16_t autonomic_ps_steps;
    
    uint64_t wakeup_triggers;

    double autonomic_ps_service_interruption;
    double maximum_packet_throughput;
    double maximum_bit_throughput;
    double wakeup_time;
    double sleeping_time;

    std::vector<double> ps_transition_times;
    std::vector<double> ps_transition_service_interruption_times;

    std::vector<CurveState> autonomic_ps_curves;
};

bool is_autonomic(const PowerState& ps);
std::ostream& operator<<(std::ostream& os, const PowerState& ps);

struct PowerStateHistory
{
    long long timestamp;
    int power_state;
};

} // namespace gal

#endif
