
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

#include "activity/gal/lcp_sync.hpp"

#include "exception/drop_exception.hpp"

#include "event/gal/slot_change.hpp"
#include "event/gal/lcp_task_started.hpp"
#include "event/network/service_element_active.hpp"
#include "event/network/service_element_disconnected.hpp"
#include "event/stop.hpp"
#include "event/quit.hpp"

#include "gal_drop/acpi.hpp"
#include "gal/power_state.hpp"
#include "gal/logical_resource.hpp"
#include "gal/optimal_config.hpp"

#include "message/network/management.hpp"

#include "network/service_element.hpp"

#include "protocol/openflow/flow_factory.hpp"

#include "router/de_control_element.hpp"

#include "service/openflow.hpp"

#include "util/pugixml.hpp"
#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/wol.hpp"

#include "dynamic_pointer_visitor.hpp"
#include "application.hpp"
#include "ip_address.hpp"
#include "ip_socket_address.hpp"
#include "containers.hpp"
#include "async.hpp"
#include "log.hpp"
#include "range.hpp"

namespace drop {
namespace {

std::string read_var_value(const std::map<std::string, std::string>& vars_table, const std::string& key)
{
    auto it = vars_table.find(key);

    return it != std::end(vars_table) ? it->second : "";
}

std::string read_var(const std::map<std::string, std::string>& vars_table, const std::string& key)
{
    return key[0] == '$' ? read_var_value(vars_table, key.substr(1)) : key;
}

} // namespace

DemoLCP::~DemoLCP()
{
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
    push_event(tnt::event::Stop());
}

gal::ReturnCode DemoLCP::discovery(const std::string& resource_id,
                                   bool /*committed*/,
                                   gal::LogicalResource* /*resource*/,
                                   gal::LogicalResourceContainer* /*logical_resources*/,
                                   gal::PhysicalResourceContainer* /*physical_resources*/,
                                   gal::SensorResourceContainer* /*sensor_resources*/,
                                   gal::PowerStateContainer* /*power_states*/,
                                   gal::OptimalConfigContainer* /*edl*/)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
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

gal::ReturnCode DemoLCP::monitor_sensor(const std::string& /*resource_id*/, gal::EntitySensorStatus& /*oper_status*/, gal::EntitySensorValue& /*sensor_value*/, gal::EntitySensorTimeStamp& /*value_timestamp*/)
{
    return gal::ReturnCode::GalNotImplemented;
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

    /*auto old_ps = committed_profile_;

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
    }*/

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode DemoLCP::rollback(const std::string& resource_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    //provisioned_profile_ = committed_profile_;

    return gal::ReturnCode::GalSuccess;
}

void DemoLCP::config()
{
    pugi::xml_document doc;
    auto result = doc.load_file(util::config_file_path("flows").c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("DemoLCP error: ") + result.description());
    }

    load_slots(doc.child("flows"));
}

void DemoLCP::load_slots(const pugi::xml_node& root)
{
    assert(root);

    std::map<std::string, std::string> vars_table;

    const auto& vars = root.child("vars");

    for (const auto& var : vars.children("var"))
    {
        vars_table.emplace(var.attribute("name").as_string(), var.attribute("value").as_string());
    }

    const auto& slots = root.child("slots");

    for (const auto& slot : slots.children("slot"))
    {
        load_slot(slot, vars_table);
    }
}

void DemoLCP::load_slot(const pugi::xml_node& root, const std::map<std::string, std::string>& vars_table)
{
    auto slot_num = root.attribute("num").as_int();
    const auto& wol = root.child("wol");

    if (wol)
    {
        wol_.emplace(slot_num, WolData { wol.attribute("num").as_int(), wol.attribute("mac").as_string() });
    }

    for (const auto& flow : root.children("flow"))
    {
        load_flow(flow, vars_table, slot_num);
    }
}

void DemoLCP::load_flow(const pugi::xml_node& root, const std::map<std::string, std::string>& vars_table, int slot_num)
{
	auto flow = protocol::Flow();

    auto flags = root.child("flags");

    for (const auto& flag : flags.children())
    {
        load_flag(flag, flow, vars_table);
    }

	auto filters = root.child("filters");

    for (const auto& filter : filters.children())
    {
        load_filter(filter, flow, vars_table);
    }

	auto actions = root.child("actions");

	for (const auto& action : actions.children())
	{
        load_action(action, flow, vars_table);
	}

	flows_.emplace(slot_num, std::move(flow));
}

void DemoLCP::load_flag(const pugi::xml_node& root, protocol::Flow& flow, const std::map<std::string, std::string>& /*vars_table*/)
{
    std::string name = root.name();

    if (name == "priority")
	{
		flow.add(priority(root.attribute("value").as_int()));
	}
	else if (name == "idle_timeout")
	{
		flow.add(idle_timeout(root.attribute("value").as_int()));
	}
    else if (name == "hard_timeout")
	{
		flow.add(hard_timeout(root.attribute("value").as_int()));
	}
    else if (name == "out_port")
	{
		flow.add(out_port(root.attribute("value").as_int()));
	}
    else if (name == "flags")
	{
        // TODO: Use child nodes for flags. 
		flow.add(flags(root.attribute("value").as_int()));
	}
}

void DemoLCP::load_filter(const pugi::xml_node& root, protocol::Flow& flow, const std::map<std::string, std::string>& vars_table)
{
    std::string name = root.name();

	if (name == "in_port")
	{
		flow.add(from_port(root.attribute("port").as_int()));
	}
	else if (name == "dl_vlan")
	{
		flow.add(from_vlan(root.attribute("tag").as_int()));
	}
	else if (name == "dl_src")
	{
		// TODO: Add mask to the filter.
		flow.add(from_hw_src(tnt::MacAddress(read_var(vars_table, root.attribute("mac").as_string()))));
	}
	else if (name == "dl_dst")
	{
		// TODO: Add mask to the filter.
		flow.add(from_hw_dst(tnt::MacAddress(read_var(vars_table, root.attribute("mac").as_string()))));
	}
	else if (name == "dl_type")
	{
		flow.add(from_proto(static_cast<L2Proto>(root.attribute("type").as_int())));
	}
	else if (name == "nw_src")
	{
		flow.add(from_ip_src(tnt::ip::Address::from_string(read_var(vars_table, root.attribute("ip").as_string())), root.attribute("prefix").as_int()));
	}
	else if (name == "nw_dst")
	{
		flow.add(from_ip_dst(tnt::ip::Address::from_string(read_var(vars_table, root.attribute("ip").as_string())), root.attribute("prefix").as_int()));
	}
	else if (name == "nw_proto")
	{
		flow.add(from_proto(static_cast<L3Proto>(root.attribute("proto").as_int())));
	}
	else if (name == "tp_src")
	{
		flow.add(from_transport_port_src(static_cast<L3Proto>(root.attribute("proto").as_int()), root.attribute("port").as_int()));
	}
	else if (name == "tp_dst")
	{
		flow.add(from_transport_port_dst(static_cast<L3Proto>(root.attribute("proto").as_int()), root.attribute("port").as_int()));
	}
	else if (name == "ip")
	{
		flow.add(from_proto(L2Proto::IPv4));
	}
	else if (name == "icmp")
	{
		flow.add(from_proto(L2Proto::Icmp));
	}
	else if (name == "tcp")
	{
		flow.add(from_proto(L3Proto::Tcp));
	}
	else if (name == "udp")
	{
		flow.add(from_proto(L3Proto::Udp));
	}
	else if (name == "arp")
	{
		flow.add(from_proto(L2Proto::Arp));
	}
}

void DemoLCP::load_action(const pugi::xml_node& root, protocol::Flow& flow, const std::map<std::string, std::string>& vars_table)
{
    std::string name = root.name();

    if (name == "controller")
	{
		flow.add(to_controller());
	}
    else if (name == "loop")
	{
		flow.add(loop());
	}
	else if (name == "drop")
	{
		// Add no actions.
	}
	else if (name == "output")
	{
		flow.add(to_port(root.attribute("port").as_int()));
	}
	else if (name == "mod_vlan_vid")
	{
		flow.add(set_vlan(root.attribute("tag").as_int()));
	}
	else if (name == "strip_vlan")
	{
		flow.add(strip_vlan());
	}
	else if (name == "mod_dl_src")
	{
		flow.add(set_hw_src(tnt::MacAddress(read_var(vars_table, root.attribute("mac").as_string()))));
	}
	else if (name == "mod_dl_dst")
	{
		flow.add(set_hw_dst(tnt::MacAddress(read_var(vars_table, root.attribute("mac").as_string()))));
	}
    else if (name == "mod_nw_src")
    {
        flow.add(set_ip_src(tnt::ip::Address::from_string(read_var(vars_table, root.attribute("ip").as_string()))));
    }
    else if (name == "mod_nw_dst")
    {
        flow.add(set_ip_dst(tnt::ip::Address::from_string(read_var(vars_table, root.attribute("ip").as_string()))));
    }
}

void DemoLCP::change_slot(const std::shared_ptr<ce::ServiceElement>& se, int num)
{
    static auto num_slots = tnt::Configuration::get("lcp.sync.slots", 300);
    static auto flow_sleep = 20; // TODO: Read from config. file.
    assert(num >= 1 && num < num_slots);

    if (num <= current_slot_)
    {
        tnt::Log::info(colors::blue, "Demo LCP: no change in this slot ");

        return;
    }

    tnt::Log::info(colors::blue, "Demo LCP: changing slot ", current_slot_, " -> ", num);

    std::shared_ptr<service::Openflow> of;

    auto ie = se && tnt::visit_any_of<service::Openflow>(se->services(), [&] (const auto& service)
    { 
        of = service;
    });

    while (true)
    {
        ++current_slot_;

        if (current_slot_ > num_slots)
        {
            break;
        }

        tnt::Log::info(colors::blue, "Searching for slot #", current_slot_, " in the flows table");

        if (ie)
        {
            assert(of);

            for (const auto& p : tnt::equal_range(flows_, current_slot_))
            {
                const auto& flow = p.second;
                tnt::Log::info(colors::blue, "Adding flow");

                try
                {
                    of->add(flow);
                }
                catch (std::exception& ex)
                {
                    tnt::Log::error("DemoLCP error sending a message to a SE: ", ex.what());
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(flow_sleep));
            }
        }

        for (const auto& p : tnt::equal_range(wol_, current_slot_))
        {
            const auto& data = p.second;
            wol::send_bcast_magic_packets(data.num, std::chrono::milliseconds(500), data.mac);
        }

        if (current_slot_ == num)
        {
            break;
        }
    }
}

bool DemoLCP::add_flows(const std::vector<std::shared_ptr<Service>>& services)
{
    static auto flow_sleep = 20; // TODO: Read from config. file.

    std::shared_ptr<service::Openflow> of;

    if (!tnt::visit_any_of<service::Openflow>(services, [&] (const auto& service) { of = service; }))
    {
        return false;
    }

    assert(of);

    for (int i = 1; i <= current_slot_; ++i)
    {
        tnt::Log::info(colors::blue, "Searching for slot #", i, " in the flows table");

        for (const auto& p : tnt::equal_range(flows_, i))
        {
            const auto& flow = p.second;
            tnt::Log::info(colors::blue, "Adding flow");

            try
            {
                of->add(flow);
            }
            catch (std::exception& ex)
            {
                tnt::Log::error("DemoLCP error sending a message to a SE: ", ex.what());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(flow_sleep));
        }
    }

    return true;
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

    std::shared_ptr<ce::ServiceElement> se;

    register_handler([&] (const event::SlotChange& event)
    {
        change_slot(se, event.num());
    });

    register_handler([&] (const event::ServiceElementActive& event)
    {
        assert(event.se());

        if (!se)
        {
            if (add_flows(event.se()->services()))
            {
                se = event.se();
            }
        }
    });

    register_handler([&] (const event::ServiceElementDisconnected& event)
    {
        assert(event.element());

        if (se && se->name() == event.element()->name())
        {
            se.reset();
        }
    });

    tnt::Application::raise(event::LCPTaskStarted());

    auto activity = tnt::async([] ()
    {
        if (tnt::Configuration::exists("lcp.sync"))
        {
            activity::LCPSync(tnt::Configuration::get("lcp.sync.ip").as<std::string>(), tnt::Configuration::get("lcp.sync.port").as<int>(), tnt::Configuration::get("lcp.sync.slots").as<int>())();
        }
    });

    // Set the configuration for the slot 1.
    change_slot(se, 1);

    while (running)
    {
        wait_event();
    }

    activity.get();
}

} // namespace drop
