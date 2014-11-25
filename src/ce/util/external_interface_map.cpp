
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

#include "external_interface_map.hpp"

#include "containers.hpp"

namespace drop {

void ExternalInterfaceMap::add(int index, const std::string& name, const std::string& node, int port)
{
    interfaces_.emplace_back(index, name, node, port);
}

int ExternalInterfaceMap::get_index(const std::string& ie, int port)
{
    for (const auto& n : interfaces_)
    {
        if (n.node == ie && n.port == port)
        {
            return n.index;
        }
    }

    throw NonExistentInterface();
}

int ExternalInterfaceMap::get_index(const std::string& ie, int port, const std::nothrow_t&) noexcept
{
    for (const auto& n : interfaces_)
    {
        if (n.node == ie && n.port == port)
        {
            return n.index;
        }
    }

    return 0; // 0 is not a valid index for an interface.
}

void ExternalInterfaceMap::for_each(const std::string& node, std::function<void(int)> func)
{
    for (const auto& n : interfaces_)
    {
        if (n.node == node)
        {
            func(n.index);
        }
    }
}

void ExternalInterfaceMap::for_each(std::function<void(const std::string&, int, int)> func)
{
    for (const auto& n : interfaces_)
    {
        func(n.name, n.index, n.port);
    }
}

/*void ExternalInterfaceMap::remove(int index)
{
    tnt::erase_if(interfaces_, [index] (const auto& node)
    {
        return node.index == index;
    });
}

void ExternalInterfaceMap::remove(const std::string& name)
{
    tnt::erase_if(interfaces_, [&name] (const auto& node)
    {
        return node.name == name;
    });
}

void ExternalInterfaceMap::remove(const std::string& ie, int port)
{
    tnt::erase_if(interfaces_, [&] (const auto& node)
    {
        return node.port.ie == ie && node.port.port == port;
    });
}

int ExternalInterfaceMap::get_index(const std::string& name)
{
    for (const auto& n : interfaces_)
    {
        if (n.name == name)
        {
            return n.index;
        }
    }

    throw NonExistentInterface();
}

void ExternalInterfaceMap::set_index(const std::string& name, int index)
{
	for (auto& n : interfaces_)
	{
		if (n.name == name)
		{
			n.index = index;
		}
	}

	throw NonExistentInterface();
}

const ExternalInterfaceMap::IEPort& ExternalInterfaceMap::get_port(int index)
{
    for (const auto& n : interfaces_)
    {
        if (n.index == index)
        {
            return n.port;
        }
    }

    throw NonExistentInterface();
}

const ExternalInterfaceMap::IEPort& ExternalInterfaceMap::get_port(const std::string& name)
{
    for (const auto& n : interfaces_)
    {
        if (n.name == name)
        {
            return n.port;
        }
    }

    throw NonExistentInterface();
}

const std::string& ExternalInterfaceMap::get_name(int index)
{
    for (const auto& n : interfaces_)
    {
        if (n.index == index)
        {
            return n.name;
        }
    }

    throw NonExistentInterface();
}

const std::string& ExternalInterfaceMap::get_name(const std::string& ie, int port)
{
    for (const auto& n : interfaces_)
    {
        if (n.port.ie == ie && n.port.port == port)
        {
            return n.name;
        }
    }

    throw NonExistentInterface();
}

int ExternalInterfaceMap::num_ports(const std::string& ie)
{
    int num = 0;

    for (const auto& n : interfaces_)
    {
        if (n.port.ie == ie)
        {
            ++num;
        }
    }

    return num;
}*/

} // namespace drop
