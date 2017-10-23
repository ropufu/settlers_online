
#ifndef ROPUFU_SETTLERS_ONLINE_ENUM_ARRAY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ENUM_ARRAY_HPP_INCLUDED

#include <array> // std::array
#include <cstddef> // std::size_t
#include <initializer_list> // std::initializer_list
#include <type_traits> // std::underlying_type_t
#include <utility> // std::pair, std::make_pair

namespace ropufu
{
    namespace settlers_online
    {
        template <typename t_enum_type>
        struct enum_capacity
        {
            static constexpr std::size_t value = 0;
        };
        
        /** An iterator for \c enum_array to allow it to be used in range-based for loops. */
        template <typename t_enum_type, typename t_data_type, std::size_t t_capacity>
        class enum_array_iterator
        {
            using type = enum_array_iterator<t_enum_type, t_data_type, t_capacity>;
            using enum_type = t_enum_type;
            using data_type = t_data_type;
            using underlying_type = std::underlying_type_t<t_enum_type>;
            using collection_type = std::array<data_type, t_capacity>;
            using result_type = std::pair<enum_type, data_type>; // Iterates over all enum key/value pairs.

            const collection_type* m_collection_pointer;
            underlying_type m_position;

        public:
            enum_array_iterator(const collection_type& collection, underlying_type position) noexcept
                : m_collection_pointer(&collection), m_position(position)
            {
            }

            /** Termination condition. */
            bool operator !=(const type& other) const noexcept { return this->m_position != other.m_position; }

            /** Returns the current enum key/value pair. Behavior undefined if iterator has reached the end of the collection. */
            result_type operator *() const noexcept
            {
                const collection_type& collection = *(this->m_collection_pointer);
                return std::make_pair(static_cast<enum_type>(this->m_position), collection[this->m_position]);
            }

            /** If not at the end, advances the position of the iterator by one. */
            type& operator ++() noexcept
            {
                if (this->m_position == t_capacity) return *this;
                ++(this->m_position);
                return *this;
            }
        };
        
        /** An array indexed by an enum type. */
        template <typename t_enum_type, typename t_data_type, bool t_is_enabled = (enum_capacity<t_enum_type>::value != 0)>
        struct enum_array
        {
            using type = enum_array<t_enum_type, t_data_type, t_is_enabled>;
            using enum_type = t_enum_type;
            using data_type = t_data_type;
            using underlying_type = std::underlying_type_t<t_enum_type>;

            static constexpr std::size_t capacity = enum_capacity<t_enum_type>::value;
            using iterator_type = enum_array_iterator<t_enum_type, t_data_type, capacity>;

        private:
            std::array<data_type, capacity> m_data = { };

        public:
            enum_array() noexcept { }

            const data_type& operator [](enum_type value) const noexcept { return this->m_data[static_cast<underlying_type>(value)]; }
            data_type& operator [](enum_type value) noexcept { return this->m_data[static_cast<underlying_type>(value)]; }

            void fill(const data_type& value) noexcept { this->m_data.fill(value); }

            iterator_type begin() const noexcept { return iterator_type(this->m_data, 0); }
            iterator_type end() const noexcept { return iterator_type(this->m_data, capacity); }
            
            /** Checks two types for equality; ignores names. */
            bool operator ==(const type& other) const noexcept { return this->m_data == other.m_data; }
            /** Checks two types for inequality. */
            bool operator !=(const type& other) const noexcept { return !(this->operator ==(other)); }
        };
        
        /** An iterator for \c flags_t to allow it to be used in range-based for loops. */
        template <typename t_enum_type, std::size_t t_capacity>
        class flags_iterator
        {
            using type = flags_iterator<t_enum_type, t_capacity>;
            using enum_type = t_enum_type;
            using underlying_type = std::underlying_type_t<t_enum_type>;
            using collection_type = std::array<bool, t_capacity>;
            using result_type = enum_type; // Iterates over the marked enum values.

            const collection_type* m_collection_pointer;
            underlying_type m_position;

            /** Advances the pointer to the next set flag (if any). */
            void advance_to_true() noexcept
            {
                const collection_type& collection = *(this->m_collection_pointer);
                while (this->m_position < t_capacity && collection[this->m_position] == false) ++(this->m_position);
            }

        public:
            flags_iterator(const collection_type& collection, underlying_type position) noexcept
                : m_collection_pointer(&collection), m_position(position)
            {
                this->advance_to_true();
            }

            /** Termination condition. */
            bool operator !=(const type& other) const noexcept { return this->m_position != other.m_position; }

            /** Dereferencing return current position. */
            result_type operator *() const noexcept { return static_cast<enum_type>(this->m_position); }

            /** Advances the pointer to the next set flag (if any). */
            type& operator ++() noexcept
            {
                ++(this->m_position);
                this->advance_to_true();
                return *this;
            }
        };

        template <typename t_enum_type>
        using flags_t = enum_array<t_enum_type, bool, (enum_capacity<t_enum_type>::value != 0)>;
        
        /** An array indexed by an enum type. */
        template <typename t_enum_type>
        struct enum_array<t_enum_type, bool, true>
        {
            using type = enum_array<t_enum_type, bool, true>;
            using enum_type = t_enum_type;
            using data_type = bool;
            using underlying_type = std::underlying_type_t<t_enum_type>;

            static constexpr std::size_t capacity = enum_capacity<t_enum_type>::value;
            using iterator_type = flags_iterator<t_enum_type, capacity>;

        private:
            std::array<bool, capacity> m_flags = { };

        public:
            enum_array() noexcept { }

            enum_array(enum_type value) noexcept
            {
                this->operator [](value) = true;
            }

            enum_array(std::initializer_list<enum_type> values) noexcept
            {
                for (enum_type value : values) this->operator [](value) = true;
            }

            bool operator [](enum_type value) const noexcept { return this->m_flags[static_cast<underlying_type>(value)]; }
            bool& operator [](enum_type value) noexcept { return this->m_flags[static_cast<underlying_type>(value)]; }

            void fill(bool value) noexcept { this->m_data.fill(value); }

            bool has(enum_type value) const noexcept { return this->operator [](value); }
            void set(enum_type value) noexcept { this->operator [](value) = true; }
            void unset(enum_type value) noexcept { this->operator [](value) = false; }
            
            iterator_type begin() const noexcept { return iterator_type(this->m_flags, 0); }
            iterator_type end() const noexcept { return iterator_type(this->m_flags, capacity); }
            
            /** Checks two types for equality; ignores names. */
            bool operator ==(const type& other) const noexcept { return this->m_flags == other.m_flags; }
            /** Checks two types for inequality. */
            bool operator !=(const type& other) const noexcept { return !(this->operator ==(other)); }

            /** Elementwise "or". */
            type& operator |=(const type& other) noexcept
            {
                for (std::size_t i = 0; i < capacity; i++) this->m_flags[i] |= other.m_flags[i];
                return *this;
            } // operator |=(...)

            /** Elementwise "and". */
            type& operator &=(const type& other) noexcept
            {
                for (std::size_t i = 0; i < capacity; i++) this->m_flags[i] &= other.m_flags[i];
                return *this;
            } // operator &=(...)

            /** Elementwise "exclusive or". */
            type& operator ^=(const type& other) noexcept
            {
                for (std::size_t i = 0; i < capacity; i++) this->m_flags[i] ^= other.m_flags[i];
                return *this;
            } // operator ^=(...)

            /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
            friend type operator |(type left, const type& right) noexcept { left |= right; return left; }
            friend type operator &(type left, const type& right) noexcept { left &= right; return left; }
            friend type operator ^(type left, const type& right) noexcept { left ^= right; return left; }
        };

        template <typename t_enum_type, typename t_data_type>
        struct enum_array<t_enum_type, t_data_type, false>
        {
            t_enum_type value;
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_ENUM_ARRAY_HPP_INCLUDED
