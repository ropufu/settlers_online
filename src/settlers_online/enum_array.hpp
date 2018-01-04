
#ifndef ROPUFU_SETTLERS_ONLINE_ENUM_ARRAY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ENUM_ARRAY_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <nlohmann/json.hpp>
#include "json.hpp"
#include "core.hpp"

#include <array> // std::array
#include <cstddef> // std::size_t
#include <initializer_list> // std::initializer_list
#include <ostream> // std::ostream
#include <string> // std::string, std::to_string
#include <type_traits> // std::underlying_type_t
#include <utility> // std::pair, std::make_pair

namespace ropufu
{
    namespace settlers_online
    {
        template <typename t_enum_type>
        bool try_parse(const std::string& str, t_enum_type& value) noexcept { return false; }

        template <typename t_enum_type>
        struct enum_capacity
        {
            static constexpr std::size_t value = 0;
        }; // struct enum_capacity
        
        /** An iterator for \c enum_array to allow it to be used in range-based for loops. */
        template <typename t_enum_type, typename t_data_type, std::size_t t_capacity>
        struct enum_array_iterator
        {
            using type = enum_array_iterator<t_enum_type, t_data_type, t_capacity>;
            using enum_type = t_enum_type;
            using data_type = t_data_type;
            using underlying_type = std::underlying_type_t<t_enum_type>;
            using collection_type = std::array<data_type, t_capacity>;
            using result_type = std::pair<enum_type, data_type>; // Iterates over all enum key/value pairs.

        private:
            const collection_type* m_collection_pointer;
            underlying_type m_position;

        public:
            enum_array_iterator(const collection_type& collection, underlying_type position) noexcept
                : m_collection_pointer(&collection), m_position(position)
            {
            } // enum_array_iterator(...)

            /** Termination condition. */
            bool operator !=(const type& other) const noexcept { return this->m_position != other.m_position; }

            /** Returns the current enum key/value pair. Behavior undefined if iterator has reached the end of the collection. */
            result_type operator *() const noexcept
            {
                const collection_type& collection = *(this->m_collection_pointer);
                return std::make_pair(static_cast<enum_type>(this->m_position), collection[this->m_position]);
            } // operator *(...)

            /** If not at the end, advances the position of the iterator by one. */
            type& operator ++() noexcept
            {
                if (this->m_position == t_capacity) return *this;
                ++(this->m_position);
                return *this;
            } // operator ++(...)
        }; // struct enum_array_iterator
        
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

            bool empty() const noexcept
            {
                data_type z { };
                for (const data_type& x : this->m_data) if (x != z) return false;
                return true;
            } // empty(...)

            iterator_type begin() const noexcept { return iterator_type(this->m_data, 0); }
            iterator_type end() const noexcept { return iterator_type(this->m_data, capacity); }
            
            /** Checks two types for equality; ignores names. */
            bool operator ==(const type& other) const noexcept { return this->m_data == other.m_data; }
            /** Checks two types for inequality. */
            bool operator !=(const type& other) const noexcept { return !(this->operator ==(other)); }

            friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
            {
                nlohmann::json j = self;
                return os << j;
            } // operator <<(...)
        }; // struct enum_array
        
        /** An iterator for \c flags_t to allow it to be used in range-based for loops. */
        template <typename t_enum_type, std::size_t t_capacity>
        struct flags_iterator
        {
            using type = flags_iterator<t_enum_type, t_capacity>;
            using enum_type = t_enum_type;
            using underlying_type = std::underlying_type_t<t_enum_type>;
            using collection_type = std::array<bool, t_capacity>;
            using result_type = enum_type; // Iterates over the marked enum values.

        private:
            const collection_type* m_collection_pointer;
            underlying_type m_position;

            /** Advances the pointer to the next set flag (if any). */
            void advance_to_true() noexcept
            {
                const collection_type& collection = *(this->m_collection_pointer);
                while (this->m_position < t_capacity && collection[this->m_position] == false) ++(this->m_position);
            } // advance_to_true(...)

        public:
            flags_iterator(const collection_type& collection, underlying_type position) noexcept
                : m_collection_pointer(&collection), m_position(position)
            {
                this->advance_to_true();
            } // flags_iterator(...)

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
            } // operator ++(...)
        }; // struct flags_iterator

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
            } // enum_array

            enum_array(std::initializer_list<enum_type> values) noexcept
            {
                for (enum_type value : values) this->operator [](value) = true;
            } // enum_array

            bool operator [](enum_type value) const noexcept { return this->m_flags[static_cast<underlying_type>(value)]; }
            bool& operator [](enum_type value) noexcept { return this->m_flags[static_cast<underlying_type>(value)]; }

            void fill(bool value) noexcept { this->m_data.fill(value); }

            bool empty() const noexcept
            {
                for (bool x : this->m_flags) if (x) return false;
                return true;
            } // empty(...)

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
                for (std::size_t i = 0; i < capacity; i++) this->m_flags[i] = (this->m_flags[i] || other.m_flags[i]);
                return *this;
            } // operator |=(...)

            /** Elementwise "and". */
            type& operator &=(const type& other) noexcept
            {
                for (std::size_t i = 0; i < capacity; i++) this->m_flags[i] = (this->m_flags[i] && other.m_flags[i]);
                return *this;
            } // operator &=(...)

            /** Elementwise "exclusive or". */
            type& operator ^=(const type& other) noexcept
            {
                for (std::size_t i = 0; i < capacity; i++) this->m_flags[i] = (this->m_flags[i] ^ other.m_flags[i]);
                return *this;
            } // operator ^=(...)

            /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
            friend type operator |(type left, const type& right) noexcept { left |= right; return left; }
            friend type operator &(type left, const type& right) noexcept { left &= right; return left; }
            friend type operator ^(type left, const type& right) noexcept { left ^= right; return left; }

            friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
            {
                nlohmann::json j = self;
                return os << j;
            } // operator <<(...)
        }; // struct enum_array

        template <typename t_enum_type, typename t_data_type>
        struct enum_array<t_enum_type, t_data_type, false>
        {
            t_enum_type value;
        }; // struct enum_array

        template <typename t_enum_type, typename t_data_type>
        void to_json(nlohmann::json& j, const enum_array<t_enum_type, t_data_type, true>& x) noexcept
        {
            // Stored as an object { ..., "<enum key>": value, ... }
            j = { };
            t_data_type z { };
            for (const auto& pair : x) if (pair.second != z) j[detail::to_str(pair.first)] = pair.second;
        } // to_json(...)

        template <typename t_enum_type>
        void to_json(nlohmann::json& j, const enum_array<t_enum_type, bool, true>& x) noexcept
        {
            // Stored as an array [ ..., "<enum key>", ... ]
            std::vector<std::string> y { };
            for (t_enum_type value : x) y.push_back(detail::to_str(value));
            j = y;
        } // to_json(...)
    
        template <typename t_enum_type, typename t_data_type>
        void from_json(const nlohmann::json& j, enum_array<t_enum_type, t_data_type, true>& x) noexcept
        {
            // Stored as an object { ..., "<enum key>": value, ... }
            static constexpr std::size_t count = enum_capacity<t_enum_type>::value;
            for (std::size_t k = 0; k < count; k++)
            {
                t_enum_type key = static_cast<t_enum_type>(k);
                t_data_type value { };
                if (quiet_json::optional(j, detail::to_str(key), value)) x[key] = value;
            }
        } // from_json(...)
    
        template <typename t_enum_type>
        void from_json(const nlohmann::json& j, enum_array<t_enum_type, bool, true>& x) noexcept
        {
            // Stored as an array [ ..., "<enum key>", ... ]
            std::vector<std::string> y { };
            if (!quiet_json::try_parse(j, "enum", y)) return;
            
            for (const std::string& z : y)
            {
                t_enum_type key;
                if (detail::try_parse_str(z, key)) x[key] = true;
                else aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::negligible,
                    std::string("Skipping unrecognized enum: ") + z + std::string("."), __FUNCTION__, __LINE__);
            }
        } // from_json(...)
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    template <typename t_enum_type, typename t_data_type>
    std::string to_string(const ropufu::settlers_online::enum_array<t_enum_type, t_data_type, true>& value) noexcept
    {
        nlohmann::json j = value;
        return j.dump();
    } // to_string(...)
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_ENUM_ARRAY_HPP_INCLUDED
