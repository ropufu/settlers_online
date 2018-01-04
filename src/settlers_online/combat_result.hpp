
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED

#include "typedef.hpp"

#include <cstddef> // std::size_t
#include <functional> // std::hash
#include <ostream> // std::ostream

namespace ropufu
{
    namespace settlers_online
    {
        namespace detail
        {
            struct combat_result
            {
                using type = combat_result;

            private:
                mask_type m_left_alive_mask = { };
                mask_type m_right_alive_mask = { };
                std::size_t m_number_of_rounds = { };

            public:
                combat_result() noexcept { };

                combat_result(mask_type left_alive_mask, mask_type right_alive_mask, std::size_t number_of_rounds) noexcept
                    : m_left_alive_mask(left_alive_mask), m_right_alive_mask(right_alive_mask), m_number_of_rounds(number_of_rounds)
                {
                } // combat_result(...)

                bool is_left_victorious() const noexcept { return this->m_left_alive_mask != 0; }

                bool is_rihgt_victorious() const noexcept { return this->m_right_alive_mask != 0; }

                mask_type left_alive_mask() const noexcept { return this->m_left_alive_mask; }

                mask_type right_alive_mask() const noexcept { return this->m_right_alive_mask; }

                std::size_t number_of_rounds() const noexcept { return this->m_number_of_rounds; }

                /** Checks two types for equality. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_left_alive_mask == other.m_left_alive_mask &&
                        this->m_right_alive_mask == other.m_right_alive_mask &&
                        this->m_number_of_rounds == other.m_number_of_rounds;
                } // operator ==(...)

                /** Checks two types for inequality. */
                bool operator !=(const type& other) const noexcept
                {
                    return !(this->operator ==(other));
                } // operator !=(...)

                friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
                {
                    os << "a" << self.m_left_alive_mask << " d" << self.m_right_alive_mask << " r" << self.m_number_of_rounds;
                    return os;
                } // operator <<(...)
            }; // struct combat_result
        } // namespace detail
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::detail::combat_result>
    {
        using argument_type = ropufu::settlers_online::detail::combat_result;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const
        {
            std::hash<ropufu::settlers_online::mask_type> mask_hash = { };
            std::hash<std::size_t> size_hash = { };

            return
                mask_hash(x.left_alive_mask()) ^
                mask_hash(x.right_alive_mask()) ^
                size_hash(x.number_of_rounds());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED
