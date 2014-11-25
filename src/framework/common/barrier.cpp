
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

#include "barrier.hpp"

namespace tnt {

Barrier::Barrier(): lock_(std::make_shared<bool>(true)), count_(std::make_shared<int>(1)), guard_(std::make_shared<std::mutex>()), cv_(std::make_shared<std::condition_variable>()) {}

Barrier::Barrier(const Barrier& other): lock_(other.lock_), count_(other.count_), guard_(other.guard_), cv_(other.cv_), ex_(other.ex_)
{
    ++(*count_);
}

Barrier::Barrier(Barrier&& other):lock_(std::move(other.lock_)), count_(std::move(other.count_)), guard_(std::move(other.guard_)), cv_(std::move(other.cv_)), ex_(std::move(other.ex_)) {}

Barrier::~Barrier()
{
    wait();
}

Barrier& Barrier::operator=(const Barrier& other)
{
    Barrier tmp(other);
    swap(tmp);

    return *this;
}

Barrier& Barrier::operator=(Barrier&& other)
{
    Barrier tmp(std::move(other));
    swap(tmp);

    return *this;
}

void Barrier::wait()
{
    if (!lock_)
    {
        return;
    }

    do_wait();

    if (ex_)
    {
        std::rethrow_exception(ex_);
    }
}

void Barrier::swap(Barrier& other)
{
    using std::swap;

    swap(lock_, other.lock_);
    swap(count_, other.count_);
    swap(guard_, other.guard_);
    swap(cv_, other.cv_);
    swap(ex_, other.ex_);
}

void Barrier::do_wait()
{
    std::unique_lock<std::mutex> lock(*guard_);

    if (!*lock_)
    {
        return;
    }

    --(*count_);

    if (*count_ == 0)
    {
        *lock_ = false;
        cv_->notify_all();

        return;
    }

    cv_->wait(lock, [&] () { return !*lock_; });
}

void Barrier::set_exception(std::exception_ptr ex)
{
    ex_ = ex;
    do_wait();
}

} // namespace tnt
