
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

#ifndef TNT_EVENT_HTTP_REQUESTS_HPP_
#define TNT_EVENT_HTTP_REQUESTS_HPP_

#include <sstream>
#include <cassert>
#include <string>

#include "protocol/http/http_method.hpp"
#include "protocol/http/http_headers.hpp"
#include "protocol/http/cpprest/uri.h"

#include "util/string.hpp"

namespace tnt {

struct Protocol;

namespace event {

class HttpRequest
{
public:
	HttpRequest(const web::http::uri& uri, const std::string& version, protocol::HttpMethod method, const protocol::HttpHeaders& headers, const std::string& body): 
		uri_(uri), version_(version), method_(method), headers_(headers), body_(body) {}

	const web::http::uri& uri() const
	{
		return uri_;
	}

	const std::string& version() const
	{
		return version_;
	}

    protocol::HttpMethod method() const
	{
		return method_;
	}

    bool connection_close() const
    {
        return !headers_.contains("Connection") || to_lower(header("Connection")) != "keep-alive";
    }

	const protocol::HttpHeaders& headers() const
	{
		return headers_;
	}

	const std::string& header(const std::string& key) const 
	{
		return headers_[key];
	}

	const std::string& body() const
	{
		return body_;
	}
private:
	web::http::uri uri_;
	std::string version_;
    protocol::HttpMethod method_;
	protocol::HttpHeaders headers_;
	std::string body_;
};

} // namespace event
} // namespace tnt

#endif
