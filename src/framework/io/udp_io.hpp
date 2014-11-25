
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

#ifndef TNT_IO_UDP_IO_HPP_
#define TNT_IO_UDP_IO_HPP_

#include "io.hpp"
#include "socket.hpp"
#include "ip_socket_address.hpp"

namespace tnt {
namespace io {

namespace udp = ip::udp;

class UdpIO: virtual public IO
{
public:
    UdpIO(const ip::SocketAddress& lep, const ip::SocketAddress& rep);
    UdpIO(udp::Socket&& sock, const ip::SocketAddress& rep);
    virtual ~UdpIO();

    virtual void reset() override;
    virtual std::string read() override;
    virtual bool try_read(std::string& data) override;
    virtual void write(const std::string& data) override;
private:
    unsigned int get_buffer_size();
private:
    udp::Socket sock_;

    ip::SocketAddress lep_;
    ip::SocketAddress rep_;

    unsigned int buffer_size_;
};

class UdpIOEndPoint: public virtual IOEndPoint
{
public:
    UdpIOEndPoint(const ip::SocketAddress& lep, const ip::SocketAddress& rep);
    virtual ~UdpIOEndPoint();

    virtual std::shared_ptr<IO> get() override;
    virtual void reset() override;
private:
    ip::SocketAddress lep_;
    ip::SocketAddress rep_;
};

} // namespace io
} // namespace tnt

#endif
