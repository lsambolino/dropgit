
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

#include "service_element_lcp.hpp"

#include <atomic>

#include <boost/asio.hpp>

#include "activity/network/forward.hpp"

#include "exception/drop_exception.hpp"

#include "event/quit.hpp"
#include "event/network/forwarding_started.hpp"
#include "event/gsi_request.hpp"
#include "event/gal/statistics_update.hpp"

#include "message/network/management.hpp"

#include "util/pugixml.hpp"
#include "util/path.hpp"
#include "util/string.hpp"
#include "util/exec.hpp"
#include "util/factory.hpp"

#include "gal_drop/load_power_states.hpp"
#include "gal_drop/load_sensors.hpp"
#include "gal_drop/local_control_policy_task.hpp"

#include "util/configuration.hpp"

#include "application.hpp"
#include "log.hpp"
#include "async.hpp"

namespace drop {
namespace activity {

ServiceElementLCP::ServiceElementLCP(): lcp_task_(factory::create_lcp_process())
{
    read_power_states();
}

ServiceElementLCP::~ServiceElementLCP()
{
    tnt::Log::debug(colors::green, "ServiceElementLCP activity terminated.");
}

void ServiceElementLCP::operator()()
{
    try
    {
        std::atomic_bool running{ true };

        register_handler([&] (const tnt::event::Quit& /*event*/)
        {
            running = false;
        });

        auto userspace_forward = tnt::Configuration::get("use_userspace_forward", false);

        if (userspace_forward)
        {
            register_handler([&] (const event::ForwardingStarted& /*event*/)
            {
                lcp_task_->initialize();
            });
        }

        auto activity = tnt::async([] ()
        {
            if (tnt::Configuration::get("use_userspace_forward", false))
            {
                drop::activity::Forward()();
            }
        });

        register_handler([&] (std::shared_ptr<event::GSIRequest> event)
        {
            event->exec(this);
        });

        register_handler([&] (const event::StatisticsUpdate& event)
        {
            rx_rate_ = event.rx_rate();
        });

        if (!userspace_forward)
        {
            lcp_task_->initialize();
        }

        while (running)
        {
            wait_event();
        }

        lcp_task_->finalize();

        activity.get();
    }
    catch (std::exception& ex)
    {
        tnt::Log::error("ServiceElementLCP::run error: ", ex.what());
    }
    catch (...)
    {
        tnt::Log::error("ServiceElementLCP::run: unknown exception raised.");
    }
}

gal::ReturnCode ServiceElementLCP::discovery(const std::string& resource_id,
											 bool committed,
											 gal::LogicalResource* resource,
											 gal::LogicalResourceContainer* /*logical_resources*/,
											 gal::PhysicalResourceContainer* physical_resources, 
											 gal::SensorResourceContainer* sensor_resources, 
											 gal::PowerStateContainer* power_states, 
											 gal::OptimalConfigContainer* /*edl*/)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Device
    {
        if (resource)
        {
            resource->id = boost::asio::ip::host_name();
            resource->type = "DROP Service Element";
            resource->description = "DROP Service Element";
        }

        if (sensor_resources)
        {
            for (const auto& s : device_sensors_)
            {
                sensor_resources->push_back(s.second);
            }
        }

        if (power_states)
        {
            for (const auto& p : power_states_)
            {
                power_states->push_back(p.second);
            }
        }
    }

    lcp_task_->discovery(resource_id, committed, nullptr, nullptr, physical_resources, sensor_resources, nullptr, nullptr);

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ServiceElementLCP::provisioning(const std::string& resource_id, int ps)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    provisioned_power_state_ = ps;
    lcp_task_->provisioning(resource_id, ps);

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ServiceElementLCP::release(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    provisioned_power_state_ = committed_power_state_ = default_power_state_;
    lcp_task_->release(resource_id);

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ServiceElementLCP::monitor_state(const std::string& resource_id, bool committed, int& ps)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0" || ids.size() > 1)
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

	ps = committed ? committed_power_state_ : provisioned_power_state_;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ServiceElementLCP::monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* /*history*/)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    // TODO:

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0" || ids.size() == 1)
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids[1] == "traffic")
    {
        oper_status = gal::EntitySensorStatus::ok;
        sensor_value = rx_rate_;
        value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;

        return gal::ReturnCode::GalSuccess;
    }
    
    return lcp_task_->monitor_sensor(resource_id, oper_status, sensor_value, value_timestamp);
}

gal::ReturnCode ServiceElementLCP::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode ServiceElementLCP::commit(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    committed_power_state_ = provisioned_power_state_;
    lcp_task_->commit(resource_id);

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ServiceElementLCP::rollback(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    provisioned_power_state_ = committed_power_state_;
    lcp_task_->rollback(resource_id);

    return gal::ReturnCode::GalSuccess;
}

const gal::PowerState& ServiceElementLCP::get_power_state(const std::string& resource_id, PowerStateId ps)
{
    if (resource_id != "0")
    {
        throw NonExistentEnergyAwareState(std::string(R"(Energy Aware State ")") + std::to_string(ps) + R"(" not found.)");
    }

    auto it = power_states_.find(ps);

    if (it != std::end(power_states_))
    {
        return it->second;
    }

    throw NonExistentEnergyAwareState(std::string(R"(Energy Aware State ")") + std::to_string(ps) + R"(" not found.)");
}

void ServiceElementLCP::read_power_states()
{
    pugi::xml_document doc;
    auto result = doc.load_file(util::config_file_path("gal").c_str());

    if (!result)
    {
        tnt::ConfigurationError(std::string("ServiceElementLCP: ") + result.description());
    }

    auto root = doc.child("gal");
    assert(root);

    max_power_consumption_ = root.attribute("max_power_consumption").as_double();

    auto ps = root.child("EnergyAwareStates");

    provisioned_power_state_ = committed_power_state_ = default_power_state_ = ps.attribute("default").as_int();

    auto states = load_power_states(ps);

    for (const auto& state : states)
    {
        power_states_[state.id] = state;
    }

    auto sr = root.child("SensorResources");

    if (sr)
    {
        device_sensors_ = load_sensors(sr);
    }
}

} // namespace activity
} // namespace drop
