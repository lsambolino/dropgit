
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

#include "ce_lcp.hpp"

#include <fstream>
#include <atomic>
#include <future>

#include "activity/gal/lcp_sync.hpp"

#include "event/quit.hpp"
#include "event/router_requests.hpp"
#include "event/service_request.hpp"
#include "event/gsi_request.hpp"
#include "event/network/service_element_disconnected.hpp"
#include "event/network/service_element_active.hpp"
#include "event/gal/traffic_profile_change.hpp"
#include "event/gal/lcp_task_started.hpp"
#include "event/gal/lcp_started.hpp"
#include "event/gal/data_update.hpp"

#include "exception/drop_exception.hpp"

#include "gal_drop/load_power_states.hpp"
#include "gal_drop/load_sensors.hpp"
#include "gal_drop/local_control_policy_task.hpp"

#include "network/service_element.hpp"

#include "router/port_info.hpp"
#include "router/interface.hpp"

#include "service/interconnection.hpp"
#include "service/forwarding.hpp"

#include "util/configuration.hpp"
#include "util/gal_resource_names.hpp"
#include "util/pugixml.hpp"
#include "util/factory.hpp"
#include "util/path.hpp"
#include "util/string.hpp"
#include "util/exec.hpp"
#include "util/wol.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "containers.hpp"
#include "log.hpp"
#include "range.hpp"
#include "application.hpp"
#include "demangle.hpp"

namespace drop {
namespace activity {
namespace {

gal::ReturnCode read_sensor(const boost::circular_buffer<gal::SensorHistory>& history, gal::EntitySensorStatus& status, gal::EntitySensorValue& value, gal::EntitySensorTimeStamp& ts)
{
    status = gal::EntitySensorStatus::unavailable;

    if (!history.empty())
    {
        status = gal::EntitySensorStatus::ok;
        const auto& last = history.back();
        ts = last.ts;
        value = last.value;
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode read_sensor(std::mutex& guard, const boost::circular_buffer<gal::SensorHistory>& history, gal::EntitySensorStatus& status, gal::EntitySensorValue& value, gal::EntitySensorTimeStamp& ts)
{
    std::unique_lock<std::mutex> lock(guard);

    return read_sensor(history, status, value, ts);
}

gal::ReturnCode read_sensor(std::mutex& guard, const std::map<std::string, boost::circular_buffer<gal::SensorHistory>>& map, const std::string& name, gal::EntitySensorStatus& status, gal::EntitySensorValue& value, gal::EntitySensorTimeStamp& ts)
{
    std::unique_lock<std::mutex> lock(guard);

    auto it = map.find(name);
    
    if (it == std::end(map))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    return read_sensor(it->second, status, value, ts);
}

void read_history(const boost::circular_buffer<gal::SensorHistory>& buffer, gal::SensorHistoryContainer* history)
{
    for (const auto& d : buffer)
    {
        history->push_back(d);
    }
}

void read_history(std::mutex& guard, const boost::circular_buffer<gal::SensorHistory>& buffer, gal::SensorHistoryContainer* history)
{
    tnt::lock(guard, [&] ()
    {
        read_history(buffer, history);
    });
}

void read_history(std::mutex& guard, const std::map<std::string, boost::circular_buffer<gal::SensorHistory>>& map, const std::string& name, gal::SensorHistoryContainer* history)
{
    tnt::lock(guard, [&] ()
    {
        auto it = map.find(name);

        if (it != std::end(map))
        {
            read_history(it->second, history);
        }
    });
}

template <class T> void sleep(const T& prev)
{
    static auto interval = tnt::Configuration::get("data.elements.polling.interval", 1);

    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - prev).count() < interval)
    {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

gal::EntitySensorValue monitor_sensor(const std::string& path)
{
    try
    {
        auto r = tnt::exec(path);
    
        return r.empty() ? 0ll : std::stoll(r);
    }
    catch (...)
    {
        return -1;
    }
}

gal::EntitySensorValue monitor_device_trafic()
{
    return monitor_sensor("/root/bin/get-traffic.sh");
}

gal::EntitySensorValue monitor_element_trafic(const std::string& name)
{
    return monitor_sensor(std::string("/root/bin/get-traffic-") + name + ".sh");
}

gal::EntitySensorValue monitor_element_consumption(const std::string& name)
{
    return monitor_sensor(std::string("/root/bin/get-pc-") + name + ".sh");
}

bool is_xlp(const std::shared_ptr<ce::ServiceElement>& se, gal::GreenStandardInterface* gsi)
{
    gal::SensorResourceContainer sr;
    gsi->discovery(std::string("0.") + se->display_name(), true, nullptr, nullptr, nullptr, &sr, nullptr, nullptr);

    return tnt::contains_if(sr, [] (const auto& s)
    {
        return s.id == "voltage";
    });
}

void fill_device_logical_resources(gal::LogicalResourceContainer* resources)
{
    assert(resources);

    auto e = std::make_shared<event::InterfacesRequest>([&] (const auto& i)
    {
        resources->push_back(gal::LogicalResource { i->name(), "DROP network interface", "Ethernet port" });
    });

    tnt::Application::raise(e);

    e->wait();
}

void fill_device_physical_resources(gal::PhysicalResourceContainer* resources)
{
    assert(resources);

    auto e = std::make_shared<event::ServiceElementsRequest>([&] (const auto& se)
    {
        resources->push_back(gal::PhysicalResource{ se->display_name(), "DROP Service Element", "Service Element", {}, {}, 0, 0 });
    });

    tnt::Application::raise(e);

    e->wait();
}

void fill_sensor_resources(gal::SensorResourceContainer* resources, const std::unordered_map<std::string, gal::SensorResource>& sensors)
{
    assert(resources);

    for (const auto& s : sensors)
    {
        resources->push_back(s.second);
    }
}

} // namespace

ControlElementLCP::ControlElementLCP() : history_size_(tnt::Configuration::get("data.history.size", 100)), lcp_task_(factory::create_lcp_process()), device_traffic_(history_size_)
{
    config();
}

ControlElementLCP::~ControlElementLCP()
{
    tnt::Log::debug(colors::green, "ControlElementLCP activity terminated.");
}

void ControlElementLCP::operator()()
{
    auto ei = std::make_shared<event::ExternalInterfacesRequest>([&] (const std::string& name, int index, int port)
    {
        interfaces_.emplace(name, ExternalInterface{ name, index, port });
    });

    tnt::Application::raise(ei);

    std::map<std::string, std::vector<std::pair<std::string, gal::SensorResource>>> sensors;

    auto es = std::make_shared<event::ServiceElementsRequest>([&] (const auto& se)
    {
        configure_service_element(se, sensors);
    });

    tnt::Application::raise(es);

    ei->wait();
    es->wait();

    /*for (const auto& s : sensors)
    {
        auto it = sensors_history_.emplace(s.first, SensorNode()).first;

        for (const auto& f : s.second)
        {
            auto& node = it->second;
            node.history.emplace(f.first, boost::circular_buffer<gal::SensorHistory>(history_size_));
        }

        auto name = s.first;
        auto ss = s.second;

        sensors_threads_.emplace_back([this, name, ss] ()
        {
            read_elements_sensor(name, ss);
        });
    }*/

    running_ = true;

    register_handler([&] (const tnt::event::Quit& /*event*/)
    {
        running_ = false;
    });

    register_handler([&] (std::shared_ptr<event::GSIRequest> event)
    {
        event->exec(this);
    });

    /*register_handler([&] (const event::ServiceElementActive& event)
    {
    });

    register_handler([&] (const event::ServiceElementDisconnected& event)
    {
    });*/

    register_handler([&] (const event::LCPTaskStarted& /*event*/)
    {
        tnt::Application::raise(event::LCPStarted());
    });

    lcp_task_->initialize();

    tnt::Thread th0([this] ()
    {
        read_elements_traffic();
    });

    tnt::Thread th1([this] ()
    {
        read_elements_consumption();
    });

    tnt::Thread th2([this] ()
    {
        read_elements_cores();
    });

    tnt::Thread th3([this]()
    {
        read_xlp_power_management();
    });

    tnt::Thread th4([this]()
    {
        read_interfaces_traffic();
    });

    tnt::Thread th5([this]()
    {
        read_device_traffic();
    });

    while (running_)
    {
        wait_event();
    }

    lcp_task_->finalize();
}

void ControlElementLCP::config()
{
    pugi::xml_document doc;
    auto result = doc.load_file(util::config_file_path("gal").c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("ControlElementLCP error: ") + result.description());
    }

    const auto root = doc.child("gal");
    const auto device = root.child("Device");
    const auto ifaces = root.child("Eth");

    if (device)
    {
        const auto ps = device.child("EnergyAwareStates");
        const auto power_states = load_power_states(ps);

        device_committed_power_state_ = device_provisioned_power_state_ = device_default_power_state_ = ps.attribute("default").as_int();

        for (const auto& ps : power_states)
        {
            device_power_states_[ps.id] = ps;
        }

        device_power_states_history_.push_back({ std::chrono::seconds(std::time(nullptr)).count(), device_default_power_state_ });

        auto sr = device.child("SensorResources");

        if (sr)
        {
            device_sensors_ = load_sensors(sr);
        }
    }

    if (ifaces)
    {
        const auto ps = ifaces.child("EnergyAwareStates");
        const auto power_states = load_power_states(ps);

        interfaces_default_power_state_ = ps.attribute("default").as_int();

        for (const auto ps : power_states)
        {
            interfaces_power_states_[ps.id] = ps;
        }

        auto e = std::make_shared<event::InterfacesRequest>([&](const std::shared_ptr<router::Interface>& i)
        {
            interfaces_provisioned_power_states_[i->name()] = interfaces_default_power_state_;
            interfaces_committed_power_states_[i->name()] = interfaces_default_power_state_;

            interfaces_power_states_history_[i->name()] = std::vector < gal::PowerStateHistory > { { std::chrono::seconds(std::time(nullptr)).count(), interfaces_default_power_state_ } };
        });

        tnt::Application::raise(e);

        auto sr = ifaces.child("SensorResources");

        if (sr)
        {
            interfaces_sensors_ = load_sensors(sr);
        }

        e->wait();
    }
}

void ControlElementLCP::configure_service_element(const std::shared_ptr<ce::ServiceElement>& se, std::map<std::string, std::vector<std::pair<std::string, gal::SensorResource>>>& sensors)
{
    const auto& name = se->display_name();

    service_elements_lcps_.emplace(name, ServiceElementLCP(se));

    gal::SensorResourceContainer sr;
    discovery(std::string("0.") + name, true, nullptr, nullptr, nullptr, &sr, nullptr, nullptr);

    for (const auto& s : sr)
    {
        auto it = sensors.find(s.id);

        if (it == std::end(sensors))
        {
            it = sensors.emplace(s.id, std::vector<std::pair<std::string, gal::SensorResource>>()).first;
        }

        it->second.emplace_back(name, s);
    }

    auto fe = tnt::any_of_is<service::Forwarding>(se->services());
    auto xlp = is_xlp(se, this);

    elements_.push_back(Element{ name, fe, xlp });
    elements_consumption_.emplace(name, boost::circular_buffer<gal::SensorHistory>(history_size_));

    if (fe)
    {
        elements_traffic_.emplace(name, boost::circular_buffer<gal::SensorHistory>(history_size_));

        if (!xlp)
        {
            elements_cores_.emplace(name, boost::circular_buffer<gal::SensorHistory>(history_size_));
        }
    }
        
    if (xlp)
    {
        elements_voltages_.emplace(name, boost::circular_buffer<gal::SensorHistory>(history_size_));

        std::map<int, boost::circular_buffer<gal::SensorHistory>> map;

        for (int i = 0; i < 8; ++i)
        {
            map.emplace(i, boost::circular_buffer<gal::SensorHistory>(history_size_));
        }

        elements_frequencies_.emplace(name, map);
    }
}

void ControlElementLCP::fill_device_power_states(gal::PowerStateContainer* power_states)
{
    assert(power_states);

    get_power_states(power_states, device_power_states_);
}

void ControlElementLCP::fill_interface_power_states(gal::PowerStateContainer* power_states)
{
    assert(power_states);

    get_power_states(power_states, interfaces_power_states_);
}

void ControlElementLCP::get_power_states(gal::PowerStateContainer* power_states, std::unordered_map<PowerStateId, gal::PowerState> power_states_)
{
    power_states->reserve(power_states_.size());

    for (const auto& ps : power_states_)
    {
        power_states->push_back(ps.second);
    }
}

gal::ReturnCode ControlElementLCP::discovery(const std::string& resource_id, 
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

    if (ids.size() == 2 && is_interface_name(ids[1]))
    {
        return interface_discovery(ids[1], committed, resource, sensor_resources, power_states, edl);
    }

    const auto& se = ids[1];
    auto name = compose(remove_fragment(ids, se));

    return lcp_forward(se, [&] (auto& lcp) { return lcp.discovery(name, committed, resource, logical_resources, physical_resources, sensor_resources, power_states, edl); });
}

gal::ReturnCode ControlElementLCP::provisioning(const std::string& resource_id, int power_state_id)
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

    if (ids.size() == 2 && is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_provisioning(ids[1], power_state_id);
    }

    const auto& se = ids[1];
    auto name = compose(remove_fragment(ids, se));

    return lcp_forward(se, [&] (auto& lcp) { return lcp.provisioning(name, power_state_id); });
}

gal::ReturnCode ControlElementLCP::release(const std::string& resource_id)
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

    if (ids.size() == 2 && is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_release(ids[1]);
    }

    const auto& fe = ids[1];
    auto name = compose(remove_fragment(ids, fe));

    return lcp_forward(fe, [&] (ServiceElementLCP& lcp) { return lcp.release(name); });
}

gal::ReturnCode ControlElementLCP::monitor_state(const std::string& resource_id, bool committed, PowerStateId& power_state_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device
    {
        return device_monitor_state(committed, power_state_id);
    }

    if (ids.size() == 2 && is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_monitor_state(ids[1], committed, power_state_id);
    }

    const auto& fe = ids[1];
    auto name = compose(remove_fragment(ids, fe));

    return lcp_forward(fe, [&] (ServiceElementLCP& lcp) { return lcp.monitor_state(name, committed, power_state_id); });
}

gal::ReturnCode ControlElementLCP::monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* history)
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

    if (ids.size() == 2 && is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_monitor_history(ids[1], history);
    }

    const auto& fe = ids[1];
    auto name = compose(remove_fragment(ids, fe));

    return lcp_forward(fe, [&] (ServiceElementLCP& lcp) { return lcp.monitor_history(name, history); });
}

gal::ReturnCode ControlElementLCP::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.size() < 2 || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 2) // Request for the device.
    {
        return device_monitor_sensor(ids[1], oper_status, sensor_value, value_timestamp);
    }

    if (is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_monitor_sensor(compose(remove_fragment(ids, "0")), oper_status, sensor_value, value_timestamp);
    }

    const auto& se = ids[1];
    const auto& sensor = ids[2];

    /*auto it = sensors_history_.find(sensor);

    if (it == std::end(sensors_history_))
    {
        auto name = compose(remove_fragment(ids, se));

        return lcp_forward(se, [&] (auto& lcp) { return lcp.monitor_sensor(name, oper_status, sensor_value, value_timestamp); });
    }

    oper_status = gal::EntitySensorStatus::unavailable;

    std::unique_lock<std::mutex> lock(*it->second.guard);

    const auto& e_history = it->second.history;
    auto e_it = e_history.find(se);

    if (e_it == std::end(e_history))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    const auto& history = e_it->second;

    if (!history.empty())
    {
        oper_status = gal::EntitySensorStatus::ok;
        const auto& last = history.back();
        value_timestamp = last.ts;
        sensor_value = last.value;
    }

    return gal::ReturnCode::GalSuccess;*/

    if (sensor == "traffic")
    {
        try
        {
            return read_sensor(elements_traffic_guard_, elements_traffic_, se, oper_status, sensor_value, value_timestamp);
        }
        catch (...)
        {
            tnt::Log::error("Reading traffic: No element ", se);

            return gal::ReturnCode::GalResourceNotFound;
        }
    }
    else if (sensor == "momentaryConsumption")
    {
        try
        {
            return read_sensor(elements_consumption_guard_, elements_consumption_, se, oper_status, sensor_value, value_timestamp);
        }
        catch (...)
        {
            tnt::Log::error("Reading consumption: No element ", se);

            return gal::ReturnCode::GalResourceNotFound;
        }
    }
    else if (sensor == "activeCores")
    {
        try
        {
            return read_sensor(elements_cores_guard_, elements_cores_, se, oper_status, sensor_value, value_timestamp);
        }
        catch (...)
        {
            tnt::Log::error("Reading cores: No element ", se);

            return gal::ReturnCode::GalResourceNotFound;
        }
    }
    else if (sensor == "voltage")
    {
        try
        {
            return read_sensor(elements_voltages_guard_, elements_voltages_, se, oper_status, sensor_value, value_timestamp);
        }
        catch (...)
        {
            tnt::Log::error("Reading voltage: No element ", se);

            return gal::ReturnCode::GalResourceNotFound;
        }
    }
    else if (sensor.find("freq") == 0)
    {
        auto i = std::stoi(sensor.substr(4));

        try
        {
            tnt::lock(elements_frequencies_guard_, [&] ()
            {
                const auto& f = elements_frequencies_.at(se);
                read_sensor(f.at(i), oper_status, sensor_value, value_timestamp);
            });
        }
        catch (...)
        {
            tnt::Log::error("Error reading frequencies: no element ", se, " or no sensor ", i);

            return gal::ReturnCode::GalResourceNotFound;
        }

        return gal::ReturnCode::GalSuccess;
    }
    else
    {
        auto name = compose(remove_fragment(ids, se));

        return lcp_forward(se, [&] (auto& lcp) { return lcp.monitor_sensor(name, oper_status, sensor_value, value_timestamp); });
    }
}

gal::ReturnCode ControlElementLCP::monitor_sensor_history(const std::string& resource_id, gal::SensorHistoryContainer* history)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (!history)
    {
        return gal::ReturnCode::GalError;
    }

    if (ids.size() == 2) // Request for the device.
    {
        if (ids[1] != "traffic")
        {
            return gal::ReturnCode::GalResourceNotFound;
        }

        read_history(device_traffic_guard_, device_traffic_, history);

        return gal::ReturnCode::GalSuccess;
    }
    else if (ids.size() == 3) // Request for an SE or an interface.
    {
        const auto& name = ids[1];
        const auto& sensor = ids[2];

        if (is_interface_name(name) && sensor == "traffic")
        {
            if (sensor != "traffic")
            {
                return gal::ReturnCode::GalResourceNotFound;
            }

            read_history(interfaces_traffic_guard_, interfaces_traffic_, name, history);

            return gal::ReturnCode::GalSuccess;
        }
        else if (sensor == "traffic")
        {
            read_history(elements_traffic_guard_, elements_traffic_, name, history);

            return gal::ReturnCode::GalSuccess;
        }
        else if (sensor == "momentaryConsumption")
        {
            read_history(elements_consumption_guard_, elements_consumption_, name, history);

            return gal::ReturnCode::GalSuccess;
        }
        else if (sensor == "activeCores")
        {
            read_history(elements_cores_guard_, elements_cores_, name, history);

            return gal::ReturnCode::GalSuccess;
        }
        else if (sensor == "voltage")
        {
            read_history(elements_voltages_guard_, elements_voltages_, name, history);

            return gal::ReturnCode::GalSuccess;
        }
        else if (sensor.find("freq") == 0)
        {
            auto i = std::stoi(sensor.substr(4));

            try
            {
                tnt::lock(elements_frequencies_guard_, [&] ()
                {
                    const auto& f = elements_frequencies_.at(name);
                    read_history(f.at(i), history);
                });
            }
            catch (...)
            {
                tnt::Log::error("Error reading frequencies: no element ", name, " or no sensor ", i);

                return gal::ReturnCode::GalResourceNotFound;
            }

            return gal::ReturnCode::GalSuccess;
        }
    }

    return gal::ReturnCode::GalResourceNotFound;
}

gal::ReturnCode ControlElementLCP::commit(const std::string& resource_id)
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

    if (ids.size() == 2 && is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_commit(ids[1]);
    }

    const auto& fe = ids[1];
    auto name = compose(remove_fragment(ids, fe));

    return lcp_forward(fe, [&] (ServiceElementLCP& lcp) { return lcp.commit(name); });
}

gal::ReturnCode ControlElementLCP::rollback(const std::string& resource_id)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.empty() || ids[0] != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids.size() == 1) // Request for the device.
    {
        device_rollback();
    }

    if (ids.size() == 2 && is_interface_name(ids[1])) // Request for an interface.
    {
        return interface_rollback(ids[1]);
    }

    const auto& fe = ids[1];
    auto name = compose(remove_fragment(ids, fe));

    return lcp_forward(fe, [&] (ServiceElementLCP& lcp) { return lcp.rollback(name); });
}

bool ControlElementLCP::is_interface_name(const std::string& name)
{
    return interfaces_.find(name) != std::end(interfaces_);
}

gal::ReturnCode ControlElementLCP::device_discovery(bool /*committed*/,
                                                    gal::LogicalResource* resource,
                                                    gal::LogicalResourceContainer* logical_resources,
                                                    gal::PhysicalResourceContainer* physical_resources,
                                                    gal::SensorResourceContainer* sensor_resources,
                                                    gal::PowerStateContainer* power_states,
                                                    gal::OptimalConfigContainer* /*edl*/)
{
    if (resource)
	{
		*resource = { "0", "DROP Distributed Software Router", "Network Router" };
	}

    if (logical_resources)
    {
        fill_device_logical_resources(logical_resources);
    }

    if (physical_resources)
    {
        fill_device_physical_resources(physical_resources);
    }

    if (sensor_resources)
    {
        fill_sensor_resources(sensor_resources, device_sensors_);
    }

    if (power_states)
    {
        fill_device_power_states(power_states);
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::device_monitor_sensor(const std::string& name, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    if (name == "traffic")
    {
        return read_sensor(device_traffic_guard_, device_traffic_, oper_status, sensor_value, value_timestamp);
    }
    
    return gal::ReturnCode::GalResourceNotFound;
}

gal::ReturnCode ControlElementLCP::device_provisioning(int power_state_id)
{
    device_provisioned_power_state_ = power_state_id;

    for (auto& p : service_elements_lcps_)
    {
        auto r = p.second.provisioning("0", power_state_id);

        switch (r)
        {
        case gal::ReturnCode::GalFailure:
        case gal::ReturnCode::GalError:
        case gal::ReturnCode::GalProtocolError:
            return gal::ReturnCode::GalFailure;
        case gal::ReturnCode::GalUndefined:
        case gal::ReturnCode::GalSuccess:
        case gal::ReturnCode::GalNotImplemented:
        case gal::ReturnCode::GalResourceNotFound:
        case gal::ReturnCode::GalResourceNotAvailable:
        default:
            break;
        }
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::device_commit()
{
    // TODO: Che fare se fallisce una commit ma altre sono già andate a buon fine?
    device_committed_power_state_ = device_provisioned_power_state_;

    for (auto& p : service_elements_lcps_)
    {
        auto r = p.second.commit("0");

        switch (r)
        {
        case gal::ReturnCode::GalFailure:
        case gal::ReturnCode::GalError:
        case gal::ReturnCode::GalProtocolError:
            return gal::ReturnCode::GalFailure;
        case gal::ReturnCode::GalUndefined:
        case gal::ReturnCode::GalSuccess:
        case gal::ReturnCode::GalNotImplemented:
        case gal::ReturnCode::GalResourceNotFound:
        case gal::ReturnCode::GalResourceNotAvailable:
        default:
            break;
        }
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::device_release()
{
    auto result = device_provisioning(device_default_power_state_);

    if (result != gal::ReturnCode::GalSuccess)
    {
        return result;
    }

    return device_commit();
}

gal::ReturnCode ControlElementLCP::device_monitor_state(bool committed, PowerStateId& power_state_id)
{
    power_state_id = committed ? device_committed_power_state_ : device_provisioned_power_state_;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::device_monitor_history(gal::PowerStateHistoryContainer* history)
{
    if (history)
    {
        *history = device_power_states_history_;
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::device_rollback()
{
    device_provisioned_power_state_ = device_committed_power_state_;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::interface_discovery(const std::string& name,
                                                       bool /*committed*/,
                                                       gal::LogicalResource* resource,
                                                       gal::SensorResourceContainer* sensor_resources,
                                                       gal::PowerStateContainer* power_states,
                                                       gal::OptimalConfigContainer* edl)
{
    if (resource)
	{
		*resource = { name, "DROP network interface", "Ethernet port" };
	}

    if (power_states)
    {
        fill_interface_power_states(power_states);
    }

    if (sensor_resources)
    {
        fill_sensor_resources(sensor_resources, interfaces_sensors_);
    }

    if (edl)
    {
        // TODO:
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::interface_provisioning(const std::string& name, int power_state_id)
{
    auto it = interfaces_provisioned_power_states_.find(name);

    if (it == std::end(interfaces_provisioned_power_states_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    it->second = power_state_id;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::interface_release(const std::string& name)
{
    auto result = interface_provisioning(name, interfaces_default_power_state_);

    if (result != gal::ReturnCode::GalSuccess)
    {
        return result;
    }

    return interface_commit(name);
}

gal::ReturnCode ControlElementLCP::interface_monitor_state(const std::string& name, bool committed, PowerStateId& power_state_id)
{
    if (committed)
    {
        auto it = interfaces_committed_power_states_.find(name);

        if (it == std::end(interfaces_committed_power_states_))
        {
            return gal::ReturnCode::GalResourceNotFound;
        }

        power_state_id = it->second;

        return gal::ReturnCode::GalSuccess;
    }
    else
    {
        auto it = interfaces_provisioned_power_states_.find(name);

        if (it == std::end(interfaces_provisioned_power_states_))
        {
            return gal::ReturnCode::GalResourceNotFound;
        }

        power_state_id = it->second;

        return gal::ReturnCode::GalSuccess;
    }

    power_state_id = -1;

    return gal::ReturnCode::GalError;
}

gal::ReturnCode ControlElementLCP::interface_monitor_history(const std::string& name, gal::PowerStateHistoryContainer* history)
{
    if (history)
    {
        auto it = interfaces_power_states_history_.find(name);

        if (it == std::end(interfaces_power_states_history_))
        {
            return gal::ReturnCode::GalResourceNotFound;
        }

        *history = it->second;
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::interface_monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto ids = tnt::split(resource_id, ".");

    if (ids.size() != 2)
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (ids[1] == "traffic")
    {
        return read_sensor(interfaces_traffic_guard_, interfaces_traffic_, ids[0], oper_status, sensor_value, value_timestamp);
    }

    return gal::ReturnCode::GalResourceNotFound;
}

gal::ReturnCode ControlElementLCP::interface_commit(const std::string& name)
{
    auto pit = interfaces_provisioned_power_states_.find(name);

    if (pit == std::end(interfaces_provisioned_power_states_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    auto ps = pit->second;

    auto cit = interfaces_committed_power_states_.find(name);

    if (cit == std::end(interfaces_committed_power_states_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    cit->second = ps;

    auto hit = interfaces_power_states_history_.find(name);

    if (hit != std::end(interfaces_power_states_history_))
    {
        hit->second.push_back({ std::chrono::seconds(std::time(nullptr)).count(), ps });
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::interface_rollback(const std::string& name)
{
    auto pit = interfaces_provisioned_power_states_.find(name);

    if (pit == std::end(interfaces_provisioned_power_states_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    auto cit = interfaces_committed_power_states_.find(name);

    if (cit == std::end(interfaces_committed_power_states_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    pit->second = cit->second;

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode ControlElementLCP::lcp_forward(const std::string& id, std::function<gal::ReturnCode(ServiceElementLCP&)> func)
{
    try
    {
        auto it = service_elements_lcps_.find(id);

        if (it == std::end(service_elements_lcps_))
        {
            return gal::ReturnCode::GalResourceNotFound;
        }

        try
        {
            return func(it->second);
        }
        catch (std::exception& ex)
        {
            tnt::Log::error("ControlElementLCP error forwarding a call to the LCP for the SE ", id, ": ", ex.what());
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error(R"(ControlElementLCP Error: wrong resource ")", id, R"(")");
    }

    return gal::ReturnCode::GalFailure;
}

void ControlElementLCP::read_elements_traffic()
{
    while (running_)
    {
        auto prev = std::chrono::steady_clock::now();
        auto now = static_cast<gal::EntitySensorTimeStamp>(std::chrono::seconds(std::time(nullptr)).count() * 1000);
        std::map<std::string, gal::SensorHistory> traffic;

        for (const auto& e : elements_)
        {
            const auto& name = e.name;

            if (e.is_fe)
            {
                auto value = monitor_element_trafic(name);

                if (value == -1)
                {
                    value = 0;
                }

                traffic.emplace(name, gal::SensorHistory{ now, value });
            }
        }

        if (!traffic.empty())
        {
            tnt::lock(elements_traffic_guard_, [&] ()
            {
                for (const auto& p : traffic)
                {
                    try
                    {
                        elements_traffic_.at(p.first).push_back(p.second);
                    }
                    catch (...)
                    {
                        tnt::Log::error("Monitoring traffic: No element ", p.first);
                    }
                }
            });
        }

        sleep(prev);
    }
}

void ControlElementLCP::read_elements_consumption()
{
    while (running_)
    {
        auto prev = std::chrono::steady_clock::now();
        auto now = static_cast<gal::EntitySensorTimeStamp>(std::chrono::seconds(std::time(nullptr)).count() * 1000);
        std::map<std::string, gal::SensorHistory> consumption;

        for (const auto& e : elements_)
        {
            const auto& name = e.name;

            auto value = monitor_element_consumption(name);

            if (value == -1)
            {
                value = 0;
            }

            consumption.emplace(name, gal::SensorHistory{ now, value });
        }

        if (!consumption.empty())
        {
            tnt::lock(elements_consumption_guard_, [&] ()
            {
                for (const auto& p : consumption)
                {
                    try
                    {
                        elements_consumption_.at(p.first).push_back(p.second);
                    }
                    catch (...)
                    {
                        tnt::Log::error("Monitoring consumption: No element ", p.first);
                    }
                }
            });
        }

        sleep(prev);
    }
}

void ControlElementLCP::read_device_traffic()
{
    while (running_)
    {
        auto prev = std::chrono::steady_clock::now();
        auto now = static_cast<gal::EntitySensorTimeStamp>(std::chrono::seconds(std::time(nullptr)).count() * 1000);
        auto value = monitor_device_trafic();

        if (value != -1)
        {
            tnt::lock(device_traffic_guard_, [&] ()
            {
                device_traffic_.push_back(gal::SensorHistory{ now, value });
            });
        }

        sleep(prev);
    }
}

void ControlElementLCP::read_interfaces_traffic()
{
    while (running_)
    {
        std::vector<InterfaceData> interfaces_data;
        uint64_t tot = 0;

        auto prev = std::chrono::steady_clock::now();

        for (const auto& p : interfaces_)
        {
            const auto& i = p.second;
            auto data = tnt::exec(std::string("/root/bin/get-port-all-traffic.sh ") + std::to_string(i.port));
            
            if (!data.empty())
            {
                std::istringstream is(data);

                uint64_t rxpkts;
                uint64_t rxbytes;
                uint64_t txpkts;
                uint64_t txbytes;

                is >> rxpkts >> rxbytes >> txpkts >> txbytes;
                interfaces_data.emplace_back(i.index, rxpkts, rxbytes, txpkts, txbytes);

                tot += rxbytes;
            }
        }

        tnt::Application::raise(event::ExternalInterfacesDataUpdate(interfaces_data));

        sleep(prev);
    }
}

void ControlElementLCP::read_elements_cores()
{
    while (running_)
    {
        auto prev = std::chrono::steady_clock::now();
        auto now = static_cast<gal::EntitySensorTimeStamp>(std::chrono::seconds(std::time(nullptr)).count() * 1000);
        std::map<std::string, gal::SensorHistory> cores;

        for (const auto& e : elements_)
        {
            const auto& name = e.name;

            if (e.is_fe && !e.is_xlp)
            {
                gal::EntitySensorStatus status;
                gal::EntitySensorValue value;
                gal::EntitySensorTimeStamp ts;

                auto res = lcp_forward(name, [&] (ServiceElementLCP& lcp) { return lcp.monitor_sensor("0.activeCores", status, value, ts); });

                if (res != gal::ReturnCode::GalSuccess || status != gal::EntitySensorStatus::ok)
                {
                    value = 0;
                }

                cores.emplace(name, gal::SensorHistory{ now, value });
            }
        }

        if (!cores.empty())
        {
            tnt::lock(elements_cores_guard_, [&] ()
            {
                for (const auto& p : cores)
                {
                    try
                    {
                        elements_cores_.at(p.first).push_back(p.second);
                    }
                    catch (...)
                    {
                        tnt::Log::error("Monitoring cores: No element ", p.first);
                    }
                }
            });
        }

        sleep(prev);
    }
}

void ControlElementLCP::read_xlp_power_management()
{
    while (running_)
    {
        auto prev = std::chrono::steady_clock::now();
        auto now = static_cast<gal::EntitySensorTimeStamp>(std::chrono::seconds(std::time(nullptr)).count() * 1000);
        std::map<std::string, gal::SensorHistory> voltages;
        std::map<std::string, std::map<int, gal::SensorHistory>> frequencies;

        for (const auto& e : elements_)
        {
            const auto& name = e.name;

            if (e.is_xlp)
            {
                gal::EntitySensorStatus status;
                gal::EntitySensorValue value;
                gal::EntitySensorTimeStamp ts;

                auto res = lcp_forward(name, [&] (ServiceElementLCP& lcp) { return lcp.monitor_sensor("0.voltage", status, value, ts); });

                if (res == gal::ReturnCode::GalSuccess && status == gal::EntitySensorStatus::ok)
                {
                    voltages.emplace(name, gal::SensorHistory{ now, value });
                }

                std::map<int, gal::SensorHistory> map;

                for (int i = 0; i < 8; ++i)
                {
                    res = lcp_forward(name, [&] (ServiceElementLCP& lcp) { return lcp.monitor_sensor(std::string("0.freq") + std::to_string(i), status, value, ts); });

                    if (res == gal::ReturnCode::GalSuccess && status == gal::EntitySensorStatus::ok)
                    {
                        map.emplace(i, gal::SensorHistory{ now, value });
                    }
                }

                frequencies.emplace(name, map);
            }
        }

        if (!voltages.empty())
        {
            tnt::lock(elements_voltages_guard_, [&] ()
            {
                for (const auto& p : voltages)
                {
                    try
                    {
                        elements_voltages_.at(p.first).push_back(p.second);
                    }
                    catch (...)
                    {
                        tnt::Log::error("Monitoring voltages: No element ", p.first);
                    }
                }
            });
        }

        if (!frequencies.empty())
        {
            tnt::lock(elements_frequencies_guard_, [&] ()
            {
                for (const auto& p : frequencies)
                {
                    try
                    {
                        auto& map = elements_frequencies_.at(p.first);

                        for (const auto& m : p.second)
                        {
                            try
                            {
                                map.at(m.first).push_back(m.second);
                            }
                            catch (...)
                            {
                                tnt::Log::error("Monitoring frequencies: No frequency ", m.first);
                            }
                        }
                    }
                    catch (...)
                    {
                        tnt::Log::error("Monitoring frequencies: No element ", p.first);
                    }
                }
            });
        }

        sleep(prev);
    }
}

void ControlElementLCP::read_elements_sensor(const std::string& /*name*/, const std::vector<std::pair<std::string, gal::SensorResource>>& /*sensors*/)
{
    /*while (running_)
    {
        auto prev = std::chrono::steady_clock::now();
        auto now = static_cast<gal::EntitySensorTimeStamp>(std::chrono::seconds(std::time(nullptr)).count() * 1000);
        std::map<std::string, gal::SensorHistory> values;

        for (const auto& s : sensors)
        {
            const auto& se = s.first;
            gal::EntitySensorStatus status;
            gal::EntitySensorValue value;
            gal::EntitySensorTimeStamp ts;
            const auto& id = std::string("0.") + s.second.id;

            auto ret = lcp_forward(se, [&] (auto& lcp) { return lcp.monitor_sensor(id, status, value, ts); });

            if (ret != gal::ReturnCode::GalSuccess)
            {
                value = 0;
            }

            values.emplace(se, gal::SensorHistory{ now, value });
        }

        if (!values.empty())
        {
            auto it = sensors_history_.find(name);

            if (it == std::end(sensors_history_))
            {
                assert(false);
            }

            tnt::lock(*it->second.guard, [&] ()
            {
                for (const auto& p : values)
                {
                    try
                    {
                        it->second.history.at(p.first).push_back(p.second);
                    }
                    catch (...)
                    {
                        tnt::Log::error("Reading sensor: No element ", p.first);
                    }
                }
            });
        }

        sleep(prev);
    }*/
}

} // namespace activity
} // namespace drop

