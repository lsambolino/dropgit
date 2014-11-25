
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

#include "http_status_code.hpp"

#include <iostream>

namespace tnt {
namespace protocol {

std::string code_to_string(HttpStatusCode code)
{
	switch (code)
	{
	case HttpStatusCode::OK:
		return "OK";
	case HttpStatusCode::MovedPermanently:
		return "Moved Permanently";
	case HttpStatusCode::BadRequest:
		return "Bad Request";
	case HttpStatusCode::Unauthorized:
		return "Unauthorized";
	case HttpStatusCode::Forbidden:
		return "Forbidden";
	case HttpStatusCode::NotFound:
		return "Not Found";
	case HttpStatusCode::MethodNotAllowed:
		return "Method Not Allowed";
	case HttpStatusCode::InternalServerError:
		return "Internal Server Error";
	case HttpStatusCode::NotImplemented:
		return "Not Implemented";
	case HttpStatusCode::ServiceUnavailable:
		return "Service Unavailable";
	default:
		return "Internal Server Error";
	}
}

std::istream& operator>>(std::istream& is, HttpStatusCode& code)
{
    int c;
    is >> c;

    code = static_cast<HttpStatusCode>(c);

    return is;
}

} // namespace protocol
} // namespace tnt
