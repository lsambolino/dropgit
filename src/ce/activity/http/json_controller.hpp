
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

#ifndef DROP_ACTIVITY_JSON_REQUEST_HPP_
#define DROP_ACTIVITY_JSON_REQUEST_HPP_

#include <string>
#include <vector>
#include <map>
#include <cstdint>

#include "activity/http/controller.hpp"

#include "gal/green_standard_interface.hpp"

namespace web {
namespace http {

class uri;

} // namespace http
} // namespace web

namespace tnt {

class JsonArray;
class JsonObject;

namespace activity {

class HttpConnection;

} // namespace tnt
} // namespace activity

namespace drop {

namespace ce {

class ServiceElement;

} // namespace ce

namespace activity {

const char N[] = "json";

class JsonController: public tnt::activity::RegisterController<JsonController, N>
{
public:
    JsonController(tnt::activity::HttpConnection* connection, const std::string& base_path);
private:
	void handle_get();
	void handle_head();
	void handle_put();
	void handle_post();
	void handle_delete();

    void index_start();
    void index_update();

    void gal();

    void graphs_start();
    void graphs_update();

    void ip_fib();

    void openflow();

    void logs();

    int element_percentage_consumption(const std::string& name);
    int element_percentage_traffic(const std::string& name);
    int element_percentage_load(const std::string& name);

    tnt::JsonArray device_traffic_history();
    tnt::JsonArray device_traffic();

    tnt::JsonArray index_elements();
    tnt::JsonArray index_elements_update();

    tnt::JsonArray profile();
    
    tnt::JsonArray gal_device_states();
    tnt::JsonArray gal_interfaces();
    tnt::JsonArray gal_elements();

    tnt::JsonArray graphs_elements_history();
    tnt::JsonArray graphs_elements_update();

    gal::EntitySensorValue max_rate();

    const std::vector<std::shared_ptr<ce::ServiceElement>>& service_elements();
private:
    tnt::activity::HttpConnection* connection_;

    static std::vector<std::shared_ptr<ce::ServiceElement>> elements_;
};

} // namespace activity
} // namespace drop

#endif
