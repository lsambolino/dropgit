
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

#ifndef TNT_MANIP_HPP_
#define TNT_MANIP_HPP_

#include <iostream>
#include <iomanip>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <Windows.h>

#endif

namespace tnt {
namespace manip {

inline std::ostream& cr(std::ostream& os)
{
	os << "\r";

	return os;
}

inline std::ostream& lf(std::ostream& os)
{
	os << "\n";

	return os;
}

inline std::ostream& back(std::ostream& os)
{
	os << "\b";

	return os;
}

inline std::ostream& tab(std::ostream& os)
{
	os << "\t";

	return os;
}

#if !defined(TNT_PLATFORM_WIN32)

inline std::ostream& clear(std::ostream& os)
{
	os << "\033[2J\033[1;1H";

	return os;
}

#else

inline std::ostream& clear(std::ostream& os)
{
	return os;
}

#endif // TNT_PLATFORM_WIN32 

} // namespace manip
} // namespace tnt

#endif
