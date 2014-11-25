
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

#ifndef DROP_ACTIVITY_CE_LOCAL_CONTROL_POLICY_HPP_
#define DROP_ACTIVITY_CE_LOCAL_CONTROL_POLICY_HPP_

#include <unordered_map>
#include <memory>
#include <vector>
#include <chrono>
#include <atomic>
#include <unordered_set>
#include <map>

#include <boost/circular_buffer.hpp>

#include "activity/concurrent_activity.hpp"
#include "activity/gal/se_lcp.hpp"

#include "gal/green_standard_interface.hpp"

#include "thread_safe_queue.hpp"
#include "thread.hpp"

namespace pugi {

class xml_node;

} // namespace pugi

namespace tnt {

struct Protocol;

} // namespace tnt

namespace drop {
namespace ce {

class ServiceElement;

} // namespace ce

struct LocalControlPolicyTask;

namespace activity {

class ControlElementLCP: public gal::GreenStandardInterface, private tnt::ConcurrentActivity
{
    using PowerStateId = gal::PowerState::PowerStateId;

    struct Element
    {
        std::string name;
        bool is_fe;
        bool is_xlp;
    };

    struct ExternalInterface
    {
        std::string name;
        int index;
        int port;
    };
public:
    ControlElementLCP();
    ~ControlElementLCP();

    virtual gal::ReturnCode discovery(const std::string& resource_id,
                                      bool committed,
                                      gal::LogicalResource* resource,
                                      gal::LogicalResourceContainer* logical_resources,
                                      gal::PhysicalResourceContainer* physical_resources,
                                      gal::SensorResourceContainer* sensor_resources,
                                      gal::PowerStateContainer* power_states,
                                      gal::OptimalConfigContainer* edl) override;
                 
    virtual gal::ReturnCode provisioning(const std::string& resource_id, PowerStateId power_state_id) override;
    virtual gal::ReturnCode release(const std::string& resource_id) override;
    virtual gal::ReturnCode monitor_state(const std::string& resource_id, bool committed, PowerStateId& power_state_id) override;
    virtual gal::ReturnCode monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* history) override;
    virtual gal::ReturnCode monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp) override;
    virtual gal::ReturnCode monitor_sensor_history(const std::string& resource_id, gal::SensorHistoryContainer* history) override;
    virtual gal::ReturnCode commit(const std::string& resource_id) override;
    virtual gal::ReturnCode rollback(const std::string& resource_id) override;

    void operator()();
private:
    void config();

    void configure_service_element(const std::shared_ptr<ce::ServiceElement>& se, std::map<std::string, std::vector<std::pair<std::string, gal::SensorResource>>>& sensors);

    void fill_device_power_states(gal::PowerStateContainer* power_states);
    void fill_interface_power_states(gal::PowerStateContainer* power_states);
    void get_power_states(gal::PowerStateContainer* power_states, std::unordered_map<PowerStateId, gal::PowerState> power_states_);

    bool is_interface_name(const std::string& name);

    gal::ReturnCode device_discovery(bool committed,
                                     gal::LogicalResource* resource,
                                     gal::LogicalResourceContainer* logical_resources,
                                     gal::PhysicalResourceContainer* physical_resources,
                                     gal::SensorResourceContainer* sensor_resources,
                                     gal::PowerStateContainer* power_states,
                                     gal::OptimalConfigContainer* edl);

    gal::ReturnCode device_provisioning(PowerStateId power_state_id);
    gal::ReturnCode device_commit();
    gal::ReturnCode device_release();
    gal::ReturnCode device_monitor_state(bool committed, PowerStateId& power_state_id);
    gal::ReturnCode device_monitor_history(gal::PowerStateHistoryContainer* history);
    gal::ReturnCode device_monitor_sensor(const std::string& name, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp);
    gal::ReturnCode device_rollback();

    gal::ReturnCode interface_discovery(const std::string& name,
                                        bool committed,
                                        gal::LogicalResource* resource,
                                        gal::SensorResourceContainer* sensor_resources,
                                        gal::PowerStateContainer* power_states,
                                        gal::OptimalConfigContainer* edl);

    gal::ReturnCode interface_provisioning(const std::string& name, PowerStateId power_state_id);
    gal::ReturnCode interface_commit(const std::string& name);
    gal::ReturnCode interface_release(const std::string& name);
    gal::ReturnCode interface_monitor_state(const std::string& name, bool committed, PowerStateId& power_state_id);
    gal::ReturnCode interface_monitor_history(const std::string& name, gal::PowerStateHistoryContainer* history);
    gal::ReturnCode interface_monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp);
    gal::ReturnCode interface_rollback(const std::string& name);

    gal::ReturnCode lcp_forward(const std::string& id, std::function<gal::ReturnCode(ServiceElementLCP&)> func);

    void read_elements_sensor(const std::string& name, const std::vector<std::pair<std::string, gal::SensorResource>>& sensors);

    void read_elements_traffic();
    void read_elements_consumption();
    void read_elements_cores();
    void read_xlp_power_management();
    void read_device_traffic();
    void read_interfaces_traffic();
private:
    std::atomic_bool running_;

    const int history_size_;

    PowerStateId device_default_power_state_     = -1;
    PowerStateId interfaces_default_power_state_ = -1;

    PowerStateId device_provisioned_power_state_ = -1;
    PowerStateId device_committed_power_state_   = -1;

    std::shared_ptr<LocalControlPolicyTask> lcp_task_;

    std::vector<Element> elements_;
    std::unordered_map<std::string, ExternalInterface> interfaces_;

    std::unordered_map<std::string, ServiceElementLCP> service_elements_lcps_;

    std::unordered_map<PowerStateId, gal::PowerState> device_power_states_;
    std::unordered_map<PowerStateId, gal::PowerState> interfaces_power_states_;

    std::unordered_map<std::string, gal::SensorResource> device_sensors_;
    std::unordered_map<std::string, gal::SensorResource> interfaces_sensors_;

    std::unordered_map<std::string, PowerStateId> interfaces_provisioned_power_states_;
    std::unordered_map<std::string, PowerStateId> interfaces_committed_power_states_;

    std::vector<gal::PowerStateHistory> device_power_states_history_;
    std::unordered_map<std::string, std::vector<gal::PowerStateHistory>> interfaces_power_states_history_;

    std::mutex device_traffic_guard_;
    boost::circular_buffer<gal::SensorHistory> device_traffic_;

    std::mutex elements_traffic_guard_;
    std::map<std::string, boost::circular_buffer<gal::SensorHistory>> elements_traffic_;

    std::mutex elements_consumption_guard_;
    std::map<std::string, boost::circular_buffer<gal::SensorHistory>> elements_consumption_;

    std::mutex elements_cores_guard_;
    std::map<std::string, boost::circular_buffer<gal::SensorHistory>> elements_cores_;

    std::mutex interfaces_traffic_guard_;
    std::map<std::string, boost::circular_buffer<gal::SensorHistory>> interfaces_traffic_;

    std::mutex elements_voltages_guard_;
    std::map<std::string, boost::circular_buffer<gal::SensorHistory>> elements_voltages_;

    std::mutex elements_frequencies_guard_;
    std::map<std::string, std::map<int, boost::circular_buffer<gal::SensorHistory>>> elements_frequencies_;
};

} // namespace activity
} // namespace drop

#endif
