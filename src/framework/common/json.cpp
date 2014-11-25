
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

#include "json.hpp"

#include <sstream>

namespace tnt {
namespace {

std::string quote(const std::string& v) 
{
    return std::string(R"(")") + v + R"(")";
}

std::string make_object(const std::string& v)
{
    return std::string("{ ") + v + " }";
}

std::string make_array(const std::string& v)
{
    return std::string("[ ") + v + " ]";
}

std::string combine(const std::vector<std::string>& v)
{
    std::stringstream ss;

    for (const auto& s : v)
    {
        ss << s << ",";
    }

    auto res = ss.str();

    if (!res.empty() && res.back() == ',')
    {
        res.pop_back();
    }

    return res;
}

std::string make_property(const std::string& k, const std::string& v)
{
    return quote(k) + ":" + v;
}

} // namespace

JsonObject& JsonObject::add(const std::string& key, const JsonObject& obj)
{
    properties_.emplace_back(std::make_unique<JsonObjectProperty>(key, obj));

    return *this;
}

JsonObject& JsonObject::add(const std::string& key, const JsonArray& array)
{
    properties_.emplace_back(std::make_unique<JsonArrayProperty>(key, array));

    return *this;
}

std::string JsonObject::str() const
{
    std::vector<std::string> res;
    res.reserve(properties_.size());

    for (const auto& p : properties_)
    {
        res.push_back(p->str());
    }

    return make_object(combine(res));
}

JsonObject::JsonValueProperty::JsonValueProperty(const std::string& key, const std::string& value): key_(key), value_(quote(value)) {}

JsonObject::JsonValueProperty::JsonValueProperty(const std::string& key, const char* value): key_(key), value_(quote(value)) {}

JsonObject::JsonValueProperty::JsonValueProperty(const std::string& key, bool value): key_(key), value_(value ? "true" : "false") {}

std::string JsonObject::JsonValueProperty::str() const
{
    return make_property(key_, value_);
}

JsonObject::JsonObjectProperty::JsonObjectProperty(const std::string& key, const JsonObject& obj): key_(key), value_(obj.str()) {}

std::string JsonObject::JsonObjectProperty::str() const
{
    return make_property(key_, value_);
}

JsonObject::JsonArrayProperty::JsonArrayProperty(const std::string& key, const JsonArray& array): key_(key), value_(array.str()) {}

std::string JsonObject::JsonArrayProperty::str() const
{
    return make_property(key_, value_);
}

JsonArray::JsonArray(const std::vector<std::string>& values)
{
    for (const auto& v : values)
    {
        elements_.push_back(quote(v));
    }
}

JsonArray& JsonArray::add(const std::string& value)
{
    elements_.push_back(quote(value));

    return *this;
}

JsonArray& JsonArray::add(const char* value)
{
	elements_.push_back(quote(value));

    return *this;
}

JsonArray& JsonArray::add(bool value)
{
    elements_.push_back(value ? "true" : "false");

    return *this;
}

JsonArray& JsonArray::add(const JsonObject& obj)
{
    elements_.push_back(obj.str());

    return *this;
}

JsonArray& JsonArray::add(const JsonArray& array)
{
    elements_.push_back(array.str());

    return *this;
}

JsonArray::size_type JsonArray::size() const
{
    return elements_.size();
}

bool JsonArray::empty() const
{
    return elements_.empty();
}

std::string JsonArray::str() const
{
    return make_array(combine(elements_));
}

} // namespace tnt
