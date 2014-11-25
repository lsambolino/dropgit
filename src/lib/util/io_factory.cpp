
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

#include "io_factory.hpp"

#include "exception/exception.hpp"

#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/pugixml.hpp"

#include "dynamic_module.hpp"
#include "log.hpp"

namespace factory {

std::shared_ptr<tnt::IOEndPoint> create_io_end_point(const std::string& name, const std::string& remote_address, int remote_port)
{
    try
    {
        auto conf_file = drop::util::config_file_path(name);

        pugi::xml_document doc;
        auto result = doc.load_file(conf_file.c_str());

        if (!result)
        {
            throw tnt::ConfigurationError(std::string("create_io_end_point(") + conf_file + ") error: " + result.description());
        }

        auto root = doc.first_child();
        auto local = root.child("local");

        std::string io_class = root.child("class").text().as_string();

        if (io_class.empty())
        {
            throw tnt::ConfigurationError(std::string("create_io_end_point(") + conf_file + ") error: Empty io class");
        }

        auto io_file = drop::util::full_path(io_class + ".so");

        auto la = local.child("address").text().as_string("0.0.0.0");
        auto lp = local.child("port").text().as_int(0);

        auto remote = root.child("remote");

        auto ra = std::string(remote.child("address").text().as_string("0.0.0.0"));
        auto rp = remote.child("port").text().as_int(0);
        
        if (!remote_address.empty())
        {
            ra = remote_address;
        }

        if (remote_port > 0)
        {
            rp = remote_port;
        }

        return tnt::module::load<tnt::IOEndPoint, const char*, int, const char*, int>(io_file, la, lp, ra.c_str(), rp);
    }
    catch (std::exception& ex)
    {
        throw tnt::IOReset(ex.what());
    }
    catch (...)
    {
        throw tnt::IOReset("Unknown error");
    }
}

} // namespace factory
