
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <cstddef>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <arpa/inet.h>

enum gal_command
{
    COMMAND_INVALID = 0,
    COMMAND_DISCOVERY,
    COMMAND_PROVISIONING,
    COMMAND_RELEASE,
    COMMAND_MONITOR_SENSOR,
    COMMAND_MONITOR_STATE,
    COMMAND_MONITOR_HISTORY,
    COMMAND_COMMIT,
    COMMAND_ROLLBACK
};

enum gal_return_code
{
    // GAL_UNDEFINED is very helpful for coding, maybe it could be useful to send to the client, I'm not sure.
    GAL_UNDEFINED = -2,
    // GAL_FAILURE should be used when an error is -1
    GAL_FAILURE = -1,
    // GAL_SUCCESS should be used in case of success
    GAL_SUCCESS = 0,
    // GAL_ERROR should be used in case of general error
    GAL_ERROR = 1,
    // GAL_NOT_IMPLEMENTED should be returned when the power management function is not implemented on the selected resouce_id
    GAL_NOT_IMPLEMENTED = 2,
    // GAL_RESOURCE_NOT_FOUND should be returned when the resource_id provided on the request is non-existent
    GAL_RESOURCE_NOT_FOUND = 4,
    // GAL_RESOURCE_NOT_AVAILABLE should be returned when the resource_id provided on the request is not available in the moment
    GAL_RESOURCE_NOT_AVAILABLE = 8,
    // GAL_PROTOCOL_ERROR should be returned when remote server answers with an invalid return code
    GAL_PROTOCOL_ERROR = 16
};

struct LogicalResource
{
    std::string id;
    std::string description;
};

struct PhysicalResource
{
    std::string id;
    std::string description;
    std::vector<std::string> depends_on;
    std::vector<std::string> used_by;
    uint16_t resource_class;
    uint16_t num_children;
};

enum class EntitySensorDataType
{
    other = 1,  // a measure other than those listed below
    unknown,    // unknown measurement, or arbitrary, relative numbers
    voltsAC,    // electric potential
    voltsDC,    // electric potential
    amperes,    // electric current
    watts,      // power
    hertz,      // frequency
    celsius,    // temperature
    percentRH,  // percent relative humidity
    rpm,        // shaft revolutions per minute
    cmm,        // cubic meters per minute (airflow)
    truthvalue  // value takes { true(1), false(2) }
};

enum class EntitySensorDataScale 
{
    yocto = 1,  // 10^-24
    zepto,      // 10^-21
    atto,       // 10^-18
    femto,      // 10^-15
    pico,       // 10^-12
    nano,       // 10^-9
    micro,      // 10^-6
    milli,      // 10^-3
    units,      // 10^0
    kilo,       // 10^3
    mega,       // 10^6
    giga,       // 10^9
    tera,       // 10^12
    exa,        // 10^15
    peta,       // 10^18
    zetta,      // 10^21
    yotta       // 10^24
};

enum class EntitySensorStatus
{
    ok = 1,
    unavailable,
    nonoperational
};

using EntitySensorPrecision = int32_t;
using EntitySensorValueUpdateRate = uint32_t;
using EntitySensorValue = int32_t;
using EntitySensorTimeStamp = uint32_t;

struct SensorResource
{
    std::string id;
    std::string description;
    EntitySensorDataType type;
    EntitySensorDataScale scale;
    EntitySensorPrecision precision;
    EntitySensorValueUpdateRate refresh_rate;
};

struct CurveState
{
    uint16_t offered_load;
    uint16_t maximum_consumption;

    double maximum_packet_service_time;
};

struct PowerState
{
    uint16_t id;

    uint16_t minimum_power_gain;
    uint16_t power_gain;
    uint16_t lpi_transition_power;
    uint16_t autonomic_ps_steps;
    
    uint64_t wakeup_triggers;

    double autonomic_ps_service_interruption;
    double maximum_packet_throughput;
    double maximum_bit_throughput;
    double wakeup_time;
    double sleeping_time;

    std::vector<double> ps_transition_times;
    std::vector<double> ps_transition_service_interruption_times;

    std::vector<CurveState> autonomic_ps_curves;
};

struct OptimalConfig
{
    uint16_t id;	// the last segment of the resource id
    std::vector<uint16_t> power_states;
};

void usage()
{
    std::cerr << "Usage: ce ADDRESS PORT" << std::endl; 
}

void discovery(int sock, const std::string& id, const sockaddr_in& addr);
void monitor_sensor(int sock, const std::string& id, const sockaddr_in& addr);
void monitor_state(std::map<std::string, int>& power_states, int sock, const std::string& id, const sockaddr_in& addr);
void monitor_history(int sock, const std::string& id, const sockaddr_in& addr);
void provisioning(std::map<std::string, int>& power_states, int sock, const std::string& id, const sockaddr_in& addr);
void commit(int sock, const std::string& id, const sockaddr_in& addr);

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        usage();

        return -1;
    }

    auto addr = sockaddr_in();

    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(argv[2]));

    if (inet_aton(argv[1], &addr.sin_addr) == 0)
    {
        std::cerr << std::strerror(errno) << std::endl;

        return -1;
    }

    auto sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0)
    {
        std::cerr << std::strerror(errno) << std::endl;

        return -1;
    }

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        close(sock);
        std::cerr << std::strerror(errno) << std::endl;

        return -1;
    }

	std::map<std::string, int> power_states_provisioned;
	std::map<std::string, int> power_states_committed;

    while (true)
    {
        std::string buf;
        buf.resize(64);
        
        socklen_t len = sizeof(addr);
        auto ret = recvfrom(sock, &buf[0], buf.size(), 0, reinterpret_cast<sockaddr*>(&addr), &len);

        if (ret < 0)
        {
            std::cerr << std::strerror(errno) << std::endl;

            continue;
        }

		gal_return_code rc = GAL_SUCCESS;
		const char* ptr = buf.data();
		gal_command cmd = COMMAND_INVALID;
		std::string id;

		if (ret < sizeof(gal_command))
		{
			rc = GAL_FAILURE;
		}
		else
		{
			cmd = *reinterpret_cast<const gal_command*>(ptr);
			ptr += sizeof(gal_command);

			id = std::string(ptr);
			ptr += id.length() + 1;
		}

        switch (cmd)
        {
        case COMMAND_DISCOVERY:

			if (id != "0" || id.find("0.eth") != 0)
			{
				rc = GAL_RESOURCE_NOT_FOUND;
				break;
			}

            discovery(sock, id, addr);
            continue;
        case COMMAND_MONITOR_SENSOR:
            monitor_sensor(sock, id, addr);
            continue;
        case COMMAND_MONITOR_STATE:

			if (id == "0")
			{
				rc = GAL_NOT_IMPLEMENTED;
				break;
			}

			if (id.find("0.eth") != 0)
			{
				rc = GAL_RESOURCE_NOT_FOUND;
				break;
			}

            monitor_state(*reinterpret_cast<const bool*>(ptr) ? power_states_committed : power_states_provisioned, sock, id, addr);
            continue;
        case COMMAND_MONITOR_HISTORY:
            monitor_history(sock, id, addr);
            continue;
        case COMMAND_PROVISIONING:
			
			if (id == "0")
			{
				rc = GAL_NOT_IMPLEMENTED;
				break;
			}

			if (id.find("0.eth") != 0)
			{
				rc = GAL_RESOURCE_NOT_FOUND;
				break;
			}

			power_states_provisioned[id] = *reinterpret_cast<const int*>(ptr);
            break;
        case COMMAND_RELEASE:
			
			if (id == "0")
			{
				rc = GAL_NOT_IMPLEMENTED;
				break;
			}

			if (id.find("0.eth") != 0)
			{
				rc = GAL_RESOURCE_NOT_FOUND;
				break;
			}

			power_states_provisioned[id] = 0;
			power_states_committed[id] = 0;
            break;
        case COMMAND_COMMIT:
			
			if (id == "0")
			{
				rc = GAL_NOT_IMPLEMENTED;
				break;
			}

			if (id.find("0.eth") != 0)
			{
				rc = GAL_RESOURCE_NOT_FOUND;
				break;
			}

			power_states_committed[id] = power_states_provisioned[id];
            break;
        case COMMAND_ROLLBACK:
            
			if (id == "0")
			{
				rc = GAL_NOT_IMPLEMENTED;
				break;
			}

			if (id.find("0.eth") != 0)
			{
				rc = GAL_RESOURCE_NOT_FOUND;
				break;
			}

			power_states_provisioned[id] = power_states_committed[id];
            break;
        case COMMAND_INVALID:
        default:
            rc = GAL_UNDEFINED;
            break;
        }

        sendto(sock, &rc, sizeof(rc), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    }
}

std::string create_discovery_reply(const std::string& id);

void discovery(int sock, const std::string& id, const sockaddr_in& addr)
{
    /* Discovery reply packets format
    
               4 bytes
    ____________________________ 
    |                           |
    |     gal response code     | 1st packet
    |___________________________|
    ____________________________ 
    |                           |
    |     packet total size     | 2nd packet
    |___________________________|
    ____________________________ 
    |                           |
    |   num logical resources   | 3rd packet
    |___________________________|
    |                           |
    |     logical resource 0    |
    |___________________________|
    |                           |
    |           ...             | 
    |___________________________|
    |                           |
    |     logical resource n    |
    |___________________________|
    |                           |
    |  num physical resources   |
    |___________________________|
    |                           |
    |           ...             |
    |___________________________|

    */

    gal_return_code rc = GAL_SUCCESS;
    bool valid_id = id == "0";

    if (!valid_id)
    {
        valid_id = id.find("eth") != std::string::npos;

        if (valid_id)
        {
            int i = std::stoi(id.substr(3));
            valid_id = (0 <= i) && (i < 10);
        }
    }

    if (!valid_id)
    {
        rc = GAL_RESOURCE_NOT_FOUND;
        sendto(sock, reinterpret_cast<const char*>(&rc), sizeof(rc), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));

        return;
    }

    auto s = create_discovery_reply(id);
    int size = s.size();

    sendto(sock, reinterpret_cast<const char*>(&rc), sizeof(rc), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    sendto(sock, reinterpret_cast<const char*>(&size), sizeof(size), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    sendto(sock, s.data(), s.size(), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
}

void monitor_sensor(int sock, const std::string& id, const sockaddr_in& addr)
{
    gal_return_code rc = GAL_NOT_IMPLEMENTED;
    sendto(sock, reinterpret_cast<const char*>(&rc), sizeof(rc), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
}

void monitor_state(std::map<std::string, int>& power_states, int sock, const std::string& id, const sockaddr_in& addr)
{
	gal_return_code rc = GAL_SUCCESS;
	auto ps = 0;
	auto it = power_states.find(id);

	if (it != std::end(power_states))
	{
		ps = it->second;
	}

	std::string buffer(reinterpret_cast<const char*>(&ps), sizeof(ps));

    sendto(sock, reinterpret_cast<const char*>(&rc), sizeof(rc), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
	sendto(sock, buffer.data(), buffer.size(), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
}

void monitor_history(int sock, const std::string& id, const sockaddr_in& addr)
{
    gal_return_code rc = GAL_NOT_IMPLEMENTED;
    sendto(sock, reinterpret_cast<const char*>(&rc), sizeof(rc), 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
}

std::string create_discovery_reply(const std::string& id)
{
    std::vector<LogicalResource> logical_resources;
    std::vector<PhysicalResource> physical_resources;
    std::vector<SensorResource> sensor_resources;
    std::vector<PowerState> power_states;
    std::vector<OptimalConfig> edl;

    if (id == "0")
    {
        for (int i=0; i<10; ++i)
        {
            auto lr = LogicalResource();
            lr.id.append("eth").append(std::to_string(i));
            lr.description = "Router network interface";

            logical_resources.push_back(lr);
        }
    }
    else
    {
        auto ps = PowerState();
        ps.id = 0;

        ps.minimum_power_gain = 0;
        ps.maximum_packet_throughput = 333672;
        ps.maximum_bit_throughput = 946518328;
        ps.autonomic_ps_service_interruption = 0.0;

        for (int i=0; i<10; ++i)
        {
            auto cs = CurveState();
            cs.maximum_consumption = i;
            cs.maximum_packet_service_time = i;
            cs.offered_load = i;
            ps.autonomic_ps_curves.push_back(cs);
        }

        power_states.push_back(ps);
        ps.id = 1;
        power_states.push_back(ps);
        ps.id = 2;
        power_states.push_back(ps);
    }

    std::string buffer;

    uint32_t size = logical_resources.size();
    buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& lr : logical_resources)
    {
        buffer.append(lr.id).append(32 - lr.id.size(), '\0');
        buffer.append(lr.description).append(32 - lr.description.size(), '\0');
    }

    size = physical_resources.size();
    buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& pr : physical_resources)
    {
        buffer.append(pr.id).append(32 - pr.id.size(), '\0');
        buffer.append(pr.description).append(32 - pr.description.size(), '\0');

        buffer.append(reinterpret_cast<const char*>(&pr.resource_class), sizeof(pr.resource_class));
        buffer.append(reinterpret_cast<const char*>(&pr.num_children), sizeof(pr.num_children));
    }

    size = sensor_resources.size();
    buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& sr : sensor_resources)
    {
        buffer.append(sr.id).append(32 - sr.id.size(), '\0');
        buffer.append(sr.description).append(32 - sr.description.size(), '\0');

        buffer.append(reinterpret_cast<const char*>(&sr.type), sizeof(sr.type));
        buffer.append(reinterpret_cast<const char*>(&sr.scale), sizeof(sr.scale));
        buffer.append(reinterpret_cast<const char*>(&sr.precision), sizeof(sr.precision));
        buffer.append(reinterpret_cast<const char*>(&sr.refresh_rate), sizeof(sr.refresh_rate));
    }

    size = power_states.size();
    buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));

    const auto ps_size = offsetof(PowerState, sleeping_time) + sizeof(double);

    for (const auto& ps : power_states)
    {
        buffer.append(reinterpret_cast<const char*>(&ps), ps_size);

        // ps_transition_times
        size = ps.ps_transition_times.size();
        buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));
        buffer.append(reinterpret_cast<const char*>(ps.ps_transition_times.data()), size * sizeof(*ps.ps_transition_times.data()));
        
        // ps_transition_service_interruption_times
        size = ps.ps_transition_service_interruption_times.size();
        buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));
        buffer.append(reinterpret_cast<const char*>(ps.ps_transition_service_interruption_times.data()), size * sizeof(*ps.ps_transition_service_interruption_times.data()));
        
        // autonomic_ps_curves
        size = ps.autonomic_ps_curves.size();
        buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));
        buffer.append(reinterpret_cast<const char*>(ps.autonomic_ps_curves.data()), size * sizeof(*ps.autonomic_ps_curves.data()));
    }

    size = edl.size();
    buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& e : edl)
    {
        buffer.append(reinterpret_cast<const char*>(&e.id), sizeof(e.id));

        // power_states
        size = e.power_states.size();
        buffer.append(reinterpret_cast<const char*>(&size), sizeof(size));
        buffer.append(reinterpret_cast<const char*>(e.power_states.data()), size * sizeof(*e.power_states.data()));
    }

    return buffer;
}
