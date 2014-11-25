
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

#include "parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <thread>
#include <cctype>

#include "exception/exception.hpp"

#include "log.hpp"

namespace tnt {
namespace {

bool is_spaces(const std::string& str)
{
    for (const auto& c : str)
    {
        if (std::isspace(c))
        {
            return true;
        }
    }

    return false;
}

} // namespace

Parser::Parser(std::istream& in, std::ostream& out): running_{ false }, in_(in), out_(out) {}

Parser::Parser(const Parser& other): running_{ false }, in_(other.in_), out_(other.out_) {}

Parser::~Parser()
{
    try
    {
        stop();
    }
    catch (...) {}
}

void Parser::operator()()
{
    init_grammar();
    running_ = true;

    while (running_)
    {
        std::string line;
        std::getline(in_, line);

        if (!running_)
        {
            break;
        }

        if (!in_)
        {
            // TODO: notify read error.

            continue;
        }

        auto cmd = line.substr(0, line.find(' '));

        if (cmd.empty() || is_spaces(cmd))
        {
            continue;
        }

        auto it = grammar_.search(cmd);

        if (it == std::end(grammar_))
        {
            out_ << "Syntax error: command " << cmd << " not found." << std::endl;

            continue;
        }

        if (it + 1 != std::end(grammar_))
        {
            out_ << "Ambiguous command " << cmd << ". It could be: " << std::endl;
        
            while (it != std::end(grammar_))
            {
                out_ << it->first << std::endl;
                ++it;
            }

            continue;
        }

        try
        {
            it->second(out_, line);
        }
        catch (SyntaxError& ex)
        {
            out_ << ex.what();
        }
        catch (std::exception& ex)
        {
            Log::error(ex.what());
        }
    }
}

void Parser::stop()
{
    running_ = false;
}

} // namespace tnt
