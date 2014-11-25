
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

#include "json_controller.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>
#include <random>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "activity/http/http_connection.hpp"

#include "exception/exception.hpp"

#include "gal_drop/gsi.hpp"

#include "event/network/connection_reset.hpp"
#include "event/http/http_request.hpp"
#include "event/gal/history_requests.hpp"
#include "event/service_request.hpp"
#include "event/router_requests.hpp"

#include "message/http/http_response.hpp"

#include "network/service_element.hpp"

#include "protocol/protocol.hpp"
#include "protocol/http/http_response.hpp"
#include "protocol/openflow/flow.hpp"
#include "protocol/openflow/flow_flags.hpp"
#include "protocol/openflow/flow_filters.hpp"
#include "protocol/openflow/flow_actions.hpp"

#include "router/interface.hpp"
#include "router/route.hpp"
#include "router/interface.hpp"

#include "service/forwarding.hpp"
#include "service/interconnection.hpp"
#include "service/openflow.hpp"
#include "service/gal.hpp"

#include "util/factory.hpp"
#include "util/string.hpp"
#include "util/pugixml.hpp"
#include "util/configuration.hpp"

#include "log.hpp"
#include "containers.hpp"
#include "range.hpp"
#include "ip_address.hpp"
#include "dynamic_pointer_visitor.hpp"
#include "json.hpp"

namespace drop {
namespace activity {
namespace {

const char* content_type = "text/json";

std::once_flag init_instance_flag_;

gal::ReturnCode read_max_value(gal::EntitySensorValue& max, std::unordered_map<std::string, gal::EntitySensorValue>& max_values, const std::string& name, const std::string& sensor)
{
    ElementGSI gsi;
    auto it = max_values.find(name);

    if (it == std::end(max_values))
    {
        gal::SensorResourceContainer sr;
        auto res = gsi.discovery(std::string("0.") + name, false, nullptr, nullptr, nullptr, &sr, nullptr, nullptr);

        if (res != gal::ReturnCode::GalSuccess)
        {
            return res;
        }

        auto e = tnt::find_if(sr, [&](const auto& s)
        {
            return s.id == sensor;
        });

        if (e == std::end(sr))
        {
            return gal::ReturnCode::GalResourceNotFound;
        }

        max = e->max_value * std::pow(10, static_cast<int>(e->scale));
        max_values[name] = max;
    }
    else
    {
        max = it->second;
    }

    return gal::ReturnCode::GalSuccess;
}

tnt::JsonArray read_sensor_history(const std::string& sensor)
{
    ElementGSI gsi;
    gal::SensorHistoryContainer values;
    tnt::JsonArray array;

    auto res = gsi.monitor_sensor_history(sensor, &values);

    if (res == gal::ReturnCode::GalSuccess)
    {
        for (const auto& d : values)
        {
            array.add(tnt::JsonArray().add(d.ts).add(d.value));
        }
    }

    return array;
}

tnt::JsonArray read_element_sensor_history(const std::string& name, const std::string& sensor)
{
    return read_sensor_history(std::string("0.") + name + "." + sensor);
}

tnt::JsonArray read_device_sensor_history(const std::string& sensor)
{
    return read_sensor_history(std::string("0.") + sensor);
}

tnt::JsonArray read_sensor(const std::string& sensor)
{
    ElementGSI gsi;
    gal::EntitySensorStatus status;
    gal::EntitySensorValue value;
    gal::EntitySensorTimeStamp ts;
    tnt::JsonArray array;

    auto res = gsi.monitor_sensor(sensor, status, value, ts);

    if (res == gal::ReturnCode::GalSuccess && status == gal::EntitySensorStatus::ok)
    {
        array.add(ts).add(value);
    }

    return array;
}

tnt::JsonArray read_element_sensor(const std::string& name, const std::string& sensor)
{
    return read_sensor(std::string("0.") + name + "." + sensor);
}

tnt::JsonArray read_device_sensor(const std::string& sensor)
{
    return read_sensor(std::string("0.") + sensor);
}

gal::ReturnCode percentage_sensor(const std::string& name, int& value, gal::EntitySensorValue max)
{
    ElementGSI gsi;
    gal::EntitySensorStatus status;
    gal::EntitySensorValue val;
    gal::EntitySensorTimeStamp ts;

    auto res = gsi.monitor_sensor(name, status, val, ts);

    if (res != gal::ReturnCode::GalSuccess)
    {
        return res;
    }

    if (status != gal::EntitySensorStatus::ok)
    {
        return gal::ReturnCode::GalError;
    }

    value = static_cast<int>(static_cast<double>(val) / static_cast<double>(max) * 100.0);

    return gal::ReturnCode::GalSuccess;
}

int percentage_sensor(std::unordered_map<std::string, gal::EntitySensorValue>& max_values, const std::string& name, const std::string& sensor)
{
    gal::EntitySensorValue max;

    if (read_max_value(max, max_values, name, sensor) != gal::ReturnCode::GalSuccess)
    {
        return -1;
    }

    int percentage;

    if (percentage_sensor(std::string("0.") + name + "." + sensor, percentage, max) != gal::ReturnCode::GalSuccess)
    {
        return -1;
    }

    return percentage;
}

bool check_return_code(gal::ReturnCode code, tnt::activity::HttpConnection* connection)
{
    if (code != gal::ReturnCode::GalSuccess)
    {
        if (code == gal::ReturnCode::GalResourceNotFound)
        {
            connection->not_found();
        }
        else
        {
            connection->internal_error();
        }

        return false;
    }

    return true;
}

std::string ip_string(const tnt::ip::Address& ip, int prefix)
{
    std::ostringstream ss;
    ss << ip.to_string();

    if (prefix < 32)
    {
        ss << "/" << prefix;
    }

    return ss.str();
}

tnt::JsonObject get_filter(protocol::FlowFilter* filter)
{
    tnt::JsonObject obj;

    if (tnt::visit<protocol::FromPort>(filter, [&] (auto f) { obj.add("IN_PORT", f->port()); }))
    {
        return obj;
    }

    if (tnt::visit<protocol::FromVlan>(filter, [&] (auto f) { obj.add("VLAN", f->tag()); }))
    {
        return obj;
    }

    if (tnt::visit<protocol::FromL2Proto>(filter, [&] (auto f) { obj.add("L2_TYPE", static_cast<int>(f->proto())); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromL3Proto>(filter, [&] (auto f) { obj.add("IP_PROTO", static_cast<int>(f->proto())); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromHwSrc>(filter, [&] (auto f) { obj.add("L2_SRC", f->mac().to_string()); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromHwDst>(filter, [&] (auto f) { obj.add("L2_DST", f->mac().to_string()); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromIpSrc>(filter, [&] (auto f) { obj.add("IP_SRC", ip_string(f->ip(), f->prefix())); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromIpDst>(filter, [&] (auto f) { obj.add("IP_DST", ip_string(f->ip(), f->prefix())); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromTransportPortSrc>(filter, [&] (auto f) { obj.add("TP_SRC", /*f->proto(),*/ f->port()); }))
    {
        return obj;
    }
       
    if (tnt::visit<protocol::FromTransportPortDst>(filter, [&] (auto f) { obj.add("TP_DST", /*f->proto(),*/ f->port()); }))
    {
        return obj;
    }

    return obj;
}

tnt::JsonArray flow_filters_to_json(const protocol::Flow& flow)
{
    tnt::JsonArray filters;

    for (const auto& f : flow.filters())
    {
        filters.add(get_filter(f.get()));
    }

    return filters;
}

void get_action(protocol::FlowAction* action, tnt::JsonArray& actions)
{
    if (tnt::visit<protocol::ToController>(action, [&] (auto /*a*/)
    {
        actions.add(tnt::JsonObject().add("OUTPUT", "CONTROLLER"));
    }))
    {
        return;
    }

    if (tnt::visit<protocol::ToPorts>(action, [&] (auto a)
    {
        for (auto a : a->ports())
        {
            actions.add(tnt::JsonObject().add("OUTPUT", a));
        }
    }))
    {
        return;
    }

    if (tnt::visit<protocol::Loop>(action, [&] (auto /*a*/)
    {
        actions.add(tnt::JsonObject().add("OUTPUT", "IN_PORT"));
    }))
    {
        return;
    }

    if (tnt::visit<protocol::Flood>(action, [&] (auto /*a*/)
    {
        actions.add(tnt::JsonObject().add("OUTPUT", "FLOOD"));
    }))
    {
        return;
    }

    if (tnt::visit<protocol::SetHwSrc>(action, [&] (auto a)
    {
        actions.add(tnt::JsonObject().add("MOD_HW_SRC", a->mac().to_string()));
    }))
    {
        return;
    }

    tnt::visit<protocol::SetHwDst>(action, [&] (auto a)
    {
        actions.add(tnt::JsonObject().add("MOD_HW_DST", a->mac().to_string()));
    });

    if (tnt::visit<protocol::SetVlan>(action, [&] (auto a)
    {
        actions.add(tnt::JsonObject().add("MOD_VLAN", a->tag()));
    }))
    {
        return;
    }

    if (tnt::is<protocol::StripVlan>(action))
    {
        actions.add(tnt::JsonObject().add("STRIP_VLAN", ""));

        return;
    }
}

tnt::JsonArray flow_actions_to_json(const protocol::Flow& flow)
{
    tnt::JsonArray actions;

    for (const auto& f : flow.actions())
    {
        get_action(f.get(), actions);
    }

    return actions;
}

tnt::JsonObject flow_to_json(const protocol::Flow& flow)
{
    tnt::JsonObject obj;
    obj.add("Filters", flow_filters_to_json(flow)).add("Actions", flow_actions_to_json(flow));

    return obj;
}

} // namespace

std::vector<std::shared_ptr<ce::ServiceElement>> JsonController::elements_;

JsonController::JsonController(tnt::activity::HttpConnection* connection, const std::string& /*base_path*/) : connection_(connection)
{
    assert(connection);

    try
    {
        switch (connection->method())
        {
        case tnt::protocol::HttpMethod::Get:
            handle_get();
            break;
        case tnt::protocol::HttpMethod::Post:
            handle_post();
            break;
        case tnt::protocol::HttpMethod::Head:
        case tnt::protocol::HttpMethod::Delete:
        case tnt::protocol::HttpMethod::Put:
        case tnt::protocol::HttpMethod::Options:
        default:
            connection->not_implemented();
            break;
        }
    }
    catch (...)
    {
        connection->internal_error();
    }
}

void JsonController::handle_get()
{
    auto path = web::http::uri::split_path(connection_->uri().path());

    if (path.empty() || path.size() == 1)
    {
        connection_->bad_request();

        return;
    }

    auto p = tnt::to_lower(path[1]);

    if (p == "index_start")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        index_start();
    }
    else if (p == "index_update")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        index_update();
    }
    else if (p == "gal")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        gal();
    }
    else if (p == "graphs_start")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        graphs_start();
    }
    else if (p == "graphs_update")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        graphs_update();
    }
    else if (p == "ip_fib")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        ip_fib();
    }
    else if (p == "openflow")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        openflow();
    }
    else if (p == "logs")
    {
        if (path.size() != 2)
        {
            return connection_->bad_request();
        }

        logs();
    }
    else
    {
        connection_->not_implemented();
    }
}

void JsonController::handle_head()
{
	connection_->not_implemented();
}

void JsonController::handle_put()
{
	connection_->not_implemented();
}

void JsonController::handle_post()
{
	connection_->not_implemented();
}

void JsonController::handle_delete()
{
	connection_->not_implemented();
}

void JsonController::index_start()
{
    tnt::JsonObject obj;

    obj.add("Elements", index_elements());
    obj.add("Profile", profile());
    obj.add("Traffic", tnt::JsonObject().add("Max", max_rate()).add("Data", device_traffic_history()));

    connection_->ok(obj.str(), content_type);
}

void JsonController::index_update()
{
    tnt::JsonObject obj;

    obj.add("Elements", index_elements_update());
    obj.add("Traffic", device_traffic());

    connection_->ok(obj.str(), content_type);
}

void JsonController::gal()
{
    tnt::JsonObject obj;
    obj.add("Id", "0").add("Name", "Drop").add("States", gal_device_states());
    obj.add("Interfaces", gal_interfaces()).add("Elements", gal_elements());

    connection_->ok(tnt::JsonObject().add("Root", obj).str(), content_type);
}

void JsonController::graphs_start()
{
    connection_->ok(tnt::JsonObject().add("Elements", graphs_elements_history()).str(), content_type);
}

void JsonController::graphs_update()
{
    connection_->ok(tnt::JsonObject().add("Elements", graphs_elements_update()).str(), content_type);
}

void JsonController::ip_fib()
{
    tnt::JsonArray routes;

    auto e = std::make_shared<event::RoutesRequest>([&] (const auto& r)
    {
        tnt::JsonObject obj;
        obj.add("Network", r->destination().to_string()).add("Prefix", r->prefix());
        obj.add("Gateway", r->gateway().to_string()).add("Interface", r->interface()).add("Metric", r->metric());

        std::string flags = "U";

        if (!r->gateway().is_any())
        {
            flags.append("G");
        }

        if (r->prefix() == 32)
        {
            flags.append("H");
        }

        obj.add("Flags", flags);

        routes.add(obj);
    });

    tnt::Application::raise(e);

    e->wait();

    connection_->ok(tnt::JsonObject().add("Routes", routes).str(), content_type);
}

void JsonController::openflow()
{
    tnt::JsonArray flows;

    for (const auto& se : service_elements())
    {
        tnt::visit_any_of<service::Openflow>(se->services(), [&] (const auto& s)
        {
            for (const auto& flow : s->flows())
            {
                flows.add(flow_to_json(flow));
            }
        });
    }

    connection_->ok(tnt::JsonObject().add("Flows", flows).str(), content_type);
}
    
void JsonController::logs()
{
    connection_->not_implemented();
}

tnt::JsonArray JsonController::device_traffic()
{
    return read_device_sensor("traffic");
}

tnt::JsonArray JsonController::index_elements()
{
    tnt::JsonArray elements;

    for (const auto& se : service_elements())
    {
        const auto& name = se->display_name();
        tnt::JsonObject obj;
        obj.add("Name", name).add("Connected", se->connected());

        auto l = element_percentage_load(name);

        if (l >= 0)
        {
            obj.add("Load", tnt::JsonArray().add("data").add(l));
        }
        
        auto c = element_percentage_consumption(name);

        if (c >= 0)
        {
            obj.add("Consumption", tnt::JsonArray().add("data").add(c));
        }

        auto t = element_percentage_traffic(name);

        if (t >= 0)
        {
            obj.add("Traffic", tnt::JsonArray().add("data").add(t));
        }

        if (tnt::any_of_is<service::Forwarding>(se->services()))
        {
            obj.add("Type", "FE");
        }

        if (tnt::any_of_is<service::Interconnection>(se->services()))
        {
            obj.add("Type", "IE");

            if (tnt::any_of_is<service::Openflow>(se->services()))
            {
                obj.add("Layout", "vertical");
            }
            else
            {
                obj.add("Layout", "horizontal");
            }
        }

        elements.add(obj);
    }

    return elements;
}

tnt::JsonArray JsonController::index_elements_update()
{
    tnt::JsonArray elements;

    for (const auto& se : service_elements())
    {
        const auto& name = se->display_name();
        tnt::JsonObject obj;
        obj.add("Name", name).add("Connected", se->connected());

        auto l = element_percentage_load(name);

        if (l >= 0)
        {
            obj.add("Load", tnt::JsonArray().add("data").add(l));
        }

        auto c = element_percentage_consumption(name);

        if (c >= 0)
        {
            obj.add("Consumption", tnt::JsonArray().add("data").add(c));
        }

        auto t = element_percentage_traffic(name);

        if (t >= 0)
        {
            obj.add("Traffic", tnt::JsonArray().add("data").add(t));
        }

        elements.add(obj);
    }

    return elements;
}

tnt::JsonArray JsonController::gal_device_states()
{
    ElementGSI gsi;
    gal::PowerStateContainer ps;

    auto result = gsi.discovery("0", false, nullptr, nullptr, nullptr, nullptr, &ps, nullptr);
    tnt::JsonArray states;

    if (result == gal::ReturnCode::GalSuccess)
    {
        for (const auto& p : ps)
        {
            states.add(tnt::JsonObject().add("Id", p.id));
        }
    }

    return states;
}

tnt::JsonArray JsonController::gal_interfaces()
{
    tnt::JsonArray interfaces;
    ElementGSI gsi;

    auto e = std::make_shared<event::InterfacesRequest>([&] (const auto& i)
    {
        gal::PowerStateContainer ps;

        auto result = gsi.discovery(std::string("0.") + i->name(), false, nullptr, nullptr, nullptr, nullptr, &ps, nullptr);
        tnt::JsonArray states;

        if (result == gal::ReturnCode::GalSuccess)
        {
            for (const auto& p : ps)
            {
                states.add(tnt::JsonObject().add("Id", p.id));
            }
        }

        tnt::JsonObject obj;
        obj.add("Id", std::string("0.") + i->name()).add("Name", i->name()).add("Index", i->index());
        obj.add("States", states);

        interfaces.add(obj);
    });

    tnt::Application::raise(e);

    e->wait();

    return interfaces;
}

int JsonController::element_percentage_consumption(const std::string& name)
{
    static std::unordered_map<std::string, gal::EntitySensorValue> max_values;

    return percentage_sensor(max_values, name, "momentaryConsumption");
}

int JsonController::element_percentage_load(const std::string& name)
{
    ElementGSI gsi;

    gal::EntitySensorStatus status;
    gal::EntitySensorValue val;
    gal::EntitySensorTimeStamp ts;

    auto res = gsi.monitor_sensor(std::string("0.") + name + ".cpuLoad", status, val, ts);

    if (res != gal::ReturnCode::GalSuccess)
    {
        return -1;
    }

    if (status != gal::EntitySensorStatus::ok)
    {
        return -1;
    }

    return val;
}

int JsonController::element_percentage_traffic(const std::string& name)
{
    static std::unordered_map<std::string, gal::EntitySensorValue> max_values;

    return percentage_sensor(max_values, name, "traffic");
}

tnt::JsonArray JsonController::device_traffic_history()
{
    return read_device_sensor_history("traffic");
}

tnt::JsonArray JsonController::gal_elements()
{
    tnt::JsonArray elements;

    for (const auto& se : service_elements())
    {
        tnt::JsonArray cores;
        tnt::JsonArray cpus;
        tnt::JsonArray states;

        tnt::visit_any_of<service::Gal>(se->services(), [&] (const auto& s)
        {
            gal::PhysicalResourceContainer pr;
            gal::PowerStateContainer ps;

            auto result = s->discovery("0", false, nullptr, nullptr, &pr, nullptr, &ps, nullptr);

            if (result == gal::ReturnCode::GalSuccess)
            {
                for (const auto& r : pr)
                {
                    if (r.type == "CPU")
                    {
                        cpus.add(tnt::JsonObject().add("Id", r.id));
                    }
                    else if (r.type == "CPU Core")
                    {
                        cores.add(tnt::JsonObject().add("Id", r.id));
                    }
                }

                for (const auto& p : ps)
                {
                    states.add(tnt::JsonObject().add("Id", p.id));
                }
            }
        });

        tnt::JsonObject obj;
        obj.add("Id", std::string("0.") + se->display_name()).add("Name", se->display_name());

        if (!cores.empty())
        {
            obj.add("Cores", cores);
        }

        if (!cpus.empty())
        {
            obj.add("Cpus", cpus);
        }

        obj.add("States", states);

        elements.add(obj);
    }

    return elements;
}

tnt::JsonArray JsonController::graphs_elements_history()
{
    tnt::JsonArray elements;

    for (const auto& se : service_elements())
    {
        const auto& name = se->display_name();

        auto traffic = read_element_sensor_history(name, "traffic");
        auto consumption = read_element_sensor_history(name, "momentaryConsumption");
        auto cores = read_element_sensor_history(name, "activeCores");
        auto voltages = read_element_sensor_history(name, "voltage");

        std::map<int, std::map<gal::EntitySensorTimeStamp, int>> load;

        if (!voltages.empty())
        {
            for (auto i : { 1500, 1000, 750, 600, 500, 429 })
            {
                load[i] = std::map<gal::EntitySensorTimeStamp, int>();
            }

            for (int i = 0; i < 8; ++i)
            {
                ElementGSI gsi;
                gal::SensorHistoryContainer values;
                tnt::JsonArray array;

                auto res = gsi.monitor_sensor_history(std::string("0.") + name + "." + std::string("freq") + std::to_string(i), &values);

                if (res == gal::ReturnCode::GalSuccess)
                {
                    for (auto i : { 1500, 1000, 750, 600, 500, 429 })
                    {
                        for (const auto& d : values)
                        {
                            auto& v = load.at(i)[d.ts];

                            if (d.value == i)
                            {
                                ++v;
                            }
                        }
                    }
                }
                else
                {
                    tnt::Log::error("Error reading sensor history ", std::string("0.") + name + "." + std::string("freq") + std::to_string(i));
                }
            }
        }

        tnt::JsonObject obj;
        obj.add("Name", name);

        if (!traffic.empty())
        {
            obj.add("Traffic", traffic);
        }

        if (!consumption.empty())
        {
            obj.add("Consumption", consumption);
        }

        if (!cores.empty())
        {
            obj.add("Cores", cores);
        }

        if (!voltages.empty())
        {
            tnt::JsonObject o;
            o.add("Voltage", voltages);

            if (!load.empty())
            {
                tnt::JsonArray freqs;

                for (const auto& p : load)
                {
                    tnt::JsonObject f;
                    f.add("Name", std::to_string(p.first));

                    tnt::JsonArray a;

                    for (const auto& d : p.second)
                    {
                        a.add(tnt::JsonArray().add(d.first).add(d.second));
                    }
                        
                    f.add("Data", a);
                    freqs.add(f);
                }

                o.add("Freqs", freqs);
            }
                
            obj.add("Load", o);
        }

        elements.add(obj);
    }

    return elements;
}

tnt::JsonArray JsonController::graphs_elements_update()
{
    tnt::JsonArray elements;

    for (const auto& se : service_elements())
    {
        const auto& name = se->display_name();

        auto traffic = read_element_sensor(name, "traffic");
        auto consumption = read_element_sensor(name, "momentaryConsumption");
        auto cores = read_element_sensor(name, "activeCores");
        auto voltages = read_element_sensor(name, "voltage");

        gal::EntitySensorTimeStamp timestamp = 0;
        std::map<int, int> load;

        if (!voltages.empty())
        {
            for (auto i : { 1500, 1000, 750, 600, 500, 429 })
            {
                load[i] = 0;
            }

            for (int i = 0; i < 8; ++i)
            {
                ElementGSI gsi;
                gal::EntitySensorStatus status;
                gal::EntitySensorValue value;
                gal::EntitySensorTimeStamp ts;

                tnt::JsonArray array;

                auto res = gsi.monitor_sensor(std::string("0.") + name + "." + std::string("freq") + std::to_string(i), status, value, ts);

                if (res == gal::ReturnCode::GalSuccess)
                {
                    if (timestamp == 0)
                    {
                        timestamp = ts;
                    }

                    load.at(value)++;
                }
                else
                {
                    tnt::Log::error("Error reading sensor ", std::string("0.") + name + "." + std::string("freq") + std::to_string(i));
                }
            }
        }

        tnt::JsonObject obj;
        obj.add("Name", name);

        if (!traffic.empty())
        {
            obj.add("Traffic", traffic);
        }

        if (!consumption.empty())
        {
            obj.add("Consumption", consumption);
        }

        if (!cores.empty())
        {
            obj.add("Cores", cores);
        }

        if (!voltages.empty())
        {
            tnt::JsonObject o;
            o.add("Voltage", voltages);

            if (!load.empty())
            {
                tnt::JsonArray freqs;

                for (const auto& p : load)
                {
                    freqs.add(tnt::JsonObject().add("Name", std::to_string(p.first)).add("Data", tnt::JsonArray().add(timestamp).add(p.second)));
                }

                o.add("Freqs", freqs);
            }
                
            obj.add("Load", o);
        }

        elements.add(obj);
    }

    return elements;
}

tnt::JsonArray JsonController::profile()
{
    tnt::JsonArray data;

    ElementGSI gsi;
    static gal::PowerStateContainer ps;

    if (ps.empty())
    {
        auto res = gsi.discovery("0", true, nullptr, nullptr, nullptr, nullptr, &ps, nullptr);

        if (!check_return_code(res, connection_))
        {
            return data;
        }
    }

    gal::PowerStateId id;
    auto res = gsi.monitor_state("0", true, id);

    if (!check_return_code(res, connection_))
    {
        return data;
    }

    auto e = tnt::find_if(ps, [&] (const auto& p)
    {
        return p.id == id;
    });

    if (e == std::end(ps))
    {
        connection_->not_found();

        return data;
    }

    for (auto p : e->autonomic_ps_curves)
    {
        data.add(tnt::JsonArray().add(p.offered_load).add(p.maximum_consumption));
    }

    return data;
}

gal::EntitySensorValue JsonController::max_rate()
{
    static gal::EntitySensorValue max = -1;
    ElementGSI gsi;

    if (max == -1)
    {
        gal::SensorResourceContainer sr;
        auto res = gsi.discovery("0", false, nullptr, nullptr, nullptr, &sr, nullptr, nullptr);

        if (res != gal::ReturnCode::GalSuccess)
        {
            return -1;
        }

        auto e = tnt::find_if(sr, [&] (const auto& s)
        {
            return s.id == "traffic";
        });

        if (e == std::end(sr))
        {
            return -1;
        }

        max = e->max_value;
    }

    return max;
}

const std::vector<std::shared_ptr<ce::ServiceElement>>& JsonController::service_elements()
{
    std::call_once(init_instance_flag_, [&] ()
    {
        auto e = std::make_shared<event::ServiceElementsRequest>([&] (const auto& se)
        {
            elements_.push_back(se);
        });

        tnt::Application::raise(e);

        e->wait();
    });

    return elements_;
}

} // namespace activity
} // namespace drop
