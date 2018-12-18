
#ifndef ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED

#include <ropufu/algebra.hpp> // aftermath::algebra::permutation

//#include "attack_sequence.hpp"
#include "battle_clock.hpp"
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
            const unit_group& attacking_group, std::size_t attacking_unit_index, double damage_factor,
            t_sequence_type& sequencer, battle_clock& clock, t_logger_type& logger, std::size_t& overshoot_damage) noexcept
        {
            const unit_type& attacker_type = attacking_group.unit();
            const unit_type& defender_type = defending_group.unit();

            std::size_t defending_units_remaining = defending_group.count_defender();
            if (defending_units_remaining == 0) return attacking_unit_index; // Check if the defending group is empty.

            std::size_t defenders_killed = defending_units_remaining;
            if constexpr (t_logger_type::is_enabled)
            {
                logger << '\t' << "..." <<
                    defending_units_remaining << " " << defender_type.first_name() <<
                    " [" << defending_group.total_hit_points_defender() << " hit points]";
            } // if constexpr (...)

            // Proceed to next attaking unit.
            std::size_t attacking_units_remaining = attacking_group.count_attacker() - attacking_unit_index;

            std::size_t effective_min_damage = damage_cast(attacker_type.damage().low(), damage_factor); // Potential effective minumum damage dealt by one attacking unit.
            std::size_t effective_max_damage = damage_cast(attacker_type.damage().high(), damage_factor); // Potential effective mamimum damage dealt by one attacking unit.
            // Loop through attacking units.
            while (attacking_units_remaining > 0)
            {
                // If this group has been eliminated proceed to the next group.
                if (defending_units_remaining == 0) 
                {
                    if constexpr (t_logger_type::is_enabled) 
                        logger << " killing " << defenders_killed << "." << nullptr;
                    return attacking_unit_index;
                } // if (...)
                std::size_t hit_points = defending_group.top_hit_points_defender(); // Hit points of the top unit.

                std::size_t count_attackers = fraction_ceiling(hit_points, effective_max_damage); // Least number of units required to kill the current defending unit.
                if (count_attackers > attacking_units_remaining) count_attackers = attacking_units_remaining; // Cap to prevent \c attacking_unit_index overflow.
                std::size_t count_high_damage = sequencer.peek_count_high_damage(count_attackers, clock); // Number of units dealing maximum damage.

                clock.next_unit(count_attackers);
                attacking_unit_index += count_attackers;
                attacking_units_remaining -= count_attackers;

                std::size_t effective_damage = effective_max_damage * count_high_damage + effective_min_damage * (count_attackers - count_high_damage); // Effective damage dealt by the attacker.
                std::size_t pure_damage = attacker_type.damage().high() * count_high_damage + attacker_type.damage().low() * (count_attackers - count_high_damage); // Pure damage dealt by the attacker.
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
                    defending_group.kill_top_defender(); // Kill the top defending unit.
                    defending_units_remaining--;
                    bool do_splash = sequencer.did_last_splash(clock); // Check whether splash damage occurs.
                    overshoot_damage = do_splash ? (pure_damage - damage_required) : 0;
                } // if (...)
                else // The inflicted damage does not exceed hit points.
                {
                    std::size_t previous_count = defending_group.count_defender();
                    defending_group.damage_no_splash(effective_damage); // Damage the top defending unit.
                    std::size_t count_killed = previous_count - defending_group.count_defender();
                    defending_units_remaining -= count_killed;
                    overshoot_damage = 0;
                } // else (...)

                // If there is overshoot damage, spread it over this group.
                while (overshoot_damage > 0)
                {
                    // If this group has been eliminated proceed to the next group.
                    if (defending_units_remaining == 0) 
                    {
                        if constexpr (t_logger_type::is_enabled)
                            logger << " killing " << defenders_killed << "." << nullptr;
                        return attacking_unit_index;
                    } // if (...)

                    // Since it is overshoot damage, each consecutive defending unit has full hit points.
                    damage_required = inverse_damage_cast(defender_type.hit_points(), damage_factor); // Pure damage required to kill the defending unit.
                    if (overshoot_damage > damage_required)
                    {
                        overshoot_damage -= damage_required;
                        defending_group.kill_top_defender(); // Kill the top defending unit.
                        defending_units_remaining--;
                    } // if (...)
                    else
                    {
                        effective_damage = damage_cast(overshoot_damage, damage_factor);
                        overshoot_damage = 0;
                        std::size_t previous_count = defending_group.count_defender();
                        defending_group.damage_no_splash(effective_damage); // Damage the top defending unit.
                        std::size_t count_killed = previous_count - defending_group.count_defender();
                        defending_units_remaining -= count_killed;
                    } // else (...)
                } // while (...)
            } // while (...)
            if constexpr (t_logger_type::is_enabled)
                logger << " killing " << (defenders_killed - defending_group.count_defender()) << "." << nullptr;
            return attacking_group.count_attacker();
        } // hit(...)

        /** @brief Inflicts \p pure_damage splash onto \p defending_group, taking into account \p damage_factor damage multiplier.
         *  @return Remaining \p pure_damage.
         */
        template <typename t_logger_type>
        static std::size_t splash(std::size_t pure_damage, double damage_factor, unit_group& defending_group, t_logger_type& logger) noexcept
        {
            if (pure_damage == 0) return 0;
            
            std::size_t defending_units_remaining = defending_group.count_defender();
            // Total hit points in the defending group.
            std::size_t total_hit_points = defending_group.total_hit_points_defender();
            // Pure damage required to eliminate the defending group.
            std::size_t damage_required = inverse_damage_cast(total_hit_points, damage_factor);
            if (damage_required > pure_damage) // Remaining damage is not enough to eliminate this group.
            {
                // Calculate the actual damage dealt.
                std::size_t effective_damage = damage_cast(pure_damage, damage_factor);
                // Damage the defending group.
                defending_group.damage_pure_splash(effective_damage);
                // All the splash damage has been accounted for.
                pure_damage = 0;
            } // if (...)
            else // The defending group is guaranteed to be eliminated.
            {
                // Kill the current defending group.
                defending_group.kill_all_defender();
                // Calculate the remaining splash damage.
                pure_damage -= damage_required;
            } // else (...)
            if constexpr (t_logger_type::is_enabled)
                logger << '\t' << "Overshoot killing " << (defending_units_remaining - defending_group.count_defender()) << " " << defending_group.unit().first_name() << nullptr;
            return pure_damage;
        } // splash(...)

        /** @brief When attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
         *  @return The attacking unit index after the attack has been completed.
         *  @remark Defending group is assumed to be alive.
         *  @remark Attacking group is assumed to be alive.
         */
        template <typename t_logger_type>
        static std::size_t one_to_one(unit_group& defending_group, const unit_group& attacking_group, std::size_t attacking_unit_index, t_logger_type& logger) noexcept
        {
            // Proceed to next attaking unit.
            std::size_t attacking_units_remaining = attacking_group.count_attacker() - attacking_unit_index;
            std::size_t defending_units_remaining = defending_group.count_defender();

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
                return attacking_group.count_attacker();
            } // else (...)
        } // one_to_one(...)
    } // namespace technical_combat
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_TECHNICAL_COMBAT_HPP_INCLUDED
