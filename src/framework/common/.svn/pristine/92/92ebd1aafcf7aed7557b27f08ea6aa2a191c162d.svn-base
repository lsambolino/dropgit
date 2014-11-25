
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

#ifndef TNT_COLORS_HPP_
#define TNT_COLORS_HPP_

#include <iostream>
#include <iomanip>

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <Windows.h>

#endif

namespace tnt {
namespace colors {

#if !defined(TNT_PLATFORM_WIN32)

inline std::ostream& def(std::ostream& os)
{
	os << "\033[m";

	return os;
}

inline std::ostream& red(std::ostream& os)
{
	os << "\033[1;31m";

	return os;
}

inline std::ostream& yellow(std::ostream& os)
{
	os << "\033[1;33m";

	return os;
}

inline std::ostream& white(std::ostream& os)
{
	os << "\033[1;37m";

	return os;
}

inline std::ostream& green(std::ostream& os)
{
	os << "\033[1;32m";

	return os;
}

inline std::ostream& blue(std::ostream& os)
{
	os << "\033[1;34m";

	return os;
}

inline std::ostream& cyan(std::ostream& os)
{
	os << "\033[1;36m";

	return os;
}

inline std::ostream& magenta(std::ostream& os)
{
	os << "\033[1;35m";

	return os;
} 

#else

void console_def();
void console_red();
void console_yellow();
void console_white();
void console_green();
void console_blue();
void console_cyan();
void console_magenta();

inline std::ostream& def(std::ostream& os)
{
	console_def();

	return os;
}

inline std::ostream& red(std::ostream& os)
{
	console_red();

	return os;
}

inline std::ostream& yellow(std::ostream& os)
{
	console_yellow();

	return os;
}

inline std::ostream& white(std::ostream& os)
{
	console_white();

	return os;
}

inline std::ostream& green(std::ostream& os)
{
	console_green();

	return os;
}

inline std::ostream& blue(std::ostream& os)
{
	console_blue();

	return os;
}

inline std::ostream& cyan(std::ostream& os)
{
	console_cyan();

	return os;
}

inline std::ostream& magenta(std::ostream& os)
{
	console_magenta();

	return os;
}

#endif // TNT_PLATFORM_WIN32 

} // namespace colors
} // namespace tnt

#endif
