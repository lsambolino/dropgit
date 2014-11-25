
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

#include "activity/http/controller.hpp"

namespace web {
namespace http {

class uri;

} // namespace http
} // namespace web

namespace tnt {
namespace activity {

class HttpConnection;

} // namespace tnt
} // namespace activity

namespace pugi {

class xml_node;

} // namespace pugi

namespace drop {
namespace activity {

constexpr const char N[] = "GAL";

class GalController: public tnt::activity::RegisterController<GalController, N>
{
public:
    GalController(tnt::activity::HttpConnection* connection, const std::string& base_path);
private:
	void handle_get();
	void handle_head();
	void handle_put();
	void handle_post();
	void handle_delete();
    void handle_options();

    void discovery(const std::string& id, int depth, const std::vector<std::string>& selectors);
	void discovery(pugi::xml_node& root, const std::string& id, int depth, const std::vector<std::string>& selectors);

    void monitoring(const std::string& id);
private:
    tnt::activity::HttpConnection* connection_;
};

} // namespace activity
} // namespace drop

#endif
