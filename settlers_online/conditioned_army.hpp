
#ifndef ROPUFU_SETTLERS_ONLINE_CONDITIONED_ARMY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CONDITIONED_ARMY_HPP_INCLUDED

#include <aftermath/algebra.hpp> // aftermath::algebra::permutation
#include <aftermath/not_an_error.hpp>

#include "army.hpp"
#include "attack_sequence.hpp"
#include "battle_phase.hpp"
#include "battle_trait.hpp"
#include "combat_result.hpp"
#include "enum_array.hpp"
#include "special_ability.hpp"
#include "technical_combat.hpp"
#include "typedef.hpp"
#include "unit_category.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <iostream> // std::cout
#include <set> // std::set
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        /** An auxiliary structure to store parameters determining how a specific group will to attack another army. */
        struct attack_group_cache
        {
        private:
            aftermath::algebra::permutation m_attack_order = { }; // Order in which enemy units are to be attacked.
            std::vector<double> m_damage_factors = { }; // Damage factors agains defender units, without frenzy bonus.
            std::vector<bool> m_is_one_to_one = { }; // Indicates that (i) there is no splash damage and (ii) the minimal effective damage is enough to kill one defeding unit.
            bool m_is_uniform_splash = false; // Indicates that (i) there is always splash damage and (ii) there is no effective damage reduction that can prevent splash optimization.
            double m_uniform_damage_factor = 0;

        public:
            /** @brief Stores battle parameters of the unit group \p g conditioned for a fight against \p other.
             *  @param other The unconditioned(!) army to prepare to fight against.
             */
            attack_group_cache(const unit_group& g, const army& other) noexcept
                : m_attack_order(), m_damage_factors(), m_is_one_to_one()
            {
                const unit_type& t = g.type();
                
                // Damage factors and suchlike optimization.
                bool is_never_splash = t.splash_chance() == 0;
                bool is_always_splash = t.splash_chance() == 1;
                bool do_ignore_tower_bonus = t.has(special_ability::ignore_tower_bonus);
                double damage_factor_normal = (1 - other.direct_damage_reduction());
                double damage_factor_in_tower = (1 - other.direct_damage_reduction()) * (1 - other.tower_damage_reduction());

                this->m_damage_factors.reserve(other.count_groups());
                this->m_is_one_to_one.reserve(other.count_groups());
                std::set<double> distinct_factors { };
                for (const unit_group& defender : other.groups())
                {
                    bool do_tower_bonus = defender.type().has(special_ability::tower_bonus);
                    if (do_ignore_tower_bonus) do_tower_bonus = false;
                    double damage_factor = do_tower_bonus ? damage_factor_in_tower : damage_factor_normal;
                    
                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    std::size_t effective_min_damage = damage_cast(t.low_damage(), damage_factor);
                    // Even though defender has not been conditioned, hit point reduction occurs only for bosses that typically don't come in large groups.
                    bool is_one_to_one = is_never_splash && (effective_min_damage >= defender.type().hit_points());

                    distinct_factors.insert(damage_factor);
                    this->m_damage_factors.push_back(damage_factor);
                    this->m_is_one_to_one.push_back(is_one_to_one);
                }
                this->m_damage_factors.shrink_to_fit();
                this->m_is_one_to_one.shrink_to_fit();

                if (distinct_factors.size() == 1) this->m_uniform_damage_factor = *(distinct_factors.begin());
                this->m_is_uniform_splash = is_always_splash && (distinct_factors.size() < 2);

                // Figure out which ordering to use.
                bool do_attack_weakest_target = t.has(special_ability::attack_weakest_target);
                this->m_attack_order = do_attack_weakest_target ? other.order_by_hp() : other.order_by_id();
            }

            const aftermath::algebra::permutation& order() const noexcept { return this->m_attack_order; }

            bool is_uniform_splash() const noexcept { return this->m_is_uniform_splash; }
            double uniform_damage_factor() const noexcept { return this->m_uniform_damage_factor; }

            void against_enemy_group(std::size_t index, double& damage_factor, bool& is_one_to_one) const noexcept
            {
                damage_factor = this->m_damage_factors[index];
                is_one_to_one = this->m_is_one_to_one[index];
            }
        };

        struct conditioned_army
        {
            template <typename t_data_type>
            using phase_array = enum_array<battle_phase, t_data_type>;

        private:
            army m_army = { }; // The army itself.
            std::vector<std::size_t> m_counts = { }; // Unit counts before the battle began.
            phase_array<std::vector<std::size_t>> m_group_indices = { }; // Indices of units to attack in a given phase.
            std::vector<attack_group_cache> m_caches = { }; // Additional parameters determining how each group will attack the enemy.

        public:
            /** @brief Constructs a version of the army \p a conditioned for a fight against \p other.
             *  @param other The unconditioned(!) army to prepare to fight against.
             */
            explicit conditioned_army(const army& a, const army& other) noexcept;

            const army& underlying() const noexcept { return this->m_army; }
            army& underlying() noexcept { return this->m_army; }
            
            const std::vector<std::size_t>& group_indices(battle_phase phase) const noexcept { return this->m_group_indices[phase]; }
            
            const std::vector<attack_group_cache>& caches() const noexcept { return this->m_caches; }
            
            std::vector<std::size_t> calculate_losses() const noexcept
            {
                std::vector<std::size_t> losses = this->m_counts;
                aftermath::algebra::elementwise::subtract_assign(losses, this->m_army.counts_by_type());
                return losses;
            }

            /** Initiates and attack by this army on its opponent \c other. */
            template <typename t_sequence_type>
            void hit(army& other, battle_phase phase, double frenzy_factor, attack_sequence<t_sequence_type>& sequencer, bool do_log) const;
        };

        /** @brief Constructs a version of the army \p a conditioned for a fight against \p other.
         *  @param other The unconditioned(!) army to prepare to fight against.
         */
        conditioned_army::conditioned_army(const army& a, const army& other) noexcept
            : m_army(a), m_counts(a.counts_by_type()), m_group_indices(), m_caches()
        {
            // Build phase grouping.
            for (std::size_t i = 0; i < a.count_groups(); i++)
            {
                for (battle_phase phase : a[i].type().attack_phases()) this->m_group_indices[phase].push_back(i);
            }

            // Condition groups prior to the battle.
            for (unit_group& g : this->m_army.groups())
            {
                unit_type t = g.type();
                unit_category cat = t.category();

                std::size_t hit_points = t.hit_points();
                std::size_t low_damage = t.low_damage() + this->m_army.low_damage_bonus_for(cat);
                std::size_t high_damage = t.high_damage() + this->m_army.high_damage_bonus_for(cat);
                double accuracy = t.accuracy() + this->m_army.accuracy_bonus_for(cat);
                double splash_chance = t.splash_chance() + this->m_army.splash_bonus_for(cat);

                // Trait: explosive ammunition.
                if (this->m_army.do_explosive_ammunition() && (cat == unit_category::ranged)) 
                {
                    t.set_ability(special_ability::attack_weakest_target, true);
                    splash_chance = 1.0;
                }
                // Trait: intercept.
                if (other.do_intercept())
                {
                    t.set_ability(special_ability::attack_weakest_target, false);
                    // <intercept_damage_percent> is defined in <battle_trait.hpp>.
                    low_damage = fraction_ceiling(intercept_damage_percent * low_damage, static_cast<std::size_t>(100));
                    high_damage = fraction_ceiling(intercept_damage_percent * high_damage, static_cast<std::size_t>(100));
                }
                // Trait: dazzle.
                if (other.do_dazzle()) accuracy = 0.0;

                // Special ability: reduce hit points of bosses.
                if (t.has(special_ability::boss)) hit_points = hit_points_cast(hit_points, 1.0 - other.boss_health_reduction());

                // Make sure all probabilities are within bounds.
                if (accuracy > 1.0) accuracy = 1.0;
                if (splash_chance > 1.0) splash_chance = 1.0;
                if (low_damage > high_damage)
                {
                    high_damage = low_damage;
                    aftermath::quiet_error::instance().push(
                        aftermath::not_an_error::logic_error,
                        aftermath::severity_level::minor,
                        "Conditioned low damage exceeds high damage. Bumped high damage up to match.", __FUNCTION__, __LINE__);
                }

                t.set_damage(low_damage, high_damage, accuracy, splash_chance);
                t.set_hit_points(hit_points);
                g.set_type(t);
            }

            // Now that the army has been conditioned, build caches.
            this->m_caches.reserve(this->m_army.count_groups());
            for (const unit_group& g : this->m_army.groups()) this->m_caches.emplace_back(g, other);
            this->m_caches.shrink_to_fit();
        }

        template <typename t_sequence_type>
        void conditioned_army::hit(army& other, battle_phase phase, double frenzy_factor, attack_sequence<t_sequence_type>& sequencer, bool do_log) const
        {
            for (std::size_t i : this->m_group_indices[phase])
            {
                const unit_group& attacking_group = this->m_army[i];
                const attack_group_cache& cache = this->m_caches[i];

                if (attacking_group.empty_at_snapshot()) continue; // Skip empty groups.
                const unit_type& attacker_t = attacking_group.type();
                if (do_log)
                {
                    std::cout << '\t' <<
                        attacking_group.count_at_snapshot() << " " <<
                        attacker_t.names().front() << " attacking..." << std::endl;
                }

                // Optimize in the uniform all splash damage case.
                if (cache.is_uniform_splash())
                {
                    double damage_factor = cache.uniform_damage_factor();
                    // Adjust damage factor for current round.
                    damage_factor *= frenzy_factor;

                    std::size_t count_attackers = attacking_group.count_at_snapshot();
                    std::size_t count_high_damage = sequencer.peek_count_high_damage(attacker_t, count_attackers); // Count the number of units in this stack that do maximum damage.
                    sequencer.next_unit(count_attackers);

                    std::size_t total_reduced_damage = 
                        damage_cast(attacker_t.low_damage(), damage_factor) * (count_attackers - count_high_damage) +
                        damage_cast(attacker_t.high_damage(), damage_factor) * count_high_damage;

                    detail::uniform_splash(total_reduced_damage, other, cache.order(), do_log); // technical_combat.hpp.
                    continue;
                }

                std::size_t attacking_unit_index = 0;
                std::size_t pure_overshoot_damage = 0; // Pure damage spilled between consequent groups.
                std::vector<unit_group>& defender_groups = other.groups();
                for (std::size_t j : cache.order())
                {
                    // Get cached properties.
                    double damage_factor;
                    bool is_one_to_one;
                    cache.against_enemy_group(j, damage_factor, is_one_to_one);
                    // Adjust damage factor for current round.
                    damage_factor *= frenzy_factor;

                    // Current defender.
                    unit_group& defending_group = defender_groups[j];

                    // First take care of the overshoot damage.
                    pure_overshoot_damage = detail::splash(pure_overshoot_damage, damage_factor, defending_group, do_log); // technical_combat.hpp.

                    std::size_t count_alive = defending_group.count();
                    if (count_alive == 0) continue; // Defender has already been killed.

                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    if (is_one_to_one) attacking_unit_index = detail::one_to_one(defending_group, attacking_group, attacking_unit_index, do_log); // technical_combat.hpp.
                    else attacking_unit_index = detail::hit(pure_overshoot_damage, defending_group, attacking_group, attacking_unit_index, damage_factor, sequencer, do_log); // technical_combat.hpp.

                    if (attacking_unit_index == attacking_group.count_at_snapshot()) break;
                    if (attacking_unit_index > attacking_group.count_at_snapshot()) throw std::logic_error("<attacking_unit_index> overflow.");
                }
            }
        }
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_CONDITIONED_ARMY_HPP_INCLUDED
