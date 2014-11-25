
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

#ifndef DROP_EVENT_ADDRESS_EVENTS_HPP_
#define DROP_EVENT_ADDRESS_EVENTS_HPP_

#include <memory>
#include <string>

#include "event/network/request.hpp"

#include "protocol/drop/values.hpp"

#include "router/address_info.hpp"

namespace drop {
namespace router {

class Address;

} // namespace router

namespace event {

class AddressList
{
public:
    explicit AddressList(const std::vector<AddressInfo>& addresses): addresses_{ addresses } { }

    const std::vector<AddressInfo>& addresses() const { return addresses_; }
private:
    std::vector<AddressInfo> addresses_;
};

class AddressAdded
{
public:
    AddressAdded(const std::shared_ptr<router::Address>& address, const std::string& interface): address_(address), interface_{ interface } { }

    std::shared_ptr<router::Address> address() const { return address_; }
    const std::string& interface() const { return interface_; }
private:
    std::shared_ptr<router::Address> address_;
    std::string interface_;
};

class AddressRemoved
{
public:
    AddressRemoved(const std::shared_ptr<router::Address>& address, const std::string& interface): address_(address), interface_{ interface } { }

    std::shared_ptr<router::Address> address() const { return address_; }
    const std::string& interface() const { return interface_; }
private:
    std::shared_ptr<router::Address> address_;
    std::string interface_;
};

class LocalAddressAdded
{
public:
    explicit LocalAddressAdded(const AddressInfo& address): address_(address) { }
    const AddressInfo& address() const { return address_; }
private:
    AddressInfo address_;
};

class LocalAddressRemoved
{
public:
    explicit LocalAddressRemoved(const AddressInfo& address): address_(address) { }
    const AddressInfo& address() const { return address_; }
private:
    AddressInfo address_;
};

class AddAddressRequest: public Request
{
public:
    AddAddressRequest(const AddressInfo& address, const ReplyInfo& reply): Request(reply), address_(address) { }

    const AddressInfo& address() const { return address_; }
private:
    AddressInfo address_;
};

class RemoveAddressRequest: public Request
{
public:
    RemoveAddressRequest(const AddressInfo& address, const ReplyInfo& reply): Request(reply), address_(address) { }

    const AddressInfo& address() const { return address_; }
private:
    AddressInfo address_;
};

} // namespace event
} // namespace drop

#endif
