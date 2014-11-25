
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

#ifndef DROP_EVENT_HISTORY_REQUESTS_HPP_
#define DROP_EVENT_HISTORY_REQUESTS_HPP_

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <cstdint>

#include <boost/circular_buffer.hpp>

#include "event/functional_request.hpp"
#include "event/gal/data.hpp"

namespace drop {
namespace event {

using ElementsTrafficRequest = FunctionalRequest<void(const std::map<std::string, event::SensorData>&)>;
using ElementsConsumptionRequest = FunctionalRequest<void(const std::map<std::string, event::SensorData>&)>;
using ElementsCoresRequest = FunctionalRequest<void(const std::map<std::string, event::SensorData>&)>;
using ElementsVoltageRequest = FunctionalRequest<void(const std::map<std::string, event::SensorData>&)>;
using ElementsFrequenciesRequest = FunctionalRequest<void(const std::map<std::string, event::SensorData>&)>;

using DeviceTrafficRequest = FunctionalRequest<void(const event::SensorData&)>;

using ElementsTrafficHistoryRequest = FunctionalRequest<void(const std::map<std::string, boost::circular_buffer<event::SensorData>>&)>;
using ElementsConsumptionHistoryRequest = FunctionalRequest<void(const std::map<std::string, boost::circular_buffer<event::SensorData>>&)>;
using ElementsCoresHistoryRequest = FunctionalRequest<void(const std::map<std::string, boost::circular_buffer<event::SensorData>>&)>;
using ElementsVoltageHistoryRequest = FunctionalRequest<void(const std::map<std::string, boost::circular_buffer<event::SensorData>>&)>;
using ElementsFrequenciesHistoryRequest = FunctionalRequest<void(const std::map<std::string, boost::circular_buffer<event::SensorData>>&)>;

using DeviceTrafficHistoryRequest = FunctionalRequest<void(const boost::circular_buffer<event::SensorData>&)>;

}  // namespace event
}  // namespace drop

#endif
