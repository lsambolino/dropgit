
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

#ifndef TNT_CONFIGURATION_HPP_
#define TNT_CONFIGURATION_HPP_

#include <string>
#include <memory>
#include <cassert>

#include <boost/property_tree/ptree.hpp>

#include "exception/exception.hpp"

#include "memory.hpp"

namespace tnt {

namespace pt = boost::property_tree;

class Configuration
{
private:
    class ConfigurationProxy
    {
    public:
		ConfigurationProxy(pt::ptree config, const std::string& key);

        template <class T> T as() const
        {
            try
            {
                return config_.get<T>(key_);
            }
            catch (std::exception& ex)
            {
                throw ConfigurationError(R"(Error reading key ")" + key_ + R"(": )" + ex.what());
            } 
        }
    private:
        pt::ptree config_;
        std::string key_;
    };

    template <class T> class ConfigurationProxyWithDefault
    {
    public:
        ConfigurationProxyWithDefault(pt::ptree config, const std::string& key, T default_value): config_{ config }, key_{ key }, default_value_{ default_value } {}

        operator T() const
        {
            try
            {
                return config_.get<T>(key_, default_value_);
            }
            catch (std::exception& ex)
            {
                throw ConfigurationError(R"(Error reading key ")" + key_ + R"(": )" + ex.what());
            }
        }
    private:
        pt::ptree config_;
        std::string key_;
        T default_value_;
    };

    template <class T> void set_value(const std::string& key, T value, bool overwrite)
    {
        if (!overwrite && instance_->config_.get_child_optional(key).is_initialized())
        {
            return;
        }

        config_.put(key, value);
    }
public:
    Configuration();
    explicit Configuration(pt::ptree config);
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    static void default_init();
    static void read(pt::ptree config);
    static bool exists(const std::string& path);
    static const ConfigurationProxy get(const std::string& key);

    template <class T> static const ConfigurationProxyWithDefault<T> get(const std::string& key, T default_value)
    {
        assert(instance_);

        return ConfigurationProxyWithDefault<T>(instance_->config_, key, default_value);
    }

    template <class T> static void set(const std::string& key, T value)
    {
        assert(instance_);
        instance_->set_value(key, value, true);
    }

    template <class T> static void set_default(const std::string& key, T value)
    {
        assert(instance_);
        instance_->set_value(key, value, false);
    }
private:
    pt::ptree config_;
    static std::unique_ptr<Configuration> instance_;
};

} // namespace tnt

#endif
