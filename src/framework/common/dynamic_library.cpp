
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

#include "dynamic_library.hpp"

#include "platform.hpp"

#if defined(TNT_PLATFORM_WIN32)

#include <Windows.h>

#else

#include <dlfcn.h>

#endif

#include "shared_library_exception.hpp"

namespace tnt {

#if defined(TNT_PLATFORM_WIN32)

class DynamicLibrary::DynamicLibraryImpl
{
public:
    DynamicLibraryImpl(const std::string& path): module_(LoadLibrary(path.c_str()))
    {
        if (!module_)
        {
            throw OpenSharedLibraryException();
        }
    }

    ~DynamicLibraryImpl() 
    {
        FreeLibrary(module_);
    }

    void* get_function(const std::string& name)
    {
        auto f = GetProcAddress(module_, name.c_str());

        if (!f)
        {
            throw OpenSharedLibraryException();
        }

        return f;
    }
private:
    HMODULE module_;
};

#else

class DynamicLibrary::DynamicLibraryImpl
{
public:
    DynamicLibraryImpl(const std::string& path): handle_(dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL))
    {
        if (!handle_)
        {
            throw OpenSharedLibraryException();
        }
    }

    ~DynamicLibraryImpl()
    {
        dlclose(handle_);
    }

    void* get_function(const std::string& name)
    {
        auto f = dlsym(handle_, name.c_str());

        if (!f)
        {
            throw FunctionNotFoundException();
        }

        return f;
    }
private:
    void* handle_;
};

#endif

DynamicLibrary::DynamicLibrary(const std::string& path): impl_(std::make_shared<DynamicLibraryImpl>(path)) {}

DynamicLibrary::~DynamicLibrary() {}

void* DynamicLibrary::get_function(const std::string& name)
{
    return impl_->get_function(name);
}

} // namespace tnt
