
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

#ifndef DROP_CE_SERVICE_ELEMENT_HPP_
#define DROP_CE_SERVICE_ELEMENT_HPP_

#include <memory>
#include <vector>
#include <string>
#include <iosfwd>

namespace tnt {

struct Protocol;
struct Message;

} // namespace tnt

namespace drop {

struct Service;

namespace ce {

class ServiceElement
{
public:
    ServiceElement(const std::string& name, const std::string& display_name, const std::vector<std::string>& services, bool force_connected, const std::string& address = "");

    const std::string& name() const;
    const std::string& display_name() const;
    const std::string& address() const;

    const std::vector<std::shared_ptr<Service>> services() const;

    void connect(const std::string& address, const std::shared_ptr<tnt::Protocol>& proto, const std::vector<std::string>& services);
    void disconnect();

    bool connected() const;

    void send(std::unique_ptr<tnt::Message>&& message);
private:
    void set_services(const std::vector<std::string>& services);
private:
    std::string name_;
    std::string display_name_;
    std::string address_;
    std::vector<std::shared_ptr<Service>> services_;
    bool force_connected_;

    std::shared_ptr<tnt::Protocol> proto_;
};

std::ostream& operator<<(std::ostream& os, const ServiceElement& element);

} // namespace ce
} // namespace drop

#endif
