
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

#ifndef TNT_HTTP_HEADERS_HPP_
#define TNT_HTTP_HEADERS_HPP_

#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

namespace tnt {
namespace protocol {

struct HttpHeaderNotFound: public std::runtime_error
{
    explicit HttpHeaderNotFound(const std::string& msg) : std::runtime_error(msg) {}
};

class HttpHeaders
{
	using HeaderMap = std::vector<std::pair<std::string, std::string>>;
	using const_iterator = HeaderMap::const_iterator;
	using key_type = std::string;
	using mapped_type = std::string;
	using value_type = HeaderMap::value_type;
public:
	HttpHeaders();
	HttpHeaders(std::initializer_list<value_type> init);

    HttpHeaders& operator+=(const HttpHeaders& other);

	void insert(const value_type& value);
	const mapped_type& operator[](const key_type& key) const;

	const_iterator begin() const;
	const_iterator end() const;

    bool contains(const key_type& key) const;

	std::string to_string() const;
private:
	HeaderMap headers_;
};

HttpHeaders operator+(const HttpHeaders& h0, const HttpHeaders& h1);

} // namespace protocol
} // namespace tnt

#endif
