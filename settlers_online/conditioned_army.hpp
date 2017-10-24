
#ifndef ROPUFU_SETTLERS_ONLINE_CONDITIONED_ARMY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CONDITIONED_ARMY_HPP_INCLUDED

#include <aftermath/algebra.hpp> // aftermath::algebra::permutation
#include <aftermath/not_an_error.hpp>

// ~~ Enumerations ~~
#include "battle_phase.hpp"
#include "battle_trait.hpp"
#include "special_ability.hpp"
#include "unit_category.hpp"
// ~~ Basic structures ~~
#include "damage.hpp"
// ~~ Misc ~~
#include "army.hpp"
#include "attack_sequence.hpp"
#include "combat_result.hpp"
#include "enum_array.hpp"
#include "technical_combat.hpp"
#include "typedef.hpp"
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
                const unit_type& t = g.unit();
                
                // Damage factors and suchlike optimization.
                bool is_never_splash = t.damage().splash_chance() == 0;
                bool is_always_splash = t.damage().splash_chance() == 1;
                bool do_ignore_tower_bonus = t.has(special_ability::ignore_tower_bonus);
                double damage_factor_normal = 1;
                double damage_factor_in_tower = (1 - other.camp().damage_reduction());

                this->m_damage_factors.reserve(other.count_groups());
                this->m_is_one_to_one.reserve(other.count_groups());
                std::set<double> distinct_factors { };
                for (const unit_group& defender : other.groups())
                {
                    bool do_tower_bonus = defender.unit().has(special_ability::tower_bonus);
                    if (do_ignore_tower_bonus) do_tower_bonus = false;
                    double damage_factor = do_tower_bonus ? damage_factor_in_tower : damage_factor_normal;
                    
                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    std::size_t effective_min_damage = damage_cast(t.damage().low(), damage_factor);
                    // Even though defender has not been conditioned, hit point reduction occurs only for bosses that typically don't come in large groups.
                    bool is_one_to_one = is_never_splash && (effective_min_damage >= defender.unit().hit_points());

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
            } // attack_group_cache(...)

            const aftermath::algebra::permutation& order() const noexcept { return this->m_attack_order; }

            bool is_uniform_splash() const noexcept { return this->m_is_uniform_splash; }
            double uniform_damage_factor() const noexcept { return this->m_uniform_damage_factor; }

            void against_enemy_group(std::size_t index, double& damage_factor, bool& is_one_to_one) const noexcept
            {
                damage_factor = this->m_damage_factors[index];
                is_one_to_one = this->m_is_one_to_one[index];
            } // against_enemy_group(...)
        }; // struct attack_group_cache

        struct conditioned_army
        {
            using type = conditioned_army;
            template <typename t_data_type>
            using phase_array = enum_array<battle_phase, t_data_type>;

        private:
            army m_army = { }; // The army itself.
            std::vector<std::size_t> m_counts = { }; // Unit counts before the battle began.
            phase_array<std::vector<std::size_t>> m_group_indices = { }; // Indices of units to attack in a given phase.
            std::vector<attack_group_cache> m_caches = { }; // Additional parameters determining how each group will attack the enemy.

            /** @brief Apply the firendly army's skill to a unit.
             *  @param level The number of books invested in this skill.
             */
            static void apply_friendly_skill(unit_type& unit, battle_skill skill, std::size_t level) noexcept
            {
                if (level == 0) return;
                // Level    0   1   2   3   4   ...
                // Factor   0   0   5   15  30  ...
                std::size_t square_factor = (level * (level - 1) * 5) / 2;
                // Level      0  1   2   3    4    ...
                // Thirds, %  0  33  66  100  133  ...
                double thirds = fraction_floor(100 * level, static_cast<std::size_t>(3)) / 100.0;

                detail::damage damage_bonus(true); // Quietly coerce all values.
                switch (skill)
                {
                    case battle_skill::juggernaut: // Increases the general's (faction: general) attack damage by 20/40/60. These attacks have a 33/66/100% chance of dealing splash damage.
                        if (!unit.is(unit_faction::general)) return;
                        damage_bonus.reset(20 * level, 20 * level, 0, thirds);
                        break;
                    case battle_skill::garrison_annex: // Increases the unit capacity (faction: general) by 5/10/15.
                        if (!unit.is(unit_faction::general)) return;
                        unit.set_capacity(unit.capacity() + 5 * level);
                        break;
                    case battle_skill::lightning_slash: // The general (faction: general) attacks twice per round. That second attack's initiative is \c last_strike.
                        if (!unit.is(unit_faction::general)) return;
                        unit.set_attack_phase(battle_phase::last_strike, true);
                        break;
                    case battle_skill::unstoppable_charge: // Increases the maximum attack damage of your swift units (faction: cavalry) by 1/2/3 and their attacks have a 33/66/100% chance of dealing splash damage.
                        if (!unit.is(unit_category::cavalry)) return;
                        damage_bonus.reset(0, level, 0, thirds);
                        break;
                    case battle_skill::weekly_maintenance: // Increases the attack damage of your heavy units (faction: artillery) by 10/20/30.
                        if (!unit.is(unit_category::artillery)) return;
                        damage_bonus.reset(10 * level, 10 * level);
                        break;
                    case battle_skill::master_planner: // Adds 10% to this army's accuracy.
                        damage_bonus.reset(0, 0, 0.1, 0);
                        break;
                    case battle_skill::rapid_fire: // Increases the maximum attack damage of your Bowmen by 5/10/15.
                        if (!unit.has(special_ability::rapid_fire)) return;
                        damage_bonus.reset(0, 5 * level);
                        break;
                    case battle_skill::sniper_training: // Increases your Longbowmen's and regular Marksmen's minimum attack damage by 45/85/130% and the maximum by 5/10/15%.
                        if (!unit.has(special_ability::sniper_training)) return;
                        damage_bonus.reset(
                            fraction_floor(level >= 3 ?
                                (unit.damage().high() * (100 + 5 * level)) :
                                (unit.damage().low() * (45 * level - square_factor)), static_cast<std::size_t>(100)),
                            fraction_floor(unit.damage().high() * (5 * level), static_cast<std::size_t>(100)));
                        break;
                    case battle_skill::cleave: // Increases the attack damage of Elite Soldiers by 4/8/12 and their attacks have a 33/66/100% chance of dealing splash damage.
                        if (!unit.has(special_ability::cleave)) return;
                        damage_bonus.reset(4 * level, 4 * level, 0, thirds);
                        break;
                    default: break;
                } // switch(...)
                
                // Add bonus, quietly coercing.
                damage_bonus += unit.damage();
                // Stop quiet coercion.
                damage_bonus.set_is_quiet(false);
                // Apply new damage.
                unit.set_damage(damage_bonus);
            } // apply_friendly_skill(...)

            /** @brief Apply the enemy army's skill to a unit.
             *  @param level The number of books invested in this skill.
             */
            static void apply_enemy_skill(unit_type& unit, battle_skill skill, std::size_t level) noexcept
            {
                switch (skill)
                {
                    case battle_skill::fast_learner: // Increases the XP gained from enemy units defeated by this army by 10/20/30%.
                        unit.set_experience(fraction_floor(unit.experience() * (100 + 10 * level), static_cast<std::size_t>(100)));
                        break;
                    case battle_skill::overrun: // Decreases the HP of enemy bosses by 8/16/25%.
                        if (!unit.has(special_ability::overrun)) return;
                        unit.set_hit_points(fraction_ceiling(unit.hit_points() * (100 - (level >= 3 ? 25 : (8 * level))), static_cast<std::size_t>(100)));
                        break;
                    default: break;
                }
            } // apply_enemy_skill(...)

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
            } // calculate_losses(...)

            /** Initiates and attack by this army on its opponent \c other. */
            template <typename t_sequence_type>
            void initiate_phase(battle_phase phase, conditioned_army& other, double frenzy_factor, attack_sequence<t_sequence_type>& sequencer, bool do_log) const noexcept;
        }; // struct conditioned_army

        /** @brief Constructs a version of the army \p a conditioned for a fight against \p other.
         *  @param other The unconditioned(!) army to prepare to fight against.
         */
        conditioned_army::conditioned_army(const army& a, const army& other) noexcept
            : m_army(a), m_counts(a.counts_by_type()), m_group_indices(), m_caches()
        {
            // Condition groups prior to the battle.
            for (std::size_t i = 0; i < this->m_army.count_groups(); i++)
            {
                unit_group& g = this->m_army[i];
                // Take the type to modify.
                unit_type t = g.unit();

                // First go through skills.
                for (const auto& pair : this->m_army.skills())
                {
                    type::apply_friendly_skill(t, pair.first, pair.second);
                    // Increases the attack damage of this army by 10/20/30% for every combat round past the first.
                    if (pair.first == battle_skill::battle_frenzy) this->m_army.set_frenzy_bonus(0.1 * pair.second);
                }
                for (const auto& pair : other.skills()) type::apply_enemy_skill(t, pair.first, pair.second);

                // Take the damage to modify.
                detail::damage damage = t.damage();
                // Allow quiet coercion.
                damage.set_is_quiet(true);
                // ~~ Traits ~~
                // Friendly trait: explosive ammunition.
                if (this->m_army.has(battle_trait::explosive_ammunition) && t.is(unit_category::ranged))
                {
                    t.set_ability(special_ability::attack_weakest_target, true);
                    damage.set_splash_chance(1);
                }
                // Enemy trait: intercept.
                if (other.has(battle_trait::intercept))
                {
                    t.set_ability(special_ability::attack_weakest_target, false);
                    // <intercept_damage_percent> is defined in <battle_trait.hpp>.
                    damage.reset(
                        fraction_ceiling(intercept_damage_percent * damage.low(), static_cast<std::size_t>(100)),
                        fraction_ceiling(intercept_damage_percent * damage.high(), static_cast<std::size_t>(100)));
                }
                // Enemy trait: dazzle.
                if (other.has(battle_trait::dazzle)) damage.set_accuracy(0);

                // Stop quiet coercion.
                damage.set_is_quiet(false);
                // Apply new damage.
                t.set_damage(damage);
                // Apply new type.
                g.set_unit(t);
            }

            // Now that the army has been conditioned, build grouping.
            for (std::size_t i = 0; i < this->m_army.count_groups(); i++)
            {
                for (battle_phase phase : this->m_army[i].unit().attack_phases()) this->m_group_indices[phase].push_back(i);
            }

            // Now that the army has been conditioned, build caches.
            this->m_caches.reserve(this->m_army.count_groups());
            for (const unit_group& g : this->m_army.groups()) this->m_caches.emplace_back(g, other);
            this->m_caches.shrink_to_fit();
        } // conditioned_army::conditioned_army(...)

        /** Initiates and attack by this army on its opponent \c other. */
        template <typename t_sequence_type>
        void conditioned_army::initiate_phase(battle_phase phase, conditioned_army& other, double frenzy_factor, attack_sequence<t_sequence_type>& sequencer, bool do_log) const noexcept
        {
            for (std::size_t i : this->m_group_indices[phase])
            {
                const unit_group& attacking_group = this->m_army[i];
                const attack_group_cache& cache = this->m_caches[i];

                const unit_type& attacker_t = attacking_group.unit();
                std::size_t count_attackers = attacking_group.count_at_snapshot();
                if (count_attackers == 0) continue; // Skip empty groups.

                // Optimize in the uniform all splash damage case.
                if (cache.is_uniform_splash())
                {
                    double damage_factor = cache.uniform_damage_factor();
                    // Adjust damage factor for current round.
                    damage_factor *= frenzy_factor;

                    std::size_t count_high_damage = sequencer.peek_count_high_damage(attacker_t, count_attackers); // Count the number of units in this stack that do maximum damage.
                    sequencer.next_unit(count_attackers);

                    std::size_t total_reduced_damage = 
                        damage_cast(attacker_t.damage().low(), damage_factor) * (count_attackers - count_high_damage) +
                        damage_cast(attacker_t.damage().high(), damage_factor) * count_high_damage;

                    if (do_log)
                    {
                        std::cout << '\t' <<
                            count_attackers << " " << attacker_t.names().front() <<
                            " dealing " << total_reduced_damage <<
                            " (" << (count_attackers - count_high_damage) << " low, " << count_high_damage << " high)" <<
                            " splash damage..." << std::endl;
                    }
                    detail::uniform_splash(total_reduced_damage, other.m_army, cache.order(), do_log); // technical_combat.hpp.
                    continue; // Proceed to the next attacking group.
                }

                if (do_log)
                {
                    std::cout << '\t' <<
                        count_attackers << " " << attacker_t.names().front() <<
                        " attacking..." << std::endl;
                }

                std::size_t attacking_unit_index = 0;
                std::size_t pure_overshoot_damage = 0; // Pure damage spilled between consequent groups.
                for (std::size_t j : cache.order())
                {
                    // Get cached properties.
                    double damage_factor;
                    bool is_one_to_one;
                    cache.against_enemy_group(j, damage_factor, is_one_to_one);
                    // Adjust damage factor for current round.
                    damage_factor *= frenzy_factor;

                    // Current defender.
                    unit_group& defending_group = other.m_army[j];

                    // First take care of the overshoot damage.
                    pure_overshoot_damage = detail::splash(pure_overshoot_damage, damage_factor, defending_group, do_log); // technical_combat.hpp.

                    std::size_t count_alive = defending_group.count();
                    if (count_alive == 0) continue; // Defender has already been killed.

                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    if (is_one_to_one) attacking_unit_index = detail::one_to_one(defending_group, attacking_group, attacking_unit_index, do_log); // technical_combat.hpp.
                    else attacking_unit_index = detail::hit(pure_overshoot_damage, defending_group, attacking_group, attacking_unit_index, damage_factor, sequencer, do_log); // technical_combat.hpp.

                    if (attacking_unit_index == attacking_group.count_at_snapshot()) break;
                    if (attacking_unit_index > attacking_group.count_at_snapshot()) 
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::logic_error,
                            aftermath::severity_level::fatal,
                            "<attacking_unit_index> overflow.", __FUNCTION__, __LINE__);
                        break;
                    }
                }
            }
        } // conditioned_army::initiate_phase(...)
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_CONDITIONED_ARMY_HPP_INCLUDED
