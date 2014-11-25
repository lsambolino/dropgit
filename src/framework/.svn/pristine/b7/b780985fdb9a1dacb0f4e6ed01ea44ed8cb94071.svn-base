
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

#include "http_method.hpp"

#include "util/string.hpp"

namespace tnt {
namespace protocol {

std::string method_to_string(HttpMethod method)
{
	switch (method)
	{
	case HttpMethod::Get:
		return "GET";
	case HttpMethod::Head:
		return "HEAD";
	case HttpMethod::Post:
		return "POST";
	case HttpMethod::Put:
		return "PUT";
	case HttpMethod::Delete:
		return "DELETE";
    case HttpMethod::Options:
        return "OPTIONS";
	default:
		throw MethodNotFoundException(tnt::to_string(static_cast<int>(method)));
	}
}

HttpMethod method_from_string(const std::string& method)
{
	if (method == "GET")
		return HttpMethod::Get;
	if (method == "HEAD")
		return HttpMethod::Head;
	if (method == "POST")
		return HttpMethod::Post;
	if (method == "PUT")
		return HttpMethod::Put;
	if (method == "DELETE")
		return HttpMethod::Delete;
    if (method == "OPTIONS")
        return HttpMethod::Options;

	throw MethodNotFoundException(method);
}

} // namespace protocol
} // namespace tnt
