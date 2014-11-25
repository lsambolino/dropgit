
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

#ifndef DROP_UTIL_EXTERNAL_INTERFACE_MAP_HPP_
#define DROP_UTIL_EXTERNAL_INTERFACE_MAP_HPP_

#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <functional>

namespace drop {

struct NonExistentInterface: public std::runtime_error
{
    NonExistentInterface(): std::runtime_error("Interface not found.") {}
};

class ExternalInterfaceMap
{
private:
    struct InterfaceNode
    {
        InterfaceNode();
        InterfaceNode(int i, const std::string& n, const std::string& nd, int p): index(i), port(p), name(n), node(nd) {}

        int index;
        int port;

        std::string name;
        std::string node;
    };
public:
    void add(int index, const std::string& name, const std::string& node, int port);

    int get_index(const std::string& ie, int port);
    int get_index(const std::string& ie, int port, const std::nothrow_t&) noexcept;

    void for_each(const std::string& ie, std::function<void(int)> func);
    void for_each(std::function<void(const std::string&, int, int)> func);
private:
    std::vector<InterfaceNode> interfaces_;
};

} //namespace drop

#endif
