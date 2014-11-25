
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

#include "gal_controller.hpp"

#include <map>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>

#if defined(WITH_BOOST)
#include <boost/algorithm/string.hpp>
#endif

#include "activity/http/http_connection.hpp"

#include "exception/exception.hpp"

#include "router/interface.hpp"

#include "event/network/connection_reset.hpp"
#include "event/http/http_request.hpp"

#include "message/http/http_response.hpp"

#include "protocol/protocol.hpp"
#include "protocol/http/http_response.hpp"

#include "gal_drop/gsi.hpp"

#include "util/string.hpp"
#include "util/pugixml.hpp"

#include "log.hpp"
#include "containers.hpp"
#include "range.hpp"

namespace drop {
namespace activity {
namespace {

const char* content_type = "text/xml";

std::multimap<std::string, std::string> split_query(std::string query) // By value to permit modification
{
	std::multimap<std::string, std::string> map;

	if (query.empty())
	{
		return map;
	}

	query = tnt::to_lower(query);

	auto pairs = tnt::split(query, "&;");

	for (const auto& p : pairs)
	{
		auto parts = tnt::split(p, "=");
		map.emplace(parts[0], parts[1]);
	}

	return map;
}

struct GalMethodNotFoundException: public std::runtime_error
{
	explicit GalMethodNotFoundException(const std::string& msg) : std::runtime_error(msg) {}
};

enum class GalMethod
{
	Discovery,
	Provisioning,
	Monitoring,
	Commit,
	Release,
};

GalMethod gal_method_from_string(const std::string& method)
{
	if (method == "discovery")
		return GalMethod::Discovery;
	if (method == "provisionedstate")
		return GalMethod::Provisioning;
	if (method == "monitoring")
		return GalMethod::Monitoring;
	if (method == "committedstate")
		return GalMethod::Commit;

	throw GalMethodNotFoundException(method);
}

void add_attribute(pugi::xml_node& node, const char* name, const char* value)
{
	node.append_attribute(name).set_value(value);
}

void add_attribute(pugi::xml_node& node, const char* name, const std::string& value)
{
	add_attribute(node, name, value.c_str());
}

void add_power_state(pugi::xml_node& node, const gal::PowerState& ps)
{
	auto ps_node = node.append_child("gal:state");

	ps_node.append_attribute("state_id").set_value(ps.id);
	ps_node.append_attribute("minimum_power_gain").set_value(ps.minimum_power_gain);
	ps_node.append_attribute("maximum_packet_throughput").set_value(ps.maximum_packet_throughput);
	ps_node.append_attribute("maximum_bit_throughput").set_value(ps.maximum_bit_throughput);

	// TODO: Append other values
}

void add_power_states(pugi::xml_node& node, const std::vector<gal::PowerState>& ps)
{
	auto ps_node = node.append_child("gal:powerStates");

	for (const auto& state : ps)
	{
		add_power_state(ps_node, state);
	}
}

void add_physical_resource(pugi::xml_node& node, const gal::PhysicalResource& pr)
{
	auto pr_node = node.append_child("gal:resource");

	pr_node.append_attribute("resource_id").set_value(pr.id.c_str());
	pr_node.append_attribute("description").set_value(pr.description.c_str());

	// TODO: Append other values
}

void add_physical_resources(pugi::xml_node& node, const std::vector<gal::PhysicalResource>& pr)
{
	auto pc_node = node.append_child("gal:physicalChildren");
	pc_node.append_attribute("count").set_value(static_cast<int>(pr.size()));

	for (const auto& child : pr)
	{
		add_physical_resource(pc_node, child);
	}
}

void add_logical_resource(pugi::xml_node& node, const gal::LogicalResource& lr)
{
	auto lr_node = node.append_child("gal:resource");

	lr_node.append_attribute("resource_id").set_value(lr.id.c_str());
	lr_node.append_attribute("description").set_value(lr.description.c_str());
	lr_node.append_attribute("type").set_value(lr.type.c_str());

	// TODO: Append other values
}

void add_logical_resources(pugi::xml_node& node, const std::vector<gal::LogicalResource>& lr)
{
	auto lc_node = node.append_child("gal:logicalChildren");
	lc_node.append_attribute("count").set_value(static_cast<int>(lr.size()));

	for (const auto& child : lr)
	{
		add_logical_resource(lc_node, child);
	}
}

void add_sensor_resource(pugi::xml_node& node, const gal::SensorResource& sr)
{
	auto sr_node = node.append_child("gal:resource");

	sr_node.append_attribute("resource_id").set_value(sr.id.c_str());
	sr_node.append_attribute("description").set_value(sr.description.c_str());
    sr_node.append_attribute("max").set_value(std::to_string(sr.max_value).c_str());
	sr_node.append_attribute("type").set_value(to_string(sr.type).c_str());
    sr_node.append_attribute("scale").set_value(to_string(sr.scale).c_str());
    sr_node.append_attribute("precision").set_value(sr.precision);
    sr_node.append_attribute("rate").set_value(sr.refresh_rate);
}

void add_sensor_resources(pugi::xml_node& node, const std::vector<gal::SensorResource>& sr)
{
	auto sr_node = node.append_child("gal:sensors");
	sr_node.append_attribute("count").set_value(static_cast<int>(sr.size()));

	for (const auto& child : sr)
	{
		add_sensor_resource(sr_node, child);
	}
}

bool extract_parameters(const web::http::uri& uri, std::string& id, std::string& gal_method, int& depth, std::vector<std::string>& selectors)
{
	auto path = web::http::uri::split_path(uri.path());

	if (path.empty() || path[0] != "GAL")
	{
		return false;
	}

	id = path.size() > 1 ? path[1] : "";
	gal_method = path.size() > 2 ? path[2] : "discovery";

	auto query = split_query(uri.query());

	auto it = query.find("id");

	if (it != std::end(query))
	{
		if (!id.empty())
		{
			return false;
		}

		id = it->second;
	}

	if (id.empty())
	{
		return false;
	}

	it = query.find("childdepth");

	depth = it != std::end(query) ? tnt::stoi(it->second) : 0;

	auto range = tnt::equal_range(query, "propertyselector");

	if (tnt::empty_range(range))
	{
		selectors.push_back("all");
	}

	for (const auto& p : range)
	{
		selectors.push_back(p.second);
	}

	return true;
}

} // namespace

GalController::GalController(tnt::activity::HttpConnection* connection, const std::string& /*base_path*/): connection_(connection)
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
        case tnt::protocol::HttpMethod::Put:
            handle_put();
            break;
        case tnt::protocol::HttpMethod::Options:
            handle_options();
            break;
        case tnt::protocol::HttpMethod::Head:
            handle_head();
            break;
        case tnt::protocol::HttpMethod::Delete:
            handle_delete();
        default:
            connection->not_implemented();
            break;
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error("GalController error: ", ex.what());
    }
    catch (...)
    {
        tnt::Log::error("GalController unknown error");
        connection->internal_error();
    }
}

void GalController::handle_get()
{
    const auto& body = connection_->body();

    if (!body.empty())
	{
		connection_->bad_request();

		return;
	}

    const auto& uri = connection_->uri();

	std::string id;
	std::string gal_method;
	int depth;
	std::vector<std::string> selectors;

	if (!extract_parameters(uri, id, gal_method, depth, selectors))
	{
		connection_->bad_request();

		return;
	}

	try
	{
		switch (gal_method_from_string(tnt::to_lower(gal_method)))
		{
		case GalMethod::Discovery:
			discovery(id, depth, selectors);
			break;
		case GalMethod::Monitoring:
            monitoring(id);
			break;
		case GalMethod::Provisioning:
			break;
		case GalMethod::Commit:
		case GalMethod::Release:
		default:
			connection_->bad_request();
			break;
		}
	}
	catch (GalMethodNotFoundException& ex)
	{
		tnt::Log::error(R"(GalRestRequest::handle_get error: GAL method ")", ex.what(), R"(" not found)");
		connection_->bad_request();
	}
}

void GalController::handle_head()
{
	connection_->not_implemented();
}

void GalController::handle_put()
{
	std::string id;
	std::string gal_method;
	int depth;
	std::vector<std::string> selectors;

    const auto& body = connection_->body();

    if (body.empty())
	{
		connection_->bad_request();

		return;
	}

    const auto& uri = connection_->uri();

	if (!extract_parameters(uri, id, gal_method, depth, selectors))
	{
		connection_->bad_request();

		return;
	}

	if (gal_method_from_string(tnt::to_lower(gal_method)) != GalMethod::Provisioning)
	{
		connection_->bad_request();

		return;
	}

	pugi::xml_document doc;
	auto result = doc.load_buffer(body.data(), body.size());

	if (!result)
	{
		connection_->bad_request();

		return;
	}

	auto ps = doc.first_child().attribute("state_id").as_int();
    ElementGSI gsi;

    tnt::Log::info(colors::cyan, "id: ", id, " - ps: ", ps);

	auto res = gsi.provisioning(id, ps);

	switch (res)
	{
	case gal::ReturnCode::GalResourceNotFound:
		connection_->not_found();
		break;
	case gal::ReturnCode::GalSuccess:
		connection_->ok();
		break;
	case gal::ReturnCode::GalUndefined:
	case gal::ReturnCode::GalError:
	case gal::ReturnCode::GalProtocolError:
	case gal::ReturnCode::GalResourceNotAvailable:
	case gal::ReturnCode::GalFailure:
		connection_->internal_error();
		break;
	case gal::ReturnCode::GalNotImplemented:
	default:
		connection_->not_implemented();
		break;
	}
}

void GalController::handle_post()
{
	std::string id;
	std::string gal_method;
	int depth;
	std::vector<std::string> selectors;

    const auto& body = connection_->body();

    if (body.empty())
	{
		connection_->bad_request();

		return;
	}

    const auto& uri = connection_->uri();

	if (!extract_parameters(uri, id, gal_method, depth, selectors))
	{
		connection_->bad_request();

		return;
	}

	auto method = gal_method_from_string(tnt::to_lower(gal_method));

	if (method != GalMethod::Commit)
	{
		connection_->bad_request();

		return;
	}

	pugi::xml_document doc;
	auto result = doc.load_buffer(body.data(), body.size());

	if (!result)
	{
		connection_->bad_request();

		return;
	}

	std::string operation = doc.root().name();

	if (operation != "gal:commit" && operation != "gal:rollback")
	{
		connection_->bad_request();

		return;
	}

    ElementGSI gsi;
	auto res = operation == "gal:commit" ? gsi.commit(id) : gsi.rollback(id);

	switch (res)
	{
	case gal::ReturnCode::GalResourceNotFound:
		connection_->not_found();
		break;
	case gal::ReturnCode::GalSuccess:
		connection_->ok();
		break;
	case gal::ReturnCode::GalUndefined:
	case gal::ReturnCode::GalError:
	case gal::ReturnCode::GalProtocolError:
	case gal::ReturnCode::GalResourceNotAvailable:
	case gal::ReturnCode::GalFailure:
		connection_->internal_error();
		break;
	case gal::ReturnCode::GalNotImplemented:
	default:
		connection_->not_implemented();
		break;
	}
}

void GalController::handle_delete()
{
    const auto& body = connection_->body();

    if (!body.empty())
	{
		connection_->bad_request();

		return;
	}

    const auto& uri = connection_->uri();

	std::string id;
	std::string gal_method;
	int depth;
	std::vector<std::string> selectors;

	if (!extract_parameters(uri, id, gal_method, depth, selectors))
	{
		connection_->bad_request();

		return;
	}

	if (gal_method_from_string(tnt::to_lower(gal_method)) != GalMethod::Release)
	{
		connection_->bad_request();

		return;
	}

    ElementGSI gsi;
	auto res = gsi.release(id);

	switch (res)
	{
	case gal::ReturnCode::GalResourceNotFound:
		connection_->not_found();
		break;
	case gal::ReturnCode::GalSuccess:
		connection_->ok();
		break;
	case gal::ReturnCode::GalUndefined:
	case gal::ReturnCode::GalError:
	case gal::ReturnCode::GalProtocolError:
	case gal::ReturnCode::GalResourceNotAvailable:
	case gal::ReturnCode::GalFailure:
		connection_->internal_error();
		break;
	case gal::ReturnCode::GalNotImplemented:
	default:
		connection_->not_implemented();
		break;
	}
}

void GalController::handle_options()
{
    const auto& body = connection_->body();

    if (!body.empty())
	{
		connection_->bad_request();

		return;
	}

    const auto& uri = connection_->uri();

	std::string id;
	std::string gal_method;
	int depth;
	std::vector<std::string> selectors;

	if (!extract_parameters(uri, id, gal_method, depth, selectors))
	{
		connection_->bad_request();

		return;
	}

	if (gal_method_from_string(tnt::to_lower(gal_method)) != GalMethod::Provisioning)
	{
		connection_->bad_request();

		return;
	}

    connection_->ok({ { "Access-Control-Allow-Origin", connection_->header("Origin") },
                      { "Access-Control-Allow-Methods", "GET, HEAD, PUT, POST, DELETE, OPTIONS" },
                      { "Access-Control-Allow-Headers", connection_->header("Access-Control-Request-Headers") },
                      { "Allow", "GET, HEAD, PUT, POST, DELETE, OPTIONS" } });
}

void GalController::discovery(const std::string& id, int depth, const std::vector<std::string>& selectors)
{
	pugi::xml_document doc;
	auto result = doc.load(R"(<gal:resource xmlns:gal="http://www.econet-project.eu/GAL" />)");

	if (!result)
	{
		connection_->internal_error();

		return;
	}

	auto root = doc.first_child();

	discovery(root, id, depth, selectors);

	std::ostringstream oss;
	doc.save(oss);

	connection_->ok(oss.str(), content_type);
}

void GalController::discovery(pugi::xml_node& root, const std::string& id, int depth, const std::vector<std::string>& selectors)
{
	gal::LogicalResource r;
	std::vector<gal::LogicalResource> lr;
	std::vector<gal::PhysicalResource> pr;
	std::vector<gal::SensorResource> sr;
	std::vector<gal::PowerState> ps;
	std::vector<gal::OptimalConfig> edl;

    ElementGSI gsi;
    
    auto ret = gsi.discovery(id, tnt::contains(selectors, "committedstate"), &r, &lr, &pr, &sr, &ps, &edl);

    if (ret == gal::ReturnCode::GalNotImplemented)
    {
        connection_->not_implemented();

        return;
    }

	if (ret == gal::ReturnCode::GalResourceNotFound)
	{
		connection_->not_found();

		return;
	}

    if (ret != gal::ReturnCode::GalSuccess)
	{
		connection_->internal_error();

		return;
	}

	if (!root.attribute("resource_id"))
	{
		add_attribute(root, "resource_id", id);
		add_attribute(root, "description", r.description);
		add_attribute(root, "type", r.type);
	}

	auto all = tnt::contains(selectors, "all");

	if (all || tnt::contains(selectors, "powerstates"))
	{
		if (!root.child("gal:powerStates"))
		{
			add_power_states(root, ps);
		}
	}

	if (all || tnt::contains(selectors, "physicalchildren"))
	{
		add_physical_resources(root, pr);
	}

	if (all || tnt::contains(selectors, "logicalchildren"))
	{
		add_logical_resources(root, lr);
	}

	if (all || tnt::contains(selectors, "sensors"))
	{
		add_sensor_resources(root, sr);
	}

	if (all || tnt::contains(selectors, "states"))
	{
		int committed_state;

        ElementGSI gsi;
		auto ret = gsi.monitor_state(id, true, committed_state);

        if (ret == gal::ReturnCode::GalSuccess)
        {
            root.append_child("gal:committedState").append_attribute("state_id").set_value(committed_state);
        }

        int provisioned_state;

		ret = gsi.monitor_state(id, false, provisioned_state);

        if (ret == gal::ReturnCode::GalSuccess)
        {
            root.append_child("gal:provisionedState").append_attribute("state_id").set_value(provisioned_state);
        }
	}

	if (depth > 0)
	{
		--depth;

		for (const auto& res : lr)
		{
			auto node = root.child("gal:logicalChildren").find_child_by_attribute("resource_id", res.id.c_str());
			discovery(node, res.id, depth, selectors);
		}
	}
}

void GalController::monitoring(const std::string& id)
{
	static std::unordered_map<std::string, std::vector<gal::SensorResource>> sr_map;
    ElementGSI gsi;

    auto it = sr_map.find(id);

    if (it == std::end(sr_map))
    {
        std::vector<gal::SensorResource> sr;
        auto ret = gsi.discovery(id, true, nullptr, nullptr, nullptr, &sr, nullptr, nullptr);

	    if (ret == gal::ReturnCode::GalResourceNotFound)
	    {
		    connection_->not_found();

		    return;
	    }

        it = sr_map.emplace(id, sr).first;
    }

    pugi::xml_document doc;
	auto result = doc.load(R"(<gal:monitoring xmlns:gal="http://www.econet-project.eu/GAL" />)");

	if (!result)
	{
        tnt::Log::error("GalController error: ", result.description());
		connection_->internal_error();

		return;
	}

    auto root = doc.first_child();

    int state_id;

	auto ret = gsi.monitor_state(id, true, state_id);

	if (ret == gal::ReturnCode::GalSuccess)
	{
		root.append_child("gal:committedState").append_attribute("value").set_value(state_id);
	}
    else if (ret == gal::ReturnCode::GalNotImplemented)
    {
        connection_->not_implemented();

        return;
    }
    else if (ret != gal::ReturnCode::GalResourceNotFound)
    {
        connection_->internal_error();

		return;
    }

    const auto& sr = it->second;

    for (const auto& s : sr)
    {
        try
        {
            gal::EntitySensorStatus oper_status;
            gal::EntitySensorValue sensor_value;
            gal::EntitySensorTimeStamp value_timestamp;

            auto ret = gsi.monitor_sensor(id + "." + s.id, oper_status, sensor_value, value_timestamp);

	        if (ret == gal::ReturnCode::GalSuccess && oper_status == gal::EntitySensorStatus::ok)
	        {
                root.append_child(std::string("gal:").append(s.id).c_str()).append_attribute("value").set_value(std::to_string(sensor_value).c_str());
	        }
        }
        catch (std::exception& ex)
        {
            tnt::Log::error("GalController error: ", ex.what());
        }
        catch (...)
        {
            tnt::Log::info("GalController sensor ", s.id, " reply ko");
        }
    }

	std::ostringstream oss;
	doc.save(oss);

	connection_->ok(oss.str(), "text/xml");
}

} // namespace activity
} // namespace drop
