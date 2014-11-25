
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

#ifndef TNT_EXPECTED_HPP_
#define TNT_EXPECTED_HPP_

#include <functional>
#include <stdexcept>

namespace tnt {

template <class> class expected;

template <class> class expected_common;

template <class T> class expected_traits
{
    friend class expected_common<T>;

    using storage = T;
    using value = T;
    using pointer = T*;
    using reference = T&;
};

template <class T> class expected_traits<T&>
{
    friend class expected_common<T&>;

    using storage = std::reference_wrapper<T>;
    using value = T;
    using pointer = T*;
    using reference = T&;
};

template <class T> class expected_traits<const T&>
{
    friend class expected_common<const T&>;

    using storage = std::reference_wrapper<const T>;
    using value = T;
    using pointer = const T*;
    using reference = const T&;
};

template <class T> class expected_common
{
    friend class expected<T>;

    using value = typename expected_traits<T>::value;
    using storage = typename expected_traits<T>::storage;
    using pointer = typename expected_traits<T>::pointer;
    using reference = typename expected_traits<T>::reference;

    expected_common() {}

    expected_common(const value& value): valid_{ true }, storage_{ value } {}

    expected_common(value& value): valid_{ true }, storage_{ value } {}

    expected_common(value&& value): valid_{ true }, storage_(std::move(value)) {}

    template<typename... AA> explicit expected_common(AA&&... arguments): valid_{ true }, storage_(std::forward<AA>(arguments)...) {}

    expected_common(const expected_common& other): valid_(other.valid_)
    {
        if (valid_)
        {
            new(&storage_) storage(other.storage_);
        }
        else
        {
            new(&exception_) std::exception_ptr(other.exception_);
        }
    }

    expected_common(expected_common&& other): valid_(other.valid_)
    {
        if (valid_)
        {
            new(&storage_) storage(std::move(other.storage_));
        }
        else
        {
            new(&exception_) std::exception_ptr(std::move(other.exception_));
        }
    }

    ~expected_common()
    {
        if (valid_)
        {
            storage_.~storage();
        }
        else
        {
            exception_.~exception_ptr();
        }
    }

    void swap(expected_common& other)
    {
        if (valid_)
        {
            if (other.valid_)
            {
                using std::swap;
                swap(storage_, other.storage_);
            }
            else
            {
                auto exception = std::move(other.exception_);
                new(&other.storage_) storage(std::move(storage_));
                new(&exception_) std::exception_ptr(exception);
            }
        }
        else if (other.valid_)
        {
            other.swap(*this);
        }
        else
        {
            exception_.swap(other.exception_);
        }
    }
public:
    template<class E> static expected<T> from_exception(const E& exception)
    {
        if (typeid (exception) != typeid (E))
        {
            throw std::invalid_argument("slicing detected");
        }

        return from_exception(std::make_exception_ptr(exception));
    }

    static expected<T> from_exception(std::exception_ptr exception)
    {
        expected<T> result;
        result.valid_ = false;
        new(&result.exception_) std::exception_ptr(exception);

        return result;
    };

    static expected<T> from_exception()
    {
        return from_exception(std::current_exception());
    }

    template<class F, class AA> static expected<T> from_code(F function, AA&& arguments)
    {
        try
        {
            return expected<T>(function());
        }
        catch (...)
        {
            return from_exception();
        };
    }

    operator bool() const
    {
        return valid_;
    }

    template <class E> bool exception_is() const
    {
        try
        {
            if (!valid_)
            {
                std::rethrow_exception(exception_);
            }
        }
        catch (const E&)
        {
            return true;
        }
        catch(...) {}

        return false;
    }
private:
    reference get()
    {
        if (!valid_)
        {
            std::rethrow_exception(exception_);
        }

        return storage_;
    }

    const reference get() const
    {
        if (!valid_)
        {
            std::rethrow_exception(exception_);
        }

        return storage_;
    }
public:
    reference operator*()
    {
        return get();
    }

    const reference operator*() const
    {
        return get();
    }

    pointer operator->()
    {
        return &get();
    }

    const pointer operator->() const
    {
        return &get();
    }
private:
    bool valid_;

    union
    {
        storage storage_;
        std::exception_ptr exception_;
    };
};

template <class T> class expected: public expected_common<T>
{
    friend class expected_common<T>;
    using common = expected_common<T>;

    expected() {}
public:
    template<class... AA> expected(AA&&... arguments): common(std::forward<AA>(arguments)...) {}

    expected(const T& value): common(value) {}

    expected(T&& value): common(value) {}

    expected(const expected& other): common(static_cast<const common&>(other)) {}

    expected(expected&& other): common(static_cast<common&&>(other)) {}

    void swap(expected& other): common::swap(static_cast<common&>(other)) {}
};

template <class T> class expected<T&>: public expected_common<T&>
{
    friend class expected_common<T&>;
    using common = expected_common<T&>;

    expected() {};
public:
    expected(T& value): common(value) {}

    expected(const expected& other): common(static_cast<const common&>(other)) {}

    expected(expected&& other): common(static_cast<common&&>(other)) {}

    void swap(expected& other): common::swap(static_cast<common&>(other)) {}
};

template <class T> class expected<const T&>: public expected_common<const T&>
{
    friend class expected_common<const T&>;
    using common = expected_common<const T&>;

    expected() {}
public:
    expected(const T& value): common(value) {}

    expected(const expected& other): common(static_cast<const common&>(other)) {}

    expected(expected&& other): common(static_cast<common&&>(other)) {}

    void swap(expected& other): common::swap(static_cast<common&>(other)) {}
};

/*#include <boost/call_traits.hpp>

namespace tnt {

template <class T> class Expected
{
    using param_type = typename boost::call_traits<T>::param_type;
public:
	Expected(param_type value): value_{ value }, valid_{ true } {}
    Expected(std::exception_ptr ex): ex_{ ex }, valid_{ false } {}
    Expected(const std::exception& ex): ex_(std::make_exception_ptr(ex)), valid_{ false } {}
	Expected(T&& value): value_(std::move(value)), valid_{ true } {}
    Expected(std::exception_ptr&& ex): ex_(std::move(ex)), valid_{ false } {}
    Expected(std::exception&& ex): ex_(std::make_exception_ptr(std::move(ex))), valid_{ false } {}

	Expected(const Expected& other): valid_(other.valid_)
	{
		if (valid_)
		{
			new (&value_) T(other.value_);
		}
		else
		{
			new (&ex_) std::exception_ptr(other.ex_);
		}
	}

	Expected(Expected&& other): valid_(other.valid_)
	{
		if (valid_)
		{
			new (&value_) T(std::move(other.value_));
		}
		else
		{
			new (&ex_) std::exception_ptr(std::move(other.ex_));
		}
	}

	~Expected() noexcept
	{
		using std::exception_ptr;

		if (valid_)
		{
			value_.~T();
		}
		else
		{
			ex_.~exception_ptr();
		}
	}

    Expected& operator=(const Expected&) = delete;
    Expected& operator=(Expected&& other) = delete;

	bool valid() const
	{
		return valid_;
	}

	param_type get() const
	{
		if (valid_)
		{
			return value_;
		}

		std::rethrow_exception(ex_);
	}

    operator param_type() const
    {
        return get();
    }
private:
	Expected() {}
private:
	union
	{
		T value_;
		std::exception_ptr ex_;
	};

	bool valid_;
};*/

} // namespace tnt

#endif
