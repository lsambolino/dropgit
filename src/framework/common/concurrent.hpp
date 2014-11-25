
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

#ifndef TNT_CONCURRENT_HPP_
#define TNT_CONCURRENT_HPP_

#include "active_object.hpp"

#include <future>
#include <exception>

namespace tnt {

template <class T> class ThreadSafe
{
public:
    explicit ThreadSafe(T obj = T()): object_{ obj } {}

    std::shared_ptr<T> operator->() const
    {
        guard_.lock();

        return std::shared_ptr<T>(&object_, [this] (T*)
        {
            guard_.unlock();      
        });
    }
private:
    mutable T object_;
    mutable std::mutex guard_;
};

template <class T> class Concurrent
{
private:
    mutable T object_;
    mutable ActiveObject thread_;
public:
    explicit Concurrent(T obj = T()): object_{ obj } {}

    template <class F> auto operator()(F func) const
    {
        auto p = std::make_shared<std::promise<decltype(func(object_))>>();
        auto ret = p->get_future();

        thread_.exec([this, p, func]
        {
            try
            {
                set_value(*p, func, object_);
            }
            catch (...)
            {
                p->set_exception(std::current_exception());
            }
        });

        return ret;
    }
private:
    template <class R, class F> void set_value(std::promise<R>& p, F& f, T& t) const
    {
        p.set_value(f(t));
    }

    template <class F> void set_value(std::promise<void>& p, F& f, T& t) const
    {
        f(t);
        p.set_value();
    }
};

} // namespace tnt

#endif
