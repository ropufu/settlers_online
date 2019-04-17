
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_MECHANICS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_MECHANICS_HPP_INCLUDED

#include <ropufu/algebra.hpp> // aftermath::algebra::matrix

#include "../algebra.hpp"
#include "../enums.hpp"

#include "army.hpp"
#include "attack_sequence.hpp"
#include "battle_clock.hpp"
#include "battle_invariant.hpp"
#include "damage.hpp"
#include "technical_combat.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef>  // std::size_t
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::make_signed_t
#include <vector>   // std::vector

namespace ropufu::settlers_online
{
    /** @brief Stores battle parameters determining how an army is to attack its opponent. */
    struct army_mechanics
    {
        using type = army_mechanics;
        using damage_type = ropufu::settlers_online::damage;
        using damage_percentage_type = typename damage_bonus_type::percentage_type;

    private:
        army m_underlying = {};

    public:
        army_mechanics() noexcept { }

        /** Prepares two conditioned armies for combat. */
        army_mechanics(const army& a) noexcept
            : m_underlying(a)
        {
        } // army_mechanics(...)

        const army& underlying() const noexcept { return this->m_underlying; }

        void snapshot() noexcept { this->m_underlying.snapshot(); }

        std::vector<std::size_t> calculate_losses(const battle_invariant& invariant) const noexcept
        {
            std::vector<std::size_t> losses = this->m_underlying.group_counts();
            for (std::size_t i = 0; i < losses.size(); ++i) losses[i] = invariant.original_counts(i) - losses[i];
            return losses;
        } // calculate_losses(...)

        /** Initiates and attack by this army on its opponent \c defender. */
        template <typename t_sequence_type, typename t_logger_type>
        void initiate_phase(battle_invariant& invariant, army_mechanics& defender, battle_phase phase, army_sequence<t_sequence_type>& sequencer, battle_clock& clock, t_logger_type& logger) const noexcept
        {
            std::size_t m = this->m_underlying.count_groups();

            for (std::size_t i = 0; i < m; ++i)
            {
                const unit_group& attacking_group = this->m_underlying[i]; // Attacking group.
                const unit_type& attacker_type = attacking_group.unit(); // Attacking group type.

                if (!attacker_type.attack_phases().has(phase)) continue; // Skip group from different phases.
                if (!attacking_group.alive_as_attacker()) continue; // Skip empty groups.

                // Optimize in the uniform all splash damage case.
                if (invariant.is_uniform_splash(i)) this->uniform_splash_at(i, invariant, defender, sequencer, clock, logger);
                else this->unoptimized_attack_at(i, invariant, defender, sequencer, clock, logger);
            } // for (...)
        } // initiate_phase(...)
        
        /** Destruct \p other's camp. */
        template <typename t_sequence_type>
        std::size_t destruct(const battle_invariant& invariant, const army_mechanics& defender, t_sequence_type& sequencer, battle_clock& clock) const noexcept
        {
            using hit_points_type = std::make_signed_t<std::size_t>;
            const std::vector<damage>& destruction_damage = invariant.destruction_damage();

            std::size_t m = this->m_underlying.count_groups();

            std::size_t count_rounds = 0;
            hit_points_type camp_hit_points = static_cast<hit_points_type>(defender.m_underlying.camp().hit_points());
            while (camp_hit_points > 0)
            {
                for (std::size_t i = 0; i < m; ++i)
                {
                    const unit_group& g = this->m_underlying[i];
                    if (!g.alive_as_attacker()) continue;

                    const damage& attacker_damage = destruction_damage[i];
                    bool do_high_damage = sequencer[i].peek_do_high_damage(clock);
                    std::size_t damage = do_high_damage ? attacker_damage.high() : attacker_damage.low();

                    camp_hit_points -= damage;
                    clock.next_unit();
                } // for (...)
                
                clock.next_round();
                ++count_rounds;
            } // while (...)
            return count_rounds;
        } // destruct(...)

    private:
        /** @brief Inflicts the reduced damage, \p reduced_damage, onto \p defender, assuming the attaker always deals splash damage and there is no effective tower bonus.
         *  @remark Attacking group is assumed to be alive.
         */
        template <typename t_sequence_type, typename t_logger_type>
        void uniform_splash_at(std::size_t i, battle_invariant& invariant, army_mechanics& defender,
            army_sequence<t_sequence_type>& sequencer, battle_clock& clock, t_logger_type& logger) const noexcept
        {
            logger.touch(); // To avoid unreferenced named parameter whenever \c t_logger_type::is_enabled is \c false.
            std::size_t n = defender.m_underlying.count_groups();
            const unit_group& attacking_group = this->m_underlying[i]; // Attacking group.
            const unit_type& attacker_type = attacking_group.unit(); // Attacking group type.
            const detail::damageplex& damage_table = invariant.at(clock.round_index());
            const aftermath::algebra::matrix<std::size_t>& attack_order = invariant.attack_order();
            const damage& attacker_damage = damage_table.effective_damage(i, 0);

            std::size_t count_attackers = attacking_group.count_as_attacker();
            std::size_t count_high_damage = sequencer[i].peek_count_high_damage(count_attackers, clock); // Count the number of units in this stack that do maximum damage.
            std::size_t count_low_damage = count_attackers - count_high_damage;
            clock.next_unit(count_attackers);

            std::size_t total_damage = 
                attacker_damage.low() * count_low_damage +
                attacker_damage.high() * count_high_damage;

            if constexpr (t_logger_type::is_enabled)
            {
                logger << '\t' <<
                    count_attackers << " " << attacker_type.first_name() <<
                    " dealing " << total_damage <<
                    " (" << count_low_damage << " low, " << count_high_damage << " high)" <<
                    " splash damage..." << nullptr;
            } // if constexpr (...)

            for (std::size_t k = 0; k < n; ++k)
            {
                std::size_t j = attack_order(i, k);
                unit_group& defending_group = defender.m_underlying[j];
                if (!defending_group.alive_as_defender()) continue; // Defender has already been killed.

                if constexpr (t_logger_type::is_enabled)
                {
                    logger.set_tag(defending_group.count_as_defender());
                    std::size_t total_hit_points = defending_group.total_hit_points_defender();
                    logger << '\t' << "...against " <<
                        logger.tag() << " " << defending_group.unit().first_name() <<
                        " [" << total_hit_points << " hit points]";
                } // if constexpr (...)

                total_damage = defending_group.damage_pure_splash(total_damage);
                
                if constexpr (t_logger_type::is_enabled)
                {
                    logger << " killing " << (logger.tag() - defending_group.count_as_defender()) << "." << nullptr;
                } // if constexpr (...)

                if (total_damage == 0) return;
            } // for (...)
        } // uniform_splash_at(...)
        
        /** @brief Inflicts the reduced damage, \p reduced_damage, onto \p defender, assuming the attaker always deals splash damage and there is no effective tower bonus.
         *  @remark Attacking group is assumed to be alive.
         *  @warning One-to-one optimization relies on the fact that damage can not go down as the battle progresses. It should be taken into account when more features are introduced to the game. 
         */
        template <typename t_sequence_type, typename t_logger_type>
        void unoptimized_attack_at(std::size_t i, battle_invariant& invariant, army_mechanics& defender,
            army_sequence<t_sequence_type>& sequencer, battle_clock& clock, t_logger_type& logger) const noexcept
        {
            logger.touch(); // To avoid unreferenced named parameter whenever \c t_logger_type::is_enabled is \c false.
            std::size_t n = defender.m_underlying.count_groups();
            const unit_group& attacking_group = this->m_underlying[i]; // Attacking group.
            const unit_type& attacker_type = attacking_group.unit(); // Attacking group type.
            const detail::damageplex& damage_table = invariant.at(clock.round_index());
            const aftermath::algebra::matrix<std::size_t>& attack_order = invariant.attack_order();
            const aftermath::algebra::matrix<bool>& is_one_to_one = invariant.is_one_to_one();

            if constexpr (t_logger_type::is_enabled)
            {
                logger << '\t' <<
                    attacking_group.count_as_attacker() << " " << attacker_type.first_name() <<
                    " attacking..." << nullptr;
            } // if constexpr (...)

            std::size_t attacking_unit_index = 0;
            detail::overshoot_damage overshoot {}; // Damage spilled between consequent groups.
            for (std::size_t k = 0; k < n; ++k)
            {
                // Current defender.
                std::size_t j = attack_order(i, k);
                unit_group& defending_group = defender.m_underlying[j];
                if (!defending_group.alive_as_defender()) continue; // Defender has already been killed.

                // Get cached properties.
                const damage& attacker_damage = damage_table.effective_damage(i, j);
                overshoot.adjust(
                    damage_table.low_damage_ratio(i, j),
                    damage_table.high_damage_ratio(i, j)
                );

                // First take care of the overshoot damage.
                technical_combat::splash(overshoot, defending_group, logger);
                if (!defending_group.alive_as_defender()) continue; // Defender has been killed by overshoot damage.

                // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                if (is_one_to_one(i, j)) attacking_unit_index = technical_combat::one_to_one(defending_group, attacking_group, attacking_unit_index, logger); // technical_combat.hpp.
                else attacking_unit_index = technical_combat::hit(defending_group, attacking_group, attacking_unit_index, attacker_damage, sequencer[i], clock, logger, overshoot); // technical_combat.hpp.

                if (attacking_unit_index == attacking_group.count_as_attacker()) break;
                if (attacking_unit_index > attacking_group.count_as_attacker()) // <attacking_unit_index> overflow.
                {
                    std::exit(05374104); // This should never happen.
                    break;
                } // if (...)
            } // for (...)
        } // unoptimized_attack_at(...)
    }; // struct army_mechanics
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_MECHANICS_HPP_INCLUDED
