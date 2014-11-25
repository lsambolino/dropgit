
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

#include "gal_rest.hpp"

#include <iostream>

#include "event/http/http_response.hpp"

#include "message/http/http_request.hpp"

#include "network/service_element.hpp"

#include "protocol/http/http_protocol.hpp"

#include "io/io.hpp"

#include "util/io_factory.hpp"
#include "util/pugixml.hpp"
#include "util/exec.hpp"

#include "application.hpp"

namespace drop {
namespace service {
namespace {

const int rest_port = 80;

std::unique_ptr<tnt::protocol::HttpProtocol> protocol(const std::string& address)
{
    auto ep = factory::create_io_end_point("http", address, rest_port);
    auto protocol = std::make_unique<tnt::protocol::HttpProtocol>(ep->get());
    protocol->start();

    return protocol;
}

tnt::protocol::HttpRequest make_request(tnt::protocol::HttpMethod method, const std::string& uri, const std::string& body = "")
{
    auto u = web::http::uri(uri);

    return tnt::protocol::HttpRequest(method, u, tnt::protocol::HttpHeaders{ { "Host", u.host() }, { "Connection", "Close" } }, body);
}
std::string make_uri(const std::string& address, const std::string& path)
{
    return std::string("http://") + address + "/GAL/" + path;
}

std::unique_ptr<tnt::message::HttpRequest> make_message(tnt::protocol::HttpMethod method, const std::string& address, const std::string& path, const std::string& body = "")
{
    return std::make_unique<tnt::message::HttpRequest>(make_request(method, make_uri(address, path), body));
}

gal::ReturnCode parse_discovery(const std::string& body,
                                gal::LogicalResource* r,
                                gal::LogicalResourceContainer* lr,
                                gal::PhysicalResourceContainer* pr,
                                gal::SensorResourceContainer* sr,
                                gal::PowerStateContainer* ps,
                                gal::OptimalConfigContainer* edl)
{
    pugi::xml_document doc;
    auto result = doc.load_buffer(body.data(), body.size());

    if (!result)
    {
        return gal::ReturnCode::GalError;
    }

    auto root = doc.child("gal:resource");

    if (!root)
    {
        return gal::ReturnCode::GalError;
    }

    if (r)
    {
        r->id = root.attribute("resource_id").as_string("");
        r->type = root.attribute("type").as_string("");
        r->description = root.attribute("description").as_string("");
    }

    if (lr)
    {
        // TODO:
    }

    if (pr)
    {
        auto node = root.child("gal:physicalChildren");

        for (auto n : node)
        {
            auto r = gal::PhysicalResource();
            r.id = n.attribute("resource_id").as_string("");
            r.description = n.attribute("description").as_string("");
            r.type = n.attribute("type").as_string("");

            pr->push_back(r);
        }
    }

    if (sr)
    {
        auto node = root.child("gal:sensors");

        for (auto n : node)
        {
            auto s = gal::SensorResource();
            s.id = n.attribute("resource_id").as_string("");
            s.description = n.attribute("description").as_string("");
            s.max_value = std::stol(n.attribute("max").as_string());
            s.type = gal::from_string<gal::EntitySensorDataType>(n.attribute("type").as_string(""));
            s.scale = gal::from_string<gal::EntitySensorDataScale>(n.attribute("scale").as_string(""));
            s.precision = n.attribute("precision").as_int();
            s.refresh_rate = n.attribute("rate").as_int();

            sr->push_back(s);
        }
    }

    if (ps)
    {
        auto node = root.child("gal:powerStates");

        for (auto n : node)
        {
            auto p = gal::PowerState();
            p.id = n.attribute("state_id").as_int();
            p.minimum_power_gain = n.attribute("minimum_power_gain").as_int();
            p.maximum_packet_throughput = n.attribute("maximum_packet_throughput").as_double();
            p.maximum_bit_throughput = n.attribute("maximum_bit_throughput").as_double();

            ps->push_back(p);
        }
    }

    if (edl)
    {
        // TODO:
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode parse_monitor_state(const std::string& body, int& id)
{
    pugi::xml_document doc;
    auto result = doc.load_buffer(body.data(), body.size());

    if (!result)
    {
        return gal::ReturnCode::GalError;
    }

    auto root = doc.child("gal:monitoring");

    if (!root)
    {
        return gal::ReturnCode::GalError;
    }

    auto cs = root.child("gal:committedState");

    if (cs)
    {
        id = cs.attribute("value").as_int();

        return gal::ReturnCode::GalSuccess;
    }

    auto ps = root.child("gal:provisionedState");

    if (ps)
    {
        id = ps.attribute("value").as_int();

        return gal::ReturnCode::GalSuccess;
    }

    return gal::ReturnCode::GalError;
}

gal::ReturnCode parse_monitor_sensor(const std::string& body, const std::string& id, gal::EntitySensorStatus& s, gal::EntitySensorValue& v, gal::EntitySensorTimeStamp& ts)
{
    pugi::xml_document doc;
    auto result = doc.load_buffer(body.data(), body.size());

    if (!result)
    {
        return gal::ReturnCode::GalError;
    }

    auto root = doc.child("gal:monitoring").child(std::string("gal:").append(id).c_str());

    if (!root)
    {
        return gal::ReturnCode::GalError;
    }

    if (root)
    {
        s = gal::EntitySensorStatus::ok;
        v = root.attribute("value").as_int();
        ts = std::chrono::seconds(std::time(nullptr)).count();

        return gal::ReturnCode::GalSuccess;
    }

    return gal::ReturnCode::GalError;
}

} // namespace

GalRest::GalRest(ce::ServiceElement* parent) : parent_(parent) {}

std::ostream& GalRest::print(std::ostream& os) const
{
    os << R"({ "Name": "Gal" })";

    return os;
}

gal::ReturnCode GalRest::discovery(const std::string& resource_id,
                                   bool committed,
                                   gal::LogicalResource* resource,
                                   gal::LogicalResourceContainer* logical_resources,
                                   gal::PhysicalResourceContainer* physical_resources,
                                   gal::SensorResourceContainer* sensor_resources,
                                   gal::PowerStateContainer* power_states,
                                   gal::OptimalConfigContainer* edl)
{
    auto proto = protocol(parent_->address());
    std::promise<gal::ReturnCode> promise;
    auto future = promise.get_future();

    auto token = tnt::Application::subscribe([&] (const tnt::event::HttpResponse& event)
    {
        if (event.code() != tnt::protocol::HttpStatusCode::OK)
        {
            promise.set_value(gal::ReturnCode::GalProtocolError);

            return;
        }

        promise.set_value(parse_discovery(event.body(), resource, logical_resources, physical_resources, sensor_resources, power_states, edl));
    },
    proto.get());

    proto->send(make_message(tnt::protocol::HttpMethod::Get, parent_->address(), resource_id + "?committed=" + (committed ? "true" : "false")));

    auto result = future.get();
    tnt::Application::unsubscribe(token);

    return result;
}

gal::ReturnCode GalRest::provisioning(const std::string& resource_id, int power_state_id)
{
    auto proto = protocol(parent_->address());
    auto body = std::string(R"(<gal:provisionedState state_id=")") + std::to_string(power_state_id) + R"(" />)";
    proto->send(make_message(tnt::protocol::HttpMethod::Put, parent_->address(), resource_id + "/provisionedState", body));
    
    // TODO: Exec a monitor state to verify the changed ps?

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode GalRest::release(const std::string& resource_id)
{
    auto proto = protocol(parent_->address());
    proto->send(make_message(tnt::protocol::HttpMethod::Delete, parent_->address(), resource_id + "/provisionedState"));

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalRest::monitor_state(const std::string& resource_id, bool committed, gal::PowerState::PowerStateId& power_state_id)
{
    auto  proto = protocol(parent_->address());
    std::promise<gal::ReturnCode> promise;
    auto future = promise.get_future();

    auto token = tnt::Application::subscribe([&] (const tnt::event::HttpResponse& event)
    {
        if (event.code() != tnt::protocol::HttpStatusCode::OK)
        {
            promise.set_value(gal::ReturnCode::GalProtocolError);

            return;
        }

        promise.set_value(parse_monitor_state(event.body(), power_state_id));
    },
    proto.get());

    proto->send(make_message(tnt::protocol::HttpMethod::Get, parent_->address(), resource_id + "/monitoring?committed=" + (committed ? "true" : "false")));

    auto result = future.get();
    tnt::Application::unsubscribe(token);

    return result;
}

gal::ReturnCode GalRest::monitor_history(const std::string& /*resource_id*/, gal::PowerStateHistoryContainer* /*history*/)
{
    /*auto proto = protocol(parent_->address());
    proto->send(make_message(tnt::protocol::HttpMethod::Get, parent_->address(), resource_id));*/

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalRest::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    if (resource_id == "cpuLoad" && !parent_->connected())
    {
        oper_status = gal::EntitySensorStatus::ok;
        sensor_value = 0;
        value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;

        return gal::ReturnCode::GalSuccess;
    }

    auto proto = protocol(parent_->address());
    std::promise<gal::ReturnCode> promise;
    auto future = promise.get_future();

    auto token = tnt::Application::subscribe([&] (const tnt::event::HttpResponse& event)
    {
        if (event.code() != tnt::protocol::HttpStatusCode::OK)
        {
            promise.set_value(gal::ReturnCode::GalProtocolError);

            return;
        }

        promise.set_value(parse_monitor_sensor(event.body(), resource_id, oper_status, sensor_value, value_timestamp));
    },
    proto.get());

    proto->send(make_message(tnt::protocol::HttpMethod::Get, parent_->address(), "0/monitoring"));

    auto result = future.get();
    tnt::Application::unsubscribe(token);

    return result;
}

gal::ReturnCode GalRest::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode GalRest::commit(const std::string& resource_id)
{
    auto proto = protocol(parent_->address());
    proto->send(make_message(tnt::protocol::HttpMethod::Post, parent_->address(), resource_id + "/committedState", "<gal:commit />"));

    // TODO: Exec a monitor state to verify the changed ps?

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode GalRest::rollback(const std::string& resource_id)
{
    auto proto = protocol(parent_->address());
    proto->send(make_message(tnt::protocol::HttpMethod::Post, parent_->address(), resource_id + "/committedState", "<gal:rollback />"));
     
    // TODO: Exec a monitor state to verify the changed ps?

    return gal::ReturnCode::GalSuccess;
}

} // namespace service
} // namespace drop
