
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

#include "http_connection.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <cctype>
#include <cassert>

#if defined(WITH_BOOST)

#include <boost/algorithm/string.hpp>

#endif

#include "activity/http/controller.hpp"

#include "event/network/connection_reset.hpp"
#include "event/http/http_request.hpp"
#include "event/quit.hpp"

#include "message/http/http_response.hpp"

#include "protocol/protocol.hpp"
#include "protocol/http/http_response.hpp"
#include "protocol/http/http_protocol.hpp"

#include "io/io.hpp"

#include "util/string.hpp"

#include "application.hpp"
#include "log.hpp"

namespace tnt {
namespace activity {

HttpConnection::HttpConnection(const std::shared_ptr<protocol::HttpProtocol>& proto, const std::string& base_path) : running_{ false }, proto_(proto), base_path_(base_path)
{
    assert(proto_);
}

HttpConnection::~HttpConnection()
{
    tnt::Log::debug(colors::green, "HttpConnection activity terminated.");
}

void HttpConnection::operator()()
{
    running_ = true;

    register_handler([&] (const tnt::event::Quit& /*event*/)
	{
		stop();
	});

    register_handler(proto_.get(), [&] (const event::ConnectionReset& event)
    {
        assert(event.protocol() == proto_.get());
        stop();
    });

    register_handler(proto_.get(), [&] (const std::shared_ptr<event::HttpRequest>& event)
    {
        handle_request(event);
    });

    proto_->start();

    while (running_)
    {
        wait_event();
    }
}

void HttpConnection::handle_request(const std::shared_ptr<event::HttpRequest>& request)
{
    try
    {
        request_ = request;
        close_ = request->connection_close();

        auto path = web::http::uri::split_path(request->uri().path());
        ControllerFromName::create(path.empty() ? "Home" : path[0])(this, base_path_);
    }
    catch (...)
    {
        internal_error();
    }
}

const web::http::uri& HttpConnection::uri() const
{
    return request_->uri();
}

const std::string& HttpConnection::version() const
{
    return request_->version();
}

protocol::HttpMethod HttpConnection::method() const
{
    return request_->method();
}

const protocol::HttpHeaders& HttpConnection::headers() const
{
    return request_->headers();
}

const std::string& HttpConnection::header(const std::string& key) const 
{
    return request_->header(key);
}

const std::string& HttpConnection::body() const
{
    return request_->body();
}

void HttpConnection::stop()
{
    running_ = false;
}

void HttpConnection::ok()
{
    auto headers = protocol::HttpHeaders({ { "Connection", protocol::get_connection(close_) }, { "Content-Length", "0" } });
    
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse(protocol::HttpStatusCode::OK, headers)));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

void HttpConnection::ok(const protocol::HttpHeaders& headers)
{
    auto hdr = protocol::HttpHeaders({ { "Connection", protocol::get_connection(close_) }, { "Content-Length", "0" } });
    
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse(protocol::HttpStatusCode::OK, hdr + headers)));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

void HttpConnection::ok(const std::string& body, const std::string& content_type)
{
    auto headers = protocol::HttpHeaders({ { "Connection", protocol::get_connection(close_) }, { "Content-Type", content_type }, { "Content-Length", std::to_string(body.size()) } });
    
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse(protocol::HttpStatusCode::OK, headers, body)));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

void HttpConnection::ok(const protocol::HttpHeaders& headers, const std::string& body, const std::string& content_type)
{
    auto hdr = protocol::HttpHeaders({ { "Connection", protocol::get_connection(close_) }, { "Content-Type", content_type }, { "Content-Length", std::to_string(body.size()) } });

    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse(protocol::HttpStatusCode::OK, hdr + headers, body)));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

void HttpConnection::bad_request()
{
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse::bad_request()));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

void HttpConnection::not_found()
{
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse::not_found()));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

void HttpConnection::internal_error()
{
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse::internal_error()));
    f.get();
    stop();
}

void HttpConnection::not_implemented()
{
    auto f = proto_->send(std::make_unique<message::HttpResponse>(protocol::HttpResponse::not_implemented()));
    
    if (close_)
    {
        f.get();
        stop();
    }
}

} // namespace activity
} // namespace drop
