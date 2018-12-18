
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED

#include <ropufu/algebra.hpp> // aftermath::algebra::permutation
#include <ropufu/enum_array.hpp> // aftermath::enum_array

#include "../enums.hpp"

#include "arithmetic.hpp"
#include "army.hpp"
#include "attack_sequence.hpp"
#include "battle_clock.hpp"
#include "army_mechanics.hpp"
#include "combat_result.hpp"

#include <cstddef> // std::size_t
#include <system_error> // std::error_code, std::errc
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    namespace detail
    {
        struct battle_snapshot
        {
        private:
            const army m_conditioned_army_left;
            const army m_conditioned_army_right;
            const army_mechanics m_mechanics_left;
            const army_mechanics m_mechanics_right;

        public:
            battle_snapshot(const army& a, const army& b, std::error_code& ec) noexcept
                : m_conditioned_army_left(army::condition(a, b, ec)), m_conditioned_army_right(army::condition(b, a, ec)),
                m_mechanics_left(this->m_conditioned_army_left, this->m_conditioned_army_right),
                m_mechanics_right(this->m_conditioned_army_right, this->m_conditioned_army_left)
            {
            } // battle_snapshot(...)

            const army_mechanics& mechanics_left() const noexcept { return this->m_mechanics_left; }
            const army_mechanics& mechanics_right() const noexcept { return this->m_mechanics_right; }
        }; // struct battle_snapshot
    } // namespace detail

    /** @brief Handles the logic of the battle between two armies. */
    template <typename t_left_sequence_type, typename t_right_sequence_type>
    struct battle
    {
        using type = battle<t_left_sequence_type, t_right_sequence_type>;

    private:
        aftermath::enum_array<battle_phase, void> m_phases = {};
        const detail::battle_snapshot m_ground_zero; // Snapshot of the battle before it begins.
        army_mechanics m_left;  // Left army mechanics.
        army_mechanics m_right; // Right army mechanics.
        army_sequence<t_left_sequence_type> m_left_sequence;   // Left sequence.
        army_sequence<t_right_sequence_type> m_right_sequence; // Right sequence.

        detail::combat_result m_outcome = {};
        battle_clock m_clock = {};

    public:
        /** Prepares two un-conditioned armies for combat. */
        battle(const army& left, const army& right, std::error_code& ec) noexcept
            : m_ground_zero(left, right, ec),
            m_left(this->m_ground_zero.mechanics_left()),
            m_right(this->m_ground_zero.mechanics_right()),
            m_left_sequence(this->m_left.underlying()), m_right_sequence(this->m_right.underlying())
        {
        } // battle(...)

        /** Restores the armies to their original state. */
        void reset() noexcept
        {
            this->m_left = this->m_ground_zero.mechanics_left();
            this->m_right = this->m_ground_zero.mechanics_right();

            this->m_outcome = {};
            this->m_clock = {};
        } // reseed(...)

        const army& left() const noexcept { return this->m_left.underlying(); }
        const army& right() const noexcept { return this->m_right.underlying(); }

        const army_mechanics& left_mechanics() const noexcept { return this->m_left; }
        const army_mechanics& right_mechanics() const noexcept { return this->m_right; }

        const battle_clock& clock() const noexcept { return this->m_clock; }

        /** @todo Think: where do we actually need it? */
        const detail::combat_result& outcome() const noexcept { return this->m_outcome; }

        /** @brief Runs the combat sequence. Can only be called once.
         *  @param ec Set to \c std::errc::operation_not_permitted if the function has already been called.
         */
        template <typename t_logger_type>
        std::size_t execute(t_logger_type& logger, std::error_code& ec) noexcept
        {
            if (this->m_clock.is_destruction())
            {
                ec = std::make_error_code(std::errc::operation_not_permitted);
                return 0;
            } // if (...)
            
            double left_frenzy_bonus = this->m_left.underlying().frenzy_bonus();
            double right_frenzy_bonus = this->m_right.underlying().frenzy_bonus();
            double left_frenzy_factor = 1;
            double right_frenzy_factor = 1;

            while (this->m_left.underlying().alive() && this->m_right.underlying().alive())
            {
                if constexpr (t_logger_type::is_enabled)
                {
                    logger << "Begin round " << (1 + this->m_clock.round_index()) << "." << nullptr;
                    if (left_frenzy_factor != 1) logger << "Left frenzy bonus: " << static_cast<std::size_t>(100 * (left_frenzy_factor - 1)) << "%." << nullptr;
                    if (right_frenzy_factor != 1) logger << "Right frenzy bonus: " << static_cast<std::size_t>(100 * (right_frenzy_factor - 1)) << "%." << nullptr;
                } // if constexpr (...)

                for (battle_phase phase : this->m_phases)
                {
                    if constexpr (t_logger_type::is_enabled) logger << "Begin " << std::to_string(phase) << " phase." << nullptr;

                    this->m_left.initiate_phase(this->m_right, phase, left_frenzy_factor, this->m_left_sequence, this->m_clock, logger);
                    this->m_right.initiate_phase(this->m_left, phase, right_frenzy_factor, this->m_right_sequence, this->m_clock, logger);

                    this->m_left.snapshot();
                    this->m_right.snapshot();

                    this->m_clock.next_phase();
                    if constexpr (t_logger_type::is_enabled) logger << "End " << std::to_string(phase) << " phase." << nullptr;
                } // for (...)

                left_frenzy_factor *= (1 + left_frenzy_bonus);
                right_frenzy_factor *= (1 + right_frenzy_bonus);

                this->m_clock.next_round();
                if constexpr (t_logger_type::is_enabled) logger << "End round " << (1 + this->m_clock.round_index()) << "." << nullptr;
            } // while (...)

            typename detail::combat_result::mask_type left_alive_mask = this->m_left.underlying().compute_alive_mask();
            typename detail::combat_result::mask_type right_alive_mask = this->m_right.underlying().compute_alive_mask();

            this->m_outcome = { left_alive_mask, right_alive_mask, this->m_clock.round_index() };
            this->m_clock.start_destruction();
            return this->m_outcome.number_of_rounds;
        } // execute(...)

        /** @brief Destruction sequence.
         *  @param ec Set to \c std::errc::operation_not_permitted if the function is called prior to \c execute(...) -> ...
         *  @remark Can be called any number of times. Could be beneficial, since destruction sequence is a lot simpler (faster) than execution.
         */
        std::size_t peek_destruction(std::error_code& ec) noexcept
        {
            if (!this->m_clock.is_destruction())
            {
                ec = std::make_error_code(std::errc::operation_not_permitted);
                return 0;
            } // if (...)
            
            battle_clock destruction_clock = this->m_clock; // Make a copy of current clock.
            if (this->m_outcome.is_left_victorious()) return this->m_left.destruct(this->m_right, this->m_left_sequence, destruction_clock);
            if (this->m_outcome.is_rihgt_victorious()) return this->m_right.destruct(this->m_left, this->m_right_sequence, destruction_clock);
            return 0;
        } // destruct(...)
    }; // struct battle
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
