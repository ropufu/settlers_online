
#ifndef ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED

#include "attack_sequence.hpp"
#include "battle_clock.hpp"
#include "damage.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    /** Attack sequence where each attack always results in the same low or high value.  */
    template <bool t_is_always_high, bool t_is_always_splash = t_is_always_high>
    struct trivial_attack_sequence : public attack_sequence<trivial_attack_sequence<t_is_always_high, t_is_always_splash>>
    {
        static constexpr bool is_always_high = t_is_always_high;
        static constexpr bool is_always_splash = t_is_always_splash;

        using type = trivial_attack_sequence<is_always_high, is_always_splash>;
        using base_type = attack_sequence<type>;

    private:
        bool m_is_always_high = type::is_always_high;
        bool m_is_always_splash = type::is_always_splash;

    public:
        trivial_attack_sequence() noexcept { }

        trivial_attack_sequence(const unit_group& g, std::size_t /*group_index*/, std::error_code& /*ec*/) noexcept
        {
            const damage& d = g.unit().damage();

            if (d.accuracy() == 0) this->m_is_always_high = false;
            if (d.accuracy() == 1) this->m_is_always_high = true;

            if (d.splash_chance() == 0) this->m_is_always_splash = false;
            if (d.splash_chance() == 1) this->m_is_always_splash = true;
        } // trivial_attack_sequence(...)

        /** @brief Indicates whether the current unit will do high damage. */
        bool peek_do_high_damage(const battle_clock& /*clock*/) noexcept
        {
            return this->m_is_always_high;
        } // peek_do_high_damage(...)
        
        /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
         *  @param count_units Number of attacking units.
         */
        std::size_t peek_count_high_damage(std::size_t count_units, const battle_clock& /*clock*/) noexcept
        {
            return this->m_is_always_high ? count_units : 0;
        } // peek_count_high_damage(...)

        /** @brief Indicates whether the current unit will do splash damage. */
        bool peek_do_splash(const battle_clock& /*clock*/) noexcept
        {
            return this->m_is_always_splash;
        } // peek_do_splash(...)

        /** @brief Indicates whether the previous unit did splash damage. */
        bool did_last_splash(const battle_clock& /*clock*/) noexcept
        {
            return this->m_is_always_splash;
        } // did_last_splash(...)
    }; // struct trivial_attack_sequence
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_TRIVIAL_ATTACK_SEQUENCE_HPP_INCLUDED
