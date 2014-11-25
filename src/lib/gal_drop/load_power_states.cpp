
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

#include "load_power_states.hpp"

#include <algorithm>

#include "exception/drop_exception.hpp"

#include "util/pugixml.hpp"

namespace drop {
namespace {

gal::PowerState load_power_state_common(const pugi::xml_node& power_state)
{
    gal::PowerState ps;
            
    ps.id = power_state.attribute("id").as_int();
    ps.minimum_power_gain = power_state.child("MinimumPowerGain").text().as_int();
    ps.maximum_packet_throughput = power_state.child("MaximumPacketThroughput").text().as_double();
    ps.maximum_bit_throughput = power_state.child("MaximumBitThroughput").text().as_double();
            
    auto node = power_state.child("PowerGain");

    if (node)
    {
        ps.power_gain = node.text().as_int();
    }

    node = power_state.child("LpiTransitionPower");

    if (node)
    {
        ps.lpi_transition_power = node.text().as_int();
    }

    node = power_state.child("WakeupTime");

    if (node)
    {
        ps.wakeup_time = node.text().as_double();
    }

    node = power_state.child("SleepingTime");

    if (node)
    {
        ps.sleeping_time = node.text().as_double();
    }

    node = power_state.child("WakeupTriggers");

    if (node)
    {
        ps.wakeup_triggers = node.text().as_int();
    }

    for (auto ptt : power_state.child("TransitionTimes").children())
    {
        ps.ps_transition_times.push_back(ptt.text().as_double());
    }

    for (auto ptsit : power_state.child("TransitionServiceInterruptionTimes").children())
    {
        ps.ps_transition_times.push_back(ptsit.text().as_double());
    }

    return ps;
}

gal::PowerState load_power_state(const pugi::xml_node& power_state)
{
    gal::PowerState ps = load_power_state_common(power_state);
    
    auto node = power_state.child("AutonomicServiceInterruption");

    if (node)
    {
        ps.autonomic_ps_service_interruption = node.text().as_double();
    }

    node = power_state.child("AutonomicCurve");

    if (node)
    {
        for (auto curve_state : node.children("Step"))
        {
            gal::CurveState cs;

            cs.offered_load = curve_state.child("OfferedLoad").text().as_double();
            cs.maximum_consumption = curve_state.child("MaximumConsumption").text().as_int();
            cs.maximum_packet_service_time = curve_state.child("MaximumPacketServiceTime").text().as_double();
            
            ps.autonomic_ps_curves.push_back(cs);
        }

        auto curve_steps = ps.autonomic_ps_curves.size();

        if (curve_steps < 2)
        {
            throw tnt::ConfigurationError("LCP Error: Autonomic Power State with less then two steps.");
        }

        ps.autonomic_ps_steps = curve_steps;
    }

    ps.autonomic_ps_steps = ps.autonomic_ps_curves.size();

    // Ensure the curve states are ordered ascending by offered_load.
    std::sort(std::begin(ps.autonomic_ps_curves), std::end(ps.autonomic_ps_curves), [] (const gal::CurveState& cs0, const gal::CurveState& cs1)
    {
        return cs0.offered_load < cs1.offered_load;
    });

    return ps;
}

} // namespace

std::vector<gal::PowerState> load_power_states(const pugi::xml_node& root)
{
    std::vector<gal::PowerState> power_states;

    for (const auto& node : root.children("EnergyAwareState"))
    {
        power_states.push_back(load_power_state(node));
    }

    return power_states;
}

} // namespace drop
