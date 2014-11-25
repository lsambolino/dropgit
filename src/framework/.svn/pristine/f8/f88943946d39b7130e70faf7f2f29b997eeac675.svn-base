
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

#ifndef TNT_ACTIVITY_HTTP_CONNECTION_HPP_
#define TNT_ACTIVITY_HTTP_CONNECTION_HPP_

#include <string>
#include <memory>
#include <atomic>

#include "event/http/http_server_request.hpp"

#include "activity/activity_from_event.hpp"
#include "activity/concurrent_activity.hpp"

#include "protocol/http/http_method.hpp"

namespace web {
namespace http {

class uri;

} // namespace http
} // namespace web

namespace tnt {
namespace protocol {

class HttpProtocol;

}

struct Protocol;

namespace event {

class HttpRequest;

} // namespace event

namespace protocol {

class HttpHeaders;

} // namespace protocol

namespace activity {

class HttpConnection: public ConcurrentActivity
{
public:
    HttpConnection(const std::shared_ptr<protocol::HttpProtocol>& proto, const std::string& base_path);
    ~HttpConnection();

    void operator()();

    void ok();
    void ok(const protocol::HttpHeaders& headers);
    void ok(const std::string& body, const std::string& content_type);
    void ok(const protocol::HttpHeaders& headers, const std::string& body, const std::string& content_type);
    void bad_request();
    void not_found();
    void internal_error();
    void not_implemented();

    const web::http::uri& uri() const;
	const std::string& version() const;
    protocol::HttpMethod method() const;
	const protocol::HttpHeaders& headers() const;
	const std::string& header(const std::string& key) const;
    const std::string& body() const;
private:
    void handle_request(const std::shared_ptr<event::HttpRequest>& request);
    void stop();
private:
    std::atomic_bool running_;
    std::shared_ptr<protocol::HttpProtocol> proto_;
    std::string base_path_;
    std::shared_ptr<event::HttpRequest> request_;
    bool close_;
};

} // namespace activity
}  // namespace tnt

#endif
