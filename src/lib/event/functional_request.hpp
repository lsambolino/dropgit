
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

#ifndef DROP_EVENT_FUNCTIONAL_REQUEST_HPP_
#define DROP_EVENT_FUNCTIONAL_REQUEST_HPP_

#include <functional>
#include <future>
#include <memory>

namespace drop {
namespace event {

template <class T> class FunctionalRequest;

template <class R, class ... Args> class FunctionalRequest<R(Args...)>
{
public:
    explicit FunctionalRequest(std::function<R(Args...)> func) : func_(func) {}

    R result()
    {
        auto future = promise_.get_future();

        return future.get();
    }

    void exec(Args&&... args)
    {
        try
		{
            auto res = func_(std::forward<Args>(args)...);
			promise_.set_value(res);
		}
		catch (...)
		{
			promise_.set_exception(std::current_exception());
		}
    }
private:
    std::function<R(Args...)> func_;
    std::promise<R> promise_;
};

template <class ... Args> class FunctionalRequest<void(Args...)>
{
public:
    explicit FunctionalRequest(std::function<void(Args...)> func) : func_(func) {}

    std::function<void(Args...)> func() const
    {
        return func_;
    }

    void wait()
    {
        auto future = promise_.get_future();
        future.get();
    }

    void notify()
    {
        promise_.set_value();
    }
private:
    std::function<void(Args...)> func_;
    std::promise<void> promise_;
};

}  // namespace event
}  // namespace drop

#endif
