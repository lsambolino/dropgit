
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

#ifndef TNT_DYNAMIC_LIBRARY_HPP_
#define TNT_DYNAMIC_LIBRARY_HPP_

#include <string>
#include <memory>
#include <functional>

#include "function_pointer_traits.hpp"

namespace tnt {

class DynamicLibrary
{
    class DynamicLibraryImpl;
    template <class T> friend struct DynamicFunction;
public:
    explicit DynamicLibrary(const std::string& path);

    DynamicLibrary(const DynamicLibrary&) = default;
    DynamicLibrary(DynamicLibrary&& other) = default;

    ~DynamicLibrary();

    DynamicLibrary& operator=(DynamicLibrary&& other) = default;
    DynamicLibrary& operator=(const DynamicLibrary&) = default;

    template <class S> std::function<S> get(const std::string& name);
private:
    void* get_function(const std::string& name);
private:
    std::shared_ptr<DynamicLibraryImpl> impl_;
};

template <class T> struct DynamicFunction;

template <class R, class ... Args> struct DynamicFunction<R(Args...)>
{
    using Func = R(*)(Args...);

    static std::function<R(Args...)> get(const std::shared_ptr<DynamicLibrary::DynamicLibraryImpl>& lib, Func func)
    {
        return [lib, func] (Args... args) -> R
        {
            auto l = lib;

            return func(std::forward<Args>(args)...);
        };
    }
private:
    std::shared_ptr<DynamicLibrary::DynamicLibraryImpl> lib_;
};

template <class ... Args> struct DynamicFunction<void(Args...)>
{
    using Func = void(*)(Args...);

    static std::function<void(Args...)> get(const std::shared_ptr<DynamicLibrary::DynamicLibraryImpl>& lib, Func func)
    {
        return [lib, func] (Args... args)
        {
            auto l = lib;
            func(std::forward<Args>(args)...);
        };
    }
};

template <class S> std::function<S> DynamicLibrary::get(const std::string& name)
{
    typename FunctionPointerTraits<S>::type fun_ptr;
    *reinterpret_cast<void**>(&fun_ptr) = get_function(name);

    return DynamicFunction<S>::get(impl_, fun_ptr);
}

} // namespace tnt

#endif
