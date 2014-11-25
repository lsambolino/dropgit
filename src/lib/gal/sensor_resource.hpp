
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

#ifndef GAL_SENSOR_INTERFACE_HPP_
#define GAL_SENSOR_INTERFACE_HPP_

#include <string>
#include <utility>
#include <cstdint>
#include <cassert>

namespace gal {

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
    yocto = -24,
    zepto = -21,
    atto  = -18,
    femto = -15,
    pico  = -12,
    nano  = -9, 
    micro = -6, 
    milli = -1, 
    units = 0,  
    kilo  = 3,  
    mega  = 6,  
    giga  = 9,  
    tera  = 12, 
    exa   = 15, 
    peta  = 18, 
    zetta = 21, 
    yotta = 24
};

enum class EntitySensorStatus
{
    ok = 1,
    unavailable,
    nonoperational
};

inline std::string to_string(EntitySensorDataType type)
{
    switch (type)
    {
    case EntitySensorDataType::other:
        return "other";
    case EntitySensorDataType::unknown:
        return "unknown";
    case EntitySensorDataType::voltsAC:
        return "voltsAC";
    case EntitySensorDataType::voltsDC:
        return "voltsDC";
    case EntitySensorDataType::amperes:
        return "amperes";
    case EntitySensorDataType::watts:
        return "watts";
    case EntitySensorDataType::hertz:
        return "hertz";
    case EntitySensorDataType::celsius:
        return "celsius";
    case EntitySensorDataType::percentRH:
        return "percentRH";
    case EntitySensorDataType::rpm:
        return "rpm";
    case EntitySensorDataType::cmm:
        return "cmm";
    case EntitySensorDataType::truthvalue:
        return "truthvalue";
    default:
        assert(false);
    }
}

template <class T> T from_string(const std::string& value);

template <> inline EntitySensorDataType from_string<EntitySensorDataType>(const std::string& type)
{
    if (type == "other") return EntitySensorDataType::other;
    if (type == "unknown") return EntitySensorDataType::unknown;
    if (type == "voltsAC") return EntitySensorDataType::voltsAC;
    if (type == "voltsDC") return EntitySensorDataType::voltsDC;
    if (type == "amperes") return EntitySensorDataType::amperes;
    if (type == "watts") return EntitySensorDataType::watts;
    if (type == "hertz") return EntitySensorDataType::hertz;
    if (type == "celsius") return EntitySensorDataType::celsius;
    if (type == "percentRH") return EntitySensorDataType::percentRH;
    if (type == "rpm") return EntitySensorDataType::rpm;
    if (type == "cmm") return EntitySensorDataType::cmm;
    if (type == "truthvalue") return EntitySensorDataType::truthvalue;

    assert(false);
}

inline std::string to_string(EntitySensorDataScale scale)
{
    switch (scale)
    {
    case EntitySensorDataScale::yocto:
        return "yocto";
    case EntitySensorDataScale::zepto:
        return "zepto";
    case EntitySensorDataScale::atto:
        return "atto";
    case EntitySensorDataScale::femto:
        return "femto";
    case EntitySensorDataScale::pico:
        return "pico";
    case EntitySensorDataScale::nano:
        return "nano";
    case EntitySensorDataScale::micro:
        return "micro";
    case EntitySensorDataScale::milli:
        return "milli";
    case EntitySensorDataScale::units:
        return "units";
    case EntitySensorDataScale::kilo:
        return "kilo";
    case EntitySensorDataScale::mega:
        return "mega";
    case EntitySensorDataScale::giga:
        return "giga";
    case EntitySensorDataScale::tera:
        return "tera";
    case EntitySensorDataScale::exa:
        return "exa";
    case EntitySensorDataScale::peta:
        return "peta";
    case EntitySensorDataScale::zetta:
        return "zetta";
    case EntitySensorDataScale::yotta:
        return "yotta";
    default:
        assert(false);
    }
}

template <> inline EntitySensorDataScale from_string<EntitySensorDataScale>(const std::string& scale)
{
    if (scale == "yocto") return EntitySensorDataScale::yocto;
    if (scale == "zepto") return EntitySensorDataScale::zepto;
    if (scale == "atto") return EntitySensorDataScale::atto;
    if (scale == "femto") return EntitySensorDataScale::femto;
    if (scale == "pico") return EntitySensorDataScale::pico;
    if (scale == "nano") return EntitySensorDataScale::nano;
    if (scale == "micro") return EntitySensorDataScale::micro;
    if (scale == "milli") return EntitySensorDataScale::milli;
    if (scale == "units") return EntitySensorDataScale::units;
    if (scale == "kilo") return EntitySensorDataScale::kilo;
    if (scale == "mega") return EntitySensorDataScale::mega;
    if (scale == "giga") return EntitySensorDataScale::giga;
    if (scale == "tera") return EntitySensorDataScale::tera;
    if (scale == "exa") return EntitySensorDataScale::exa;
    if (scale == "peta") return EntitySensorDataScale::peta;
    if (scale == "zetta") return EntitySensorDataScale::zetta;
    if (scale == "yotta") return EntitySensorDataScale::yotta;

    assert(false);
}

using EntitySensorPrecision = int32_t;
using EntitySensorValueUpdateRate = uint32_t;
using EntitySensorValue = int64_t;
using EntitySensorTimeStamp = uint64_t;

struct SensorResource
{
    std::string id;
    std::string description;
    EntitySensorDataType type;
    EntitySensorDataScale scale;
    EntitySensorPrecision precision;
    EntitySensorValueUpdateRate refresh_rate;
    EntitySensorValue max_value;
};

struct SensorHistory
{
    EntitySensorTimeStamp ts;
    EntitySensorValue value;
};

} // namespace gal

#endif
