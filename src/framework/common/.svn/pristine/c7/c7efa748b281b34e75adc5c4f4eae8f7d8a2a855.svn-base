
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

#include "colors.hpp"

#include <mutex>

#include "log.hpp"

#if defined(TNT_PLATFORM_WIN32)

namespace tnt {
namespace colors {
namespace {

unsigned short default_color;
unsigned short bkg_color;

void set_color(unsigned short color)
{
    static std::once_flag init_instance_flag;

    static HANDLE handle;
    static CONSOLE_SCREEN_BUFFER_INFO info;

    std::call_once(init_instance_flag, [] ()
    {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
	    
        if (!GetConsoleScreenBufferInfo(handle, &info))
        {
            tnt::Log::error("Unable to get Console information.");

            return;
        }

	    default_color = info.wAttributes;
	    bkg_color = default_color & 0x00f0;        
    });

    SetConsoleTextAttribute(handle, color);
}

} // namespace

void console_def()
{
	set_color(default_color);
}

void console_red()
{
	set_color(FOREGROUND_RED | FOREGROUND_INTENSITY | bkg_color);
}

void console_yellow()
{
	set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | bkg_color);
}

void console_white()
{
	set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | bkg_color);
}

void console_green()
{
	set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY | bkg_color);
}

void console_blue()
{
	set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY | bkg_color);
}

void console_cyan()
{
	set_color(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | bkg_color);
}

void console_magenta()
{
	set_color(FOREGROUND_RED | bkg_color);
}

} // namespace colors
} // namespace tnt

#endif
