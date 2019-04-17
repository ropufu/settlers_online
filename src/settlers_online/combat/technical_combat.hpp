
#ifndef ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED

#include <ropufu/algebra.hpp> // aftermath::algebra::permutation

#include "../algebra.hpp"

#include "battle_clock.hpp"
#include "battle_invariant.hpp"
#include "damage.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t, nullptr
#include <vector> // std::vector

namespace ropufu::settlers_online
{
    namespace technical_combat
    {
        /** @brief Initiates and attack on a group (defender) by another group (attacker).
         *  @return The attacking unit index after the attack has been completed.
         */
        template <typename t_sequence_type, typename t_logger_type>
        static std::size_t hit(unit_group& defending_group,
            const unit_group& attacking_group, std::size_t attacking_unit_index, const damage& attacker_damage,
            t_sequence_type& sequencer, battle_clock& clock, t_logger_type& logger, detail::overshoot_damage& overshoot) noexcept
        {
            logger.touch(); // To avoid unreferenced named parameter whenever \c t_logger_type::is_enabled is \c false.
            const unit_type& defender_type = defending_group.unit();

            std::size_t defending_units_remaining = defending_group.count_as_defender();
            if (defending_units_remaining == 0) return attacking_unit_index; // Check if the defending group is empty.

            if constexpr (t_logger_type::is_enabled)
            {
                logger.set_tag(defending_units_remaining);
                logger << '\t' << "..." <<
                    defending_units_remaining << " " << defender_type.first_name() <<
                    " [" << defending_group.total_hit_points_defender() << " hit points]";
            } // if constexpr (...)

            // Proceed to next attaking unit.
            std::size_t attacking_units_remaining = attacking_group.count_as_attacker() - attacking_unit_index;

            // Loop through attacking units.
            while (attacking_units_remaining > 0)
            {
                // If this group has been eliminated proceed to the next group.
                if (defending_units_remaining == 0) 
                {
                    if constexpr (t_logger_type::is_enabled) 
                        logger << " killing " << logger.tag() << "." << nullptr;
                    return attacking_unit_index;
                } // if (...)
                std::size_t hit_points = defending_group.top_hit_points_defender(); // Hit points of the top unit.

                std::size_t count_attackers = fraction_ceiling(hit_points, attacker_damage.high()); // Least number of units required to kill the current defending unit.
                if (count_attackers > attacking_units_remaining) count_attackers = attacking_units_remaining; // Cap to prevent \c attacking_unit_index overflow.
                std::size_t count_high_damage = sequencer.peek_count_high_damage(count_attackers - 1, clock); // Number of units less one dealing maximum damage.
                overshoot.indicator_high = sequencer.peek_count_high_damage(1, clock); // Indicates if the last unit dealt maximum damage.
                count_high_damage += overshoot.indicator_high;

                clock.next_unit(count_attackers);
                attacking_unit_index += count_attackers;
                attacking_units_remaining -= count_attackers;

                std::size_t effective_damage = attacker_damage.high() * count_high_damage + attacker_damage.low() * (count_attackers - count_high_damage); // Damage (defender's damage reduction ignored) dealt by the attacker.
                std::size_t damage_required = hit_points; // Damage required to kill the defending unit.

                if (effective_damage > damage_required)
                {
                    defending_group.kill_top_defender(); // Kill the top defending unit.
                    --defending_units_remaining;
                    bool do_splash = sequencer.did_last_splash(clock); // Check whether splash damage occurs.
                    if (!do_splash) overshoot.clear();
                    else overshoot += (effective_damage - damage_required);
                } // if (...)
                else // The inflicted damage does not exceed hit points.
                {
                    std::size_t previous_count = defending_group.count_as_defender();
                    defending_group.damage_no_splash(effective_damage); // Damage the top defending unit.
                    std::size_t count_killed = previous_count - defending_group.count_as_defender();
                    defending_units_remaining -= count_killed;
                    overshoot.clear();
                } // else (...)

                // If there is overshoot damage, spread it over this group.
                while (!overshoot.empty())
                {
                    // If this group has been eliminated proceed to the next group.
                    if (defending_units_remaining == 0) 
                    {
                        if constexpr (t_logger_type::is_enabled)
                            logger << " killing " << logger.tag() << "." << nullptr;
                        return attacking_unit_index;
                    } // if (...)

                    // Since it is overshoot damage, each consecutive defending unit has full hit points.
                    damage_required = defender_type.hit_points(); // Damage required to kill the defending unit.
                    if (overshoot.total() > damage_required)
                    {
                        overshoot -= damage_required;
                        defending_group.kill_top_defender(); // Kill the top defending unit.
                        --defending_units_remaining;
                    } // if (...)
                    else
                    {
                        std::size_t previous_count = defending_group.count_as_defender();
                        defending_group.damage_no_splash(overshoot.total()); // Damage the top defending unit.
                        std::size_t count_killed = previous_count - defending_group.count_as_defender();
                        defending_units_remaining -= count_killed;
                        overshoot.clear();
                    } // else (...)
                } // while (...)
            } // while (...)
            if constexpr (t_logger_type::is_enabled)
                logger << " killing " << (logger.tag() - defending_group.count_as_defender()) << "." << nullptr;
            return attacking_group.count_as_attacker();
        } // hit(...)

        /** @brief Inflicts \p pure_damage splash onto \p defending_group, taking into account \p damage_factor damage multiplier.
         *  @return Remaining \p pure_damage.
         */
        template <typename t_logger_type>
        static void splash(detail::overshoot_damage& overshoot, unit_group& defending_group, t_logger_type& logger) noexcept
        {
            logger.touch(); // To avoid unreferenced named parameter whenever \c t_logger_type::is_enabled is \c false.
            std::size_t effective_damage = overshoot.total();
            if (effective_damage == 0) return;
            
            if constexpr (t_logger_type::is_enabled) logger.set_tag(defending_group.count_as_defender());
            // Damage required to eliminate the defending group.
            std::size_t damage_required = defending_group.total_hit_points_defender();
            if (damage_required > effective_damage) // Remaining damage is not enough to eliminate this group.
            {
                // Damage the defending group.
                defending_group.damage_pure_splash(effective_damage);
                // All the splash damage has been accounted for.
                overshoot.clear();
            } // if (...)
            else // The defending group is guaranteed to be eliminated.
            {
                // Kill the current defending group.
                defending_group.kill_all_defender();
                // Calculate the remaining splash damage.
                overshoot -= damage_required;
            } // else (...)
            if constexpr (t_logger_type::is_enabled)
                logger << '\t' << "Overshoot killing " << (logger.tag() - defending_group.count_as_defender()) << " " << defending_group.unit().first_name() << nullptr;
        } // splash(...)

        /** @brief When attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
         *  @return The attacking unit index after the attack has been completed.
         *  @remark Defending group is assumed to be alive.
         *  @remark Attacking group is assumed to be alive.
         */
        template <typename t_logger_type>
        static std::size_t one_to_one(unit_group& defending_group, const unit_group& attacking_group, std::size_t attacking_unit_index, t_logger_type& logger) noexcept
        {
            logger.touch(); // To avoid unreferenced named parameter whenever \c t_logger_type::is_enabled is \c false.
            // Proceed to next attaking unit.
            std::size_t attacking_units_remaining = attacking_group.count_as_attacker() - attacking_unit_index;
            std::size_t defending_units_remaining = defending_group.count_as_defender();

            if constexpr (t_logger_type::is_enabled)
                logger << '\t' << "...against " <<
                    defending_units_remaining << " " << defending_group.unit().first_name() <<
                    " [" << defending_group.total_hit_points_defender() << " hit points]";

            if (attacking_units_remaining > defending_units_remaining)
            {
                defending_group.kill_all_defender(); // The entire group has been eliminated.
                if constexpr (t_logger_type::is_enabled)
                    logger << " killing " << defending_units_remaining << "." << nullptr;
                return attacking_unit_index + defending_units_remaining; // All defenders have been killed, and there still may be attackers left.
            } // if (...)
            else
            {
                // All attacking units have made their hit, and there still may be survivors left.
                defending_units_remaining -= attacking_units_remaining;
                defending_group.reset_count_defender(defending_units_remaining);
                if constexpr (t_logger_type::is_enabled)
                    logger << " killing " << attacking_units_remaining << "." << nullptr;
                return attacking_group.count_as_attacker();
            } // else (...)
        } // one_to_one(...)
    } // namespace technical_combat
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED
