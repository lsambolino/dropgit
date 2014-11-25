
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

#include "http_protocol.hpp"

#include <sstream>
#include <cassert>
#include <string>

#include "event/http/http_request.hpp"
#include "event/http/http_response.hpp"
#include "event/event_from_message.hpp"

#include "message/http/http_request.hpp"
#include "message/http/http_response.hpp"

#include "util/string.hpp"

#include "http_response.hpp"
#include "http_request.hpp"
#include "http_method.hpp"
#include "log.hpp"
#include "application.hpp"

namespace tnt {
namespace protocol {
namespace {

protocol::HttpHeaders get_headers(std::istringstream& stm)
{
    protocol::HttpHeaders hdr;
    std::string line;

    while (std::getline(stm, line, '\r'))
	{
		if (line == "\n")
		{
			continue;
		}

		auto pos = line.find(": ");
		assert(pos != std::string::npos);

        auto s_pos = 0;
        auto e_pos = pos;

        if (line[0] == '\n')
        {
            s_pos = 1;
            --e_pos;
        }

		hdr.insert(std::make_pair(line.substr(s_pos, e_pos), line.substr(pos + 2)));
	}

    return hdr;
}

const std::string end_line = "\r\n";
const std::string end_headers = "\r\n\r\n";

} // namespace

std::string get_connection(bool close)
{
    return close ? "Close" : "Keep-Alive";
}

HttpProtocol::HttpProtocol(const std::shared_ptr<IO>& io) : AsyncProtocol(io) {}

void HttpProtocol::invoke_message(const std::string& message)
{
	try
	{
        auto pos = message.find(end_headers);
		auto headers = message.substr(0, pos);
		auto body = message.substr(pos + end_headers.size());

		std::istringstream stm(headers);
		std::string line;
		std::getline(stm, line);

        auto is_request = line.find("HTTP/") != 0;

        if (is_request)
        {
            std::istringstream iss(line);
		    std::string method;
		    std::string uri;
		    std::string version;

		    iss >> method >> uri >> version;

            Application::raise(std::make_shared<event::HttpRequest>(web::http::uri(uri), version, method_from_string(method), get_headers(stm), body), this);
        }
        else // Response
        {
            std::istringstream iss(line);
		    std::string version;
            HttpStatusCode code;
            std::string code_msg;

		    iss >> version >> code >> code_msg;

            Application::raise(event::HttpResponse(version, code, code_msg, get_headers(stm), body));
        }
	}
	catch (...)
	{
		send(HttpResponse::internal_error());
	}
}

std::vector<std::string> HttpProtocol::parse(std::string& raw_input)
{
    auto pos = raw_input.find(end_headers);

    if (pos == std::string::npos)
    {
        return {};
    }

	auto headers = raw_input.substr(0, pos);
    auto body = raw_input.substr(pos + end_headers.size());

    auto s_pos = headers.find("Content-Length: ");

    if (s_pos == std::string::npos)
    {
        auto message = raw_input;
	    raw_input.clear();

	    return { message };
    }

    s_pos += 15;
    auto e_pos = headers.find(end_line, s_pos);
    decltype(body.size()) len = std::stoi(headers.substr(s_pos, e_pos - s_pos));

    if (body.size() == len)
    {
        auto message = raw_input;
	    raw_input.clear();

	    return { message };
    }

    return {};
}

void HttpProtocol::register_messages()
{
	register_message<message::HttpResponse>([this] (message::HttpResponse* message)
    {
        send(message->response());
    });

    register_message<message::HttpRequest>([this] (message::HttpRequest* message)
    {
        send(message->request());
    });
}

void HttpProtocol::send(const HttpResponse& response)
{
	std::ostringstream oss;

	oss << response.version() << " " << static_cast<int>(response.code()) << " " << code_to_string(response.code()) << end_line;
	oss << response.headers().to_string() << end_line;

	if (!response.body().empty())
	{
		oss << response.body();
	}

	write(oss.str());
}

void HttpProtocol::send(const HttpRequest& request)
{
	std::ostringstream oss;

	oss << method_to_string(request.method()) << " " << request.uri().to_string() << " " << request.version() << end_line;
	oss << request.headers().to_string() << end_line;

	if (!request.body().empty())
	{
		oss << request.body();
	}

	write(oss.str());
}

} // namespace protocol
} // namespace tnt
