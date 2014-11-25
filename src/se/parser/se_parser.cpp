
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

#include "se_parser.hpp"

#include <iostream>
#include <string>
#include <exception>
#include <memory>

#include "event/quit.hpp"
#include "event/network/routes_management.hpp"
#include "event/network/toggle_rate.hpp"

#include "log.hpp"
#include "manip.hpp"
#include "application.hpp"

namespace drop {
namespace parser {

SeParser::SeParser(std::istream& in, std::ostream& out): Parser(in, out) {}

void SeParser::init_grammar()
{
    grammar_.insert(std::make_pair("quit", [this] (std::ostream& /*os*/, const std::string& /*line*/)
    {
        tnt::Application::raise(tnt::event::Quit());
        stop();
    }));

    grammar_.insert(std::make_pair("clear", [this] (std::ostream& os, const std::string& /*line*/)
    { 
        os << tnt::manip::clear;
    }));

    grammar_.insert(std::make_pair("routes", [this] (std::ostream& os, const std::string& /*line*/)
    {
        tnt::Application::raise(event::RouteListRequest(os));
    }));

    grammar_.insert(std::make_pair("togglerate", [this] (std::ostream& /*os*/, const std::string& /*line*/)
    {
        tnt::Application::raise(event::ToggleRate());
    }));
}

} // namespace parser
} // namespace drop
