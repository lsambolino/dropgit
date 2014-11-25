
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

#ifndef TCP_IO_HPP_
#define TCP_IO_HPP_

#include <memory>

#include "io.hpp"
#include "socket.hpp"
#include "ip_socket_address.hpp"

namespace tnt {
namespace io {

namespace tcp = ip::tcp;

class TcpIO: public virtual IO
{
    friend class TcpIOServer;
    friend class TcpIOClient;
public:
    explicit TcpIO(tcp::Socket&& sock);

    virtual void reset() override;
    virtual std::string read() override;
    virtual bool try_read(std::string& data) override;
    virtual void write(const std::string& data) override;
private:
    unsigned int get_buffer_size();
private:
    tcp::Socket sock_;
    unsigned int buffer_size_;
};

class TcpIOServer: public virtual IOEndPoint
{
public:
    explicit TcpIOServer(const ip::SocketAddress& ep);
    virtual ~TcpIOServer();

    virtual std::shared_ptr<IO> get() override;
    virtual void reset() override;
private:
    tcp::Socket sock_;
};

class TcpIOClient: public virtual IOEndPoint
{
public:
    explicit TcpIOClient(const ip::SocketAddress& rep);
    TcpIOClient(const ip::SocketAddress& lep, const ip::SocketAddress& rep);

    virtual std::shared_ptr<IO> get() override;
    virtual void reset() override;
private:
    ip::SocketAddress lep_;
    ip::SocketAddress rep_;
};

} // namespace io
} // namespace tnt

#endif
