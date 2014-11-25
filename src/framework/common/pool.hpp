
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

#ifndef TNT_POOL_HPP_
#define TNT_POOL_HPP_

#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>

#include "lock.hpp"

namespace tnt {

template <class T> class Pool
{
public:
    using container = typename std::vector<std::unique_ptr<T>>;
    using size_type = typename container::size_type;

    Pool(size_type start_num, size_type max_num, bool shrink = false): curr_objs_{ start_num }, max_num_{ max_num }, shrink_{ shrink }
    {
        for (size_type i=0; i<start_num; ++i)
        {
            pool_.push_back(std::move(std::make_unique<T>()));
        }
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    typename std::shared_ptr<T> get()
    {
        if (pool_.empty())
        {
            if (curr_objs_ < max_num_)
            {
                auto ptr = std::make_unique<T>();
                ++curr_objs_;

                if (shrink_)
                {
                    return std::shared_ptr<T>(ptr.release());
                }
                else
                {
                    return std::shared_ptr<T>(ptr.release(), std::bind(&Pool::put, this, std::placeholders::_1));
                }
            }
            else
            {
                return nullptr;
            }
        }

        auto ptr = std::move(pool_.back());
        pool_.pop_back();

        return std::shared_ptr<T>(ptr.release(), std::bind(&Pool::put, this, std::placeholders::_1));
    }

    size_type size() const
    {
        return pool_.size();
    }

    bool empty() const
    {
        return pool_.empty();
    }

    void clear()
    {
        pool_.clear();
    }
private:
    void put(T* ptr)
    {
        pool_.push_back(std::unique_ptr<T>(ptr));
    }
private:
    size_type curr_objs_;
    size_type max_num_;
    bool shrink_;
    container pool_;
};

template <class T> class ThreadSafePool
{
#if defined(TNT_CPP14)
    using Mutex = std::shared_mutex;
#else
    using Mutex = std::mutex;
#endif

    class PoolImpl
    {
    public:
        using container = typename std::vector<std::unique_ptr<T>>;
        using size_type = typename container::size_type;

        PoolImpl(size_type initial_number, size_type max_number, bool shrink): initial_number_{ initial_number }, curr_objs_{ 0 }, max_num_{ max_number }, shrink_{ shrink }
        {
            for (size_type i=0; i<initial_number; ++i)
            {
                pool_.push_back(std::move(std::make_unique<T>()));
            }

            curr_objs_ = initial_number;
        }

        std::unique_ptr<T> get()
        {
            std::unique_lock<std::mutex> lock(guard_);
            empty_condition_.wait(lock, [=] { return !pool_.empty() || (curr_objs_ < max_num_); });

            if (pool_.empty())
            {
                auto ptr = std::make_unique<T>();
                ++curr_objs_;

                return ptr;
            }

            auto ptr = std::move(pool_.back());
            pool_.pop_back();

            return ptr;
        }

        void put(T* ptr)
        {
            std::unique_ptr<T> p(ptr);

            lock_unique(guard_, [&] ()
            {
                if (!shrink_ || pool_.size() < initial_number_)
                {
                    pool_.push_back(std::move(p));
                    empty_condition_.notify_one();
                }
            });
        }

        size_type size() const
        {
#if defined(TNT_CPP14)
            std::shared_lock<Mutex> lock(guard_);
#else
            std::lock_guard<Mutex> lock(guard_);
#endif
      
            return pool_.size();
        }

        bool empty() const
        {
#if defined(TNT_CPP14)
            std::shared_lock<Mutex> lock(guard_);
#else
            std::lock_guard<Mutex> lock(guard_);
#endif
            
            return pool_.empty();
        }

        void clear()
        {
            lock_unique(guard_, [&] ()
            {
                pool_.clear();
            });
        }
    private:
        size_type initial_number_;
        size_type curr_objs_;
        size_type max_num_;
        bool shrink_;
        container pool_;

        mutable std::mutex guard_;
        std::condition_variable empty_condition_;
    };
public:
    using size_type = typename PoolImpl::size_type;

    ThreadSafePool(size_type initial_number, size_type max_number, bool shrink = false): impl_(std::make_shared<PoolImpl>(initial_number, max_number, shrink)) {}

    typename std::shared_ptr<T> get()
    {
        auto ptr = std::move(impl_->get());
        auto impl = impl_;

        return std::shared_ptr<T>(ptr.release(), [impl] (T* ptr)
        {
            impl->put(ptr);        
        });
    }

    size_type size() const
    {
        return impl_->size();
    }

    bool empty() const
    {
        return impl_->empty();
    }

    void clear()
    {
        impl_->clear();
    }
private:
    std::shared_ptr<PoolImpl> impl_;
};

} // namespace tnt

#endif
