
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

#include "instance.hpp"

#include <boost/asio.hpp>

#include "io/udp_io.hpp"
#include "ip_socket_address.hpp"

#include "util/pugixml.hpp"

#include "log.hpp"

class UdpIOEndPoint: public virtual tnt::IOEndPoint
{
public:
    UdpIOEndPoint(tnt::ip::udp::Socket&& sock, const tnt::ip::SocketAddress& rep): sock_(std::move(sock)), rep_{ rep } {}
    virtual ~UdpIOEndPoint() {}

    virtual std::shared_ptr<tnt::IO> get() override 
    {
        return std::make_shared<tnt::io::UdpIO>(std::move(sock_), rep_);
    }

    virtual void reset() override
    {
        // Nothing to do. 
    }
private:
    tnt::ip::udp::Socket sock_;
    tnt::ip::SocketAddress rep_;
};

tnt::IOEndPoint* create(const char* conf_file)
{
    try
    {
        pugi::xml_document doc;
        auto result = doc.load_file(conf_file);

        if (!result)
        {
            tnt::Log::error("UdpIOServer::create(", conf_file, ") error: ", result.description());

            return nullptr;
        }

        auto root = doc.first_child().child(boost::asio::ip::host_name().c_str());

        auto local = root.child("local");
        auto la = local.child("address").text().as_string();
        auto lp = local.child("port").text().as_int();
    
        auto remote = root.child("remote");
        auto ra = remote.child("address").text().as_string();
        auto rp = remote.child("port").text().as_int();

        auto buffer = root.child("buffer");

        tnt::ip::udp::Socket sock;
        sock.bind(tnt::ip::SocketAddress(la, lp));

        if (buffer)
        {
            auto size = buffer.child("size");

            if (size)
            {
                sock.set_option(tnt::ip::ReceiveBuffer(size.text().as_uint()));
            }
        }

        return new UdpIOEndPoint(std::move(sock), tnt::ip::SocketAddress(ra, rp));
    }
    catch (...) { }

    return nullptr;
}

void destroy(tnt::IOEndPoint* io)
{
    delete io;
}
