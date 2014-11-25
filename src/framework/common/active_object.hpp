
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

#ifndef TNT_ACTIVE_OBJECT_HPP_
#define TNT_ACTIVE_OBJECT_HPP_

#include <functional>
#include <future>
#include <atomic>

#include "thread.hpp"
#include "thread_safe_fifo.hpp"

namespace tnt {

class ActiveObject
{
	struct FunctionObject
	{
		virtual ~FunctionObject() {}
		virtual void exec() = 0;
	};

	template <class T> class Function: public FunctionObject
	{
	public:
		explicit Function(std::function<T()> func) : func_{ func } {}

		virtual void exec() override
		{
			try
			{
				set_value(promise, func_);
			}
			catch (...)
			{
				promise.set_exception(std::current_exception());
			}
		}

		std::promise<T> promise;
	private:
		template <class R, class F> void set_value(std::promise<R>& p, F& f) const
		{
			p.set_value(f());
		}

		template <class F> void set_value(std::promise<void>& p, F& f) const
		{
			f();
			p.set_value();
		}
	private:
		std::function<T()> func_;
	};
public:
	ActiveObject();
	ActiveObject(const ActiveObject&) = delete;
	~ActiveObject();

	ActiveObject& operator=(const ActiveObject&) = delete;

    template <class F> auto exec(F&& func) const
	{
        using R = typename std::result_of<F()>::type;

		auto ptr = std::make_unique<Function<R>>(func);
		auto future = ptr->promise.get_future();
		functions_.push(std::move(ptr));

		return future;
	}

    void join();

	std::thread::id id() const { return run_thread_.get_id(); }
private:
	void stop() const;
	void run() const;
private:
	mutable std::atomic_bool running_;
	mutable tnt::Thread run_thread_;
	mutable ThreadSafeFIFO<std::unique_ptr<FunctionObject>> functions_;
};

} // namespace tnt

#endif
