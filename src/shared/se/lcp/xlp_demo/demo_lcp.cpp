
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
#include <cstdint>
#include <fstream>

#include <boost/asio.hpp>

#include <sys/ioctl.h>

#include "exception/drop_exception.hpp"

#include "event/gal/profile_change.hpp"
#include "event/gal/slot_change.hpp"
#include "event/gal/active_cores_change.hpp"
#include "event/gal/profile_change.hpp"
#include "event/poweroff.hpp"
#include "event/stop.hpp"
#include "event/quit.hpp"
#include "event/gal/statistics_update.hpp"

#include "message/network/management.hpp"

#include "gal_drop/acpi.hpp"
#include "gal/power_state.hpp"
#include "gal/logical_resource.hpp"
#include "gal/optimal_config.hpp"

#include "util/pugixml.hpp"
#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/string.hpp"
#include "util/random.hpp"
#include "util/exec.hpp"

#include "ip_address.hpp"
#include "ip_socket_address.hpp"
#include "containers.hpp"
#include "log.hpp"
#include "file_descriptor.hpp"

namespace drop {
namespace {

struct request
{
	uint32_t address;
	uint32_t lenght;
	uint32_t data[10];
};

std::map<int, int> frequencies_map = 
{
    {  1, 1600 },
    {  2, 1066 },
    {  3,  800 },
    {  4,  640 },
    {  5,  533 },
    {  6,  457 },
    {  7,  400 },
    {  8,  320 },
    {  9,  266 },
    { 10,  229 },
    { 11,  200 }
};

std::map<int, int> voltages_map = 
{
    { 0x5a, 1043 },
    { 0x5b, 1036 },
    { 0x5d, 1024 },
    { 0x5e, 1018 },
    { 0x60, 1005 },
    { 0x62,  992 },
    { 0x63,  986 },
    { 0x65,  973 },
    { 0x66,  967 },
    { 0x68,  955 },
    { 0x6a,  942 },
    { 0x6b,  936 },
    { 0x6d,  923 },
    { 0x6e,  917 },
    { 0x70,  905 }
};

uint64_t get_packets(const char* path)
{
    std::ifstream ifs(path);

    if (!ifs)
    {
        return 0;
    }

    uint64_t rx;
    ifs >> rx;

    return rx;
}

} // namespace

void DemoLCP::initialize()
{
    config();

    run_thread_.start([this]
    {
        run();
    });

    stats_thread_.start([this]
    {
        update_stats();
    });
}

void DemoLCP::finalize() 
{
    push_event(std::make_shared<tnt::event::Stop>());
}

gal::ReturnCode DemoLCP::discovery(const std::string& resource_id,
                                   bool /*committed*/,
                                   gal::LogicalResource* resource,
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
            s.max_value = 1043;
            s.description = "CPU Voltage";
            s.id = "voltage";
            s.scale = gal::EntitySensorDataScale::milli;
            s.type = gal::EntitySensorDataType::voltsDC;
        
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

        for (int i = 0; i < 8; ++i)
        {
            auto s = gal::SensorResource();
            s.max_value = 1600;
            s.description = std::string("Core ") + std::to_string(i) + " Frequency";
            s.id = std::string("freq") + std::to_string(i);
            s.scale = gal::EntitySensorDataScale::mega;
            s.type = gal::EntitySensorDataType::hertz;
        
            sensor_resources->push_back(s);
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

    if (ids[1] == "voltage")
    {
        oper_status = gal::EntitySensorStatus::ok;
        sensor_value = voltages_map[current_voltage_];
        value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
    
        return gal::ReturnCode::GalSuccess;
    }
    else if (ids[1] == "cpuLoad")
    {
        oper_status = gal::EntitySensorStatus::ok;
        sensor_value = cpu_load();
        value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
    
        return gal::ReturnCode::GalSuccess;
    }
    else if (ids[1].find("freq") == 0)
    {
        try
        {
            auto n = std::stoi(ids[1].substr(4));

            oper_status = gal::EntitySensorStatus::ok;
            sensor_value = frequencies_map[current_freqs_[n]];
            value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;
        }
        catch (...)
        {
            return gal::ReturnCode::GalError;
        }

        return gal::ReturnCode::GalSuccess;
    }

    return gal::ReturnCode::GalResourceNotFound;
}

gal::ReturnCode DemoLCP::monitor_sensor_history(const std::string& resource_id, gal::SensorHistoryContainer* history)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode DemoLCP::commit(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

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

        do_change_slot(slot);

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
        throw tnt::ConfigurationError(std::string("DemoLCP: ") + result.description());
    }

    const auto& root = doc.child("demo");
    assert(root);

    load_slots(root);
}

void DemoLCP::load_slots(const pugi::xml_node& root)
{
    const auto& profiles = root.child("profiles");

    for (const auto& profile : profiles.children("profile"))
    {
        auto& table = profiles_[profile.attribute("id").as_int()];

        const auto& slots = profile.child("slots");

        for (const auto& slot : slots.children("slot"))
        {
            Slot s;
     
            sscanf(slot.child("cores").attribute("a0").as_string(), "0x%x", &s.active_cores[0]);
	        sscanf(slot.child("cores").attribute("a1").as_string(), "0x%x", &s.active_cores[1]);
	        sscanf(slot.child("cores").attribute("a2").as_string(), "0x%x", &s.active_cores[2]);
	        sscanf(slot.child("cores").attribute("a3").as_string(), "0x%x", &s.active_cores[3]);
            sscanf(slot.child("freq").attribute("value").as_string(), "0x%x", &s.freq);
            sscanf(slot.child("voltage").attribute("value").as_string(), "0x%x", &s.voltage);
            
            /*s.active_cores[0] = slot.child("cores").attribute("a0").as_int();
	        s.active_cores[1] = slot.child("cores").attribute("a1").as_int();
	        s.active_cores[2] = slot.child("cores").attribute("a2").as_int();
	        s.active_cores[3] = slot.child("cores").attribute("a3").as_int();
            s.freq = slot.child("freq").attribute("value").as_int();
            s.voltage = slot.child("voltage").attribute("value").as_int();*/

            table.emplace(slot.attribute("num").as_int(), s);
        }
    }

    default_profile_ = profiles.attribute("default").as_int();
    committed_profile_ = default_profile_;
}

void DemoLCP::change_slot(int num)
{
    const auto& slots = profiles_[committed_profile_];
    auto it = slots.find(num);

    if (it == std::end(slots))
    {
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

        if (!slot.command.empty())
        {
            std::system(slot.command.c_str());

            return;
        }

        // Set parameters for the current slot.
        do_change_slot(slot);
        
        current_slot_ = num;
    }
}

void DemoLCP::do_change_slot(const Slot& slot)
{
    uint32_t dvs[4];
		
	tnt::Log::info(colors::cyan, "Active cores0 ", std::hex, slot.active_cores[0] * 4);
	tnt::Log::info(colors::cyan, "Active cores1 ", std::hex, slot.active_cores[1] * 4);
	tnt::Log::info(colors::cyan, "Active cores2 ", std::hex, slot.active_cores[2] * 4);
	tnt::Log::info(colors::cyan, "Active cores3 ", std::hex, slot.active_cores[3] * 4, std::dec);       
	
	dvs[0] = slot.active_cores[0] * 4;
	dvs[1] = slot.active_cores[1] * 4;
	dvs[2] = slot.active_cores[2] * 4;
	dvs[3] = slot.active_cores[3] * 4;
	
    current_voltage_ = slot.voltage;

    if (!set_configuration(slot.freq, slot.voltage, dvs))
    {
        tnt::Log::error("Demo LCP error: unable to open system files for the configuration.");
    }
}

bool DemoLCP::set_configuration(uint32_t freq, uint32_t volt, uint32_t* dv)
{
    tnt::FileDescriptor freq_file("/dev/xlp_power_freq", O_RDWR);
    tnt::FileDescriptor volt_file("/dev/xlp_power_volt", O_RDWR);
    tnt::FileDescriptor dv_file("/dev/xlp_power_dv", O_RDWR);

	char request[255];
	
    if (!freq_file || !volt_file || !dv_file)
    {
        return false;
    }
		
	// Setting frequencies
	for (int i = 0; i < 8; ++i)
    {
       	current_freqs_[i] = (freq & (0xf << i * 4)) >> (i * 4);
       	sprintf(request, "%d %x", i, current_freqs_[i]);
       	tnt::Log::info("Setting frequency: ", std::hex, request);
       	write(*freq_file, request, 255);
    }
       	
    // Setting voltage
    sprintf(request, "%x", volt);
    tnt::Log::info("Setting voltage: ", request);
    write(*volt_file, request, 255);
       	
    // Setting distribution vectors
    sprintf(request,"2 %x %x %x %x", dv[0], dv[1], dv[2], dv[3]);
    tnt::Log::info("Setting dvs: ", request, std::dec);
    write(*dv_file, request, 255);
       	
    return true;
}

int DemoLCP::cpu_load()
{
    auto sum = 0.0;

    for (int i = 0; i < 8; ++i)
    {
        sum += frequencies_map[current_freqs_[i]];
    }

    return static_cast<int>(sum / (1600.0 * 8.0) * 100.0);
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

    // Set the configuration for the slot 1.
    change_slot(1);

    while (running)
    {
        wait_event();
    }
}

void DemoLCP::update_stats()
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

    auto i0 = "/sys/class/net/eth0/statistics/rx_packets";
    auto i1 = "/sys/class/net/eth2/statistics/rx_packets";
    auto prev = std::chrono::steady_clock::now();
    uint64_t rx_prev = 0;

    while (running)
    {
        auto rx0 = get_packets(i0);
        auto rx1 = get_packets(i1);

        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - prev);
        auto rx = rx0 + rx1;

        uint64_t rx_rate = static_cast<double>(rx - rx_prev) / static_cast<double>(diff.count()) * 1000.0;
        rx_prev = rx;

        tnt::Application::raise(event::StatisticsUpdate(0, 0, rx_rate, 0));

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

} // namespace drop
