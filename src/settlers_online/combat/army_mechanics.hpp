
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_MECHANICS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_MECHANICS_HPP_INCLUDED

#include <ropufu/algebra.hpp>    // aftermath::algebra::matrix

#include "../enums.hpp"
#include "arithmetic.hpp"
#include "army.hpp"
#include "attack_sequence.hpp"
#include "battle_clock.hpp"
#include "technical_combat.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef>  // std::size_t
#include <optional> // std::optional, std::nullopt
#include <set>      // std::set
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::make_signed_t
#include <vector>   // std::vector

namespace ropufu::settlers_online
{
    /** @brief Stores battle parameters determining how an army is to attack its opponent. */
    struct army_mechanics
    {
        using type = army_mechanics;

    private:
        army m_underlying;
        std::vector<std::size_t> m_original_counts; // Unit counts before the battle.
        std::vector<std::optional<double>> m_uniform_splash_factor = {}; // Damage reduction factors to be used when: (i) there is always splash damage; and (ii) all defender units share the same damage reduction factor.
        // ~~ Paired group properties ~~
        aftermath::algebra::matrix<bool> m_one_to_one; // Indicates that (i) there is no splash damage and (ii) the minimal effective damage is enough to kill one defending unit.
        aftermath::algebra::matrix<double> m_reduction_factor; // Damage reduction factors agains defender groups.
        aftermath::algebra::matrix<std::size_t> m_attack_order; // Order in which enemy groups are to be attacked.

    public:
        army_mechanics() noexcept { }

        /** Prepares two conditioned armies for combat. */
        army_mechanics(const army& a, const army& other) noexcept
            : m_underlying(a),
            m_original_counts(a.group_counts()),
            m_one_to_one(a.count_groups(), other.count_groups()),
            m_reduction_factor(a.count_groups(), other.count_groups()),
            m_attack_order(a.count_groups(), other.count_groups())
        {
            std::size_t m = a.count_groups();
            std::size_t n = other.count_groups();

            this->m_uniform_splash_factor.reserve(m);
            double damage_factor_normal = 1;
            double damage_factor_in_tower = (1 - other.camp().damage_reduction());

            std::error_code ec {};
            for (std::size_t i = 0; i < m; ++i)
            {
                const unit_group& g = a[i];
                const unit_type& t = g.unit();

                // Damage factors and related properties.
                bool is_never_splash = t.damage().splash_chance() == 0;
                bool is_always_splash = t.damage().splash_chance() == 1;
                bool do_ignore_tower_bonus = t.has(special_ability::ignore_tower_bonus);

                std::set<double> distinct_factors {}; // Set of distinct damage factors: important if there is a chance of splash damage and overshoot has a different reduction in the next group.
                for (std::size_t j = 0; j < n; ++j)
                {
                    const unit_group& defender = other[j];
                    bool do_tower_bonus = defender.unit().has(special_ability::tower_bonus);
                    if (do_ignore_tower_bonus) do_tower_bonus = false;
                    double damage_factor = do_tower_bonus ? damage_factor_in_tower : damage_factor_normal;
                    
                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    std::size_t effective_min_damage = damage_cast(t.effective_damage(0).low, damage_factor);
                    // Even though defender has not been conditioned, hit point reduction occurs only for bosses that typically don't come in large groups.
                    bool is_one_to_one = is_never_splash && (effective_min_damage >= defender.unit().hit_points());

                    distinct_factors.insert(damage_factor);
                    this->m_reduction_factor(i, j) = damage_factor;
                    this->m_one_to_one(i, j) = is_one_to_one;
                } // for (...)

                // Populate single group properties.
                std::optional<double> uniform_splash_factor = std::nullopt;
                if (is_always_splash && distinct_factors.size() == 1) uniform_splash_factor = (*(distinct_factors.begin()));
                this->m_uniform_splash_factor.push_back(uniform_splash_factor);

                // Figure out which ordering to use.
                bool do_attack_weakest_target = t.has(special_ability::attack_weakest_target);
                aftermath::algebra::permutation<std::size_t> attack_order = do_attack_weakest_target ? other.order_by_hp() : other.order_by_id();
                for (std::size_t j = 0; j < n; ++j)
                {
                    this->m_attack_order(i, j) = attack_order[j];
                } // for (...)

                if (ec) std::exit(ec.value()); // This should never happen.
            } // for (...)

            this->m_uniform_splash_factor.shrink_to_fit();
        } // army_mechanics(...)

        const army& underlying() const noexcept { return this->m_underlying; }

        void snapshot() noexcept { this->m_underlying.snapshot(); }

        std::vector<std::size_t> calculate_losses() const noexcept
        {
            std::vector<std::size_t> losses = this->m_underlying.group_counts();
            for (std::size_t i = 0; i < losses.size(); ++i) losses[i] = this->m_original_counts[i] - losses[i];
            return losses;
        } // calculate_losses(...)

        /** Initiates and attack by this army on its opponent \c defender. */
        template <typename t_sequence_type, typename t_logger_type>
        void initiate_phase(army_mechanics& defender, battle_phase phase, double frenzy_rate, army_sequence<t_sequence_type>& sequencer, battle_clock& clock, t_logger_type& logger) const noexcept
        {
            std::size_t m = this->m_underlying.count_groups();

            for (std::size_t i = 0; i < m; ++i)
            {
                const unit_group& attacking_group = this->m_underlying[i]; // Attacking group.
                const unit_type& attacker_type = attacking_group.unit(); // Attacking group type.

                if (!attacker_type.attack_phases().has(phase)) continue; // Skip group from different phases.
                if (!attacking_group.alive_attacker()) continue; // Skip empty groups.

                // Optimize in the uniform all splash damage case.
                if (this->m_uniform_splash_factor[i].has_value())
                {
                    double damage_factor = this->m_uniform_splash_factor[i].value(); // Adjusted damage factor for current round.
                    this->uniform_splash_at(i, defender, damage_factor, frenzy_rate, sequencer, clock, logger);
                } // if (...)
                else
                {
                    this->unoptimized_attack_at(i, defender, frenzy_rate, sequencer, clock, logger);
                } // else (...)
            } // for (...)
        } // initiate_phase(...)
        
        /** Destruct \p other's camp. */
        template <typename t_sequence_type>
        std::size_t destruct(const army_mechanics& defender, t_sequence_type& sequencer, battle_clock& clock) const noexcept
        {
            using hit_points_type = std::make_signed_t<std::size_t>;

            std::size_t m = this->m_underlying.count_groups();

            std::size_t count_rounds = 0;
            hit_points_type camp_hit_points = static_cast<hit_points_type>(defender.m_underlying.camp().hit_points());
            while (camp_hit_points > 0)
            {
                for (std::size_t i = 0; i < m; ++i)
                {
                    const unit_group& g = this->m_underlying[i];
                    const unit_type& t = g.unit();
                    if (!g.alive_attacker()) continue;

                    detail::damage_pair<std::size_t> attacker_damage = t.effective_damage(0);
                    bool do_high_damage = sequencer[i].peek_do_high_damage(clock);
                    std::size_t damage = do_high_damage ? attacker_damage.high : attacker_damage.low;
                    if (t.is(unit_category::artillery)) damage *= 2;

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
        void uniform_splash_at(std::size_t i, army_mechanics& defender,
            double damage_factor, double frenzy_rate, army_sequence<t_sequence_type>& sequencer, battle_clock& clock, t_logger_type& logger) const noexcept
        {
            std::size_t n = defender.m_underlying.count_groups();
            const unit_group& attacking_group = this->m_underlying[i]; // Attacking group.
            const unit_type& attacker_type = attacking_group.unit(); // Attacking group type.
            detail::damage_pair<std::size_t> attacker_damage = attacker_type.effective_damage(frenzy_rate);

            std::size_t count_attackers = attacking_group.count_attacker();
            std::size_t count_high_damage = sequencer[i].peek_count_high_damage(count_attackers, clock); // Count the number of units in this stack that do maximum damage.
            std::size_t count_low_damage = count_attackers - count_high_damage;
            clock.next_unit(count_attackers);

            std::size_t total_reduced_damage = 
                damage_cast(attacker_damage.low, damage_factor) * count_low_damage +
                damage_cast(attacker_damage.high, damage_factor) * count_high_damage;

            if constexpr (t_logger_type::is_enabled)
            {
                logger << '\t' <<
                    count_attackers << " " << attacker_type.first_name() <<
                    " dealing " << total_reduced_damage <<
                    " (" << count_low_damage << " low, " << count_high_damage << " high)" <<
                    " splash damage..." << nullptr;
            } // if constexpr (...)

            for (std::size_t k = 0; k < n; ++k)
            {
                std::size_t j = this->m_attack_order(i, k);
                unit_group& defending_group = defender.m_underlying[j];
                if (!defending_group.alive_defender()) continue; // Defender has already been killed.

                std::size_t defending_units_remaining = defending_group.count_defender();

                if constexpr (t_logger_type::is_enabled)
                {
                    std::size_t total_hit_points = defending_group.total_hit_points_defender();
                    logger << '\t' << "...against " <<
                        defending_units_remaining << " " << defending_group.unit().first_name() <<
                        " [" << total_hit_points << " hit points]";
                } // if constexpr (...)

                total_reduced_damage = defending_group.damage_pure_splash(total_reduced_damage);
                
                if constexpr (t_logger_type::is_enabled)
                {
                    logger << " killing " << (defending_units_remaining - defending_group.count_defender()) << "." << nullptr;
                } // if constexpr (...)

                if (total_reduced_damage == 0) return;
            } // for (...)
        } // uniform_splash_at(...)
        
        /** @brief Inflicts the reduced damage, \p reduced_damage, onto \p defender, assuming the attaker always deals splash damage and there is no effective tower bonus.
         *  @remark Attacking group is assumed to be alive.
         */
        template <typename t_sequence_type, typename t_logger_type>
        void unoptimized_attack_at(std::size_t i, army_mechanics& defender,
            double frenzy_rate, army_sequence<t_sequence_type>& sequencer, battle_clock& clock, t_logger_type& logger) const noexcept
        {
            std::size_t n = defender.m_underlying.count_groups();
            const unit_group& attacking_group = this->m_underlying[i]; // Attacking group.
            const unit_type& attacker_type = attacking_group.unit(); // Attacking group type.

            if constexpr (t_logger_type::is_enabled)
            {
                logger << '\t' <<
                    attacking_group.count_attacker() << " " << attacker_type.first_name() <<
                    " attacking..." << nullptr;
            } // if constexpr (...)

            std::size_t attacking_unit_index = 0;
            std::size_t pure_overshoot_damage = 0; // Pure damage spilled between consequent groups.
            for (std::size_t k = 0; k < n; ++k)
            {
                // Current defender.
                std::size_t j = this->m_attack_order(i, k);
                unit_group& defending_group = defender.m_underlying[j];
                if (!defending_group.alive_defender()) continue; // Defender has already been killed.

                // Get cached properties.
                double damage_factor = this->m_reduction_factor(i, j);
                bool is_one_to_one = this->m_one_to_one(i, j);

                // First take care of the overshoot damage.
                pure_overshoot_damage = technical_combat::splash(pure_overshoot_damage, damage_factor, defending_group, logger);
                if (!defending_group.alive_defender()) continue; // Defender has been killed by overshoot damage.

                // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                if (is_one_to_one) attacking_unit_index = technical_combat::one_to_one(defending_group, attacking_group, attacking_unit_index, logger); // technical_combat.hpp.
                else attacking_unit_index = technical_combat::hit(defending_group, attacking_group, attacking_unit_index, damage_factor, frenzy_rate, sequencer[i], clock, logger, pure_overshoot_damage); // technical_combat.hpp.

                if (attacking_unit_index == attacking_group.count_attacker()) break;
                if (attacking_unit_index > attacking_group.count_attacker()) // <attacking_unit_index> overflow.
                {
                    std::exit(05374104); // This should never happen.
                    break;
                } // if (...)
            } // for (...)
        } // unoptimized_attack_at(...)
    }; // struct army_mechanics
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_MECHANICS_HPP_INCLUDED
