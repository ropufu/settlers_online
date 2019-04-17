
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED

#include "../algebra.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::uint_fast64_t
#include <functional> // std::hash
#include <ostream>    // std::ostream

namespace ropufu::settlers_online
{
    namespace detail
    {
        struct combat_result
        {
            using type = combat_result;
            // using mask_type = std::bitset<max_size>;
            using mask_type = std::uint_fast64_t; // TODO: think: why do we even need binary masks??

            static constexpr std::size_t byte_size_in_bits = 8;
            // The maximum number of groups in an army.
            static constexpr std::size_t army_capacity = byte_size_in_bits * sizeof(mask_type);

            mask_type left_alive_mask = {};
            mask_type right_alive_mask = {};
            std::size_t number_of_rounds = {};

            // combat_result() noexcept { }

            bool is_left_victorious() const noexcept { return this->left_alive_mask != 0; }
            bool is_rihgt_victorious() const noexcept { return this->right_alive_mask != 0; }

            /** Checks two types for equality. */
            bool operator ==(const type& other) const noexcept
            {
                return
                    this->left_alive_mask == other.left_alive_mask &&
                    this->right_alive_mask == other.right_alive_mask &&
                    this->number_of_rounds == other.number_of_rounds;
            } // operator ==(...)

            /** Checks two types for inequality. */
            bool operator !=(const type& other) const noexcept
            {
                return !(this->operator ==(other));
            } // operator !=(...)

            friend std::ostream& operator <<(std::ostream& os, const type& self)
            {
                os << "a" << self.left_alive_mask << " d" << self.right_alive_mask << " r" << self.number_of_rounds;
                return os;
            } // operator <<(...)
        }; // struct combat_result
    } // namespace detail
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::detail::combat_result>
    {
        using argument_type = ropufu::settlers_online::detail::combat_result;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<typename ropufu::settlers_online::detail::combat_result::mask_type> mask_hash = {};
            std::hash<std::size_t> size_hash = {};

            return
                mask_hash(x.left_alive_mask) ^
                mask_hash(x.right_alive_mask) ^
                size_hash(x.number_of_rounds);
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED
