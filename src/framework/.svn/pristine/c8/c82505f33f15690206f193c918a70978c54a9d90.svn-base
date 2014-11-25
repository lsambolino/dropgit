
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

#include "http_response.hpp"

#include "protocol/http/http_protocol.hpp"

#include "util/string.hpp"

namespace tnt {
namespace protocol {

HttpResponse::HttpResponse(HttpStatusCode code, const HttpHeaders& headers, const std::string& body): version_("HTTP/1.1"), code_(code), headers_(headers), body_(body) {}

HttpResponse HttpResponse::bad_request(bool close)
{
	auto resp = HttpResponse(HttpStatusCode::BadRequest, { { "Connection", protocol::get_connection(close) }, { "Content-Type", "text/plain" } }, "Bad Request");
	resp.headers_.insert({ "Content-Length", tnt::to_string(resp.body_.size()) });

	return resp;
}

HttpResponse HttpResponse::not_found(bool close)
{
	auto resp = HttpResponse(HttpStatusCode::NotFound, { { "Connection", protocol::get_connection(close) }, { "Content-Type", "text/plain" } }, "Not Found");
	resp.headers_.insert({ "Content-Length", tnt::to_string(resp.body_.size()) });

	return resp;
}

HttpResponse HttpResponse::internal_error(bool close)
{
	auto resp = HttpResponse(HttpStatusCode::InternalServerError, { { "Connection", protocol::get_connection(close) }, { "Content-Type", "text/plain" } }, "Internal Server Error");
	resp.headers_.insert({ "Content-Length", tnt::to_string(resp.body_.size()) });

	return resp;
}

HttpResponse HttpResponse::not_implemented(bool close)
{
	auto resp = HttpResponse(HttpStatusCode::NotImplemented, { { "Connection", protocol::get_connection(close) }, { "Content-Type", "text/plain" } }, "Not Implemented");
	resp.headers_.insert({ "Content-Length", tnt::to_string(resp.body_.size()) });

	return resp;
}

} // namespace protocol
} // namespace tnt