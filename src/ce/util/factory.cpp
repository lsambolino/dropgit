
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

#include "factory.hpp"

#include <boost/asio.hpp>

#include "exception/exception.hpp"

#include "io/netlink_io.hpp"

#include "protocol/netlink/kernel_netlink.hpp"
#include "protocol/netlink/netlink_deth.hpp"
#include "protocol/drop/drop.hpp"
#include "protocol/drop_xml/drop_xml.hpp"

#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/pugixml.hpp"

#include "dynamic_module.hpp"
#include "log.hpp"

namespace factory {

std::shared_ptr<drop::LocalControlPolicyTask> create_lcp_process()
{
    auto lcp_class = tnt::Configuration::get("lcp.class").as<std::string>();

    if (lcp_class.empty())
    {
        return nullptr;
    }

    auto lcp_file = drop::util::full_path(lcp_class + ".so");
    
    return tnt::module::load<drop::LocalControlPolicyTask>(lcp_file);
}

std::shared_ptr<activity::ProfileScheduler> create_scheduler()
{
    auto scheduler_class = tnt::Configuration::get("scheduler.class").as<std::string>();

    if (scheduler_class.empty())
    {
        return nullptr;
    }

    auto scheduler_file = drop::util::full_path(scheduler_class + ".so");
    auto conf_file = drop::util::full_path(tnt::Configuration::get("scheduler.conf").as<std::string>());

    return tnt::module::load<activity::ProfileScheduler>(scheduler_file, conf_file.c_str());
}

std::shared_ptr<tnt::Protocol> create_kernel_protocol()
{
    return std::make_shared<drop::protocol::KernelNetlinkProtocol>();
}

std::shared_ptr<tnt::Protocol> create_deth_protocol()
{
    return std::make_shared<drop::protocol::NetlinkDethProtocol>(std::make_shared<drop::io::NetlinkIO>(drop::io::NetlinkSocketProtocol::NetlinkDeth));
}

std::shared_ptr<tnt::Protocol> create_drop_protocol(const std::shared_ptr<tnt::IO>& io)
{
    return std::make_shared<drop::protocol::DropXml>(io);
}

} // namespace factory
