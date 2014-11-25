
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

#include "se_lcp.hpp"

#include <unordered_map>
#include <atomic>
#include <cassert>

#include "event/quit.hpp"
#include "event/gal/slot_change.hpp"

#include "network/service_element.hpp"

#include "router/address.hpp"

#include "exception/exception.hpp"

#include "service/gal.hpp"

#include "util/wol.hpp"
#include "util/string.hpp"
#include "util/configuration.hpp"
#include "util/gal_resource_names.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "containers.hpp"
#include "log.hpp"
#include "range.hpp"

namespace drop {
namespace activity {

ServiceElementLCP::ServiceElementLCP(const std::shared_ptr<ce::ServiceElement>& se): se_(se) {}

gal::ReturnCode ServiceElementLCP::discovery(const std::string& resource_id, 
                                             bool committed,
										     gal::LogicalResource* resource,
                                             gal::LogicalResourceContainer* logical_resources,
                                             gal::PhysicalResourceContainer* physical_resources,
                                             gal::SensorResourceContainer* sensor_resources,
                                             gal::PowerStateContainer* power_states,
                                             gal::OptimalConfigContainer* edl)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_discovery(committed, resource, logical_resources, physical_resources, sensor_resources, power_states, edl);
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::provisioning(const std::string& resource_id, PowerStateId power_state_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_provisioning(power_state_id);
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::release(const std::string& resource_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_release();
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::monitor_state(const std::string& resource_id, bool committed, PowerStateId& power_state_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_monitor_state(committed, power_state_id);
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* history)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_monitor_history(history);
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0" || ids.size() < 2)
    {
        return gal::ReturnCode::GalResourceNotFound;
    }
    
    return device_monitor_sensor(ids[1], oper_status, sensor_value, value_timestamp);
}

gal::ReturnCode ServiceElementLCP::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::commit(const std::string& resource_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_commit();
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::rollback(const std::string& resource_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        return device_rollback();
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::device_discovery(bool committed,
                                                    gal::LogicalResource* resource,
                                                    gal::LogicalResourceContainer* logical_resources,
                                                    gal::PhysicalResourceContainer* physical_resources,
                                                    gal::SensorResourceContainer* sensor_resources,
                                                    gal::PowerStateContainer* power_states,
                                                    gal::OptimalConfigContainer* edl)
{
    return forward_to_service([&] (const auto& s)
    {
        return s->discovery("0", committed, resource, logical_resources, physical_resources, sensor_resources, power_states, edl);
    });
}

gal::ReturnCode ServiceElementLCP::device_monitor_state(bool committed, PowerStateId& power_state_id)
{
    return forward_to_service([&] (const auto& s)
    {
        return s->monitor_state("0", committed, power_state_id);
    });
}

gal::ReturnCode ServiceElementLCP::device_monitor_history(gal::PowerStateHistoryContainer* history)
{
    return forward_to_service([&] (const auto& s)
    {
        return s->monitor_history("0", history);
    });
}

gal::ReturnCode ServiceElementLCP::device_provisioning(PowerStateId power_state_id)
{
    return forward_to_service([&] (const auto& s)
    {
        return s->provisioning("0", power_state_id);
    });
}

gal::ReturnCode ServiceElementLCP::device_commit()
{
    return forward_to_service([&] (const auto& s)
    {
        return s->commit("0");
    });
}

gal::ReturnCode ServiceElementLCP::device_rollback()
{
    return forward_to_service([&] (const auto& s)
    {
        return s->rollback("0");
    });
}

gal::ReturnCode ServiceElementLCP::device_release()
{
    return forward_to_service([&] (const auto& s)
    {
        return s->release("0");
    });
}

gal::ReturnCode ServiceElementLCP::device_monitor_sensor(const std::string& name, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    return forward_to_service([&] (const auto& s)
    {
        return s->monitor_sensor(name, oper_status, sensor_value, value_timestamp);
    });
}

gal::ReturnCode ServiceElementLCP::forward_to_service(std::function<gal::ReturnCode(const std::shared_ptr<service::Gal>&)> func)
{
    auto result = gal::ReturnCode::GalNotImplemented;

    try
    {
        for (const auto& s : se_->services())
        {
            if (tnt::visit<service::Gal>(s, func, result))
            {
                break;
            }
        }
    }
    catch (...)
    {
        result = gal::ReturnCode::GalProtocolError;
    }

    return result;
}

} // namespace activity
} // namespace drop
