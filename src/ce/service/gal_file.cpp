
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

#include "gal_file.hpp"

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

#include "application.hpp"

namespace drop {
namespace service {
namespace {

} // namespace

GalFile::GalFile(ce::ServiceElement* parent) : parent_(parent)
{
    load_configuration();
}

std::ostream& GalFile::print(std::ostream& os) const
{
    os << R"({ "Name": "Gal" })";

    return os;
}

gal::ReturnCode GalFile::discovery(const std::string& resource_id,
                                   bool /*committed*/,
                                   gal::LogicalResource* resource,
                                   gal::LogicalResourceContainer* logical_resources,
                                   gal::PhysicalResourceContainer* /*physical_resources*/,
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
    
    if (sensor_resources)
    {
        *sensor_resources = sensor_resources_;
    }

    if (power_states)
    {
        *power_states = power_states_;
    }
    
    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode GalFile::provisioning(const std::string& /*resource_id*/, int /*power_state_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalFile::release(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalFile::monitor_state(const std::string& resource_id, bool /*committed*/, gal::PowerState::PowerStateId& power_state_id)
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

gal::ReturnCode GalFile::monitor_history(const std::string& /*resource_id*/, gal::PowerStateHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalFile::monitor_sensor(const std::string& /*resource_id*/, gal::EntitySensorStatus& /*oper_status*/, gal::EntitySensorValue& /*sensor_value*/, gal::EntitySensorTimeStamp& /*value_timestamp*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalFile::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalFile::commit(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalFile::rollback(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

void GalFile::load_configuration()
{
    auto path = util::full_path(util::combine_path("config", parent_->display_name(), "gal.xml"));

    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("GalFile: ") + result.description());
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
        for (const auto& s : load_sensors(sr))
        {
            sensor_resources_.push_back(s.second);
        }
    }

    power_states_ = load_power_states(root.child("EnergyAwareStates"));
}

} // namespace service
} // namespace drop
