
#ifndef ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED

#include <aftermath/algebra.hpp> // aftermath::algebra::permutation
#include <aftermath/not_an_error.hpp>

#include "attack_sequence.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t, nullptr
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        namespace detail
        {
            /** @brief Initiates and attack on a group (defender) by another group (attacker).
             *  @return The attacking unit index after the attack has been completed.
             */
            template <typename t_sequence_type, typename t_logger_type>
            static std::size_t hit(std::size_t& overshoot_damage, unit_group& defending_group,
                const unit_group& attacking_group, std::size_t attacking_unit_index, double damage_factor,
                attack_sequence<t_sequence_type>& sequencer, t_logger_type& logger) noexcept
            {
                const unit_type& attacker_t = attacking_group.unit();
                const unit_type& defender_t = defending_group.unit();

                std::size_t defending_units_remaining = defending_group.count();
                if (defending_units_remaining == 0) return attacking_unit_index; // Check if the defending group is empty.

                std::size_t defenders_killed = defending_units_remaining;
                // if (t_logger_type::is_enabled)
                // {
                    logger << '\t' << "..." <<
                        defending_units_remaining << " " << defender_t.names().front() <<
                        " [" << defending_group.total_hit_points() << " hit points]";
                // }

                // Proceed to next attaking unit.
                std::size_t attacking_units_remaining = attacking_group.count_at_snapshot() - attacking_unit_index;

                std::size_t effective_min_damage = damage_cast(attacker_t.damage().low(), damage_factor); // Potential effective minumum damage dealt by one attacking unit.
                std::size_t effective_max_damage = damage_cast(attacker_t.damage().high(), damage_factor); // Potential effective mamimum damage dealt by one attacking unit.
                // Loop through attacking units.
                while (attacking_units_remaining > 0)
                {
                    // If this group has been eliminated proceed to the next group.
                    if (defending_units_remaining == 0) 
                    {
                        // if (t_logger_type::is_enabled) 
                            logger << " killing " << defenders_killed << "." << nullptr;
                        return attacking_unit_index;
                    }
                    std::size_t hit_points = defending_group.top_hit_points(); // Hit points of the top unit.

                    std::size_t count_attackers = fraction_ceiling(hit_points, effective_max_damage); // Least number of units required to kill the current defending unit.
                    if (count_attackers > attacking_units_remaining) count_attackers = attacking_units_remaining; // Cap to prevent \c attacking_unit_index overflow.
                    std::size_t count_high_damage = sequencer.peek_count_high_damage(attacker_t, count_attackers); // Number of units dealing maximum damage.

                    sequencer.next_unit(count_attackers);
                    attacking_unit_index += count_attackers;
                    attacking_units_remaining -= count_attackers;

                    std::size_t effective_damage = effective_max_damage * count_high_damage + effective_min_damage * (count_attackers - count_high_damage); // Effective damage dealt by the attacker.
                    std::size_t pure_damage = attacker_t.damage().high() * count_high_damage + attacker_t.damage().low() * (count_attackers - count_high_damage); // Pure damage dealt by the attacker.
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
                        if (defending_units_remaining == 0) 
                        {
                            // if (t_logger_type::is_enabled)
                                logger << " killing " << defenders_killed << "." << nullptr;
                            return attacking_unit_index;
                        }

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
                // if (t_logger_type::is_enabled)
                    logger << " killing " << (defenders_killed - defending_group.count()) << "." << nullptr;
                return attacking_group.count_at_snapshot();
            } // hit(...)

            /** @brief Inflicts \p pure_damage splash onto \p defending_group, taking into account \p damage_factor damage multiplier.
             *  @return Remaining \p pure_damage.
             */
            template <typename t_logger_type>
            static std::size_t splash(std::size_t pure_damage, double damage_factor, unit_group& defending_group, t_logger_type& logger) noexcept
            {
                if (pure_damage == 0) return 0;
                
                std::size_t defending_units_remaining = defending_group.count();
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
                // if (t_logger_type::is_enabled)
                    logger << '\t' << "Overshoot killing " << (defending_units_remaining - defending_group.count()) << " " << defending_group.unit().names().front() << nullptr;
                return pure_damage;
            } // splash(...)

            /** Inflicts the reduced damage, \p reduced_damage, onto \p defender, assuming the attaker always deals splash damage and there is no effective tower bonus. */
            template <typename t_logger_type>
            static void uniform_splash(std::size_t reduced_damage, army& defender, const aftermath::algebra::permutation& defender_ordering, t_logger_type& logger) noexcept
            {
                //std::vector<unit_group>& defender_groups = defender.groups();
                for (std::size_t j : defender_ordering)
                {
                    unit_group& defending_group = defender[j];//defender_groups[j];
                    std::size_t defending_units_remaining = defending_group.count();
                    std::size_t total_hit_points = defending_group.total_hit_points();
                    if (total_hit_points == 0) continue;

                    // if (t_logger_type::is_enabled)
                    // {
                        logger << '\t' << "...against " <<
                            defending_units_remaining << " " << defending_group.unit().names().front() <<
                            " [" << total_hit_points << " hit points]";
                    // }

                    if (total_hit_points >= reduced_damage) // All the splash damage has been accounted for.
                    {
                        defending_group.set_total_hit_points(total_hit_points - reduced_damage); // Damage the defending group.
                        // if (t_logger_type::is_enabled)
                            logger << " killing " << (defending_units_remaining - defending_group.count()) << "." << nullptr;
                        return;
                    }
                    defending_group.kill_all();
                    // if (t_logger_type::is_enabled)
                        logger << " killing " << defending_units_remaining << "." << nullptr;
                    reduced_damage -= total_hit_points;
                }
            } // uniform_splash(...)

            /** @brief When attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
             *  @return The attacking unit index after the attack has been completed.
             */
            template <typename t_logger_type>
            static std::size_t one_to_one(unit_group& defending_group, const unit_group& attacking_group, std::size_t attacking_unit_index, t_logger_type& logger) noexcept
            {
                // Proceed to next attaking unit.
                std::size_t attacking_units_remaining = attacking_group.count_at_snapshot() - attacking_unit_index;
                std::size_t defending_units_remaining = defending_group.count();

                // if (t_logger_type::is_enabled)
                // {
                    logger << '\t' << "...against " <<
                        defending_units_remaining << " " << defending_group.unit().names().front() <<
                        " [" << defending_group.total_hit_points() << " hit points]";
                // }

                if (attacking_units_remaining > defending_units_remaining)
                {
                    defending_group.kill_all(); // The entire group has been eliminated.
                    // if (t_logger_type::is_enabled)
                        logger << " killing " << defending_units_remaining << "." << nullptr;
                    return attacking_unit_index + defending_units_remaining; // All defenders have been killed, and there still may be attackers left.
                }
                else
                {
                    // All attacking units have made their hit, and there still may be survivors left.
                    defending_group.kill(attacking_units_remaining);
                    // if (t_logger_type::is_enabled)
                        logger << " killing " << attacking_units_remaining << "." << nullptr;
                    return attacking_group.count_at_snapshot();
                }
            } // one_to_one(...)
        } // namespace detail
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED