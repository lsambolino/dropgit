
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

#include "demo_lcp.hpp"

#include <cmath>
#include <utility>
#include <sstream>
#include <limits>

#include <boost/asio.hpp>

#include "activity/service_element.hpp"
#include "activity/gal/lcp_sync.hpp"

#include "exception/drop_exception.hpp"

#include "event/gal/slot_change.hpp"
#include "event/gal/active_cores_change.hpp"
#include "event/gal/profile_change.hpp"
#include "event/poweroff.hpp"
#include "event/stop.hpp"
#include "event/quit.hpp"

#include "message/network/management.hpp"

#include "gal_drop/acpi.hpp"

#include "gal/power_state.hpp"
#include "gal/logical_resource.hpp"
#include "gal/optimal_config.hpp"

#include "router/de_control_element.hpp"

#include "util/pugixml.hpp"
#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/string.hpp"

#include "application.hpp"
#include "containers.hpp"
#include "async.hpp"
#include "log.hpp"

namespace drop {

DemoLCP::~DemoLCP()
{
    run_thread_.join();
    tnt::Log::debug(colors::green, "DemoLCP activity terminated.");
}

void DemoLCP::initialize()
{
    config();

    run_thread_.start([this]
    {
        run();
    });
}

void DemoLCP::finalize() 
{
    push_event(std::make_shared<tnt::event::Stop>());
}

gal::ReturnCode DemoLCP::discovery(const std::string& resource_id,
                                   bool /*committed*/,
                                   gal::LogicalResource* /*resource*/,
                                   gal::LogicalResourceContainer* /*logical_resources*/,
                                   gal::PhysicalResourceContainer* physical_resources,
                                   gal::SensorResourceContainer* sensor_resources,
                                   gal::PowerStateContainer* /*power_states*/,
                                   gal::OptimalConfigContainer* /*edl*/)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (sensor_resources)
    {
        {
            auto s = gal::SensorResource();
            s.max_value = num_cores_;
            s.description = "Number of active cores";
            s.id = "activeCores";
            s.scale = gal::EntitySensorDataScale::units;
            s.type = gal::EntitySensorDataType::other;
        
            sensor_resources->push_back(s);
        }
        
        {
            auto s = gal::SensorResource();
            s.max_value = 100;
            s.description = "CPU percentage load";
            s.id = "cpuLoad";
            s.scale = gal::EntitySensorDataScale::units;
            s.type = gal::EntitySensorDataType::other;
        
            sensor_resources->push_back(s);
        }
    }

    if (physical_resources)
    {
        for (auto i = 0; i < num_cores_; ++i)
        {
            auto r = gal::PhysicalResource();
            r.id = resource_id + "." + boost::asio::ip::host_name() + ".core" + std::to_string(i);
            r.description = "CPU Core";
            r.type = "CPU Core";

            physical_resources->push_back(r);
        }
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode DemoLCP::provisioning(const std::string& resource_id, gal::PowerStateId power_state_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    provisioned_profile_ = power_state_id;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode DemoLCP::release(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    committed_profile_ = provisioned_profile_ = default_profile_;

    return commit(resource_id);
}

gal::ReturnCode DemoLCP::monitor_state(const std::string& resource_id, bool committed, gal::PowerStateId& power_state_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    power_state_id = committed ? committed_profile_ : provisioned_profile_;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode DemoLCP::monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* /*history*/)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode DemoLCP::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.size() < 2)
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids[1] == "activeCores")
    {
        oper_status = gal::EntitySensorStatus::ok;
        sensor_value = active_cores_;
        value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
    
        return gal::ReturnCode::GalSuccess;
    }
    else if (ids[1] == "cpuLoad")
    {
        oper_status = gal::EntitySensorStatus::ok;
        sensor_value = static_cast<double>(active_cores_) / static_cast<double>(num_cores_) * 100.0;
        value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
    
        return gal::ReturnCode::GalSuccess;
    }

    return gal::ReturnCode::GalResourceNotFound;
}

gal::ReturnCode DemoLCP::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode DemoLCP::commit(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    auto old_ps = committed_profile_;

    committed_profile_ = provisioned_profile_;

    const auto& slots = profiles_[committed_profile_];

    for (int i = current_slot_; i >= 1; --i)
    {
        auto it = slots.find(i);

        if (it == std::end(slots))
        {
            continue;
        }

        current_slot_ = i;
        const auto& slot = it->second;

        tnt::Log::info(colors::green, "Changing power profile #", old_ps, " ==> #", committed_profile_, " -> slot #", current_slot_, " (", slot.active_cores, " active cores, sleep: ", std::boolalpha, slot.sleep, ")");

        tnt::Application::raise(event::ActiveCoresChange(slot.active_cores, slot.sleep));

        break;
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode DemoLCP::rollback(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    provisioned_profile_ = committed_profile_;

    return gal::ReturnCode::GalSuccess;
}

void DemoLCP::config()
{
    auto lcp_path = util::config_file_path("demo");

    pugi::xml_document doc;
    auto result = doc.load_file(lcp_path.c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string(R"(DemoLCP error loading lcp file ")") + lcp_path + R"(" : )" + result.description());
    }

    const auto& root = doc.child("demo");
    assert(root);

    num_cores_ = root.attribute("num_cores").as_int();

    load_slots(root);
}

void DemoLCP::load_slots(const pugi::xml_node& root)
{
    assert(root);

    const auto& profiles = root.child("profiles");

    for (const auto& profile : profiles.children("profile"))
    {
        auto& table = profiles_[profile.attribute("id").as_int()];

        const auto& slots = profile.child("slots");

        for (const auto& slot : slots.children("slot"))
        {
            auto cores = slot.child("cores");
            Slot s;

            if (cores)
            {
                s.active_cores = cores.attribute("active").as_int();
                s.sleep = cores.attribute("sleep").as_bool(true);
            }

            auto cmd = slot.child("command");

            if (cmd)
            {
                s.command = cmd.child_value();
            }

            table.emplace(slot.attribute("num").as_int(), s);
        }
    }

    default_profile_ = profiles.attribute("default").as_int();
    provisioned_profile_ = committed_profile_ = default_profile_;
}

void DemoLCP::change_slot(int num)
{
    auto current_slot = num;

    const auto& slots = profiles_[committed_profile_];
    auto it = slots.find(num);
    auto no_back = true;

    if (it == std::end(slots))
    {
        no_back = false;

        // No change in this slot. Go backward to find the last slot with a change.

        while (true)
        {
            --num;

            if (num <= 1)
            {
                break;
            }

            it = slots.find(num);

            if (it != std::end(slots))
            {
                break;
            }
        }
    }

    if (num > current_slot_)
    {
        tnt::Log::info(colors::cyan, "Slot -> ", num);
        const auto& slot = it->second;

        if (no_back && !slot.command.empty())
        {
            std::system(slot.command.c_str());

            return;
        }

        tnt::Application::raise(event::ActiveCoresChange(slot.active_cores, slot.sleep));
        active_cores_ = slot.active_cores;
        current_slot_ = current_slot;
    }
}

void DemoLCP::run()
{
    std::atomic_bool running{ true };

    register_handler([&] (const tnt::event::Stop& /*event*/)
    {
        running = false;
    });

    register_handler([&] (const tnt::event::Quit& /*event*/)
    {
        running = false;
    });

    register_handler([&] (const event::SlotChange& event)
    {
        change_slot(event.num());
    });

    auto activity = tnt::async([] ()
    {
        if (tnt::Configuration::exists("lcp.sync"))
        {
            activity::LCPSync(tnt::Configuration::get("lcp.sync.ip").as<std::string>(), tnt::Configuration::get("lcp.sync.port").as<int>(), tnt::Configuration::get("lcp.sync.slots").as<int>())();
        }
    });

    // Set the configuration for the slot 1.
    change_slot(1);

    while (running)
    {
        wait_event();
    }

    activity.get();
}

} //namespace drop
