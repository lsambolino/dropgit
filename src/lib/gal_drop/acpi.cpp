
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

#include "acpi.hpp"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <fstream>

#include <boost/format.hpp>

#include "exception/drop_exception.hpp"

#include "util/configuration.hpp"

#include "math.hpp"
#include "log.hpp"

namespace drop {
namespace acpi {
namespace {

const char* available_frequencies_path = "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_available_frequencies";
const char* available_latencies_path = "/proc/acpi/processor/P%03u/power";
const char* related_cpus_path = "/sys/devices/system/cpu/cpu%u/topology/core_siblings_list";
const char* scaling_governor = "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_governor";
const char* frequency_path = "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_setspeed";
const char* current_frequency_path = "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_cur_freq";
const char* latency_path = "/sys/devices/system/cpu/cpu%u/cpufreq/core_latency";
const char* online_cpus_path = "/sys/devices/system/cpu/online";
const char* smp_affinity_path = "/proc/irq/%u/smp_affinity";
const char* interrupts_path = "/proc/interrupts";
const char* net_statistics_path = "/proc/net/dev";
const char* state_latency_path = "/sys/devices/system/cpu/cpu%u/cpuidle/state%u/latency";
const char* dfs_path = "/dev/xlp_power_freq";
const char* voltage_path = "/dev/xlp_power_volt";
const char* suspend_file = "/sys/power/state";

std::vector<int> get_values(std::ifstream& file)
{
    std::vector<int> values;
    std::string line;
    std::getline(file, line);
    std::stringstream ss(line);

    if (line.find("-") != std::string::npos)
    {
        std::string t;

        std::getline(ss, t, '-');
        int s = std::stoi(t);
        std::getline(ss, t, '-');
        int e = std::stoi(t);
        values.reserve(e - s);

        for (int i=s; i<=e; ++i)
        {
            values.push_back(i);
        }
    }
    else if (line.find(",") != std::string::npos)
    {
        std::string t;

        while (std::getline(ss, t, ','))
        {
            values.push_back(std::stoi(t));
        }
    }
    else
    {
        throw tnt::ReadFileError();
    }

    return values;
}

int seek_irq(const std::string& iface)
{
    if (iface.empty())
    {
        throw tnt::EmptyArgument("iface");
    }
    
    std::ifstream file(interrupts_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string line;
    std::string pattern = iface + "-TxRx-";

    while (std::getline(file, line))
    {
        if (line.rfind(pattern) == std::string::npos)
        {
            continue;
        }

        std::stringstream ss(line);
        int irq;

        ss >> irq;

        return irq;
    }

    throw std::invalid_argument("Error: the iface does not exists.");
}

int seek_irq(const std::string& iface, int core)
{
    if (iface.empty())
    {
        throw tnt::EmptyArgument("iface");
    }
    
    std::ifstream file(interrupts_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string line;
    std::string pattern = iface + "-" + std::to_string(core);

    while (std::getline(file, line))
    {
        if (line.rfind(pattern) == std::string::npos)
        {
            continue;
        }

        std::stringstream ss(line);
        int irq;

        ss >> irq;

        return irq;
    }

    throw std::invalid_argument("Error: the iface does not exists.");
}

void set_irq_affinity(int irq, int affinity)
{
    std::string path = boost::str(boost::format(smp_affinity_path) % irq);
    std::ofstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    file << std::hex << affinity; // affinity is an hexadecimal value

    if (file.fail())
    {
        throw tnt::WriteFileError();
    }
}

int current_irq_affinity(int irq)
{
    std::string path = boost::str(boost::format(smp_affinity_path) % irq);
    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    int tmp = 0;
    
    file >> std::hex >> tmp; // affinity is an hexadecimal value

    if (file.fail())
    {
        throw tnt::ReadFileError();
    }

    return tmp;
}

} // namespace

void set_dfs(int core, int dfs)
{
    std::ofstream file(dfs_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    file << core << " " << dfs << std::endl;
}

void set_voltage(int voltage)
{
    std::ofstream file(voltage_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    file << std::hex << voltage << std::endl;
}

std::vector<int> get_cores()
{
    std::ifstream file(online_cpus_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    return get_values(file);
}

std::vector<int> get_related_cores(int core)
{
    std::vector<int> values;
    std::string path = boost::str(boost::format(related_cpus_path) % core);

    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    return get_values(file);
}

std::vector<int> get_available_frequencies(int core)
{
    std::vector<int> values;
    std::string path = boost::str(boost::format(available_frequencies_path) % core);
    
    std::ifstream file(path);
    
    if (!file)
    {
        throw tnt::OpenFileError();
    }

    int value;

    while (!file.eof())
    {
        file >> value;

        if (file.fail())
        {
            // string ends with a "blank space",
            // so use fail to check end of file
            break;
        }

        values.push_back(value);
    }

    std::reverse(std::begin(values), std::end(values));

    return values;
}

std::vector<int> get_available_latencies(int core)
{
    std::vector<int> values;

    // Add first latency for status0 (C0). This is not in "proc file system"
    values.push_back(0);

    // In this path the CPU numbering starts from 1 (cpu_0 = 1, cpu_1 = 2 ...)
    std::string path = boost::str(boost::format(available_latencies_path) % (core + 1));
    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string line;

    while (std::getline(file, line))
    {
        size_t s = line.find("latency[");
        
        if (s == std::string::npos)
        {
            continue;
        }

        s += 8;

        size_t e = line.find("]", s);

        if (e == std::string::npos)
        {
            throw tnt::ReadFileError();
        }

        values.push_back(std::stoi(line.substr(s, e - s)));
    }

    return values;
}

int current_frequency(int core)
{
    std::string path = boost::str(boost::format(current_frequency_path) % core);
    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }
    
    int value;
    file >> value;

    return value;
}

void set_frequency(int core, int frequency)
{
    std::string path = boost::str(boost::format(frequency_path) % core);
    std::ofstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    file << frequency;

    if (file.fail())
    {
        throw tnt::WriteFileError();
    }
}

int state_latency(int core, int state)
{
    std::string path = boost::str(boost::format(state_latency_path) % core % state);
    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    int value;
    file >> value;

    return value;
}

int c1_latency(int core)
{
    return state_latency(core, 1);
}

int c3_latency(int core)
{
    return state_latency(core, 3);
}

int current_latency(int core)
{
    std::string path = boost::str(boost::format(latency_path) % core);
    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    int latency = 0;

    file >> latency;

    if (file.fail())
    {
        throw tnt::ReadFileError();
    }

    return latency;
}

void set_latency(int core, int latency)
{
    std::string path = boost::str(boost::format(latency_path) % core);
    std::ofstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    file << latency << std::flush;

    if (file.fail())
    {
        throw tnt::WriteFileError();
    }
}

std::string current_governor(int core)
{
    std::string path = boost::str(boost::format(scaling_governor) % core);
    std::ifstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    std::string governor;
    file >> governor;

    return governor;
}

void set_governor(int core, const std::string& governor)
{
    if (governor.empty())
    {
        throw tnt::EmptyArgument("governor");
    }

    std::string path = boost::str(boost::format(scaling_governor) % core);
    std::ofstream file(path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    file << governor;

    if (file.fail())
    {
        throw tnt::WriteFileError();
    }
}

int get_affinity(const std::string& iface, int queue)
{
    auto driver = tnt::Configuration::get("adapter.driver").as<std::string>();

    int irq = (driver == "mlx") ? 
        seek_irq(iface, queue) : // For mlx
        seek_irq(iface); // For igb

    int curr_affinity = current_irq_affinity(irq);
    
    return tnt::log2(curr_affinity);
}

void set_affinity(const std::string& iface, CoreId core)
{
    auto driver = tnt::Configuration::get("adapter.driver").as<std::string>();

    int irq = (driver == "mlx") ? 
        seek_irq(iface, core) : // For mlx
        seek_irq(iface); // For igb

    set_irq_affinity(irq, std::pow(2, core));
}

uint64_t get_packets(const std::string& iface)
{
    if (iface.empty())
    {
        throw tnt::EmptyArgument("iface");
    }

    std::ifstream file(net_statistics_path);

    if (!file)
    {
        throw tnt::OpenFileError();
    }

    uint64_t tot_packets = 0;
    std::string line;

    while (std::getline(file, line))
    {
        size_t s = line.find(iface);
        
        if (s == std::string::npos)
        {
            continue;
        }

        size_t e = line.find(":", s + iface.size());

        if (s == std::string::npos)
        {
            //throw tnt::SyntaxError("Error: File has wrong format.");
        }

        line = line.substr(e + 1);

        std::stringstream ss(line);
        std::string tmp;
        uint64_t value;

        ss >> tmp;
        ss >> value;

        tot_packets += value;

        ss >> tmp;
        ss >> value;

        tot_packets += value;

        break;
    }

    return tot_packets;
}

void set_state(State state)
{
    std::fstream file(suspend_file);

	if (!file)
	{
		throw tnt::OpenFileError();
	}

	switch (state)
	{
	case State::S1:
		file << "standby";
		break;
	case State::S3:
		file << "mem";
		break;
    case State::S4:
		file << "disk";
		break;
    case State::S5:
        std::system("/sbin/halt -p");
		break;
	case State::S0:
    default:
		break; // Do nothing
	}
}

} // namespace acpi
} // namespace drop
