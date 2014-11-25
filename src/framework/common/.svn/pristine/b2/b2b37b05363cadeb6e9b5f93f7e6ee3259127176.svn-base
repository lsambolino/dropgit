
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

#ifndef TNT_THREAD_SAFE_QUEUE_HPP_
#define TNT_THREAD_SAFE_QUEUE_HPP_

#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>

#if defined (WITH_BOOST)
#include <boost/call_traits.hpp>
#endif

#include "containers.hpp"
#include "lock.hpp"
#include "platform.hpp"

namespace tnt {

template <class T> class ThreadSafeQueue
{
public:

#if defined(WITH_BOOST)
	using param_type = typename boost::call_traits<T>::param_type;
#else
	using param_type = typename std::deque<T>::const_reference;
#endif

	using value_type = typename std::deque<T>::value_type;
	using size_type = typename std::deque<T>::size_type;

	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue&) = delete;

	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

	size_type size() const
	{
        return lock_unique(guard_, [&] ()
        {
            return queue_.size();
        });
	}

	bool empty() const
	{
        return lock_unique(guard_, [&] ()
        {
            return queue_.empty();
        });
	}

	template <class ... Args> void emplace_back(Args&& ... args)
	{
        // Not using tnt::lock due to a GCC bug.
        {
            std::unique_lock<std::mutex> lock(guard_);
            queue_.emplace_back(std::forward<Args>(args)...);
        }

        cv_.notify_one();
	}

	void push_back(param_type elem)
	{
		lock_unique(guard_, [&] ()
        {
            queue_.push_back(elem);
        });

        cv_.notify_one();
	}

	void push_back(T&& elem)
	{
		lock_unique(guard_, [&] ()
        {
		    queue_.push_back(std::move(elem));
        });

        cv_.notify_one();
	}

	template <class ... Args> void emplace_front(Args&& ... args)
	{
        // Not using tnt::lock due to a GCC bug.
        {
            std::unique_lock<std::mutex> lock(guard_);
            queue_.emplace_front(std::forward<Args>(args)...);
        }

        cv_.notify_one();
	}

	void push_front(param_type elem)
	{
        lock_unique(guard_, [&] ()
        {
            queue_.push_front(elem);
        });
		
		cv_.notify_one();
	}

	void push_front(T&& elem)
	{
		lock_unique(guard_, [&] ()
        {
		    queue_.push_front(std::move(elem));
        });

        cv_.notify_one();
	}

	value_type pop_front()
	{
		std::unique_lock<std::mutex> lock(guard_);
		cv_.wait(lock, [&] { return !queue_.empty(); });

		auto elem = std::move(queue_.front());
		queue_.pop_front();

		return elem;
	}

    template <class D> bool pop_front_for(value_type& elem, D rel_time)
	{
		std::unique_lock<std::mutex> lock(guard_);
		auto res = cv_.wait_for(lock, rel_time, [&] { return !queue_.empty(); });

        if (res)
        {
            elem = std::move(queue_.front());
		    queue_.pop_front();
        }
		
		return res;
	}

	value_type pop_back()
	{
		std::unique_lock<std::mutex> lock(guard_);
		cv_.wait(lock, [&] { return !queue_.empty(); });

		auto elem = std::move(queue_.back());
		queue_.pop_back();

		return elem;
	}

	template <class D> bool pop_back_for(value_type& elem, D rel_time)
	{
		std::unique_lock<std::mutex> lock(guard_);
		auto res = cv_.wait_for(lock, rel_time, [&] { return !queue_.empty(); });

        if (res)
        {
            elem = std::move(queue_.back());
		    queue_.pop_back();
        }
		
		return res;
	}

	template <class F> auto for_all(F func)
	{
        return lock_unique(guard_, [&] ()
        {
            return tnt::for_all(queue_, func);
        });
	}
private:
	std::deque<T> queue_;

	mutable std::mutex guard_;
	std::condition_variable cv_;
};

} // namespace tnt

#endif
