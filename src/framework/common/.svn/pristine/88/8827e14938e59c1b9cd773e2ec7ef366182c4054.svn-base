
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

#ifndef TNT_THREAD_SAFE_LIFO_HPP_
#define TNT_THREAD_SAFE_LIFO_HPP_

#include "thread_safe_queue.hpp"

namespace tnt {

template <class T> class ThreadSafeLIFO
{
public:
    using param_type = typename ThreadSafeQueue<T>::param_type;
    using value_type = typename ThreadSafeQueue<T>::value_type;
    using size_type = typename ThreadSafeQueue<T>::size_type;

    size_type size() const
    {
        return queue_.size();
    }

    bool empty() const
    {
        return queue_.empty();
    }

	template <class ... Args> void emplace(Args && ...args)
	{
		queue_.emplace_back(std::forward<Args>(args)...);
	}

    void push(param_type elem)
    {
        queue_.push_back(elem);
    }

	void push(T && elem)
	{
		queue_.push_back(std::move(elem));
	}

    value_type pop()
    {
        return queue_.pop_back();
    }

    template <class F> auto for_all(F func) -> decltype(ThreadSafeQueue<T>().for_all(func))
    {
        return queue_.for_all(func);
    }
private:
    ThreadSafeQueue<T> queue_;
};

} // namespace tnt

#endif
