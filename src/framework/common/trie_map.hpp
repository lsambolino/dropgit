
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

#ifndef TNT_TRIE_MAP_HPP_
#define TNT_TRIE_MAP_HPP_

#include <string>
#include <iterator>
#include <initializer_list>
#include <cstdint>
#include <cctype>
#include <cstddef>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <array>
#include <memory>

namespace tnt {

template <class T> class TrieMap
{
    const static size_t N = 62;
public:
    using mapped_type = T;
    using reference = mapped_type&;
    using const_reference = const mapped_type&;
    using key_type = std::string;
    using value_type = std::pair<const key_type, T>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = mapped_type*;
    using const_pointer = const mapped_type*;
private:
    class TrieMapValue
    {
    public:
        explicit TrieMapValue(const_reference value): value_{ value } {}

        bool operator==(const TrieMapValue& other) const
        {
            return value_ == other.value_;
        }

        reference value()
        {
            return value_;
        }

        const_reference value() const
        {
            return value_;
        }
    private:
        mapped_type value_;
    };

    class TrieMapIterator;
    class TrieMapConstIterator;

    class TrieMapNode
    {
        friend class TrieMapIterator;
        friend class TrieMapConstIterator;
    public:
        TrieMapNode(): has_children_{ false }, value_{ nullptr } {}

        TrieMapNode(const TrieMapNode& other): has_children_{ false }, value_{ nullptr }
        {
            if (other.value_)
            {
                value_ = std::make_unique<TrieMapValue>(other.value_->value());
            }

            for (size_t i=0; i<N; ++i)
            {
                if (other.children_[i])
                {
                    children_[i] = std::make_unique<TrieMapNode>(*other.children_[i]);
                }
                else
                {
                    children_[i] = nullptr;
                }
            }
        }

        bool operator==(const TrieMapNode& other) const
        {
            return same_value(other) && same_children(other);
        }

        reference value()
        {
            return value_->value();
        }

        const_reference value() const
        {
            return value_->value();
        }

        std::pair<std::string, TrieMapNode*> search(const std::string& str)
        {
            return search_node(str, 0, false);
        }

        const TrieMapNode* search(const std::string& str) const
        {
            return search_node(str, 0, false);
        }

        TrieMapNode* at(const std::string& str) 
        {
            return search_node(str, 0, true);
        }

        const TrieMapNode* at(const std::string& str) const
        {
            return search_node(str, 0, true);
        }

        std::pair<TrieMapNode*, bool> insert(const std::string& str, const_reference value)
        {
            return insert_key(str, 0, value);
        }
    private:
        static int8_t get_base_value(char c) // See table below.
        {
            if (std::islower(c))
            {
                return c - 97;
            }
            
            if (std::isupper(c))
            {
                return c - 39;
            }
            
            if (std::isdigit(c))
            {
                return c + 4;
            }

            return -1;
        }

        static char get_base_char(int8_t v) // See table below.
        {
            if (v < 25)
            {
                return 'a' + v;
            }

            if (v < 51)
            {
                return 'A' + v - 25;
            }

            if (v < 61)
            {
                return '0' + v - 51;
            }

            return -1;
        }

        std::pair<TrieMapNode*, bool> insert_key(const std::string& str, size_t pos, const_reference value)
        {
            if (pos == str.size())
            {
                if (value_)
                {
                    return std::make_pair(this, false);
                }

                value_ = std::make_unique<TrieMapValue>(value);
                
                return std::make_pair(this, true);
            }

            int8_t i = get_base_value(str[pos]);

            if (!children_[i])
            {
                children_[i] = std::make_unique<TrieMapNode>();
                has_children_ = true;
            }

            return children_[i]->insert_key(str, pos + 1, value);
        }

        std::pair<std::string, TrieMapNode*> search_node(const std::string& str, size_t pos, bool exact_match)
        {
            if (pos == str.size())
            {
                return std::make_pair(str, value_ || (!exact_match && has_children_) ? this : nullptr);
            }

            int8_t i = get_base_value(str[pos]);

            return children_[i] ? children_[i]->search_node(str, pos + 1, exact_match) : std::make_pair(str.substr(0, pos), nullptr);
        }

        const TrieMapNode* search_node(const std::string& str, size_t pos, bool exact_match) const
        {
            if (pos == str.size())
            {
                return value_ || (!exact_match && has_children_) ? this : nullptr;
            }

            int8_t i = get_base_value(str[pos]);

            return children_[i] ? children_[i]->search_node(str, pos + 1, exact_match) : nullptr;
        }

        bool same_value(const TrieMapNode& other) const
        {
            return value_ == nullptr ? other.value_ == nullptr : other.value_ != nullptr && *value_ == *other.value_;
        }

        bool same_children(const TrieMapNode& other) const
        {
            for (size_t i=0; i<N; ++i)
            {
                if (!(children_[i] == nullptr ? other.children_[i] == nullptr : other.children_[i] != nullptr && *children_[i] == *other.children_[i]))
                {
                    return false;
                }
            }

            return true;
        }
    private:
        bool has_children_;
        std::unique_ptr<TrieMapValue> value_;
        std::array<std::unique_ptr<TrieMapNode>, N> children_;
    };

    /*
        int8_t get_base_value(char c);

        a -> 0
        ...
        z -> 25
        A -> 26
        ...
        Z -> 51
        0 -> 52
        ...
        9 -> 61
    */

    class TrieMapIterator: public std::iterator<std::bidirectional_iterator_tag, T>
    {
    public:
        TrieMapIterator(): current_(std::begin(values_)) {}

        explicit TrieMapIterator(TrieMapNode* root): values_(tree("", root)), current_(std::begin(values_)) {}

        explicit TrieMapIterator(const std::pair<std::string, TrieMapNode*>& pair): values_(tree(pair.first, pair.second)), current_(std::begin(values_)) {}

        TrieMapIterator(const TrieMapIterator& other): values_(other.values_), current_(std::begin(values_))
        {
            typename std::vector<std::pair<std::string, reference>>::const_iterator current = other.current_;
            std::advance(current_, std::distance(std::begin(other.values_), current));
        }

        TrieMapIterator(TrieMapIterator&& other): values_(std::move(other.values_)), current_(std::move(other.current_)) {}

        TrieMapIterator& operator=(const TrieMapIterator& other)
        {
            auto tmp{other};
            swap(tmp);

            return *this;
        }

        TrieMapIterator& operator=(TrieMapIterator&& other)
        {
            auto tmp{std::move(other)};
            swap(tmp);

            return *this;
        }

        std::pair<std::string, reference> operator*()
        {
            return *current_;
        }

        std::pair<std::string, reference>* operator->()
        {
            return &*current_;
        }

        bool operator==(const TrieMapIterator& other) const
        {
            return std::distance<decltype(std::begin(values_))>(current_, std::end(values_)) ==
                std::distance<decltype(std::begin(other.values_))>(other.current_, std::end(other.values_));
        }

        bool operator!=(const TrieMapIterator& other) const
        {
            return !(*this == other);
        }

        TrieMapIterator& operator++()
        {
            ++current_;

            return *this;
        }

        TrieMapIterator operator++(int)
        {
            auto it = *this;
            ++(*this);

            return it;
        }

        TrieMapIterator& operator--()
        {
            --current_;

            return *this;
        }

        TrieMapIterator operator--(int)
        {
            auto it = *this;
            --(*this);

            return it;
        }

        TrieMapIterator operator+(int num)
        {
            auto it = *this;
            it.current_ += num;

            return it;
        }

        void swap(TrieMapIterator& other)
        {
            using std::swap;

            swap(values_, other.values_);
            swap(current_, other.current_);
        }
    private:
        std::vector<std::pair<std::string, reference>> tree(const std::string& prefix, TrieMapNode* root)
        {
            std::vector<std::pair<std::string, reference>> values;

            if (root)
            {
                traverse(prefix, root, values);
            }

            return values;
        }

        void traverse(const std::string& prefix, TrieMapNode* node, std::vector<std::pair<std::string, reference>>& values)
        {
            if (node->value_)
            {
                values.push_back(std::pair<std::string, reference>(prefix, node->value_->value()));
            }

            for (size_t i=0; i<N; ++i)
            {
                if (node->children_[i])
                {
                    traverse(prefix + TrieMapNode::get_base_char(i), node->children_[i].get(), values);
                }
            }
        }
    private:
        std::vector<std::pair<std::string, reference>> values_;
        typename std::vector<std::pair<std::string, reference>>::iterator current_;
    };

    class TrieMapConstIterator: public std::iterator<std::bidirectional_iterator_tag, T>
    {
    public:
        TrieMapConstIterator(): current_(std::begin(values_)) {}

        explicit TrieMapConstIterator(const TrieMapNode* root): values_(tree("", root)), current_(std::begin(values_)) {}

        explicit TrieMapConstIterator(const std::pair<std::string, const TrieMapNode*>& pair): values_(tree(pair.first, pair.second)), current_(std::begin(values_)) {}

        TrieMapConstIterator(const TrieMapConstIterator& other): values_(other.values_), current_(std::begin(values_))
        {
            std::advance(current_, std::distance(std::begin(other.values_), other.current_));
        }

        TrieMapConstIterator(TrieMapConstIterator&& other): values_(std::move(other.values_)), current_(std::move(other.current_)) {}

        TrieMapConstIterator& operator=(const TrieMapConstIterator& other)
        {
            auto tmp{other};
            swap(tmp);

            return *this;
        }

        TrieMapConstIterator& operator=(TrieMapConstIterator&& other)
        {
            auto tmp{std::move(other)};
            swap(tmp);

            return *this;
        }

        std::pair<std::string, const_reference> operator*()
        {
            return *current_;
        }

        std::pair<std::string, const_reference>* operator->()
        {
            return &*current_;
        }

        bool operator==(const TrieMapConstIterator& other) const
        {
            return std::distance<decltype(std::begin(values_))>(current_, std::end(values_)) ==
                std::distance<decltype(std::begin(other.values_))>(other.current_, std::end(other.values_));
        }

        bool operator!=(const TrieMapConstIterator& other) const
        {
            return !(*this == other);
        }

        TrieMapConstIterator& operator++()
        {
            ++current_;

            return *this;
        }

        TrieMapConstIterator operator++(int)
        {
            auto it = *this;
            ++(*this);

            return it;
        }

        TrieMapConstIterator& operator--()
        {
            --current_;

            return *this;
        }

        TrieMapConstIterator operator--(int)
        {
            auto it = *this;
            --(*this);

            return it;
        }

        TrieMapConstIterator operator+(int num)
        {
            auto it = *this;
            it.current_ += num;

            return it;
        }

        void swap(TrieMapConstIterator& other)
        {
            using std::swap;

            swap(values_, other.values_);
            swap(current_, other.current_);
        }
    private:
        std::vector<std::pair<std::string, const_reference>> tree(const std::string& prefix, const TrieMapNode* root)
        {
            std::vector<std::pair<std::string, const_reference>> values;

            if (root)
            {
                traverse(prefix, root, values);
            }

            return values;
        }

        void traverse(const std::string& prefix, const TrieMapNode* node, std::vector<std::pair<std::string, const_reference>>& values)
        {
            if (node->value_)
            {
                values.push_back(std::pair<std::string, const_reference>(prefix, node->value_->value()));
            }

            for (size_t i=0; i<N; ++i)
            {
                if (node->children_[i])
                {
                    traverse(prefix + TrieMapNode::get_base_char(i), node->children_[i].get(), values);
                }
            }
        }
    private:
        std::vector<std::pair<std::string, const_reference>> values_;
        typename std::vector<std::pair<std::string, const_reference>>::const_iterator current_;
    };
public:
    using iterator = TrieMapIterator;
    using const_iterator = TrieMapConstIterator;
public:
    TrieMap(): size_{ 0 } {}

    TrieMap(std::initializer_list<value_type> list): size_{ 0 }
    {
        for (const auto& v : list)
        {
            insert(v);
        }
    }

    size_type size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

    bool operator==(const TrieMap& other)
    {
        return node_ == other.node_;
    }

    void swap(TrieMap& other)
    {
        using std::swap;

        swap(size_, other.size_);
        swap(node_, other.node_);
    }

    size_type count(const key_type& key) const
    {
        return std::distance(search(key), end());
    }

    reference operator[](const key_type& key)
    {
        auto& p = insert(std::make_pair(key, T()));

        return *p.first;
    }

    const_reference operator[](const key_type& key) const
    {
        auto& p = insert(std::make_pair(key, T()));

        return p.first;
    }

    reference at(const key_type& key)
    {
        auto node = node_.at(key);

        if (!node)
        {
            throw std::out_of_range("Invalid TrieMap<T> key");
        }

        return node->value();
    }

    const_reference at(const key_type& key) const
    {
        auto node = node_.at(key);

        if (!node)
        {
            throw std::out_of_range("Invalid TrieMap<T> key");
        }

        return node->value();
    }

    iterator search(const key_type& key)
    {
        return iterator(node_.search(key));
    }

    const_iterator search(const key_type& key) const
    {
        return const_iterator(node_.search(key));
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        const auto& key = value.first;
        auto p = node_.insert(key, value.second);

        if (p.second)
        {
            ++size_;
        }

        return std::make_pair(iterator(p.first), p.second);
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        for (const auto& value : ilist)
        {
            insert(value);
        }
    }

    /*template <class... Args> iterator emplace(Args&&... args)
    {

    }

    iterator erase(const_iterator position)
    {
        
    }

    iterator erase(const_iterator first, const_iterator last)
    {

    }

    size_type erase(const key_type& key)
    {
        
    }

    void clear()
    {
        erase(begin(), end());
    }*/

    iterator begin()
    {
        return iterator(&node_);
    }

    const_iterator begin() const
    {
        return const_iterator(&node_);
    }

    const_iterator cbegin() const
    {
        return const_iterator(&node_);
    }

    iterator end()
    {
        return iterator();
    }

    const_iterator end() const
    {
        return const_iterator();
    }

    const_iterator cend() const
    {
        return const_iterator();
    }
private:
    size_t size_;
    TrieMapNode node_;
};

} // namespace tnt

#endif
