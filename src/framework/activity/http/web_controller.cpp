
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

#include "web_controller.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <fstream>
#include <cctype>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "activity/http/http_connection.hpp"

#include "exception/exception.hpp"

#include "event/network/connection_reset.hpp"

#include "message/http/http_response.hpp"

#include "protocol/protocol.hpp"
#include "protocol/http/http_response.hpp"
#include "protocol/http/http_method.hpp"
#include "protocol/http/cpprest/uri.h"

#include "util/string.hpp"

#include "log.hpp"
#include "containers.hpp"
#include "range.hpp"

namespace tnt {
namespace activity {

namespace fs = boost::filesystem;

namespace {

std::string content_type(const std::string& path)
{
    const auto& ext = fs::extension(path);

    if (ext == ".html" || ext == ".htm")
        return "text/html";
    if (ext == ".xml")
        return "text/xml";
    if (ext == ".jpe" || ext == ".jpeg" || ext == ".jpg")
        return "image/jpeg";
    if (ext == ".png")
        return "image/png";
    if (ext == ".gif")
        return "image/gif";
    if (ext == ".js")
        return "application/x-javascript";
    if (ext == ".css")
        return "text/css";

    return "application/octet-stream";
}

} // namespace

WebController::WebController(HttpConnection* connection, const std::string& base_path) : connection_(connection), base_path_(base_path)
{
    assert(connection_);

    try
    {
        switch (connection->method())
        {
        case protocol::HttpMethod::Get:
            handle_get();
            break;
        case protocol::HttpMethod::Post:
            handle_post();
            break;
        case protocol::HttpMethod::Head:
        case protocol::HttpMethod::Delete:
        case protocol::HttpMethod::Put:
        case protocol::HttpMethod::Options:
        default:
            connection->not_implemented();
            break;
        }
    }
    catch (...)
    {
        connection->internal_error();
    }
}

void WebController::handle_get()
{
    const auto& uri = connection_->uri();

    auto file_path = fs::initial_path<fs::path>() / base_path_;
    auto path = web::http::uri::split_path(uri.path());

    if (path.empty())
    {
        file_path = file_path / "Home" / "Index.html";
    }
    else if (path.size() == 1)
    {
        file_path = file_path / path[0] / "Index.html";
    }
    else if (path.size() == 2)
    {
        file_path = file_path / path[0] / path[1];

        if (fs::extension(file_path).empty())
        {
            file_path += ".html";
        }
    }
    else if (path.size() > 2)
    {
        for (const auto& p : path)
        {
            file_path = file_path / p;
        }

        if (fs::is_directory(file_path))
        {
            connection_->not_found();

            return;
        }
    }

    if (!fs::exists(file_path))
    {
        connection_->not_found();

        return;
    }

    get_file(file_path.string());
}

void WebController::handle_post()
{
    connection_->not_implemented();
}

/*void WebController::handle_head()
{
    connection_->not_implemented();
}

void WebController::handle_put()
{
    connection_->not_implemented();
}

void WebController::handle_delete()
{
    connection_->not_implemented();
}*/

void WebController::get_file(const std::string& path)
{
    try
    {
        std::ifstream is(path, std::ifstream::binary);

        if (is)
        {
            is.seekg(0, is.end);
            auto length = is.tellg();
            is.seekg(0, is.beg);

            auto buffer = std::string(length, '\0');
            is.read(&buffer[0], length);

            connection_->ok(buffer, content_type(path));

            return;
        }
    }
    catch (...) {}

    connection_->internal_error();
}

} // namespace activity
} // namespace tnt
