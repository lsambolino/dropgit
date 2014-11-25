
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

#ifndef TNT_LOG_HPP_
#define TNT_LOG_HPP_

#include <mutex>
#include <iosfwd>
#include <memory>
#include <cstdint>
#include <cassert>
#include <string>
#include <iostream>
#include <atomic>
#include <type_traits>

#include "colors.hpp"
#include "manip.hpp"
#include "lock.hpp"

namespace tnt {

enum class LogLevel
{
    None,
    Error,
    Warning,
    Info,
    Debug,
    All // This should be the last
};

LogLevel parse_log_level(const std::string& level);

class Log
{
    class LogImpl
    {
    public:
        LogImpl() = default;

        ~LogImpl()
        {
            Log::valid_ = false;
        }

        template <class... T> void log_line(T&& ... args)
        {
            if (Log::valid_)
            {
                // Not using tnt::lock due to a GCC bug.
                {
                    std::unique_lock<std::mutex> lock(guard_);
                    do_log(std::forward<T>(args)...);
                    std::cerr << colors::def << std::endl;
                }
            }
            else
            {
                do_log(std::forward<T>(args)...);
                std::cerr << colors::def << std::endl;
            }
        }

        template <class... T> void log(T&&... args)
        {
            if (Log::valid_)
            {
                // Not using tnt::lock due to a GCC bug.
                {
                    std::unique_lock<std::mutex> lock(guard_);
                    do_log(std::forward<T>(args)...);
                    std::cerr << colors::def << std::endl;
                }
            }
            else
            {
                do_log(std::forward<T>(args)...);
                std::cerr << colors::def << std::flush;
            }
        }
    private:
        template <class T, class... R> void do_log(T&& arg, R&& ... rest)
        {
            print(std::forward<T>(arg));
            do_log(std::forward<R>(rest)...);
        }

        template <class T> void do_log(T&& arg)
        {
            print(std::forward<T>(arg));
        }

        template <class T> void print(T&& arg)
        {
            do_print(is_enum<T>(), std::forward<T>(arg));
        }

        template <class T> void do_print_enum_8_bit(std::true_type /*is_signed*/, T value)
        {
            using U = typename std::underlying_type_t<T>;
            std::cerr << static_cast<unsigned short>(value);
        }

        template <class T> void do_print_enum_8_bit(std::false_type /*is_signed*/, T value)
        {
            using U = typename std::underlying_type_t<T>;
            std::cerr << static_cast<short>(value);
        }

        template <class T> void do_print_enum(std::true_type /*is_8_bit*/, T value)
        {
            using U = typename std::underlying_type_t<T>;
            do_print_enum_8_bit(is_signed<U>(), value);
        }

        template <class T> void do_print_enum(std::false_type /*is_8_bit*/, T value)
        {
            using U = typename std::underlying_type_t<T>;
            std::cerr << static_cast<U>(value);
        }

        template <class T> void do_print(std::true_type /*is_enum*/, T value)
        {
            do_print_enum(is_8_bit<T>(), value);
        }

        template <class T> void do_print(std::false_type /*is_enum*/, T value)
        {
            std::cerr << value;
        }

        template <class T> constexpr auto is_enum()
        {
            return std::integral_constant<bool, std::is_enum<T>::value>();
        };

        template <class T> constexpr auto is_8_bit()
        {
            return std::integral_constant<bool, sizeof(T) == 1>();
        };

        template <class T> constexpr auto is_signed()
        {
            return std::integral_constant<bool, std::is_signed<T>::value>();
        };
    private:
        std::mutex guard_;
    };
public:
    static void level(LogLevel level);

    template <class... T> static void debug(T&& ... args)
    {
        log(LogLevel::Debug, std::forward<T>(args)...);
    }

    template <class... T> static void info(T&& ... args)
    {
        log(LogLevel::Info, std::forward<T>(args)...);
    }

    template <class... T> static void warning(T&& ... args)
    {
        log(LogLevel::Warning, std::forward<T>(args)...);
    }

    template <class... T> static void error(T&& ... args)
    {
        log(LogLevel::Error, colors::red, std::forward<T>(args)...);
    }

    template <class... T> static void output(T&& ... args)
    {
        try { instance()->log(std::forward<T>(args)...); } catch (...) {}
    }
private:
    Log() = delete;
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static LogImpl* instance();

    template <class... T> static void log(LogLevel level, T&& ... args)
    {
        if (level_ >= level)
        {
            try { instance()->log_line(std::forward<T>(args)...); } catch (...) {}
        }
    }
private:
    static std::atomic_bool valid_;
    static std::unique_ptr<LogImpl> instance_;
    static LogLevel level_;
};

std::istream& operator>>(std::istream& is, LogLevel& level);
std::ostream& operator<<(std::ostream& os, const LogLevel& level);

} // namespace tnt

namespace colors = tnt::colors;
namespace manip = tnt::manip;

#endif
