
#include <iostream>
#include <exception>
#include <memory>
#include <future>
#include <cstdlib>
#include <cstdio>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "platform.hpp"

#if !defined(TNT_PLATFORM_WIN32)

#include <unistd.h>
#include <sys/types.h>

#endif

#include "activity/control_element.hpp"

#include "parser/ce_parser.hpp"

#include "util/configuration.hpp"
#include "util/path.hpp"
#include "util/io_factory.hpp"

#include "log.hpp"
#include "async.hpp"
#include "application.hpp"

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
            drop::parser::CeParser parser(std::cin, std::cout);
            parser();
        });

        std::vector<std::future<void>> activities;

        activities.push_back(tnt::async([] ()
        {
            drop::activity::ControlElement ce;
            ce();
        }));

        tnt::Application::run();

        tnt::Log::info(colors::blue, "Exiting CE service, please wait for termination...");

        for (auto& a : activities)
        {
            try
            {
                a.get();
            }
            catch (std::exception& ex)
            {
                tnt::Log::error("CE main error: ", ex.what(), " (", tnt::get_name(ex), ")");
            }
            catch (...)
            {
                tnt::Log::error("CE main error: Unknown exception");
            }
        }
    }
    catch (std::exception& ex)
    {
        std::cerr << "CE main error: " << ex.what() << " (" << tnt::get_name(ex) << ")" << std::endl;
    }
    catch (...) 
    {
        std::cerr << "CE Unexpected error" << std::endl;
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
    pt::read_xml(drop::util::config_file_path("ce"), config_file);

    try
    {
        tnt::Configuration::read(config_file.get_child("ce"));
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    tnt::Configuration::set("log.level", tnt::parse_log_level(log_level));
}
