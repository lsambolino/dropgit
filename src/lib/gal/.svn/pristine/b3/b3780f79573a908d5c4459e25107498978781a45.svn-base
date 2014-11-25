
/*

Copyright (c) 2012, Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#ifndef GREEN_STANDARD_INTERFACE_HPP_
#define GREEN_STANDARD_INTERFACE_HPP_

#include <vector>
#include <string>

#include "gal/power_state.hpp"
#include "gal/logical_resource.hpp"
#include "gal/sensor_resource.hpp"
#include "gal/physical_resource.hpp"
#include "gal/optimal_config.hpp"

namespace gal {

enum class ReturnCode: int32_t
{
    // GAL_UNDEFINED is very helpful for coding, maybe it could be useful to send to the client, I'm not sure.
    GalUndefined = -2,
    // GAL_FAILURE should be used when an error is -1
    GalFailure = -1,
    // GAL_SUCCESS should be used in case of success
    GalSuccess = 0,
    // GAL_ERROR should be used in case of general error
    GalError = 1,
    // GAL_NOT_IMPLEMENTED should be returned when the power management function is not implemented on the selected resouce_id
    GalNotImplemented = 2,
    // GAL_RESOURCE_NOT_FOUND should be returned when the resource_id provided on the request is non-existent
    GalResourceNotFound = 4,
    // GAL_RESOURCE_NOT_AVAILABLE should be returned when the resource_id provided on the request is not available in the moment
    GalResourceNotAvailable = 8,
    // GAL_PROTOCOL_ERROR should be returned when remote server answers with an invalid return code
    GalProtocolError = 16
};

using PowerStateId = PowerState::PowerStateId;

using PowerStateContainer = std::vector<PowerState>;
using LogicalResourceContainer = std::vector<LogicalResource>;
using SensorResourceContainer = std::vector<SensorResource>;
using PhysicalResourceContainer = std::vector<PhysicalResource>;
using OptimalConfigContainer = std::vector<OptimalConfig>;
using PowerStateHistoryContainer = std::vector<PowerStateHistory>;
using SensorHistoryContainer = std::vector<SensorHistory>;

struct GreenStandardInterface
{
    virtual ~GreenStandardInterface() = default;

    virtual ReturnCode discovery(const std::string& resource_id,
                                 bool committed,
								 LogicalResource* resource,
                                 LogicalResourceContainer* logical_resources,
                                 PhysicalResourceContainer* physical_resources,
                                 SensorResourceContainer* sensor_resources,
                                 PowerStateContainer* power_states,
                                 OptimalConfigContainer* edl) = 0;

    virtual ReturnCode provisioning(const std::string& resource_id, PowerStateId power_state_id) = 0;
    virtual ReturnCode release(const std::string& resource_id) = 0;
    virtual ReturnCode monitor_state(const std::string& resource_id, bool committed, PowerStateId& power_state_id) = 0;
    virtual ReturnCode monitor_history(const std::string& resource_id, PowerStateHistoryContainer* history) = 0;
    virtual ReturnCode monitor_sensor(const std::string& resource_id, EntitySensorStatus& oper_status, EntitySensorValue& sensor_value, EntitySensorTimeStamp& value_timestamp) = 0;
    virtual ReturnCode monitor_sensor_history(const std::string& resource_id, SensorHistoryContainer* history) = 0;
    virtual ReturnCode commit(const std::string& resource_id) = 0;
    virtual ReturnCode rollback(const std::string& resource_id) = 0;
};

} // namespace gal

#endif
