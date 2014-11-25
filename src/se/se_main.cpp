
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

#include <iostream>
#include <memory>
#include <fstream>
#include <future>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "platform.hpp"

#if !defined(TNT_PLATFORM_WIN32)

#include <unistd.h>
#include <sys/types.h>

#endif

#include "activity/service_element.hpp"
#include "activity/gal/service_element_lcp.hpp"
#include "activity/http/http_server.hpp"
#include "activity/network/connect_control_element.hpp"

#include "parser/se_parser.hpp"

#include "event/network/forwarding_started.hpp"

#include "exception/drop_exception.hpp"

#include "util/serialization.hpp"
#include "util/configuration.hpp"
#include "util/pugixml.hpp"
#include "util/path.hpp"
#include "util/io_factory.hpp"

#include "gal_drop/acpi.hpp"

#include "application.hpp"
#include "async.hpp"
#include "log.hpp"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

void read_configuration(int argc, char* argv[]);

int main(int argc, char* argv[])
{
#if !defined(TNT_PLATFORM_WIN32)

    if (geteuid() != 0)
    {
        std::cerr << "The program requires elevated privileges to run" << std::endl;

        return -1;
    }

#endif

    try
    {
        read_configuration(argc, argv);

        tnt::Log::level(tnt::Configuration::get("log.level").as<tnt::LogLevel>());

        tnt::async([] ()
        {
            drop::parser::SeParser(std::cin, std::cout)();
        });

        std::vector<std::future<void>> activities;

        activities.push_back(tnt::async([] ()
        {
            drop::activity::ServiceElement()();
        }));

        activities.push_back(tnt::async([] ()
        {
            tnt::activity::HttpServer server;
            server(factory::create_io_end_point("httpd"), "html");
        }));

        activities.push_back(tnt::async([] ()
        {
            drop::activity::ServiceElementLCP()();
        }));

        activities.push_back(tnt::async([] ()
        {
            drop::activity::ConnectControlElement()();
        }));

        tnt::Application::run();

        tnt::Log::info(colors::blue, "Exiting SE service, please wait for termination...");

        for (auto& a : activities)
        {
            try
            {
                a.get();
            }
            catch (std::exception& ex)
            {
                tnt::Log::error("SE main error: ", ex.what(), " (", tnt::get_name(ex), ")");
            }
            catch (...)
            {
                tnt::Log::error("SE main error: Unknown exception");
            }
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error(R"(Unrecoverable error: ")", ex.what(), R"(" exiting service.)");
    }
    catch (...)
    {
        tnt::Log::error("Unrecoverable error, exiting service.");
    }
}

void read_configuration(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
        
    std::string log_level;

    desc.add_options()
    ("help,h", "produce help message")
    ("debug,d", po::value<std::string>(&log_level)->default_value("None"), "debug level");

    po::variables_map vm;

    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        std::cout << desc << std::endl;
        std::exit(-1);
    }

    if (vm.count("help") != 0)
    {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    pt::ptree config_file;
    pt::read_xml(drop::util::config_file_path("se"), config_file);

    try
    {
        tnt::Configuration::read(config_file.get_child("se"));
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    tnt::Configuration::set("log.level", tnt::parse_log_level(log_level));
}
