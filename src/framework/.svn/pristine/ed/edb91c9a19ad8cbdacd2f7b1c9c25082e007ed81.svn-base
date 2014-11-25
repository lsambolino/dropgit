
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

#include "http_server.hpp"

#include <future>
#include <atomic>

#include "activity/http/http_connection.hpp"

#include "event/quit.hpp"

#include "protocol/http/http_protocol.hpp"

#include "io/io.hpp"

#include "socket_exception.hpp"
#include "application.hpp"
#include "memory.hpp"
#include "async.hpp"
#include "log.hpp"
#include "demangle.hpp"

namespace tnt {
namespace activity {

HttpServer::~HttpServer()
{
    tnt::Log::debug(colors::green, "HttpServer activity terminated.");
}

void HttpServer::operator()(const std::shared_ptr<IOEndPoint>& ep, const std::string& base_path)
{
    assert(ep);

    std::atomic_bool running{ true };
    
    Application::subscribe([&] (const event::Quit& /*event*/)
    {
        running = false;
        ep->reset();
    });

    std::vector<std::future<void>> activities;

    while (running)
    {
        try
        {
            auto io = ep->get();

            if (!running)
            {
                break;
            }

            activities.push_back(tnt::async([=] ()
            {
                HttpConnection(std::make_shared<protocol::HttpProtocol>(io), base_path)();
            }));
        }
        catch (AcceptSocketException& ex)
        {
            if (running)
            {
                Log::error("HttpServer::run exception: ", ex.what(), " (", tnt::get_name(ex), ")");
            }
        }
        catch (std::exception& ex)
        {
            Log::error("HttpServer::run exception: ", ex.what(), " (", tnt::get_name(ex), ")");
        }
        catch (...)
        {
            Log::error("HttpServer::run unexpected exception.");
        }
    }

    for (auto& a : activities)
    {
        try
        {
            a.get();
        }
        catch (std::exception& ex)
        {
            tnt::Log::error("HttpServer::run error: ", ex.what(), " (", tnt::get_name(ex), ")");
        }
        catch (...)
        {
            tnt::Log::error("HttpServer::run: Unknown exception");
        }
    }
}

} // namespace activity
} // namespace tnt
