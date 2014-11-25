
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

#ifndef TNT_HTTP_RESPONSE_HPP_
#define TNT_HTTP_RESPONSE_HPP_

#include <string>

#include "protocol/http/http_status_code.hpp"
#include "protocol/http/http_headers.hpp"

namespace tnt {
namespace protocol {

class HttpResponse
{
public:
	HttpResponse(HttpStatusCode code, const HttpHeaders& headers, const std::string& body = "");

	static HttpResponse bad_request(bool close = false);
	static HttpResponse not_found(bool close = false);
	static HttpResponse internal_error(bool close = false);
	static HttpResponse not_implemented(bool close = false);

	const std::string& version() const
	{
		return version_;
	}

	HttpStatusCode code() const
	{
		return code_;
	}

	const HttpHeaders& headers() const
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
	std::string version_;
	HttpStatusCode code_;
	HttpHeaders headers_;
	std::string body_;
};

} // namespace protocol
} // namespace tnt

#endif
