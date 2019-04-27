
#ifndef ROPUFU_SETTLERS_ONLINE_INDIRECT_VECTOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_INDIRECT_VECTOR_HPP_INCLUDED

#include <algorithm> // std::iter_swap
#include <initializer_list> // std::initializer_list
#include <map>       // std::map
#include <memory>    // std::allocator
#include <stdexcept> // std::out_of_range
#include <utility>   // std::forward
#include <vector>    // std::vector

namespace ropufu::settlers_online
{
    /** @brief A contiguous container maintaining persistent access by key at the expense of not maintaining original element order. */
    template <typename t_value_type, typename t_allocator_type = std::allocator<t_value_type>>
    struct indirect_vector
    {
        using type = indirect_vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;

        using vector_type = std::vector<value_type, allocator_type>;
        using size_type = typename vector_type::size_type;
        using iterator_type = typename vector_type::iterator;
        using const_iterator_type = typename vector_type::const_iterator;
        using reverse_iterator_type = typename vector_type::reverse_iterator;
        using const_reverse_iterator_type = typename vector_type::const_reverse_iterator;

        /** @todo Maybe create a wrapper class to make indexing overload tidier? */
        using key_type = size_type;

    private:
        vector_type m_container = {};
        std::vector<key_type> m_index_to_key = {};
        std::map<key_type, size_type> m_key_to_index = {};
        key_type m_next_key = 0;

        size_type key_to_index(key_type key) const
        {
            auto it = this->m_key_to_index.find(key);
            if (it != this->m_key_to_index.end()) return it->second;
            return 0;
        } // key_to_index(...)

        key_type index_to_key(size_type index) const
        {
            return this->m_index_to_key[index];
        } // index_to_key(...)

    public:
        indirect_vector() noexcept = default;

        /*implicit*/ indirect_vector(std::initializer_list<value_type> values) noexcept
            : m_container(values)
        {
            this->m_index_to_key.reserve(this->m_container.size());
            for (size_type i = 0; i < this->m_container.size(); ++i)
            {
                this->m_index_to_key.push_back(i);
                this->m_key_to_index.emplace(i, i);
            } // for (...)
            this->m_next_key = this->m_container.size();
        } // indirect_vector(...)

        template <typename t_other_allocator_type>
        /*implicit*/ indirect_vector(const std::vector<value_type, t_other_allocator_type>& other) noexcept
            : m_container(other)
        {
            this->m_index_to_key.reserve(this->m_container.size());
            for (size_type i = 0; i < this->m_container.size(); ++i)
            {
                this->m_index_to_key.push_back(i);
                this->m_key_to_index.emplace(i, i);
            } // for (...)
            this->m_next_key = this->m_container.size();
        } // indirect_vector(...)

        template <typename t_other_allocator_type>
        /*implicit*/ indirect_vector(std::vector<value_type, t_other_allocator_type>&& other) noexcept
            : m_container(std::forward<std::vector<value_type, t_other_allocator_type>>(other))
        {
            this->m_index_to_key.reserve(this->m_container.size());
            for (size_type i = 0; i < this->m_container.size(); ++i)
            {
                this->m_index_to_key.push_back(i);
                this->m_key_to_index.emplace(i, i);
            } // for (...)
            this->m_next_key = this->m_container.size();
        } // indirect_vector(...)

        explicit indirect_vector(key_type first_key) noexcept
            : m_next_key(first_key)
        {
        } // indirect_vector(...)
        
        type& operator ++() noexcept { ++this->m_next_key; return *this; }

        bool empty() const noexcept { return this->m_container.empty(); }
        size_type size() const noexcept { return this->m_container.size(); }
        size_type capcacity() const noexcept { return this->m_container.capcacity(); }
        void reserve(size_type capacity) { this->m_container.reserve(capacity); }

        void shrink_to_fit()
        {
            this->m_container.shrink_to_fit();
            this->m_index_to_key.shrink_to_fit();
        } // shrink_to_fit(...)

        void clear() noexcept
        {
            this->m_container.clear();
            this->m_index_to_key.clear();
            this->m_key_to_index.clear();
        } // clear(...)

        auto begin() noexcept { return this->m_container.begin(); }
        auto begin() const noexcept { return this->m_container.begin(); }
        auto end() noexcept { return this->m_container.end(); }
        auto end() const noexcept { return this->m_container.end(); }

        auto rbegin() noexcept { return this->m_container.rbegin(); }
        auto rbegin() const noexcept { return this->m_container.rbegin(); }
        auto rend() noexcept { return this->m_container.rend(); }
        auto rend() const noexcept { return this->m_container.rend(); }

        auto cbegin() const noexcept { return this->m_container.cbegin(); }
        auto cend() const noexcept { return this->m_container.cend(); }
        auto crbegin() const noexcept { return this->m_container.crbegin(); }
        auto crend() const noexcept { return this->m_container.crend(); }

        bool contains_key(key_type key) const noexcept
        {
            for (key_type maybe : this->m_index_to_key) if (maybe == key) return true;
            return false;
        } // contains_key(...)

        // bool contains_value(const value_type& item) const noexcept
        // {
        //     for (const value_type& maybe : this->m_container) if (maybe == item) return true;
        //     return false;
        // } // contains_value(...)

        /** @exception std::out_of_range \p key is not in the range of the container. */
        value_type& by_key(key_type key) { return this->m_container.at(this->key_to_index(key)); }
        /** @exception std::out_of_range \p key is not in the range of the container. */
        const value_type& by_key(key_type key) const { return this->m_container.at(this->key_to_index(key)); }

        /** @exception std::out_of_range \p index is not in the range of the container. */
        value_type& by_index(size_type index) { return this->m_container.at(index); }
        /** @exception std::out_of_range \p index is not in the range of the container. */
        const value_type& by_index(size_type index) const { return this->m_container.at(index); }

        // /** @exception std::out_of_range \p key is not in the range of the container. */
        // value_type& at(key_type key) { return this->by_key(key); }
        // /** @exception std::out_of_range \p key is not in the range of the container. */
        // const value_type& at(key_type key) const { return this->by_key(key); }
        
        /** @exception std::out_of_range \p index is not in the range of the container. */
        value_type& at(size_type index) { return this->by_index(index); }
        /** @exception std::out_of_range \p index is not in the range of the container. */
        const value_type& at(size_type index) const { return this->by_index(index); }
        
        value_type& operator [](size_type index) { return this->by_index(index); }
        const value_type& operator [](size_type index) const { return this->by_index(index); }

        value_type& front() { return this->m_container.front(); }
        const value_type& front() const { return this->m_container.front(); }

        value_type& back() { return this->m_container.back(); }
        const value_type& back() const { return this->m_container.back(); }

        key_type push_back(const value_type& item)
        {
            key_type key = this->m_next_key;
            this->m_key_to_index.emplace(key, this->m_container.size());
            this->m_container.push_back(item);
            this->m_index_to_key.push_back(key);
            ++this->m_next_key;
            return key;
        } // push_back(...)

        template <typename... t_arg_types>
        key_type emplace_back(t_arg_types&&... args)
        {
            key_type key = this->m_next_key;
            this->m_key_to_index.emplace(key, this->m_container.size());
            this->m_container.emplace_back(std::forward<t_arg_types>(args)...);
            this->m_index_to_key.push_back(key);
            ++this->m_next_key;
            return key;
        } // emplace_back(...)

        /** Removes the element at specified position, and replaces it with the last element in the collection. */
        void remove(const_iterator_type it)
        {
            const_iterator_type past_the_last = this->m_container.cend();
            const_iterator_type last = past_the_last - 1;
            size_type index = it - this->m_container.cbegin();
            key_type old_key = this->m_index_to_key[index];
            key_type new_key = this->m_index_to_key.back();
            // If this is not the last element, swap it with the last.
            // Original:
            // --- ... --- A --- ... --- o --- B
            // Intermediate:
            // --- ... --- B --- ... --- o --- A
            // Final:
            // --- ... --- B --- ... --- o
            if (it != last)
            {
                std::iter_swap(it, last); // Swap container items.
                this->m_index_to_key[index] = new_key; // Overwrite the old key.
                this->m_key_to_index[new_key] = index; // Overwrite the last element index.
            } // if (...)
            this->m_key_to_index.erase(old_key);
            this->m_container.pop_back();
            this->m_index_to_key.pop_back();
        } // remove(...)

        // /** Inequality operator, used as termination condition. */
        // bool operator !=(const type& other) const noexcept { return this->m_container != other.m_container; }
        // /** Equality operator. */
        // bool operator ==(const type& other) const noexcept { return this->m_container == other.m_container; }
    }; // struct indirect_vector
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_INDIRECT_VECTOR_HPP_INCLUDED
