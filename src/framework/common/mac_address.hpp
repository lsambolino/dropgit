
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

#ifndef TNT_MAC_ADDRESS_HPP_
#define TNT_MAC_ADDRESS_HPP_

#include <string>
#include <cstdint>
#include <iosfwd>
#include <algorithm>
#include <iomanip>
#include <array>
#include <initializer_list>
#include <sstream>

namespace tnt {

template <size_t S> class BasicMacAddress
{
public:
    BasicMacAddress()
    {
        raw_.fill(0);
    }

    BasicMacAddress(std::array<uint8_t, S> raw): raw_(raw) {}

    BasicMacAddress(const char* mac)
    {
        parse(mac);
    }

    BasicMacAddress(const std::string& mac)
    {
        parse(mac);
    }

    BasicMacAddress(std::initializer_list<uint8_t> l)
    {
        size_t i = 0;

        for (auto c : l)
        {
            if (i == S)
            {
                return;
            }

            raw_[i++] = c;
        }

        if (i != S)
        {
            for (; i<S; ++i)
            {
                raw_[i] = 0;
            }
        }
    }

    static BasicMacAddress broadcast()
    {
        std::array<uint8_t, S> raw;
        raw.fill(0xff);

        return BasicMacAddress(raw);
    }

    bool operator==(const BasicMacAddress& other) const
    {
        return raw_ == other.raw_;
    }

    bool empty() const
    {
        return *this == BasicMacAddress();
    }

    bool is_broadcast() const
    {
        return std::all_of(std::begin(raw_), std::end(raw_), [] (uint8_t c)
        {
            return c == 0xff;
        });
    }

    std::string raw() const
    {
        return std::string(reinterpret_cast<const char*>(raw_.data()), S);
    }

    std::string to_string(const std::string& sep = ":", int base = 16) const
    {
        std::ostringstream ss;

        return print(ss, sep, base).str();
    }

    template <class T> T& print(T& os, const std::string& sep = ":", int base = 16) const
    {
        auto flags = os.flags();
        os  << std::setbase(base);

        for (size_t i=0; i<S-1; ++i)
        {
            os << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(raw_[i]) << sep;
        }

        os << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(raw_[S-1]);
        os.flags(flags);

        return os;
    }
private:
    void parse(const std::string& mac)
    {
        if (mac.size() == S)
        {
            parse_raw(mac);
        }
        else
        {
            parse_formatted(mac);
        }
    }

    void parse_raw(const std::string& mac)
    {
        size_t i = 0;

        for (unsigned char c : mac)
        {
            raw_[i++] = c;
        }
    }

    void parse_formatted(const std::string& mac)
    {
        std::string s = mac;
        size_t pos = 0;
        size_t i = 0;

        while (true)
        {
            if (pos >= s.size())
            {
                break;
            }

            s = s.substr(pos);
            raw_[i++] = std::stoi(s, &pos, 16);
            ++pos;
        }
    }
private:
    std::array<uint8_t, S> raw_;
};

template <size_t S> std::ostream& operator<<(std::ostream& os, const BasicMacAddress<S>& mac)
{
    return mac.print(os);
}

template <size_t S> std::istream& operator>>(std::istream& is, BasicMacAddress<S>& mac)
{
	std::string str;
	is >> str;
	mac = BasicMacAddress<S>(str);

    return is;
}

using MacAddress = BasicMacAddress<6>;

} // namespace tnt

#endif
