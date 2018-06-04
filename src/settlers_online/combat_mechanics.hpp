
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED

#include <ropufu/algebra.hpp> // aftermath::algebra::permutation
#include <ropufu/enum_array.hpp>

// ~~ Basic structures ~~
#include "combat_result.hpp"
#include "enums.hpp"
// ~~ Misc ~~
#include "army.hpp"
#include "attack_sequence.hpp"
#include "conditioned_army.hpp"
#include "typedef.hpp"

#include <cstddef> // std::size_t
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        struct combat_mechanics
        {
            using type = combat_mechanics;

        private:
            //static constexpr std::size_t count_phases = enum_capacity<battle_phase>::value;
            aftermath::enum_array<battle_phase, void> m_phases = { };
            conditioned_army m_left; // Left army.
            conditioned_army m_right; // Right army.
            detail::combat_result m_outcome = { };
            bool m_is_in_destruction_phase = false;

        public:
            /** Prepares two armies for combat. */
            combat_mechanics(army& left, army& right)
                : m_left(left, right), m_right(right, left)
            {
            } // combat_mechanics(...)

            const conditioned_army& left() const noexcept { return this->m_left; }
            const conditioned_army& right() const noexcept { return this->m_right; }

            const detail::combat_result& outcome() const noexcept { return this->m_outcome; }

            /** @brief Runs the combat sequence. Can only be called once.
             *  @exception std::logic_error \c execute is called more than once.
             */
            template <typename t_left_sequence_type, typename t_right_sequence_type, typename t_logger_type>
            std::size_t execute(attack_sequence<t_left_sequence_type>& left_sequencer, attack_sequence<t_right_sequence_type>& right_sequencer, t_logger_type& logger)
            {
                if (this->m_is_in_destruction_phase) throw std::logic_error("<execute> cannot be called twice.");
                
                double left_frenzy_bonus = this->m_left.underlying().frenzy_bonus();
                double right_frenzy_bonus = this->m_right.underlying().frenzy_bonus();
                double left_frenzy_factor = 1;
                double right_frenzy_factor = 1;

                std::size_t count_rounds = 0;
                while (this->m_left.underlying().count_units() > 0 && this->m_right.underlying().count_units() > 0)
                {
                    logger << "Begin round " << (1 + count_rounds) << "." << nullptr;
                    for (battle_phase phase : this->m_phases)
                    {
                        logger << "Begin " << std::to_string(phase) << " phase." << nullptr;

                        this->m_left.initiate_phase(phase, this->m_right, left_frenzy_factor, left_sequencer, logger);
                        this->m_right.initiate_phase(phase, this->m_left, right_frenzy_factor, right_sequencer, logger);

                        this->m_left.underlying().snapshot();
                        this->m_right.underlying().snapshot();

                        left_sequencer.next_phase();
                        right_sequencer.next_phase();
                        logger << "End " << std::to_string(phase) << " phase." << nullptr;
                    }

                    left_frenzy_factor *= (1 + left_frenzy_bonus);
                    right_frenzy_factor *= (1 + right_frenzy_bonus);

                    left_sequencer.next_round();
                    right_sequencer.next_round();

                    logger << "End round " << (1 + count_rounds) << "." << nullptr;
                    ++count_rounds;
                }

                mask_type left_alive_mask = this->m_left.underlying().compute_alive_mask();
                mask_type right_alive_mask = this->m_right.underlying().compute_alive_mask();

                this->m_is_in_destruction_phase = true;
                this->m_outcome = detail::combat_result(left_alive_mask, right_alive_mask, count_rounds);
                return count_rounds;
            } // execute(...)

            /** @brief Destruction sequence.
             *  @remark Can be called any number of times. Could be beneficial, since destruction sequence is a lot simpler (faster) than execution.
             *  @exception std::logic_error \c destruct is called prior to \c execute.
             */
            template <typename t_left_sequence_type, typename t_right_sequence_type>
            std::size_t destruct(attack_sequence<t_left_sequence_type>& left_sequencer, attack_sequence<t_right_sequence_type>& right_sequencer) const
            {
                if (!this->m_is_in_destruction_phase) throw std::logic_error("<execute> has to be called prior to destruction phase.");

                if (this->m_outcome.is_left_victorious())
                    return combat_mechanics::destruct(this->m_right.underlying(), this->m_left.underlying().by_mask(this->m_outcome.left_alive_mask()), left_sequencer);
                if (this->m_outcome.is_rihgt_victorious())
                    return combat_mechanics::destruct(this->m_left.underlying(), this->m_right.underlying().by_mask(this->m_outcome.right_alive_mask()), right_sequencer);
                return 0;
            } // destruct(...)

        private:
            /** Destruct the army's camp by a surviving army (attacker). */
            template <typename t_sequence_type>
            static std::size_t destruct(const army& defender, const std::vector<unit_group>& attacker, attack_sequence<t_sequence_type>& sequencer) noexcept
            {
                if (attacker.empty()) return 0;
                sequencer.start_destruction();

                std::size_t count_rounds = 0;
                std::size_t camp_hit_points = defender.camp().hit_points();
                while (camp_hit_points > 0)
                {
                    for (const unit_group& g : attacker)
                    {
                        const unit_type& t = g.unit();

                        bool do_high_damage = sequencer.peek_do_high_damage(t);
                        sequencer.next_unit();

                        std::size_t damage = do_high_damage ? t.damage().high() : t.damage().low();
                        if (t.is(unit_category::artillery)) damage *= 2;

                        if (camp_hit_points < damage) camp_hit_points = 0;
                        else camp_hit_points -= damage;
                    }
                    
                    sequencer.next_round();
                    ++count_rounds;
                }
                return count_rounds;
            } // destruct(...)
        }; // struct combat_mechanics
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
