
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_HPP_INCLUDED

#include <ropufu/algebra.hpp>
#include <ropufu/enum_array.hpp>
#include <ropufu/on_error.hpp>

#include "../enums.hpp"
#include "../char_string.hpp"

#include "arithmetic.hpp"
#include "camp.hpp"
#include "combat_result.hpp"
#include "damage.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <algorithm>  // std::sort
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <initializer_list> // std::initializer_list
#include <ostream>    // std::ostream
#include <set>        // std::set
#include <stdexcept>  // std::out_of_range
#include <system_error>  // std::error_code, std::errc
#include <unordered_map> // std::unordered_map
#include <vector>     // std::vector

namespace ropufu::settlers_online
{
    /** Descriptor for colletions of groups of units. */
    struct army
    {
        using type = army;
        using camp_type = ropufu::settlers_online::camp;
        using mask_type = typename ropufu::settlers_online::detail::combat_result::mask_type;

        static constexpr std::size_t capacity = ropufu::settlers_online::detail::combat_result::army_capacity;

    private:
        std::vector<unit_group> m_groups = {}; // Unit groups, sorted by \c id.
        std::vector<mask_type> m_metagroup_masks = {}; // For each non-singleton metagroup store its mask (default-sorted).
        camp_type m_camp = {}; // Determines the defensive capabilities of the army.
        // ~~ Permutations of <m_groups> ~~
        aftermath::algebra::permutation<std::size_t> m_order_by_id = {}; // Permutation when defending agains units with \c do_attack_weakest_target.
        aftermath::algebra::permutation<std::size_t> m_order_by_hp = {}; // Permutation when defending agains units with \c do_attack_weakest_target.
        // ~~ Battle modifiers and traits ~~
        double m_frenzy_bonus = 0; // Increases (multiplicative) the attack damage of this army for every combat round past the first.
        aftermath::enum_array<battle_skill, std::size_t> m_skills = {}; // Skills affecting various aspects of the battle.
        aftermath::flags_t<battle_trait> m_traits = {}; // If at least one unit in the army has this trait, it kicks in.

        bool initialize(std::error_code& ec) noexcept;

    public:
        /** An empty army. */
        army() noexcept { }

        /** @brief Constructs an army from the collection of \c unit_groups.
         *  @param ec Set to std::errc::argument_list_too_long if there are two groups sharing the same \c id.
         *  @param ec Set to std::errc::invalid_argument if the number of groups exceeds \c capacity constant.
         *  @param ec Set to std::errc::interrupted if any other unexpected error happens.
         */
        army(std::initializer_list<unit_group> groups, const camp_type& camp, std::error_code& ec) noexcept
            : m_groups(groups), m_camp(camp)
        {
            if (!this->initialize(ec)) this->m_groups.clear();
        } // army(...)

        /** @brief Constructs an army from the collection of \c unit_groups.
         *  @param ec Set to std::errc::argument_list_too_long if there are two groups sharing the same \c id.
         *  @param ec Set to std::errc::invalid_argument if the number of groups exceeds \c capacity constant.
         *  @param ec Set to std::errc::interrupted if any other unexpected error happens.
         */
        army(const std::vector<unit_group>& groups, const camp_type& camp, std::error_code& ec) noexcept
            : m_groups(groups), m_camp(camp)
        {
            if (!this->initialize(ec)) this->m_groups.clear();
        } // army(...)

        /** Applies weather conditions to the army. */
        void weather(battle_weather w) noexcept;

        /** Creates a copy of \p other with weather conditions applied. */
        army(const type& other, battle_weather w) noexcept
            : army(other)
        {
            this->weather(w);
        } // army(...)

        /** @brief Constructs a modified version of the army \p a conditioned for a fight against \p other.
         *  @param a The unconditioned(!) army to modify.
         *  @param other The unconditioned(!) army to prepare to fight against.
         */
        static type condition(const type& a, const type& other, std::error_code& ec) noexcept;

        /** Groups (ordered by \c id). */
        const std::vector<unit_group>& groups() const noexcept { return this->m_groups; }

        bool empty() const noexcept { return this->m_groups.empty(); }

        bool has(unit_faction faction) const noexcept;
        bool has(unit_category category) const noexcept;

        /** Discard the latest \c snapshot and record the current state of the army. */
        void snapshot() noexcept
        {
            for (unit_group& g : this->m_groups) g.snapshot();
        } // snapshot(...)

        /** Metagroup masks (ordered). */
        const std::vector<mask_type>& metagroup_masks() const noexcept { return this->m_metagroup_masks; }

        /** Determines the defensive capabilities of the army. */
        camp_type camp() const noexcept { return this->m_camp; }
        /** Determines the defensive capabilities of the army. */
        void set_camp(const camp_type& value) noexcept { this->m_camp = value; }

        /** Increases (multiplicative) the attack damage of this army for every combat round past the first. */
        double frenzy_bonus() const noexcept { return this->m_frenzy_bonus; }
        /** @brief Increases (multiplicative) the attack damage of this army for every combat round past the first.
         *  @param ec Set to std::errc::invalid_argument if \p value is negative.
         */
        void set_frenzy_bonus(double value, std::error_code& ec)
        {
            if (value < 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Frenzy bonus cannot be negative.");
            this->m_frenzy_bonus = value;
        } // set_frenzy_bonus(...)

        /** Skills affecting various aspects of the battle. */
        const aftermath::enum_array<battle_skill, std::size_t>& skills() const noexcept { return this->m_skills; }
        /** The level of the specified battle skill. */
        std::size_t level(battle_skill skill) const noexcept { return this->m_skills[skill]; }
        /** Skills affecting various aspects of the battle. */
        void set_level(battle_skill skill, std::size_t value) noexcept { this->m_skills[skill] = value; }
        /** Reset the skills. */
        void reset_skills() noexcept { this->m_skills = {}; }
        /** Text representation of skills. */
        std::string skills_string() const noexcept;

        /** Special traits that affect the entire battle. */
        const aftermath::flags_t<battle_trait>& traits() const noexcept { return this->m_traits; }
        /** Special traits that affect the entire battle. */
        void set_trait(battle_trait trait, bool value) noexcept { this->m_traits[trait] = value; }
        /** Checks whether this army has the specified battle trait. */
        bool has(battle_trait trait) const noexcept { return this->m_traits.has(trait); }
        /** Text representation of skills. */
        std::string traits_string() const noexcept;

        /** Unit types in each group (ordered by \c id). */
        std::vector<unit_type> types() const noexcept;

        /** Number of units in each group (ordered by \c id). */
        std::vector<std::size_t> group_counts() const noexcept;

        /** Total number of units in the army. */
        std::size_t count_units() const noexcept;

        /** Number of groups in the army. */
        std::size_t count_groups() const noexcept { return this->m_groups.size(); }
        
        /** Indicates whether there is a single unit in the army. */
        bool alive() const noexcept { return this->count_units() > 0; }

        /** Access groups (ordered by \c id). */
        const unit_group& operator [](std::size_t index) const { return this->m_groups[index]; }
        /** Access groups (ordered by \c id). */
        unit_group& operator [](std::size_t index) { return this->m_groups[index]; }

        /** @brief Access groups (ordered by \c id).
         *  @exception std::out_of_range \p index is greater of equal to the number of groups in the army.
         */
        const unit_group& at(std::size_t index) const
        {
            if (index >= this->m_groups.size()) throw std::out_of_range("<index> must be smaller than the number of groups in the army.");
            return this->operator [](index);
        } // at(...)

        /** @brief Access groups (ordered by \c id).
         *  @exception std::out_of_range \p index is greater of equal to the number of groups in the army.
         */
        unit_group& at(std::size_t index)
        {
            if (index >= this->m_groups.size()) throw std::out_of_range("<index> must be smaller than the number of groups in the army.");
            return this->operator [](index);
        } // at(...)

        /** Ordering by unit id. */
        const aftermath::algebra::permutation<std::size_t>& order_by_id() const noexcept { return this->m_order_by_id; }

        /** Ordering by unit hit points. */
        const aftermath::algebra::permutation<std::size_t>& order_by_hp() const noexcept { return this->m_order_by_hp; }

        /** The mask, specific to this instance of \c army, indicating the surviving groups. */
        mask_type compute_alive_mask() const noexcept
        {
            mask_type result {};
            aftermath::algebra::elementwise::to_binary_mask(this->m_groups, [] (const unit_group& g) { return g.alive_attacker(); }, result);
            return result;
        } // compute_alive_mask(...)

        /** Returns only the unit groups masked by \p alive_mask. */
        std::vector<unit_group> by_mask(mask_type alive_mask) const noexcept
        {
            std::vector<unit_group> result {};
            result.reserve(this->m_groups.size());
            aftermath::algebra::elementwise::from_binary_mask(this->m_groups, alive_mask, [&](const auto& g) { result.push_back(g); });
            result.shrink_to_fit();
            return result;
        } // by_mask(...)

        /** Total number of metagroups (groups of unit groups). */
        std::size_t count_metagroups() const noexcept { return this->m_metagroup_masks.size(); }

        /** Checks two armies for equality. */
        bool operator ==(const type& other) const noexcept
        {
            std::size_t count_groups = this->m_groups.size();
            if (count_groups != other.m_groups.size()) return false;
            // The groups are already properly sorted; no need to bother with ordering.
            for (std::size_t i = 0; i < count_groups; ++i) if (this->m_groups[i] != other.m_groups[i]) return false;
            
            // Ignore metagroups.
            // std::size_t count_metagroups = this->m_metagroup_masks.size();
            // if (count_metagroups != other.m_metagroup_masks.size()) return false;
            // for (std::size_t i = 0; i < count_metagroups; ++i) if (this->m_metagroup_masks[i] != other.m_metagroup_masks[i]) return false;
            
            return
                this->m_camp == other.m_camp &&
                this->m_frenzy_bonus == other.m_frenzy_bonus &&
                this->m_skills == other.m_skills &&
                this->m_traits == other.m_traits;
        } // operator ==(...)

        /** Checks two armies for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self)
        {
            bool is_first = true;
            if (self.m_groups.empty()) os << "empty";
            for (const unit_group& g : self.m_groups)
            {
                if (is_first) is_first = false;
                else os << " ";
                os << g;
            } // if (...)
            return os;
        } // operator <<(...)

        template <typename t_format_type>
        std::string to_string(const t_format_type& format) const noexcept
        {
            if (this->m_groups.empty()) return "empty";
            std::vector<std::string> group_names {};
            group_names.reserve(this->m_groups.size());
            for (const unit_group& g : this->m_groups)
            {
                group_names.push_back(std::to_string(g.count_attacker()) + format(g.unit()));
            } // for (...)
            return char_string::join(group_names, " ");
        } // to_string(...)
    }; // struct army

    void army::weather(battle_weather w) noexcept
    {
        std::error_code ec {};

        for (unit_group& g : this->m_groups)
        {
            unit_type t = g.unit(); // Copy of unit type to adjust.
            damage d = t.damage(); // Damage to modify.
            std::size_t hp = t.hit_points(); // Hit points to modify.

            switch (w)
            {
                case battle_weather::hard_frost:
                    if (t.is(unit_category::melee)) d.set_splash_chance(1, ec);
                    break;
                case battle_weather::bright_sunshine:
                    hp = product_floor(hp, 1.2);
                    break;
                case battle_weather::heavy_fog:
                    t.set_ability(special_ability::attack_weakest_target, true);
                    break;
                case battle_weather::hurricane:
                    d.reset(
                        product_floor(d.low(), 1.2),
                        product_floor(d.high(), 1.2));
                    break;
            } // switch (...)
            t.set_hit_points(hp, ec);
            t.set_damage(d, ec);
            g.set_unit(t); // Apply new type.
        } // for (...)
    } // army::weather(...)

    army army::condition(const army& a, const army& other, std::error_code& ec) noexcept
    {
        army result = a; // Conditioned version of \p a.
        for (unit_group& g : result.m_groups)
        {
            unit_type t = g.unit(); // Copy of unit type to adjust.

            // First, go through skills.
            for (const auto& pair : result.skills())
            {
                t.apply_friendly_skill(pair.key(), pair.value());
                // Increases the attack damage of this army by 10/20/30% for every combat round past the first.
                if (pair.key() == battle_skill::battle_frenzy) result.set_frenzy_bonus(0.1 * pair.value(), ec);
                if (ec) return a; // Error: panic, do nothing!
            }
            for (const auto& pair : other.skills()) t.apply_enemy_skill(pair.key(), pair.value());

            // Second, go through traits.
            damage d = t.damage(); // Damage to modify.
            // ~~ Traits ~~
            // Friendly trait: explosive ammunition.
            if (result.has(battle_trait::explosive_ammunition) && t.is(unit_category::ranged))
            {
                t.set_ability(special_ability::attack_weakest_target, true);
                d.set_splash_chance(1, ec);
            } // if (...)
            // Friendly trait: bombastic.
            if (result.has(battle_trait::bombastic) && t.is(unit_category::artillery))
            {
                // <bombastic_damage_factor> is defined in <battle_trait.hpp>.
                d.reset(
                    bombastic_damage_factor * d.low(),
                    bombastic_damage_factor * d.high());
            } // if (...)
            // Enemy trait: intercept.
            if (other.has(battle_trait::intercept))
            {
                t.set_ability(special_ability::attack_weakest_target, false);
                // <intercept_damage_percent> is defined in <battle_trait.hpp>.
                d.reset(
                    fraction_ceiling(intercept_damage_percent * d.low(), static_cast<std::size_t>(100)),
                    fraction_ceiling(intercept_damage_percent * d.high(), static_cast<std::size_t>(100)));
            } // if (...)
            // Enemy trait: dazzle.
            if (other.has(battle_trait::dazzle)) d.set_accuracy(0, ec);

            t.set_damage(d, ec); // Apply new damage.
            g.set_unit(t); // Apply new type.
            if (ec) return a; // Error: panic, do nothing!
        } // for (...)
        return result;
    } // army::condition(...)

    bool army::initialize(std::error_code& ec) noexcept
    {
        using type = army;

        // ~~ Validation ~~
        // Check mask capacity.
        if (this->m_groups.size() > type::capacity) return aftermath::detail::on_error(ec, std::errc::argument_list_too_long, "<mask_type> not large enought to store this army.", false);

        // Check for duplicate id's.
        std::set<std::size_t> unit_ids {};
        for (const unit_group& g : this->m_groups)
        {
            std::size_t id = g.unit().id();
            bool has_inserted = unit_ids.insert(id).second;
            if (!has_inserted) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "<groups> cannot have duplicate ids.", false);
        } // for (...)

        // ~~ Construction ~~
        // Store groups ordered by id.
        std::sort(this->m_groups.begin(), this->m_groups.end(), unit_group::compare_by_id);
        // Store permutations.
        this->m_order_by_id = aftermath::algebra::permutation<std::size_t>(this->m_groups.size());
        this->m_order_by_hp = aftermath::algebra::permutation<std::size_t>(this->m_groups.size());
        if (!this->m_order_by_hp.try_order_by(this->m_groups, unit_group::compare_by_hit_points)) return aftermath::detail::on_error(ec, std::errc::interrupted, "Sorting by hit points failed.", false);

        // Count non-singleton metagroups, and build traits.
        std::unordered_map<std::int_fast32_t, std::size_t> metagroup_ids {}; // List of used metagroup ids and corresponding counts.
        std::size_t count_metagroups = 0; // Count metagroups that occur at least twice.
        for (const unit_group& g : this->m_groups)
        {
            this->m_traits |= g.unit().traits(); // If a unit has a trait, the army has it.

            auto metagroup_it = metagroup_ids.find(g.metagroup_id()); // Find the current metagroup id in the list.
            if (metagroup_it == metagroup_ids.end()) metagroup_ids.emplace(g.metagroup_id(), 1); // Add to the list if it's not there.
            else
            {
                std::size_t n = ++(metagroup_it->second); // If it's on the list already, increment the count.
                if (n == 2) ++count_metagroups; // If it is the second time the metagroup id has been observed, increment the non-singleton count.
            } // if (...)
        } // for (...)

        // Build metagroup masks.
        this->m_metagroup_masks.reserve(count_metagroups);
        // Remove singletons and trim.
        for (const auto& p : metagroup_ids)
        {
            if (p.second == 1) continue; // Skip singletons.
            mask_type metagroup_mask {};
            aftermath::algebra::elementwise::to_binary_mask(
                this->m_groups, [&] (const unit_group& g) { return g.metagroup_id() == p.first; }, metagroup_mask);
            this->m_metagroup_masks.push_back(metagroup_mask);
        }
        this->m_metagroup_masks.shrink_to_fit();
        // Sort masks.
        std::sort(this->m_metagroup_masks.begin(), this->m_metagroup_masks.end());

        return true;
    } // army::initialize(...)

    std::string army::skills_string() const noexcept
    {
        if (this->m_skills.empty()) return "none";

        std::vector<std::string> names {};
        for (const auto& pair : this->m_skills)
        {
            if (pair.value() == 0) continue;
            names.push_back(std::to_string(pair.key()) + " (" + std::to_string(pair.value()) + ")");
        }
        return char_string::join(names, ", ");
    } // army::skills_string(...)

    std::string army::traits_string() const noexcept
    {
        if (this->m_traits.empty()) return "none";

        std::vector<std::string> names {};
        for (battle_trait trait : this->m_traits) names.push_back(std::to_string(trait));
        return char_string::join(names, ", ");
    } // army::traits_string(...)

    std::vector<unit_type> army::types() const noexcept
    {
        std::vector<unit_type> result {};
        result.reserve(this->m_groups.size());
        for (const unit_group& g : this->m_groups) result.push_back(g.unit());
        result.shrink_to_fit();
        return result;
    } // army::types(...)

    std::vector<std::size_t> army::group_counts() const noexcept
    {
        std::vector<std::size_t> result {};
        result.reserve(this->m_groups.size());
        for (const unit_group& g : this->m_groups) result.push_back(g.count_attacker());
        result.shrink_to_fit();
        return result;
    } // army::group_counts(...)

    std::size_t army::count_units() const noexcept
    {
        std::size_t value = 0;
        for (const unit_group& g : this->m_groups) value += g.count_attacker();
        return value;
    } // army::count_units(...)

    bool army::has(unit_faction faction) const noexcept
    {
        for (const unit_group& g : this->m_groups) if (g.unit().is(faction)) return true;
        return false;
    } // army::has(...)

    bool army::has(unit_category category) const noexcept
    {
        for (const unit_group& g : this->m_groups) if (g.unit().is(category)) return true;
        return false;
    } // army::has(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::army>
    {
        using argument_type = ropufu::settlers_online::army;
        using result_type = std::size_t;
        using mask_type = typename ropufu::settlers_online::detail::combat_result::mask_type;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<ropufu::settlers_online::unit_group> group_hash = {};
            std::hash<mask_type> mask_hash = {};
            std::hash<ropufu::settlers_online::camp> camp_hash = {};
            std::hash<ropufu::settlers_online::battle_skill> skill_hash = {};
            std::hash<ropufu::settlers_online::battle_trait> trait_hash = {};
            std::hash<std::size_t> size_hash = {};
            
            result_type result = 0;
            for (const ropufu::settlers_online::unit_group& g : x.groups()) result ^= group_hash(g);
            for (const mask_type& m : x.metagroup_masks()) result ^= mask_hash(m);
            for (const auto& skill : x.skills()) result ^= (skill_hash(skill.key()) ^ size_hash(skill.value()));
            for (ropufu::settlers_online::battle_trait trait : x.traits()) result ^= trait_hash(trait);

            return
                camp_hash(x.camp()) ^
                result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_HPP_INCLUDED
