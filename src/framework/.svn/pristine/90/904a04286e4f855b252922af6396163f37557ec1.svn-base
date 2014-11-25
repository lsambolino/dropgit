
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

#include "udp_io.hpp"

#include "exception/exception.hpp"

#include "log.hpp"

namespace tnt {
namespace io {

UdpIO::UdpIO(const ip::SocketAddress& lep, const ip::SocketAddress& rep): lep_(lep), rep_(rep), buffer_size_(get_buffer_size())
{
    sock_.bind(lep_);
}

UdpIO::UdpIO(ip::udp::Socket&& sock, const ip::SocketAddress& rep): sock_(std::move(sock)), rep_(rep), buffer_size_(get_buffer_size()) {}

UdpIO::~UdpIO()
{
    try
    {
        reset();
    }
    catch (...) {}
}

void UdpIO::reset()
{
    sock_.shutdown(Shutdown::Both);
    sock_.close();
}

std::string UdpIO::read()
{
    std::string buf;
    buf.resize(buffer_size_);

    auto length = sock_.receive_from(buf, rep_);

    if (length == 0)
    {
        throw IOReset("Socket closed.");
    }

    buf.resize(length);

    return buf;
}

bool UdpIO::try_read(std::string& data)
{
    if (!sock_)
    {
        throw IOReset("Socket closed");
    }

    data.resize(buffer_size_);
    size_t length = 0;

    if (sock_.try_receive_from(data, rep_, length))
    {
        data.resize(length);

        return true;
    }

    return false;
}

void UdpIO::write(const std::string& data)
{
    auto dim = sock_.send_to(data, rep_);

    if (dim < data.size())
    {
        Log::warning("UdpIO::write(): sent only ", dim, " bytes of ", data.size());
    }
}

unsigned int UdpIO::get_buffer_size()
{
    ip::ReceiveBuffer opt;
    sock_.get_option(opt);

    return opt.value();
}

UdpIOEndPoint::UdpIOEndPoint(const ip::SocketAddress& lep, const ip::SocketAddress& rep): lep_{ lep }, rep_{ rep } {}

UdpIOEndPoint::~UdpIOEndPoint() {}

std::shared_ptr<IO> UdpIOEndPoint::get()
{
    return std::make_shared<UdpIO>(lep_, rep_);
}

void UdpIOEndPoint::reset()
{
    // Nothing to do.
}

} // namespace io
} // namespace tnt
