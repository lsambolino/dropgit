
/*

Copyright (c) 2013, Sergio Mangialardi (sergio@reti.dist.unige.it), Fabio Podda (fabio@reti.dist.unige.it), Chiara Lombardo (chiara@reti.dist.unige.it)
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

#ifndef DROP_GAL_DEMO_LCP_HPP_
#define DROP_GAL_DEMO_LCP_HPP_

#include <string>
#include <map>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

#include "activity/concurrent_activity.hpp"

#include "gal_drop/local_control_policy_task.hpp"

#include "router/port_info.hpp"

#include "thread.hpp"

namespace pugi {

class xml_node;

} // namespace pugi

namespace drop {

class DemoLCP: public virtual LocalControlPolicyTask, private tnt::ConcurrentActivity
{
    struct Slot
    {
        int voltage;
        int freq;
        int active_cores[4];
        std::string command;
    };

    using SlotTable = std::map<int, Slot>;
    using ProfilesTable = std::map<int, SlotTable>;
public:
    DemoLCP() = default;
    virtual ~DemoLCP() = default;

    virtual void initialize() override;
    virtual void finalize() override;

    virtual gal::ReturnCode discovery(const std::string& resource_id,
                                      bool committed,
								      gal::LogicalResource* resource,
                                      gal::LogicalResourceContainer* logical_resources,
                                      gal::PhysicalResourceContainer* physical_resources,
                                      gal::SensorResourceContainer* sensor_resources,
                                      gal::PowerStateContainer* power_states,
                                      gal::OptimalConfigContainer* edl) override;

    virtual gal::ReturnCode provisioning(const std::string& resource_id, gal::PowerStateId power_state_id) override;
    virtual gal::ReturnCode release(const std::string& resource_id) override;
    virtual gal::ReturnCode monitor_state(const std::string& resource_id, bool committed, gal::PowerStateId& power_state_id) override;
    virtual gal::ReturnCode monitor_history(const std::string& resource_id, gal::PowerStateHistoryContainer* history) override;
    virtual gal::ReturnCode monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp) override;
    virtual gal::ReturnCode monitor_sensor_history(const std::string& resource_id, gal::SensorHistoryContainer* history) override;
    virtual gal::ReturnCode commit(const std::string& resource_id) override;
    virtual gal::ReturnCode rollback(const std::string& resource_id) override;
private:
    void config();
    void load_slots(const pugi::xml_node& root);

    void change_slot(int num);
    void do_change_slot(const Slot& slot);
    
    bool set_configuration(uint32_t freq, uint32_t volt, uint32_t* dv);

    int cpu_load();

    void run();
    void update_stats();
private:
    int current_slot_        =  0;
    int default_profile_     = -1;
    int provisioned_profile_ = -1;
    int committed_profile_   = -1;

    int current_voltage_;
    uint32_t current_freqs_[8];

    tnt::Thread run_thread_;
    tnt::Thread stats_thread_;

    ProfilesTable profiles_;
};

} // namespace drop

#endif
