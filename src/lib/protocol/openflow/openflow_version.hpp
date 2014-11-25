
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

#ifndef DROP_PROTOCOL_OPENFLOW_VERSION_HPP_
#define DROP_PROTOCOL_OPENFLOW_VERSION_HPP_

#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

namespace drop {
namespace protocol {

class Openflow;
class Flow;

struct OpenflowProtocol
{
    virtual ~OpenflowProtocol() = default;

    virtual void init() = 0;
    virtual void remove_all() = 0;

    virtual void add(const Flow& flow) = 0;
    virtual void remove(const Flow& flow) = 0;

    virtual void packet(const std::string& message) = 0;

    virtual void send_packet(const std::string& buffer, uint16_t port) = 0;

    virtual void request_port_stats(uint16_t port) = 0;
};

using OpenflowFactoryMethod = std::function<std::unique_ptr<OpenflowProtocol>(Openflow*)>;

struct OpenflowRegEntry;

class OpenflowVersion
{
    friend struct OpenflowRegEntry;
    using FactoryMethodMap = std::unordered_map<int, OpenflowFactoryMethod>;
public:
    static std::unique_ptr<OpenflowProtocol> create(int version, Openflow* ptr);
private:
    OpenflowVersion();

    template <class... VS> static void Register(OpenflowFactoryMethod m, int v0)
    {
        init();

        factories_->emplace(v0, m);
    }

    template <class... VS> static void Register(OpenflowFactoryMethod m, int v0, VS... vs)
    {
        init();

        factories_->emplace(v0, m);
        Register(m, vs...);
    }

    static void init_map();
    static void init();
private:
    static std::unique_ptr<FactoryMethodMap> factories_;
};

struct OpenflowRegEntry 
{
    template <class... VS> OpenflowRegEntry(OpenflowFactoryMethod m, int v0, VS... vs) 
    { 
        OpenflowVersion::Register(m, v0, vs...) ;
    }
};

template <class T, int... V> class RegisterOpenflowVersion: public OpenflowProtocol
{
protected:
    RegisterOpenflowVersion()
    {
        const OpenflowRegEntry& dummy = r;
        (void)dummy;
    }
private:
    static std::unique_ptr<T> new_instance(Openflow* ptr) 
    {
        static_assert(std::is_base_of<OpenflowProtocol, T>::value, "Invalid class type registered as OpenflowProtocol.");
        static_assert(std::is_constructible<T, Openflow*>::value, "Invalid constructor of the class registered as OpenflowProtocol.");

        return std::make_unique<T>(ptr);
    }
private:
    static const OpenflowRegEntry r;
};

template <class T, int... V> const OpenflowRegEntry RegisterOpenflowVersion<T, V...>::r = OpenflowRegEntry(RegisterOpenflowVersion<T, V...>::new_instance, V...);

} // namespace protocol
} // namespace drop

#endif
