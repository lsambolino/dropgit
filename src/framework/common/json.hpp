
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

#ifndef TNT_JSON_HPP_
#define TNT_JSON_HPP_

#include <vector>
#include <string>
#include <memory>

namespace tnt {

class JsonArray;

class JsonObject
{
    struct JsonProperty
    {
        virtual ~JsonProperty() = default;
        virtual std::string str() const = 0;
    };

    class JsonValueProperty: public JsonProperty
    {
    public:
        template <class T> JsonValueProperty(const std::string& key, const T& value) : key_(key), value_(std::to_string(value)) {}
        JsonValueProperty(const std::string& key, const std::string& value);
        JsonValueProperty(const std::string& key, const char* value);
        JsonValueProperty(const std::string& key, bool value);

        virtual std::string str() const override;
    private:
        std::string key_;
        std::string value_;
    };

    class JsonObjectProperty: public JsonProperty
    {
    public:
        JsonObjectProperty(const std::string& key, const JsonObject& obj);

        virtual std::string str() const override;
    private:
        std::string key_;
        std::string value_;
    };

    class JsonArrayProperty: public JsonProperty
    {
    public:
        JsonArrayProperty(const std::string& key, const JsonArray& array);

        virtual std::string str() const override;
    private:
        std::string key_;
        std::string value_;
    };
public:
    JsonObject() = default;
    JsonObject(const JsonObject&) = delete;
    JsonObject(JsonObject&&) = default;

    JsonObject& operator=(const JsonObject&) = delete;
    JsonObject& operator=(JsonObject&&) = default;

    template <class T> JsonObject& add(const std::string& key, const T& value)
    {
        properties_.emplace_back(std::make_unique<JsonValueProperty>(key, value));

        return *this;
    }

    template <class T> JsonObject& add(const std::string& key, const std::vector<T>& array)
    {
        return add(key, JsonArray(array));
    }

    JsonObject& add(const std::string& key, const JsonObject& obj);
    JsonObject& add(const std::string& key, const JsonArray& array);

    std::string str() const;
private:
    std::vector<std::unique_ptr<JsonProperty>> properties_;
};

class JsonArray
{
    using ElementsBag = std::vector<std::string>;
    using size_type = ElementsBag::size_type;
public:
    JsonArray() = default;

    template <class T> JsonArray(const std::vector<T>& values)
    {
        for (const auto& v : values)
        {
            elements_.push_back(std::to_string(v));
        }
    }

    JsonArray(const std::vector<std::string>& values);

    template <class T> JsonArray& add(const T& value)
    {
        elements_.push_back(std::to_string(value));

        return *this;
    }

    JsonArray& add(const std::string& value);
	JsonArray& add(const char* value);
    JsonArray& add(bool value);
    JsonArray& add(const JsonObject& obj);
    JsonArray& add(const JsonArray& array);

    size_type size() const;
    bool empty() const;

    std::string str() const;
private:
    ElementsBag elements_;
};

} // namespace tnt

#endif
