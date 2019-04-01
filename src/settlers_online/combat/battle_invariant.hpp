
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_INVARIANT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_INVARIANT_HPP_INCLUDED

#include <ropufu/algebra.hpp> // aftermath::algebra::matrix, aftermath::algebra::permutation, aftermath::algebra::fraction

#include "../arithmetic.hpp"
#include "../enums.hpp"

#include "army.hpp"
#include "damage.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <system_error>  // std::error_code
#include <unordered_set> // std::unordered_set
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    namespace detail
    {
        struct damageplex
        {
            using damage_type = ropufu::settlers_online::damage;
            using fraction_type = aftermath::algebra::fraction<std::size_t>;
            
            aftermath::algebra::matrix<damage_type> effective_damage;
            aftermath::algebra::matrix<fraction_type> low_damage_ratio;
            aftermath::algebra::matrix<fraction_type> high_damage_ratio;
        }; // struct damageplex
    } // namespace detail

    /** @brief Stores deterministic battle information that does not change between simulations, e.g., damage tables, etc. */
    struct battle_invariant
    {
        using type = battle_invariant;
        using damage_percentage_type = typename damage_bonus_type::percentage_type;
        using fraction_type = typename detail::damageplex::fraction_type; 

        // Make sure there is room for at least this many rounds' worth of tables.
        static constexpr std::size_t default_capacity = 10;

    private:
        aftermath::algebra::matrix<std::size_t> m_attack_order = {}; // Order in which enemy groups are to be attacked.
        std::vector<std::size_t> m_original_counts = {}; // Unit counts before the battle.
        std::vector<detail::damageplex> m_damage_tables = {}; // Damage agains defender groups (including frenzy bonus and damage reduction).
        // ~~ Optimization ~~
        aftermath::algebra::matrix<bool> m_is_one_to_one = {}; // Indicates that: (i) there is no splash damage; and (ii) the minimal effective damage is enough to kill one defending unit.
        aftermath::algebra::matrix<fraction_type> m_damage_ratio = {}; // Stores damage ratios between consecutive groups.
        std::vector<bool> m_is_uniform_splash = {}; // Indicates that: (i) there is always splash damage; and (ii) all defender units share the same damage reduction.
        // ~~ Auxiliary ~~
        aftermath::algebra::matrix<damage_percentage_type> m_tower_damage_modifier = {};
        std::vector<unit_type> m_unit_types = {};
        damage_percentage_type m_frenzy_bonus = {};

        /** @warning Please, use responsibly: no checks on the existing \c m_damage_tables.size() are performed. */
        void build_damage_table_at(std::size_t round_index) noexcept
        {
            std::size_t m = this->m_attack_order.height();
            std::size_t n = this->m_attack_order.width();
            std::error_code ec {};

            // Effective damage.
            aftermath::algebra::matrix<damage> effective_damage {m, n};
            damage_percentage_type frenzy_bonus { static_cast<typename damage_percentage_type::integer_type>(round_index) * this->m_frenzy_bonus };

            for (std::size_t i = 0; i < m; ++i)
            {
                const unit_type& t = this->m_unit_types[i];
                for (std::size_t j = 0; j < n; ++j)
                {
                    damage_percentage_type damage_modifier = this->m_tower_damage_modifier(i, j);
                    damage_modifier += frenzy_bonus;
                    effective_damage(i, j) = t.damage(damage_modifier);
                } // for (...)
            } // for (...)

            // Damage ratios.
            aftermath::algebra::matrix<fraction_type> low_damage_ratio {m, n};
            aftermath::algebra::matrix<fraction_type> high_damage_ratio {m, n};
            for (std::size_t i = 0; i < m; ++i)
            {
                damage previous_damage {};
                if (n > 0)
                {
                    std::size_t j = this->m_attack_order(i, 0);
                    previous_damage = effective_damage(i, j);
                    low_damage_ratio(i, j) = {};
                    high_damage_ratio(i, j) = {};
                } // if (...)

                for (std::size_t k = 1; k < n; ++k)
                {
                    std::size_t j = this->m_attack_order(i, k);
                    fraction_type low {effective_damage(i, j).low(), previous_damage.low(), ec};
                    fraction_type high {effective_damage(i, j).high(), previous_damage.high(), ec};
                    low_damage_ratio(i, j) = low;
                    high_damage_ratio(i, j) = high;
                } // for (...)
            } // for (...)
            
            this->m_damage_tables.push_back({ effective_damage, low_damage_ratio, high_damage_ratio });
        } // build_damage_table_at(...)

    public:
        battle_invariant() noexcept { }

        /** Prepares two conditioned armies for combat. */
        battle_invariant(const army& a, const army& other) noexcept
            : m_attack_order(a.count_groups(), other.count_groups()),
            m_original_counts(a.group_counts()),
            m_is_one_to_one(a.count_groups(), other.count_groups()),
            m_tower_damage_modifier(a.count_groups(), other.count_groups()),
            m_frenzy_bonus(a.frenzy_bonus())
        {
            std::size_t m = a.count_groups();
            std::size_t n = other.count_groups();

            this->m_damage_tables.reserve(type::default_capacity);
            this->m_is_uniform_splash.reserve(m);
            this->m_unit_types.reserve(m);

            const damage_percentage_type& damage_reduction_in_tower = other.camp().damage_reduction();

            for (std::size_t i = 0; i < m; ++i)
            {
                const unit_group& g = a[i];
                const unit_type& t = g.unit();

                // Damage factors and related properties.
                bool does_never_splash = t.never_splash();
                bool does_always_splash = t.always_splash();
                bool has_ignore_tower_bonus = t.has(special_ability::ignore_tower_bonus);
                bool does_attack_weakest_target = t.has(special_ability::attack_weakest_target);

                std::unordered_set<typename damage_percentage_type::integer_type> distinct_damage_modifiers {};
                aftermath::algebra::permutation<std::size_t> attack_order = does_attack_weakest_target ? other.order_by_hp() : other.order_by_id();
                for (std::size_t j = 0; j < n; ++j)
                {
                    const unit_group& defender = other[j];

                    bool has_tower_bonus = defender.unit().has(special_ability::tower_bonus);
                    if (has_ignore_tower_bonus) has_tower_bonus = false;
                    damage_percentage_type damage_modifier {}; // No damage modifier.
                    if (has_tower_bonus) damage_modifier -= damage_reduction_in_tower;

                    // Optimize when attacking units with low hit points: each non-splash hit will always kill exactly 1 defending unit.
                    std::size_t effective_min_damage = t.damage(damage_modifier).low();
                    bool is_one_to_one = does_never_splash && (effective_min_damage >= defender.unit().hit_points());

                    distinct_damage_modifiers.insert(damage_modifier.numerator());
                    this->m_attack_order(i, j) = attack_order[j];
                    this->m_is_one_to_one(i, j) = is_one_to_one;
                    this->m_tower_damage_modifier(i, j) = damage_modifier;
                } // for (...)

                // Populate single group properties.
                bool is_uniform_splash = does_always_splash && (distinct_damage_modifiers.size() == 1);
                this->m_is_uniform_splash.push_back(is_uniform_splash);
                this->m_unit_types.push_back(t);
            } // for (...)

            this->m_is_uniform_splash.shrink_to_fit();
            this->m_unit_types.shrink_to_fit();

            this->build_damage_table_at(0);
        } // battle_invariant(...)

        const aftermath::algebra::matrix<std::size_t>& attack_order() const noexcept { return this->m_attack_order; }

        const std::vector<std::size_t>& original_counts() const noexcept { return this->m_original_counts; }
        std::size_t original_counts(std::size_t index) const noexcept { return this->m_original_counts[index]; }

        const detail::damageplex& at(std::size_t round_index) noexcept
        {
            for (std::size_t k = this->m_damage_tables.size(); k <= round_index; ++k) this->build_damage_table_at(k);
            return this->m_damage_tables[round_index];
        } // at(...)

        const aftermath::algebra::matrix<bool>& is_one_to_one() const noexcept { return this->m_is_one_to_one; }

        const std::vector<bool>& is_uniform_splash() const noexcept { return this->m_is_uniform_splash; }
        bool is_uniform_splash(std::size_t index) const noexcept { return this->m_is_uniform_splash[index]; }
    }; // struct battle_invariant
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_INVARIANT_HPP_INCLUDED
