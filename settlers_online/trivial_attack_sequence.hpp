
#ifndef ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED

#include <settlers_online/attack_sequence.hpp>

#include <cstddef>

namespace ropufu
{
    namespace settlers_online
    {
        /** Attack sequence where each attack always results in the same low or high value.  */
        template <bool t_is_always_high, bool t_is_always_splash = t_is_always_high>
        struct trivial_attack_sequence : public attack_sequence<trivial_attack_sequence>
        {
            static constexpr bool is_always_high = t_is_always_high;
            static constexpr bool is_always_splash = t_is_always_splash;

        private:
            /** @brief Indicates whether the current unit will do high damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_high_damage_impl(const unit_type& unit) const noexcept
            {
                if (unit.accuracy() == 0) return false;
                if (unit.accuracy() == 1) return true;
                return is_always_high;
            }
            
            /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
             *  @param unit Type of attacking units.
             *  @param count_units Number of attacking units.
             */
            std::size_t peek_count_high_damage_impl(const unit_type& unit, std::size_t count_units) const noexcept
            {
                if (unit.accuracy() == 0) return 0;
                if (unit.accuracy() == 1) return count_units;
                return is_always_high ? count_units : 0;
            }

            /** @brief Indicates whether the current unit will do splash damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_splash_impl(const unit_type& unit) const noexcept
            {
                if (unit.splash_chance() == 0) return false;
                if (unit.splash_chance() == 1) return true;
                return is_always_splash;
            }

            /** @brief Indicates whether the previous unit did splash damage.
             *  @param unit Type of attacking unit.
             */
            bool did_last_splash_impl(const unit_type& unit) const noexcept
            {
                if (unit.splash_chance() == 0) return false;
                if (unit.splash_chance() == 1) return true;
                return is_always_splash;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED
