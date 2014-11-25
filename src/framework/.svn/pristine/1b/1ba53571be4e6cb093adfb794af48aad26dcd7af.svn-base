
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

#include "tcp_io.hpp"

#include <sstream>

#include "exception/exception.hpp"

#include "socket.hpp"
#include "socket_options.hpp"

#include "log.hpp"

namespace tnt {
namespace io {

TcpIO::TcpIO(ip::tcp::Socket&& sock): sock_(std::move(sock)), buffer_size_(get_buffer_size()) {}

void TcpIO::reset()
{
    try
    {
        sock_.shutdown(Shutdown::Both);
        sock_.close();
    }
    catch (...) {}
}

std::string TcpIO::read()
{
    if (!sock_)
    {
        throw IOReset("Socket closed");
    }

    std::string buf;
    buf.resize(buffer_size_);

    auto length = sock_.receive(buf);

    if (length == 0)
    {
        throw IOReset("Socket closed.");
    }

    buf.resize(length);

    return buf;
}

bool TcpIO::try_read(std::string& data)
{
    if (!sock_)
    {
        throw IOReset("Socket closed");
    }

    data.resize(buffer_size_);
    size_t length = 0;

    if (sock_.try_receive(data, length))
    {
        data.resize(length);

        return true;
    }

    return false;
}

void TcpIO::write(const std::string& data)
{
    if (!sock_)
    {
        throw IOReset("Socket closed");
    }

    sock_.send(data);
}

unsigned int TcpIO::get_buffer_size()
{
    ip::ReceiveBuffer opt;
    sock_.get_option(opt);

    return opt.value();
}

TcpIOServer::TcpIOServer(const ip::SocketAddress& ep)
{
    sock_.set_option(ip::ReuseAddress(true));
    sock_.bind(ep);
    sock_.listen(4);
}

TcpIOServer::~TcpIOServer()
{
    reset();
}

std::shared_ptr<IO> TcpIOServer::get()
{
    auto s = sock_.accept();
    
	return std::make_shared<TcpIO>(std::move(s));
}

void TcpIOServer::reset()
{
    try
    {
        sock_.shutdown(Shutdown::Both);
        sock_.close();
    }
    catch (...) {}
}

TcpIOClient::TcpIOClient(const ip::SocketAddress& rep): rep_(rep) {}
TcpIOClient::TcpIOClient(const ip::SocketAddress& lep, const ip::SocketAddress& rep): lep_(lep), rep_(rep) {}

std::shared_ptr<IO> TcpIOClient::get()
{
    tcp::Socket sock;
    sock.set_option(ip::ReuseAddress(true));

    if (lep_.address() != ip::Address::any() && lep_.port() != 0)
    {
        sock.bind(lep_);
    }

    if (sock.connect(rep_))
    {
        return std::make_shared<TcpIO>(std::move(sock));
    }

    std::ostringstream ss;
    ss << "Unable to connect to " << rep_;

    throw IOConnectionError(ss.str());
}

void TcpIOClient::reset()
{
    // Nothing to do.
}

} // namespace io
} // namespace tnt
