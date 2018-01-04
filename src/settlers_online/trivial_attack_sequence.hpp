
#ifndef ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED

#include "attack_sequence.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t

namespace ropufu
{
    namespace settlers_online
    {
        /** Attack sequence where each attack always results in the same low or high value.  */
        template <bool t_is_always_high, bool t_is_always_splash = t_is_always_high>
        struct trivial_attack_sequence : public attack_sequence<trivial_attack_sequence<t_is_always_high, t_is_always_splash>>
        {
            static constexpr bool is_always_high = t_is_always_high;
            static constexpr bool is_always_splash = t_is_always_splash;

            using type = trivial_attack_sequence<is_always_high, is_always_splash>;
            using base_type = attack_sequence<type>;

            /** @brief Indicates whether the current unit will do high damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_high_damage(const unit_type& unit) noexcept
            {
                double x = unit.damage().accuracy();
                if (x == 0) return false;
                if (x == 1) return true;
                return is_always_high;
            } // peek_do_high_damage(...)
            
            /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
             *  @param unit Type of attacking units.
             *  @param count_units Number of attacking units.
             */
            std::size_t peek_count_high_damage(const unit_type& unit, std::size_t count_units) noexcept
            {
                double x = unit.damage().accuracy();
                if (x == 0) return 0;
                if (x == 1) return count_units;
                return is_always_high ? count_units : 0;
            } // peek_count_high_damage(...)

            /** @brief Indicates whether the current unit will do splash damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_splash(const unit_type& unit) noexcept
            {
                double x = unit.damage().splash_chance();
                if (x == 0) return false;
                if (x == 1) return true;
                return is_always_splash;
            } // peek_do_splash(...)

            /** @brief Indicates whether the previous unit did splash damage.
             *  @param unit Type of attacking unit.
             */
            bool did_last_splash(const unit_type& unit) noexcept
            {
                double x = unit.damage().splash_chance();
                if (x == 0) return false;
                if (x == 1) return true;
                return is_always_splash;
            } // did_last_splash(...)
        }; // struct trivial_attack_sequence
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED
