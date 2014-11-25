
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

#include "fe_gal.hpp"

#include <iostream>

#include "exception/exception.hpp"

#include "event/http/http_response.hpp"

#include "message/http/http_request.hpp"

#include "network/service_element.hpp"

#include "protocol/http/http_protocol.hpp"

#include "io/io.hpp"

#include "gal_drop/load_power_states.hpp"
#include "gal_drop/load_sensors.hpp"

#include "util/path.hpp"
#include "util/pugixml.hpp"
#include "util/exec.hpp"
#include "util/string.hpp"
#include "util/random.hpp"

#include "application.hpp"

namespace drop {
namespace service {
namespace {

} // namespace

MockFEGal::MockFEGal(ce::ServiceElement* parent) : parent_(parent)
{
    load_configuration();
}

std::ostream& MockFEGal::print(std::ostream& os) const
{
    os << R"({ "Name": "MockFEGal" })";

    return os;
}

gal::ReturnCode MockFEGal::discovery(const std::string& resource_id,
                                     bool /*committed*/,
                                     gal::LogicalResource* resource,
                                     gal::LogicalResourceContainer* logical_resources,
                                     gal::PhysicalResourceContainer* physical_resources,
                                     gal::SensorResourceContainer* sensor_resources,
                                     gal::PowerStateContainer* power_states,
                                     gal::OptimalConfigContainer* /*edl*/)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (resource)
    {
        *resource = resource_;
    }
    
    if (logical_resources)
    {
        *logical_resources = logical_resources_;
    }
    
    if (physical_resources)
    {
        for (auto i = 0; i < 8; ++i)
        {
            auto r = gal::PhysicalResource();
            r.id = resource_id + "." + parent_->display_name() + ".core" + std::to_string(i);
            r.description = "CPU Core";
            r.type = "CPU Core";

            physical_resources->push_back(r);
        }
    }

    if (sensor_resources)
    {
        for (const auto& s : sensor_resources_)
        {
            sensor_resources->push_back(s.second);
        }
    }

    if (power_states)
    {
        *power_states = power_states_;
    }
    
    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode MockFEGal::provisioning(const std::string& /*resource_id*/, int /*power_state_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockFEGal::release(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockFEGal::monitor_state(const std::string& resource_id, bool /*committed*/, gal::PowerState::PowerStateId& power_state_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (power_states_.empty())
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    power_state_id = power_states_[0].id;
    
    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode MockFEGal::monitor_history(const std::string& /*resource_id*/, gal::PowerStateHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockFEGal::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto it = sensor_resources_.find(resource_id);

    if (it == std::end(sensor_resources_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    oper_status = gal::EntitySensorStatus::ok;
    sensor_value = tnt::random_value(0, it->second.max_value);
    value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode MockFEGal::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockFEGal::commit(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockFEGal::rollback(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

void MockFEGal::load_configuration()
{
    auto path = util::full_path(util::combine_path("config", parent_->display_name(), "gal.xml"));

    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("MockFEGal: ") + result.description());
    }

    const auto& root = doc.child("Gal");

    assert(root);

    resource_.id = parent_->display_name();
    resource_.type = root.child("Type").child_value();
    resource_.description = root.child("Description").child_value();

    auto lr = root.child("LogicalResources");

    if (lr)
    {
        for (auto r : lr)
        {
            gal::LogicalResource res;

            res.id = r.attribute("id").as_string();
            res.type = r.child("Type").child_value();
            res.description = r.child("Description").child_value();

            logical_resources_.push_back(res);
        }
    }

    auto pr = root.child("PhysicalResources");

    if (pr)
    {
        for (auto r : pr)
        {
            gal::PhysicalResource res;

            res.id = r.attribute("id").as_string();
            res.type = r.child("Type").child_value();
            res.description = r.child("Description").child_value();

            physical_resources_.push_back(res);
        }
    }

    auto sr = root.child("SensorResources");

    if (sr)
    {
        sensor_resources_ = load_sensors(sr);
    }

    power_states_ = load_power_states(root.child("EnergyAwareStates"));
}

} // namespace service
} // namespace drop
