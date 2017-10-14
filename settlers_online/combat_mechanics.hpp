
#ifndef ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED

#include <aftermath/algebra.hpp>

#include "army.hpp"
#include "attack_sequence.hpp"
#include "special_abilities.hpp"
#include "typedef.hpp"
#include "unit_category.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ropufu
{
    namespace settlers_online
    {
        struct combat_mechanics
        {
        private:
            army m_left;
            army m_right;
            bool m_is_left_conditioned = false;
            bool m_is_right_conditioned = false;

        public:
            /** Adjusts \p damage by \p factor (multiplicative). */
            static std::size_t damage_cast(std::size_t damage, double factor)
            {
                return product_floor(damage, factor);
            }

            /** Determines the smallest pure damage required to eliminate a unit with \p hit_points with (multiplicative) \p factor damage modifier. */
            static std::size_t inverse_damage_cast(std::size_t hit_points, double factor)
            {
                return product_ceiling(hit_points, 1.0 / factor);
            }

            /** Adjusts \p hit_points by \p factor (multiplicative). */
            static std::size_t hit_points_cast(std::size_t hit_points, double factor)
            {
                return product_ceiling(hit_points, factor);
            }

            /** Prepares two armies for combat. */
            combat_mechanics(army& left, army& right)
                : m_left(left), m_right(right)
            {
                this->condition();
            }

        private:
            /** @brief Inflicts \p pure_damage splash onto \p defending_group, taking into account \p damage_factor damage multiplier.
             *  @return Remaining \p pure_damage.
             */
            static double splash(std::size_t pure_damage, double damage_factor, unit_group& defending_group);

            /** Inflicts the reduced damage, \p reduced_damage, onto \p defender, assuming the attaker always deals splash damage and there is no effective tower bonus. */
            static void uniform_splash(std::size_t reduced_damage, army& defender, const aftermath::algebra::permutation& defender_ordering);

            /** @brief When attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
             *  @return The attacking unit index after the attack has been completed.
             */
            static std::size_t one_to_one(unit_group& defending_group, const unit_group& attacking_group, std::size_t attacking_unit_index);

            /** @brief Initiates and attack on a group (defender) by another group (attacker).
             *  @return The attacking unit index after the attack has been completed.
             */
            template <typename t_sequence_type>
            static std::size_t hit(std::size_t& overshoot_damage, unit_group& defending_group,
                const unit_group& attacking_group, std::size_t attacking_unit_index, double damage_factor, attack_sequence<t_sequence_type>& sequencer) noexcept
            {
                const unit_type& attacker_t = attacking_group.type();
                const unit_type& defender_t = defending_group.type();

                std::size_t defending_units_remaining = defending_group.count();
                if (defending_units_remaining == 0) return attacking_unit_index; // Check if the defending group is empty.

                // Proceed to next attaking unit.
                std::size_t attacking_units_remaining = attacking_group.count_at_snapshot() - attacking_unit_index;

                std::size_t effective_min_damage = damage_cast(attacker_t.min_damage(), damage_factor); // Potential effective minumum damage dealt by one attacking unit.
                std::size_t effective_max_damage = damage_cast(attacker_t.max_damage(), damage_factor); // Potential effective mamimum damage dealt by one attacking unit.
                // Loop through attacking units.
                while (attacking_units_remaining > 0)
                {
                    // If this group has been eliminated proceed to the next group.
                    if (defending_units_remaining == 0) return attacking_unit_index;
                    std::size_t hit_points = defending_group.top_hit_points(); // Hit points of the top unit.

                    std::size_t count_attackers = fraction_ceiling(hit_points, effective_max_damage); // Least number of units required to kill the current defending unit.
                    if (count_attackers > attacking_units_remaining) count_attackers = attacking_units_remaining; // Cap to prevent \c attacking_unit_index overflow.
                    std::size_t count_high_damage = sequencer.peek_count_high_damage(attacker_t, count_attackers); // Number of units dealing maximum damage.
                    sequencer.next_unit(count_attackers);
                    attacking_unit_index += count_attackers;
                    attacking_units_remaining -= count_attackers;

                    std::size_t effective_damage = effective_max_damage * count_high_damage + effective_min_damage * (count_attackers - count_high_damage); // Effective damage dealt by the attacker.
                    std::size_t pure_damage = attacker_t.max_damage() * count_high_damage + attacker_t.min_damage() * (count_attackers - count_high_damage); // Pure damage dealt by the attacker.
                    std::size_t damage_required = inverse_damage_cast(hit_points, damage_factor); // Pure damage required to kill the defending unit.

                    // In general, it is not(!) enough to check (effective_damage > hit_points).
                    // For example:
                    //     11 pure damage vs. 5 hp and 0.5 reducion; or
                    //     10 pure damage vs. 5 hp and 0.5 reducion.
                    // The first case yields 1 damage overshoot, whereas the second does not.
                    // However, in the case that there is at least one more unit from this group,
                    // the check (pure_damage > damage_required) is unnecessary, as the overshoot
                    // damage will be reduced to 0 anyway.
                    if (pure_damage > damage_required)
                    {
                        defending_group.kill_top(); // Kill the top defending unit.
                        defending_units_remaining--;
                        bool do_splash = sequencer.did_last_splash(attacker_t); // Check whether splash damage occurs.
                        overshoot_damage = do_splash ? (pure_damage - damage_required) : 0;
                    }
                    else // The inflicted damage does not exceed hit points.
                    {
                        bool is_dead = defending_group.try_kill_top(effective_damage); // Damage the top defending unit.
                        if (is_dead) defending_units_remaining--;
                        overshoot_damage = 0;
                    }

                    // If there is overshoot damage, spread it over this group.
                    while (overshoot_damage > 0)
                    {
                        // If this group has been eliminated proceed to the next group.
                        if (defending_units_remaining == 0) return attacking_unit_index;

                        // Since it is overshoot damage, each consecutive defending unit has full hit points.
                        damage_required = inverse_damage_cast(defender_t.hit_points(), damage_factor); // Pure damage required to kill the defending unit.
                        if (overshoot_damage > damage_required)
                        {
                            overshoot_damage -= damage_required;
                            defending_group.kill_top(); // Kill the top defending unit.
                            defending_units_remaining--;
                        }
                        else
                        {
                            effective_damage = damage_cast(overshoot_damage, damage_factor);
                            overshoot_damage = 0;
                            bool is_dead = defending_group.try_kill_top(effective_damage); // Damage the top defending unit.
                            if (is_dead) defending_units_remaining--;
                        }
                    }
                }
            }

            /** Initiates and attack on this army (defender) by a \c unit_group (attacker). */
            template <typename t_sequence_type>
            static void hit(army& defender, const unit_group& attacking_group, double frenzy_factor, attack_sequence<t_sequence_type>& sequencer)
            {
                if (attacking_group.empty()) return;
                const unit_type& attacker_t = attacking_group.type();

                // Damage factors.
                bool do_ignore_tower_bonus = attacker_t.has(special_abilities::ignore_tower_bonus);
                bool has_effective_tower_bonus = (!do_ignore_tower_bonus && defender.has_tower_bonus());
                double damage_factor_normal = frenzy_factor * (1.0 - defender.direct_damage_reduction());
                double damage_factor_in_tower = frenzy_factor * (1.0 - defender.direct_damage_reduction()) * (1.0 - defender.tower_damage_reduction());

                // Figure out which ordering to use.
                bool do_attack_weakest_target = defender.do_intercept() ? false : attacker_t.do_attack_weakest_target();
                const aftermath::algebra::permutation& defender_ordering = do_attack_weakest_target ? defender.order_by_hp() : defender.order_by_id();

                // Optimize in the uniform all splash damage case.
                if (!has_effective_tower_bonus && attacker_t.splash_chance() == 1.0)
                {
                    std::size_t count_attackers = attacking_group.count_at_snapshot();
                    std::size_t count_high_damage = sequencer.peek_count_high_damage(attacker_t, count_attackers); // Count the number of units in this stack that do maximum damage.
                    sequencer.next_unit(count_attackers);

                    std::size_t total_reduced_damage = 
                        damage_cast(attacker_t.min_damage(), damage_factor_normal) * (count_attackers - count_high_damage) +
                        damage_cast(attacker_t.max_damage(), damage_factor_normal) * count_high_damage;

                    combat_mechanics::uniform_splash(total_reduced_damage, defender, defender_ordering);
                    return;
                }

                std::size_t attacking_unit_index = 0;
                std::size_t overshoot_damage = 0; // Pure damage spilled between consequent groups.
                std::vector<unit_group>& defender_groups = defender.groups();
                for (std::size_t j : defender_ordering)
                {
                    unit_group& defending_group = defender_groups[j];
                    const unit_type& defender_t = defending_group.type();
                    bool do_tower_bonus = (!do_ignore_tower_bonus && defender_t.has(special_abilities::tower_bonus));
                    double damage_factor = do_tower_bonus ? damage_factor_in_tower : damage_factor_normal;
                    // First take care of the overshoot damage.
                    overshoot_damage = combat_mechanics::splash(overshoot_damage, damage_factor, defending_group);

                    // Defender has already been killed.
                    std::size_t count_alive = defending_group.count();
                    if (count_alive == 0) continue;

                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    std::size_t effective_min_damage = damage_cast(attacker_t.min_damage(), damage_factor);
                    bool is_one_to_one = (effective_min_damage >= defender_t.hit_points()) && (attacker_t.splash_chance() == 0.0);
                    if (is_one_to_one) attacking_unit_index = combat_mechanics::one_to_one(defending_group, attacking_group, attacking_unit_index);
                    else attacking_unit_index = hit(defending_group, attacking_group, attacking_unit_index, sequencer);

                    if (attacking_unit_index == attacking_group.count_at_snapshot()) break;
                    if (attacking_unit_index > attacking_group.count_at_snapshot()) throw std::logic_error("<attacking_unit_index> overflow");
                }
            }

            /** Destruct the army's camp by a surviving army (attacker). */
            template <typename t_sequence_type>
            std::size_t destruct(army& defender, const std::vector<unit_group>& attacker, attack_sequence<t_sequence_type>& sequencer) const
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

                        std::size_t damage = do_high_damage ? t.max_damage() : t.min_damage();
                        if (t.category() == unit_category::artillery) damage *= 2;

                        if (camp_hit_points < damage) camp_hit_points = 0;
                        else camp_hit_points -= damage;
                    }
                    count_rounds++;
                }
                defender.set_camp_hit_points(camp_hit_points);
                return count_rounds;
            }

            /** @brief Conditions the left and right armies.
             *  @exception std::logic_error Can only be executed once.
             */
            void condition()
            {
                if (this->m_is_left_conditioned || this->m_is_right_conditioned) throw std::logic_error("<condition> cannot be called twice");

                this->m_is_left_conditioned = true;
                this->m_is_right_conditioned = true;

                this->condition_defender(this->m_left, this->m_right);
                this->condition_defender(this->m_right, this->m_left);
            }

            /** Conditions the \p defender against \p attacker. */
            static void condition_defender(army& defender, const army& attacker);
        };

        double combat_mechanics::splash(std::size_t pure_damage, double damage_factor, unit_group& defending_group)
        {
            if (pure_damage == 0) return 0;
            // Total hit points in the defending group.
            std::size_t total_hit_points = defending_group.total_hit_points();
            // Pure damage required to eliminate the defending group.
            std::size_t damage_required = inverse_damage_cast(total_hit_points, damage_factor);
            if (damage_required > pure_damage) // Remaining damage is not enough to eliminate this group.
            {
                // Calculate the actual damage dealt.
                std::size_t effective_damage = damage_cast(pure_damage, damage_factor);
                // Damage the defending group.
                total_hit_points -= effective_damage;
                defending_group.set_total_hit_points(total_hit_points);
                // All the splash damage has been accounted for.
                pure_damage = 0;
            }
            else // The defending group is guaranteed to be eliminated.
            {
                // Kill the current defending group.
                defending_group.kill_all();
                // Calculate the remaining splash damage.
                pure_damage -= damage_required;
            }
            return pure_damage;
        }

        void combat_mechanics::uniform_splash(std::size_t reduced_damage, army& defender, const aftermath::algebra::permutation& defender_ordering)
        {
            std::vector<unit_group>& defender_groups = defender.groups();
            for (std::size_t j : defender_ordering)
            {
                unit_group& defending_group = defender_groups[j];
                std::size_t total_hit_points = defending_group.total_hit_points();
                if (total_hit_points == 0) continue;
                if (total_hit_points >= reduced_damage) // All the splash damage has been accounted for.
                {
                    defending_group.set_total_hit_points(total_hit_points - reduced_damage); // Damage the defending group.
                    return;
                }
                defending_group.kill_all();
                reduced_damage -= total_hit_points;
            }
        }

        std::size_t combat_mechanics::one_to_one(unit_group& defending_group, const unit_group& attacking_group, std::size_t attacking_unit_index)
        {
            // Proceed to next attaking unit.
            std::size_t attacking_units_remaining = attacking_group.count_at_snapshot() - attacking_unit_index;
            std::size_t count_alive = defending_group.count();
            if (attacking_units_remaining > count_alive)
            {
                defending_group.kill_all(); // The entire group has been eliminated.
                return attacking_unit_index + count_alive; // All defenders have been killed, and there still may be attackers left.
            }
            else
            {
                // All attacking units have made their hit, and there still may be survivors left.
                defending_group.kill(attacking_units_remaining);
                return attacking_group.count_at_snapshot();
            }
        }

        void combat_mechanics::condition_defender(army& defender, const army& attacker)
        {
            for (unit_group& g : defender.groups())
            {
                unit_type t = g.type();
                unit_category cat = t.category();

                std::size_t hit_points = t.hit_points();
                std::size_t min_damage = t.min_damage() + defender.min_damage_bonus_for(cat);
                std::size_t max_damage = t.max_damage() + defender.max_damage_bonus_for(cat);
                double accuracy = t.accuracy() + defender.accuracy_bonus_for(cat);
                double splash_chance = t.splash_chance() + defender.splash_bonus_for(cat);

                if (defender.do_explosive_ammunition() && (cat == unit_category::ranged)) t.set_do_attack_weakest_target(true);
                if (attacker.do_intercept()) t.set_do_attack_weakest_target(false);

                if (t.has(special_abilities::boss)) hit_points = hit_points_cast(hit_points, 1.0 - attacker.boss_health_reduction());

                if (attacker.do_dazzle()) accuracy = 0.0;
                if (accuracy > 1.0) accuracy = 1.0;

                t.set_damage(min_damage, max_damage, accuracy, splash_chance);
                t.set_hit_points(hit_points);
                g.set_type(t);
            }
        }
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_COMBAT_MECHANICS_HPP_INCLUDED
