
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

#include "load_sensors.hpp"

#include "util/pugixml.hpp"

#include "log.hpp"

namespace drop {

std::unordered_map<std::string, gal::SensorResource> load_sensors(const pugi::xml_node& root)
{
    std::unordered_map<std::string, gal::SensorResource> sensors;

    for (const auto& r : root.children("SensorResource"))
    {
        gal::SensorResource res;

        res.id = r.attribute("id").as_string();
        res.type = static_cast<gal::EntitySensorDataType>(r.attribute("type").as_int());
        res.description = r.child("Description").child_value();
        res.scale = static_cast<gal::EntitySensorDataScale>(r.attribute("scale").as_int());
        res.precision = static_cast<gal::EntitySensorPrecision>(r.attribute("precision").as_int());
        res.refresh_rate = static_cast<gal::EntitySensorValueUpdateRate>(r.attribute("refresh_rate").as_int());

        try
        {
            res.max_value = std::stoll(r.attribute("max").as_string());
        }
        catch (std::exception& ex)
        {
            tnt::Log::error("load_sensors error (", res.id, "): ", ex.what(), " - attribute: ==>", r.attribute("max").as_string(), "<==");
        }
        
        sensors.emplace(res.id, res);
    }

    return sensors;
}

} // namespace drop
