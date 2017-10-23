
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED

#include "typedef.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>

namespace ropufu
{
    namespace settlers_online
    {
        struct combat_result
        {
        private:
            mask_type m_left_alive_mask = {};
            mask_type m_right_alive_mask = {};
            std::size_t m_number_of_rounds = {};

        public:
            combat_result() = default;

            combat_result(mask_type left_alive_mask, mask_type right_alive_mask, std::size_t number_of_rounds)
                : m_left_alive_mask(left_alive_mask), m_right_alive_mask(right_alive_mask), m_number_of_rounds(number_of_rounds)
            {

            }

            bool is_left_victorious() const { return this->m_left_alive_mask != 0; }

            bool is_rihgt_victorious() const { return this->m_right_alive_mask != 0; }

            mask_type left_alive_mask() const { return this->m_left_alive_mask; }

            mask_type right_alive_mask() const { return this->m_right_alive_mask; }

            std::size_t number_of_rounds() const { return this->m_number_of_rounds; }

            bool operator ==(const combat_result& other) const
            {
                return
                    this->m_left_alive_mask == other.m_left_alive_mask &&
                    this->m_right_alive_mask == other.m_right_alive_mask &&
                    this->m_number_of_rounds == other.m_number_of_rounds;
            }

            bool operator !=(const combat_result& other) const
            {
                return !(this->operator ==(other));
            }

            friend std::ostream& operator <<(std::ostream& os, const combat_result& that)
            {
                os << "a" << that.m_left_alive_mask << " d" << that.m_right_alive_mask << " r" << that.m_number_of_rounds;
                return os;
            }
        };
    }
}

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::combat_result>
    {
        typedef ropufu::settlers_online::combat_result argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& x) const
        {
            std::hash<ropufu::settlers_online::mask_type> mask_hash = {};
            std::hash<std::size_t> size_hash = {};

            return
                mask_hash(x.left_alive_mask()) ^
                mask_hash(x.right_alive_mask()) ^
                size_hash(x.number_of_rounds());
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_RESULT_HPP_INCLUDED
