
#include <iostream>
#include <fstream>
#include <vector>

#include <boost/program_options.hpp>

#include "forward.hpp"

namespace po = boost::program_options;

int get_num_cores()
{
    auto online_cpus_path = "/sys/devices/system/cpu/online";
    std::ifstream file(online_cpus_path);

    if (!file)
    {
        return 0;
    }

    std::string line;
    std::getline(file, line);
    std::stringstream ss(line);
    int num = 0;

    if (line.find("-") != std::string::npos)
    {
        std::string t;

        std::getline(ss, t, '-');
        int s = std::stoi(t);
        std::getline(ss, t, '-');
        int e = std::stoi(t);
        
        num = e - s;

        if (s == 0)
        {
            ++num;
        }
    }
    else if (line.find(",") != std::string::npos)
    {
        std::string t;

        while (std::getline(ss, t, ','))
        {
            ++num;
        }
    }
    else
    {
        return 0;
    }

    return num;
}

int main(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
        
    std::string name;
    int n_cores;
    int start_core;
    int delay;

    auto num_cores = get_num_cores();

    desc.add_options()
    ("help,h", "produce help message")
    ("interface,i", po::value<std::vector<std::string>>(), "Interfaces")
    ("num-cores,n", po::value<int>(&n_cores)->default_value(num_cores), "Number of active cores")
    ("start-core,s", po::value<int>(&start_core)->default_value(0), "Index of first cores")
    ("delay,d", po::value<int>(&delay)->default_value(0), "Delay in seconds among threads start");

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

    if (n_cores == 0)
    {
        std::cerr << "Invalid number of cores selected" << std::endl;
        std::exit(-1);
    }

    if (start_core + n_cores > num_cores)
    {
        std::cerr << "Invalid number of cores selected" << std::endl;
        std::exit(-1);
    }

    start(vm["interface"].as<std::vector<std::string>>(), start_core, n_cores, std::chrono::seconds(delay));
}