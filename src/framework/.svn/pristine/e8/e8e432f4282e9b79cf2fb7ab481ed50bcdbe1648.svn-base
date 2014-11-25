
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

#ifndef TNT_ACTIVITY_CONTROLLER_HPP_
#define TNT_ACTIVITY_CONTROLLER_HPP_

#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

namespace tnt {
namespace activity {

class HttpConnection;

struct Controller
{
    virtual ~Controller() = 0;
};

using ControllerFactoryMethod = std::function<std::unique_ptr<Controller>(HttpConnection*, const std::string&)>;

struct ControllerRegEntry;

class ControllerFromName
{
    friend struct ControllerRegEntry;
    using FactoryMethodMap = std::unordered_map<std::string, ControllerFactoryMethod>;
public:
    static ControllerFactoryMethod create(const std::string& name);
private:
    ControllerFromName();

    static void Register(const std::string& name, ControllerFactoryMethod m);
    static void init_map();
private:
    static std::unique_ptr<FactoryMethodMap> factories_;
};

struct ControllerRegEntry 
{
    ControllerRegEntry(const std::string& name, ControllerFactoryMethod m) 
    { 
        ControllerFromName::Register(name, m) ;
    }
};

template <class T, const char N[]> class RegisterController: public Controller
{
protected:
    RegisterController()
    {
        const ControllerRegEntry& dummy = r;
        (void)dummy;
    }
private:
    static std::unique_ptr<T> new_instance(HttpConnection* connection, const std::string& base_path) 
    {
        static_assert(std::is_base_of<Controller, T>::value, "Invalid class type registered as Controller.");
        static_assert(std::is_constructible<T, HttpConnection*, const std::string&>::value, "Invalid constructor of the class registered as Controller.");

        return std::make_unique<T>(connection, base_path);
    }
private:
    static const ControllerRegEntry r;
};

template <class T, const char N[]> const ControllerRegEntry RegisterController<T, N>::r = ControllerRegEntry(N, RegisterController<T, N>::new_instance);

} // namespace activity
} // namespace tnt

#endif
