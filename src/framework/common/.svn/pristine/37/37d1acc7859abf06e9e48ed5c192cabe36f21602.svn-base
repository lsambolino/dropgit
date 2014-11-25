
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

#ifndef TNT_IP_BASIC_SOCKET_HPP_
#define TNT_IP_BASIC_SOCKET_HPP_

#include <string>
#include <type_traits>
#include <chrono>
#include <cassert>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <WinSock2.h>

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#if !defined(__CYGWIN__)

#include <linux/netlink.h>

#endif

#endif

#include "init_sockets.hpp"
#include "socket_options.hpp"
#include "socket_exception.hpp"

namespace tnt {

enum class AddressFamily
{
    Unspecified = AF_UNSPEC,
    Inet = AF_INET,
    Inet6 = AF_INET6,

#if defined(TNT_PLATFORM_LINUX)

    Netlink = AF_NETLINK

#endif

};

enum class SocketType
{
    Stream = SOCK_STREAM,
    Datagram = SOCK_DGRAM,
    Raw = SOCK_RAW,
};

enum class SocketProtocol
{
    IP = IPPROTO_IP,
    Icmp = IPPROTO_ICMP,
    Tcp = IPPROTO_TCP,
    Udp = IPPROTO_UDP,

#if defined(TNT_PLATFORM_LINUX)
    
    NetlinkRoute = NETLINK_ROUTE,

#endif

};

enum class Shutdown
{
#if defined(TNT_PLATFORM_WIN32)
    Read = SD_RECEIVE,
    Write = SD_SEND,
    Both = SD_BOTH
#else
    Read = SHUT_RD,
    Write = SHUT_WR,
    Both = SHUT_RDWR
#endif
};

#if defined(TNT_PLATFORM_WIN32)

using socklen_t = int;

#endif

template <SocketType T, SocketProtocol P> struct SocketTypeCheck
{
    static const bool value = false;
};

template <> struct SocketTypeCheck<SocketType::Datagram, SocketProtocol::Udp>
{
    static const bool value = true;
};

template <> struct SocketTypeCheck<SocketType::Stream, SocketProtocol::Tcp>
{
    static const bool value = true;
};

#if defined(TNT_PLATFORM_LINUX)

template <> struct SocketTypeCheck<SocketType::Stream, SocketProtocol::NetlinkRoute>
{
    static const bool value = true;
};

template <> struct SocketTypeCheck<SocketType::Raw, SocketProtocol::NetlinkRoute>
{
    static const bool value = true;
};

#endif

template <AddressFamily af, SocketType Type, SocketProtocol Proto> class BasicSocket: private InitSockets
{
    static_assert(SocketTypeCheck<Type, Proto>::value, "Invalid type and protocol combination");
    using socket_type = int;
    static const socket_type Invalid = -1;
public:
    BasicSocket(): sock_(create())
    {
        if (sock_ == Invalid)
        {
            throw OpenSocketException();
        }
    }

    BasicSocket(const BasicSocket& other) = delete;

    BasicSocket(BasicSocket&& other): sock_(other.sock_)
    {
        other.sock_ = Invalid;
    }

    ~BasicSocket()
    {
        close(true);
    }

    BasicSocket& operator=(const BasicSocket& other) = delete;

    BasicSocket& operator=(BasicSocket&& other)
    {
        BasicSocket tmp(std::move(other));
        using std::swap;

        swap(sock_, tmp.sock_);

        return *this;
    }

    bool operator!()
    {
        return sock_ == Invalid;
    }

    operator bool()
    {
        return sock_ != Invalid;
    }

    socket_type get() const
    {
        return sock_;
    }

    void open()
    {
        assert(sock_ == Invalid);
        sock_ = create();
    }

    void close()
    {
        close(false);
    }

    void shutdown(Shutdown how)
    {
        if (sock_ == Invalid)
        {
            return;
        }

        if (::shutdown(sock_, static_cast<int>(how)) == Invalid)
        {
            if (socket_error_number() != ENOTCONN)
            {
                throw ShutdownSocketException();
            }
        }
    }

    template <class A> void bind(const A& address)
    {
        if (::bind(sock_, address, address.size()) == Invalid)
        {
            throw BindSocketException();
        }
    }

    template <class A> bool connect(const A& address)
    {
        if (::connect(sock_, address, address.size()) == Invalid)
        {
            if (socket_error_number() == ECONNREFUSED)
            {
                return false;
            }

            throw ConnectSocketException();
        }

        return true;
    }

    void listen(int backlog)
    {
        static_assert(Proto == SocketProtocol::Tcp, "This member function is available only on TCP sockets.");

        if (::listen(sock_, backlog) == Invalid)
        {
            throw ListenSocketException();
        }
    }

    template <class A> BasicSocket accept(A& address)
    {
        static_assert(Proto == SocketProtocol::Tcp, "This member function is available only on TCP sockets.");
        auto len = address.size();

        return BasicSocket(accept(address, &len));
    }

    BasicSocket accept()
    {
        static_assert(Proto == SocketProtocol::Tcp, "This member function is available only on TCP sockets.");

        return BasicSocket(accept(nullptr, nullptr));
    }

    template <class T> void get_option(T& option)
    {
        if (::getsockopt(sock_, option.level(), option.name(), option.data(), option.len()) == Invalid)
        {
            throw GetSocketOptionException();
        }
    }

    template <class T> void set_option(const T& option)
    {
        if (::setsockopt(sock_, option.level(), option.name(), option.data(), *option.len()) == Invalid)
        {
            throw SetSocketOptionException();
        }
    }

    template <class A> size_t send_to(const std::string& buf, const A& address, int flags = 0)
    {
        static_assert(Proto == SocketProtocol::Udp, "This member function is available only on UDP sockets.");

        auto ret = ::sendto(sock_, buf.data(), buf.size(), flags, address, address.size());

        if (ret == Invalid)
        {
            throw SendToException();
        }

        return ret;
    }

    ssize_t send(const std::string& buf, int flags = 0)
    {
        static_assert(Proto == SocketProtocol::Tcp, "This member function is available only on TCP sockets.");

        auto ret = ::send(sock_, buf.data(), buf.size(), flags);

        if (ret == Invalid)
        {
            throw SendException();
        }

        return ret;
    }

    template <class A> size_t receive_from(std::string& buf, A& address, int flags = 0)
    {
        static_assert(Proto == SocketProtocol::Udp, "This member function is available only on UDP sockets.");

        socklen_t len = sizeof(address);
        auto ret = ::recvfrom(sock_, &buf[0], buf.size(), flags, reinterpret_cast<sockaddr*>(&address), &len);

        if (ret == Invalid)
        {
            throw ReceiveFromException();
        }

        return ret;
    }

    template <class A> bool try_receive_from(std::string& buf, A& address, size_t& size, int flags = 0)
    {
        static_assert(Proto == SocketProtocol::Udp, "This member function is available only on UDP sockets.");

#if defined(TNT_PLATFORM_LINUX)
        socklen_t len = sizeof(address);

        set_flags(O_NONBLOCK);

        auto res = ::recvfrom(sock_, &buf[0], buf.size(), flags, reinterpret_cast<sockaddr*>(&address), &len);

        unset_flags(O_NONBLOCK);

        if (res == Invalid)
        {
            if (errno == EAGAIN)
            {
                return false;
            }
            else
            {
                throw ReceiveException();
            }
        }

        size = res;

        return true;
#else
        // TODO:
#endif
        return false;
    }

    size_t receive(std::string& buf, int flags = 0)
    {
        static_assert(Proto == SocketProtocol::Tcp, "This member function is available only on TCP sockets.");

        auto ret = ::recv(sock_, &buf[0], buf.size(), flags);

        if (ret == Invalid)
        {
            throw ReceiveException();
        }

        return ret;
    }

    bool try_receive(std::string& buf, size_t& size, int flags = 0)
    {
        static_assert(Proto == SocketProtocol::Tcp, "This member function is available only on TCP sockets.");

#if defined(TNT_PLATFORM_LINUX)

        set_flags(O_NONBLOCK);

        auto ret = ::recv(sock_, &buf[0], buf.size(), flags);

        unset_flags(O_NONBLOCK);

        if (ret == Invalid)
        {
            if (errno == EAGAIN)
            {
                return false;
            }
            else
            {
                throw ReceiveException();
            }
        }

        size = ret;

        return true;
#else
        //TODO:
#endif

        return false;
    }
private:
    explicit BasicSocket(socket_type sock): sock_{ sock } {}

    void close(bool shutdown_sock)
    {
        if (shutdown_sock)
        {
            shutdown(Shutdown::Both);
        }

        if (sock_ != Invalid)
        {
#if defined(TNT_PLATFORM_WIN32)
            ::closesocket(sock_);
#else
            ::close(sock_);
#endif
        }

        sock_ = Invalid;
    }

    socket_type create()
    {
        return ::socket(static_cast<int>(af), static_cast<int>(Type), static_cast<int>(Proto));
    }

    socket_type accept(sockaddr* address, socklen_t* len)
    {
        socket_type sock = ::accept(sock_, address, len);

        if (sock == Invalid)
        {
            throw AcceptSocketException();
        }

        return sock;
    }

    void set_flags(int flags)
    {
        fcntl(sock_, F_SETFL, fcntl(sock_, F_GETFL) | flags);
    }

    void unset_flags(int flags)
    {
        fcntl(sock_, F_SETFL, fcntl(sock_, F_GETFL) & ~flags);
    }
private:
    socket_type sock_;
};

} // namespace tnt

#endif
