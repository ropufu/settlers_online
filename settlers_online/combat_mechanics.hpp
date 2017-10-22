
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED

#include <aftermath/algebra.hpp> // aftermath::algebra::permutation
#include <aftermath/not_an_error.hpp>

#include "army.hpp"
#include "attack_sequence.hpp"
#include "battle_phase.hpp"
#include "combat_result.hpp"
#include "conditioned_army.hpp"
#include "enum_array.hpp"
#include "typedef.hpp"

#include <cstddef> // std::size_t
#include <iostream> // std::cout
#include <stdexcept> // std::logic_error
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        struct combat_mechanics
        {
        private:
            static constexpr std::size_t count_phases = enum_capacity<battle_phase>::value;

            bool m_do_log = false;
            conditioned_army m_left; // Left army.
            conditioned_army m_right; // Right army.
            combat_result m_outcome = { };
            bool m_is_in_destruction_phase = false;

        public:

            /** Prepares two armies for combat. */
            combat_mechanics(army& left, army& right)
                : m_left(left, right), m_right(right, left)
            {
            }

            bool do_log() const noexcept { return this->m_do_log; }
            void set_do_log(bool value) noexcept { this->m_do_log = value; }

            const conditioned_army& left() const noexcept { return this->m_left; }
            const conditioned_army& right() const noexcept { return this->m_right; }

            /** Runs the combat sequence.
             *  @todo Think of merging the two sequencers into one, to reduce the chances of errors.
             */
            template <typename t_left_sequence_type, typename t_right_sequence_type>
            combat_result execute(attack_sequence<t_left_sequence_type>& left_sequencer, attack_sequence<t_right_sequence_type>& right_sequencer)
            {
                if (this->m_is_in_destruction_phase) throw std::logic_error("<execute> cannot be called twice.");
                
                double left_frenzy_bonus = this->m_left.underlying().frenzy_bonus();
                double right_frenzy_bonus = this->m_right.underlying().frenzy_bonus();
                double left_frenzy_factor = 1;
                double right_frenzy_factor = 1;

                std::size_t count_rounds = 0;
                while (this->m_left.underlying().count_units() > 0 && this->m_right.underlying().count_units() > 0)
                {
                    if (this->m_do_log) std::cout << "Begin round " << (1 + count_rounds) << "." << std::endl;
                    for (std::size_t k = 0; k < count_phases; k++)
                    {
                        battle_phase phase = static_cast<battle_phase>(k);
                        if (this->m_do_log) std::cout << "Begin " << std::to_string(phase) << " phase." << std::endl;

                        this->m_left.initiate_phase(phase, this->m_right, left_frenzy_factor, left_sequencer, this->m_do_log);
                        this->m_right.initiate_phase(phase, this->m_left, right_frenzy_factor, right_sequencer, this->m_do_log);

                        this->m_left.underlying().snapshot();
                        this->m_right.underlying().snapshot();

                        left_sequencer.next_phase();
                        right_sequencer.next_phase();
                        if (this->m_do_log) std::cout << "End " << std::to_string(phase) << " phase." << std::endl;
                    }

                    left_frenzy_factor *= (1 + left_frenzy_bonus);
                    right_frenzy_factor *= (1 + right_frenzy_bonus);

                    left_sequencer.next_round();
                    right_sequencer.next_round();

                    if (this->m_do_log) std::cout << "End round " << (1 + count_rounds) << "." << std::endl;
                    ++count_rounds;
                }

                mask_type left_alive_mask = this->m_left.underlying().compute_alive_mask();
                mask_type right_alive_mask = this->m_right.underlying().compute_alive_mask();

                this->m_is_in_destruction_phase = true;
                this->m_outcome = combat_result(left_alive_mask, right_alive_mask, count_rounds);
                return this->m_outcome;
            }

            /** @brief Destruvtion sequence.
             *  @remark Can be called any number of times. Could be beneficial, since destruction sequence is a lot simpler (faster) than execution.
             */
            template <typename t_left_sequence_type, typename t_right_sequence_type>
            std::size_t destruct(attack_sequence<t_left_sequence_type>& left_sequencer, attack_sequence<t_right_sequence_type>& right_sequencer) const
            {
                if (!this->m_is_in_destruction_phase) throw std::logic_error("<execute> has to be called prior to destruction phase.");
                left_sequencer.start_destruction();
                right_sequencer.start_destruction();

                if (this->m_outcome.is_left_victorious())
                    return combat_mechanics::destruct(this->m_right.underlying(), this->m_left.underlying().by_mask(this->m_outcome.left_alive_mask()), left_sequencer);
                else if (this->m_outcome.is_rihgt_victorious())
                    return combat_mechanics::destruct(this->m_left.underlying(), this->m_right.underlying().by_mask(this->m_outcome.right_alive_mask()), right_sequencer);
                else return 0;
            }

        private:
            /** Destruct the army's camp by a surviving army (attacker). */
            template <typename t_sequence_type>
            static std::size_t destruct(const army& defender, const std::vector<unit_group>& attacker, attack_sequence<t_sequence_type>& sequencer)
            {
                if (attacker.empty()) return 0;

                std::size_t count_rounds = 0;
                std::size_t camp_hit_points = defender.camp_hit_points();
                while (camp_hit_points > 0)
                {
                    for (const unit_group& g : attacker)
                    {
                        const unit_type& t = g.type();

                        bool do_high_damage = sequencer.peek_do_high_damage(t);
                        sequencer.next_unit();

                        std::size_t damage = do_high_damage ? t.damage().high() : t.damage().low();
                        if (t.category() == unit_category::artillery) damage *= 2;

                        if (camp_hit_points < damage) camp_hit_points = 0;
                        else camp_hit_points -= damage;
                    }
                    count_rounds++;
                }
                //defender.set_camp_hit_points(camp_hit_points);
                return count_rounds;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
