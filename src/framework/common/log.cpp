
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

#include "log.hpp"

#include <fstream>
#include <cstdlib>

namespace tnt {
namespace {

std::once_flag init_instance_flag_;

} // namespace

std::atomic_bool Log::valid_;
std::unique_ptr<Log::LogImpl> Log::instance_; 
LogLevel Log::level_ = LogLevel::All;

LogLevel parse_log_level(const std::string& level)
{
    if (level == "All" || level == "all")
    {
        return tnt::LogLevel::All;
    }

    if (level == "Debug" || level == "debug")
    {
        return tnt::LogLevel::Debug;
    }
    
    if (level == "Info" || level == "info")
    {
        return tnt::LogLevel::Info;
    }
    
    if (level == "Warning" || level == "warning")
    {
        return tnt::LogLevel::Warning;
    }
    
    if (level == "Error" || level == "error")
    {
        return tnt::LogLevel::Error;
    }

    return tnt::LogLevel::None;
}

void Log::level(LogLevel level)
{
    level_ = level;
}

Log::LogImpl* Log::instance()
{
    std::call_once(init_instance_flag_, [&] ()
    {
        instance_ = std::make_unique<LogImpl>();
        valid_ = true;
    });

    return instance_.get();
}

std::istream& operator>>(std::istream& is, LogLevel& level)
{
    int value;
    is >> value;
    level = static_cast<LogLevel>(value);

    return is;
}

std::ostream& operator<<(std::ostream& os, const LogLevel& level)
{
    os << static_cast<int>(level);

    return os;
}

} // namespace tnt
