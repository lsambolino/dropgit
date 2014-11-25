
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

#ifndef DROP_UTIL_SERIALIZATION_STRUCTS_HPP_
#define DROP_UTIL_SERIALIZATION_STRUCTS_HPP_

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/level.hpp>

#include "gal_drop/power_info.hpp"

#include "gal/logical_resource.hpp"
#include "gal/physical_resource.hpp"
#include "gal/sensor_resource.hpp"
#include "gal/optimal_config.hpp"
#include "gal/power_state.hpp"

#include "router/address_info.hpp"
#include "router/port_info.hpp"
#include "router/route_info.hpp"

#include "mac_address.hpp"

namespace boost {
namespace serialization {

template <class Archive> void serialize(Archive & ar, drop::PowerInfo& pi, const unsigned int /*version*/)
{
    ar & pi.index & pi.frequencies & pi.latencies & pi.curr_frequency & pi.curr_latency & pi.curr_affinity & pi.curr_governor;
}

template <class Archive> void serialize(Archive & ar, gal::LogicalResource& lr, const unsigned int /*version*/)
{
    ar & lr.id & lr.description /*& lr.depends_on*/;
}

template <class Archive> void serialize(Archive & ar, gal::PhysicalResource& pr, const unsigned int /*version*/)
{
    ar & pr.id & pr.description & pr.depends_on & pr.used_by & pr.resource_class & pr.num_children;
}

template <class Archive> void serialize(Archive & ar, gal::SensorResource& sr, const unsigned int /*version*/)
{
    ar & sr.id & sr.description & sr.type & sr.scale & sr.precision & sr.refresh_rate;
}

template <class Archive> void serialize(Archive & ar, gal::OptimalConfig& edl, const unsigned int /*version*/)
{
    ar & edl.id & edl.power_states;
}

template <class Archive> void serialize(Archive & ar, gal::CurveState& cs, const unsigned int /*version*/)
{
    ar & cs.offered_load & cs.maximum_consumption & cs.maximum_packet_service_time;
}

template <class Archive> void serialize(Archive & ar, gal::PowerState& ps, const unsigned int /*version*/)
{
    ar & ps.id & ps.minimum_power_gain & ps.power_gain & ps.lpi_transition_power & ps.autonomic_ps_steps & ps.wakeup_triggers & ps.autonomic_ps_service_interruption & ps.maximum_packet_throughput & ps.maximum_bit_throughput &
        ps.wakeup_time & ps.sleeping_time & ps.ps_transition_times & ps.ps_transition_service_interruption_times & ps.autonomic_ps_curves;
}

template <class Archive> void serialize(Archive & ar, gal::PowerStateHistory h, const unsigned int /*version*/)
{
    ar & h.power_state;
}

template <class Archive> void serialize(Archive & ar, drop::AddressInfo& ai, const unsigned int /*version*/)
{
    ar & ai.address & ai.prefix & ai.port_index;
}

template <class Archive> void serialize(Archive& ar, drop::PortInfo& pi, const unsigned int /*version*/)
{
    ar & pi.index & pi.mtu & pi.flags & pi.wol_enabled & pi.name & pi.hw_address;
}

template <class Archive> void serialize(Archive& ar, drop::RouteInfo& ri, const unsigned int /*version*/)
{
    ar & ri.prefix & ri.destination & ri.gateway & ri.port_index & ri.metric & ri.source & ri.origin;
}

template <class Archive> void serialize(Archive& ar, tnt::MacAddress& mac, const unsigned int /*version*/)
{
    std::string hw = mac.raw();

    ar & hw;

    mac = hw;
}

} // namespace serialization
} // namespace boost

BOOST_CLASS_IMPLEMENTATION(drop::PowerInfo, boost::serialization::object_serializable)

BOOST_CLASS_IMPLEMENTATION(gal::LogicalResource, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(gal::PhysicalResource, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(gal::SensorResource, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(gal::OptimalConfig, boost::serialization::object_serializable)

BOOST_CLASS_IMPLEMENTATION(gal::PowerState, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(gal::CurveState, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(gal::PowerStateHistory, boost::serialization::object_serializable)

BOOST_CLASS_IMPLEMENTATION(drop::AddressInfo, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(drop::RouteInfo, boost::serialization::object_serializable)
BOOST_CLASS_IMPLEMENTATION(drop::PortInfo, boost::serialization::object_serializable)

BOOST_CLASS_IMPLEMENTATION(tnt::MacAddress, boost::serialization::object_serializable)

#endif
