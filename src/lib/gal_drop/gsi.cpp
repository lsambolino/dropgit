
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

#include "gsi.hpp"

#include "event/gsi_request.hpp"

#include "application.hpp"

namespace drop {

gal::ReturnCode ElementGSI::discovery(const std::string& resource_id,
                                      bool committed,
                                      gal::LogicalResource* resource,
                                      gal::LogicalResourceContainer* logical_resources,
                                      gal::PhysicalResourceContainer* physical_resources,
                                      gal::SensorResourceContainer* sensor_resources,
                                      gal::PowerStateContainer* power_states,
                                      gal::OptimalConfigContainer* edl)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->discovery(resource_id, committed, resource, logical_resources, physical_resources, sensor_resources, power_states, edl);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::provisioning(const std::string& resource_id, int power_state_id)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->provisioning(resource_id, power_state_id);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::release(const std::string& resource_id)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->release(resource_id);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::monitor_state(const std::string& resource_id, bool committed, gal::PowerState::PowerStateId& power_state_id)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->monitor_state(resource_id, committed, power_state_id);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* history)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->monitor_history(resource_id, history);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->monitor_sensor(resource_id, oper_status, sensor_value, value_timestamp);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::monitor_sensor_history(const std::string& resource_id, gal::SensorHistoryContainer* history)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->monitor_sensor_history(resource_id, history);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::commit(const std::string& resource_id)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->commit(resource_id);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

gal::ReturnCode ElementGSI::rollback(const std::string& resource_id)
{
    auto e = std::make_shared<event::GSIRequest>([&] (gal::GreenStandardInterface* gsi)
    {
        return gsi->rollback(resource_id);
    });
    
    tnt::Application::raise(e);

    return e->result();
}

} // namespace drop
