
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

#ifndef TNT_PORTABLE_BINARY_ARCHIVE_HPP_
#define TNT_PORTABLE_BINARY_ARCHIVE_HPP_

#include <functional>
#include <string>
#include <sstream>
#include <typeinfo>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <type_traits>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/impl/basic_binary_oprimitive.ipp>
#include <boost/archive/impl/basic_binary_oarchive.ipp>

#include <boost/archive/impl/basic_binary_iprimitive.ipp>
#include <boost/archive/impl/basic_binary_iarchive.ipp>

#include <boost/serialization/level.hpp>

#include <boost/asio.hpp>

#include "endianness.hpp"

#include "containers.hpp"
#include "endianness.hpp"

namespace tnt {

template <class T> struct has_mapped_type
{
private:
    template <class C> static int test(typename C::mapped_type*);
    template <class> static void test(...);
public:
    static const bool value = !std::is_void<decltype(test<T>(nullptr))>::value;
};

template <class C> constexpr typename std::enable_if<!has_mapped_type<C>::value, typename C::value_type>::type container_type_helper(C&);
template <class C> constexpr typename std::enable_if<has_mapped_type<C>::value, std::pair<typename C::key_type, typename C::mapped_type>>::type container_type_helper(C&);

template <class C> using container_type_helper_t = decltype(container_type_helper(C()));

} // namespace tnt

namespace boost {
namespace archive {

template<class Archive, class Elem, class Tr> class portable_binary_oarchive_impl:
    public basic_binary_oprimitive<Archive, Elem, Tr>,
    public basic_binary_oarchive<Archive>
{
#ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
    friend class detail::interface_oarchive<Archive>;
    friend class basic_binary_oarchive<Archive>;
    friend class save_access;
protected:
#endif
    portable_binary_oarchive_impl(std::basic_streambuf<Elem, Tr>& bsb, unsigned int flags):
        basic_binary_oprimitive<Archive, Elem, Tr>(bsb, (flags & no_codecvt) != 0),
        basic_binary_oarchive<Archive>(flags)
    {
        init(flags);
    }

    portable_binary_oarchive_impl(std::basic_ostream<Elem, Tr>& os, unsigned int flags):
        basic_binary_oprimitive<Archive, Elem, Tr>(*os.rdbuf(), (flags & no_codecvt) != 0),
        basic_binary_oarchive<Archive>(flags)
    {
        init(flags);
    }

    void init(unsigned int flags)
    {
        if ((flags & no_header) != 0)
        {
            return;
        }

        this->basic_binary_oarchive<Archive>::init();
        this->basic_binary_oprimitive<Archive, Elem, Tr>::init();
    }

    template <class C> void save_container(const C& c, BOOST_PFTO int x)
    {
        uint32_t len = c.size();
        this->save(static_cast<uint32_t>(htonl(len)));

        for (auto i=std::begin(c); i!=std::end(c); ++i)
        {
            this->save_override(*i, x);
        }
    }

    template<class T> void save_override(T& t, BOOST_PFTO int)
    {
        this->basic_binary_oarchive<Archive>::save_override(t, 0L);
    }

    void save_override(const std::string& str, BOOST_PFTO int)
    {
        uint32_t len = str.size();
        this->save(static_cast<uint32_t>(htonl(len)));
        this->save_binary(&(str[0]), len);
    }

    template <class T, size_t n> void save_override(T (&a)[n], BOOST_PFTO int x)
    {
        uint32_t len = n;
        this->save(static_cast<uint32_t>(htonl(len)));

        for (uint32_t i=0; i<len; ++i)
        {
            this->save_override(a[i], x);
        }
    }

    template <class F, class S> void save_override(const std::pair<F, S>& p, BOOST_PFTO int x)
    {
        this->save_override(p.first, x);
        this->save_override(p.second, x);
    }

    template <class T, std::size_t N> void save_override(const std::array<T, N>& a, BOOST_PFTO int x)
    {
        save_container(a, x);
    }

    template <class T> void save_override(const std::list<T>& l, BOOST_PFTO int x)
    {
        save_container(l, x);
    }

    template <class T> void save_override(const std::vector<T>& v, BOOST_PFTO int x)
    {
        save_container(v, x);
    }

    template <class K, class V> void save_override(const std::map<K, V>& m, BOOST_PFTO int x)
    {
        save_container(m, x);
    }

    template <class K, class V> void save_override(const std::multimap<K, V>& m, BOOST_PFTO int x)
    {
        save_container(m, x);
    }

    template <class K, class V> void save_override(const std::unordered_map<K, V>& m, BOOST_PFTO int x)
    {
        save_container(m, x);
    }

    template <class K, class V> void save_override(const std::unordered_multimap<K, V>& m, BOOST_PFTO int x)
    {
        save_container(m, x);
    }

    template <class T> void save_override(const std::set<T>& s, BOOST_PFTO int x)
    {
        save_container(s, x);
    }

    template <class T> void save_override(const std::multiset<T>& s, BOOST_PFTO int x)
    {
        save_container(s, x);
    }

    template <class T> void save_override(const std::unordered_set<T>& s, BOOST_PFTO int x)
    {
        save_container(s, x);
    }

    template <class T> void save_override(const std::unordered_multiset<T>& s, BOOST_PFTO int x)
    {
        save_container(s, x);
    }

    template <class T> void save_override(const std::shared_ptr<T>& ptr, BOOST_PFTO int)
    {
        this->save(ptr.get());
    }

    void save_override(short value, BOOST_PFTO int)
    {
        this->save(htons(value));
    }

    void save_override(unsigned short value, BOOST_PFTO int)
    {
        this->save(htons(value));
    }

    void save_override(int value, BOOST_PFTO int)
    {
        this->save(htonl(value));
    }

    void save_override(unsigned int value, BOOST_PFTO int)
    {
        this->save(htonl(value));
    }

    void save_override(int64_t value, BOOST_PFTO int)
    {
        this->save(htonll(value));
    }

    void save_override(uint64_t value, BOOST_PFTO int)
    {
        this->save(htonll(value));
    }
};

class portable_binary_oarchive: public portable_binary_oarchive_impl<portable_binary_oarchive, std::ostream::char_type, std::ostream::traits_type>
{
public:
    portable_binary_oarchive(std::ostream& os, unsigned int flags = 0):
        portable_binary_oarchive_impl<portable_binary_oarchive, std::ostream::char_type, std::ostream::traits_type>(os, flags)
    {}

    portable_binary_oarchive(std::streambuf& bsb, unsigned int flags = 0):
        portable_binary_oarchive_impl<portable_binary_oarchive, std::ostream::char_type, std::ostream::traits_type>(bsb, flags)
    {}
};

using naked_portable_binary_oarchive = portable_binary_oarchive;

template<class Archive, class Elem, class Tr> class portable_binary_iarchive_impl:
    public basic_binary_iprimitive<Archive, Elem, Tr>,
    public basic_binary_iarchive<Archive>
{
#ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
    friend class detail::interface_iarchive<Archive>;
    friend class basic_binary_iarchive<Archive>;
    friend class load_access;
protected:
#endif
    portable_binary_iarchive_impl(std::basic_streambuf<Elem, Tr>& bsb, unsigned int flags):
        basic_binary_iprimitive<Archive, Elem, Tr>(bsb, (flags & no_codecvt) != 0),
        basic_binary_iarchive<Archive>(flags)
    {
        init(flags);
    }

    portable_binary_iarchive_impl(std::basic_istream<Elem, Tr>& os, unsigned int flags):
        basic_binary_iprimitive<Archive, Elem, Tr>(*os.rdbuf(), (flags & no_codecvt) != 0),
        basic_binary_iarchive<Archive>(flags)
    {
        init(flags);
    }

    void init(unsigned int flags)
    {
        if ((flags & no_header) != 0)
        {
            return;
        }

        this->basic_binary_iarchive<Archive>::init();
        this->basic_binary_iprimitive<Archive, Elem, Tr>::init();
    }

    template <class C> void load_container(C& c, BOOST_PFTO int x)
    {
        uint32_t len;
        this->load(len);
        len = static_cast<uint32_t>(ntohl(len));
        
        for (uint32_t i=0; i<len; ++i)
        {
            //decltype(tnt::container_type_helper(c)) value;
            tnt::container_type_helper_t<C> value;
            this->load_override(value, x);
            tnt::insert_back(c, value);
        }
    }

    template<class T> void load_override(T& t, BOOST_PFTO int)
    {
        this->basic_binary_iarchive<Archive>::load_override(t, 0L);
    }

    void load_override(std::string& str, BOOST_PFTO int)
    {
        uint32_t len;
        this->load(len);
        len = static_cast<uint32_t>(ntohl(len));
        str.resize(len);
        this->load_binary(&(str[0]), len);
    }

    template <class F, class S> void load_override(std::pair<F, S>& p, BOOST_PFTO int x)
    {
        this->load_override(p.first, x);
        this->load_override(p.second, x);
    }

    template <class T, size_t n> void load_override(T (&arr)[n], BOOST_PFTO int x)
    {
        uint32_t len;
        this->load(len);
        len = static_cast<uint32_t>(ntohl(len));
        assert(n == len);

        for (uint32_t i=0; i<len; ++i)
        {
            T value;
            this->load_override(value, x);
            arr[i] = value;
        }
    }

    template <class T, std::size_t N> void load_override(std::array<T, N>& arr, BOOST_PFTO int x)
    {
        uint32_t len;
        this->load(len);
        len = static_cast<uint32_t>(ntohl(len));
        assert(N == len);

        for (uint32_t i=0; i<len; ++i)
        {
            T value;
            this->load_override(value, x);
            arr[i] = value;
        }
    }

    template <class T> void load_override(std::vector<T>& v, BOOST_PFTO int x)
    {
        uint32_t len;
        this->load(len);
        len = static_cast<uint32_t>(ntohl(len));
        v.reserve(len);

        for (uint32_t i=0; i<len; ++i)
        {
            T value;
            this->load_override(value, x);
            tnt::insert_back(v, value);
        }
    }

    template <class T> void load_override(std::list<T>& l, BOOST_PFTO int x)
    {
        load_container(l, x);
    }

    template <class K, class V> void load_override(std::map<K, V>& m, BOOST_PFTO int x)
    {
        load_container(m, x);
    }

    template <class K, class V> void load_override(std::multimap<K, V>& m, BOOST_PFTO int x)
    {
        load_container(m, x);
    }

    template <class K, class V> void load_override(std::unordered_map<K, V>& m, BOOST_PFTO int x)
    {
        load_container(m, x);
    }

    template <class K, class V> void load_override(std::unordered_multimap<K, V>& m, BOOST_PFTO int x)
    {
        load_container(m, x);
    }

    template <class T> void load_override(std::set<T>& s, BOOST_PFTO int x)
    {
        load_container(s, x);
    }

    template <class T> void load_override(std::multiset<T>& s, BOOST_PFTO int x)
    {
        load_container(s, x);
    }

    template <class T> void load_override(std::unordered_set<T>& s, BOOST_PFTO int x)
    {
        load_container(s, x);
    }

    template <class T> void load_override(std::unordered_multiset<T>& s, BOOST_PFTO int x)
    {
        load_container(s, x);
    }

    template <class T> void load_override(std::shared_ptr<T>& ptr, BOOST_PFTO int)
    {
        T* t;
        this->load(t);

        ptr.reset(t);
    }

    void load_override(short& value, BOOST_PFTO int)
    {
        short val;
        this->load(val);
        value = ntohs(val);
    }

    void load_override(unsigned short& value, BOOST_PFTO int)
    {
        unsigned short val;
        this->load(val);
        value = ntohs(val);
    }

    void load_override(int& value, BOOST_PFTO int)
    {
        int val;
        this->load(val);
        value = ntohl(val);
    }

    void load_override(unsigned int& value, BOOST_PFTO int)
    {
        unsigned int val;
        this->load(val);
        value = ntohl(val);
    }

    void load_override(int64_t& value, BOOST_PFTO int)
    {
        int64_t val;
        this->load(val);
        value = ntohll(val);
    }

    void load_override(uint64_t& value, BOOST_PFTO int)
    {
        uint64_t val;
        this->load(val);
        value = ntohll(val);
    }
};

class portable_binary_iarchive: public portable_binary_iarchive_impl<portable_binary_iarchive, std::istream::char_type, std::istream::traits_type>
{
public:
    portable_binary_iarchive(std::istream& os, unsigned int flags = 0):
        portable_binary_iarchive_impl<portable_binary_iarchive, std::istream::char_type, std::istream::traits_type>(os, flags)
    {}

    portable_binary_iarchive(std::streambuf& bsb, unsigned int flags = 0):
        portable_binary_iarchive_impl<portable_binary_iarchive, std::istream::char_type, std::istream::traits_type>(bsb, flags)
    {}
};

using naked_portable_binary_iarchive = portable_binary_iarchive;

} // namespace archive
} // namespace boost

namespace ar = boost::archive;

// required by export
BOOST_SERIALIZATION_REGISTER_ARCHIVE(ar::portable_binary_oarchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(ar::portable_binary_oarchive)

BOOST_SERIALIZATION_REGISTER_ARCHIVE(ar::portable_binary_iarchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(ar::portable_binary_iarchive)

#endif
