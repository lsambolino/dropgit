
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

#include "http_headers.hpp"

#include <sstream>

#include "containers.hpp"
#include "log.hpp"

namespace tnt {
namespace protocol {

const char* end_line = "\r\n";

HttpHeaders::HttpHeaders() {}

HttpHeaders::HttpHeaders(std::initializer_list<value_type> init) : headers_{ init } {}

HttpHeaders& HttpHeaders::operator+=(const HttpHeaders& other)
{
    for (const auto& p : other)
    {
        insert(p);
    }

    return *this;
}

void HttpHeaders::insert(const value_type& value)
{
	headers_.push_back(value);
}

const HttpHeaders::mapped_type& HttpHeaders::operator[](const key_type& key) const
{
    auto it = tnt::find_if(headers_, [&] (const auto& p)
    {
        return p.first == key;
    });

    if (it == std::end(headers_))
    {
        throw HttpHeaderNotFound(std::string("Header ") + key + "not found");
    }
    
    return it->second;
}

HttpHeaders::const_iterator HttpHeaders::begin() const
{
	return std::begin(headers_);
}

HttpHeaders::const_iterator HttpHeaders::end() const
{
	return std::end(headers_);
}

bool HttpHeaders::contains(const key_type& key) const
{
    return tnt::contains_if(headers_, [&] (const auto& p)
    {
        return p.first == key;
    });
}

std::string HttpHeaders::to_string() const
{
	if (headers_.empty())
	{
		return "";
	}

	std::ostringstream oss;

	for (const auto& p : headers_)
	{
		oss << p.first << ": " << p.second << end_line;
	}

	return oss.str();
}

HttpHeaders operator+(const HttpHeaders& h0, const HttpHeaders& h1)
{
    HttpHeaders tmp = h0;
    tmp += h1;

    return tmp;
}

} // namespace protocol
} // namespace tnt
