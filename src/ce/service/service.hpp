
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

#ifndef DROP_SERVICE_HPP_
#define DROP_SERVICE_HPP_

#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <iosfwd>

namespace tnt {

struct Message;

} // namespace tnt

namespace drop {
namespace ce {

class ServiceElement;

} // namespace ce

struct Service
{
    virtual ~Service() = default;
    virtual std::ostream& print(std::ostream& os) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Service& service);

using ServiceFactoryMethod = std::function<std::shared_ptr<Service>(ce::ServiceElement*)>;

struct ServiceRegEntry;

class ServiceFromName
{
    friend struct ServiceRegEntry;
    using FactoryMethodMap = std::unordered_map<std::string, ServiceFactoryMethod>;
public:
    static ServiceFactoryMethod create(const std::string& name);
private:
    ServiceFromName();

    static void Register(const std::string& name, ServiceFactoryMethod m);
    static void init_map();
private:
    static std::unique_ptr<FactoryMethodMap> factories_;
};

struct ServiceRegEntry 
{
    ServiceRegEntry(const std::string& name, ServiceFactoryMethod m) 
    { 
        ServiceFromName::Register(name, m) ;
    }
};

template <class T, const char N[]> class RegisterService: public virtual Service
{
protected:
    RegisterService()
    {
        const ServiceRegEntry& dummy = r;
        (void)dummy;
    }
private:
    static std::shared_ptr<T> new_instance(ce::ServiceElement* parent) 
    {
        static_assert(std::is_base_of<Service, T>::value, "Invalid class type registered as Service.");
        static_assert(std::is_constructible<T, ce::ServiceElement*>::value, "Invalid constructor of the class registered as Service.");

        return std::make_shared<T>(parent);
    }
private:
    static const ServiceRegEntry r;
};

template <class T, const char N[]> const ServiceRegEntry RegisterService<T, N>::r = ServiceRegEntry(N, RegisterService<T, N>::new_instance);

} // namespace drop

#endif
