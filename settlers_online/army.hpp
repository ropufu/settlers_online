
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_HPP_INCLUDED

#include <aftermath/algebra.hpp>

#include "attack_sequence.hpp"
#include "battle_trait.hpp"
#include "battle_skill.hpp"
#include "camp.hpp"
#include "combat_result.hpp"
#include "damage.hpp"
#include "enum_array.hpp"
#include "special_ability.hpp"
#include "typedef.hpp"
#include "unit_category.hpp"
#include "unit_faction.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <algorithm> // std::sort
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <random>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace ropufu
{
    namespace settlers_online
    {
        /** Descriptor for colletions of groups of units. */
        struct army
        {
            //friend struct combat_mechanics;

        private:
            std::vector<unit_group> m_groups; // Unit groups, sorted by \c id.
            std::vector<mask_type> m_metagroup_masks; // For each non-singleton metagroup store its mask (default-sorted).
            detail::camp m_camp; // Determines the defensive capabilities of the army.
            // ~~ Permutations of <m_groups> ~~
            aftermath::algebra::permutation m_order_original; // Original permutation.
            aftermath::algebra::permutation m_order_by_id;    // Identity permutation.
            aftermath::algebra::permutation m_order_by_hp;    // Permutation when defending agains units with \c do_attack_weakest_target.
            // ~~ Battle modifiers and traits ~~
            double m_frenzy_bonus = 0.0; // Increases (multiplicative) the attack damage of this army for every combat round past the first.
            enum_array<battle_skill, std::size_t> m_skills = { }; // Skills affecting various aspects of the battle.
            flags_t<battle_trait> m_traits = { }; // If at least one unit in the army has this trait, it kicks in.

        public:
            /** An empty army. */
            army();

            /** @brief Constructs an army from the collection of \c unit_groups.
             *  @exception std::logic_error There are two groups sharing the same \c id.
             *  @exception std::out_of_range The number of groups exceeds \c army_capacity constant.
             */
            army(std::initializer_list<unit_group> groups);

            /** @brief Constructs an army from the collection of \c unit_groups.
             *  @exception std::logic_error There are two groups sharing the same \c id.
             *  @exception std::out_of_range The number of groups exceeds \c army_capacity constant.
             */
            army(const std::vector<unit_group>& groups, std::size_t camp_hit_points = 0);

            /** Groups (ordered by \c id). */
            const std::vector<unit_group>& groups() const noexcept { return this->m_groups; }
            /** Groups (ordered by \c id). */
            std::vector<unit_group>& groups() noexcept { return this->m_groups; }

            bool empty() const noexcept { return this->m_groups.empty(); }

            bool has(unit_faction faction) const noexcept;
            bool has(unit_category category) const noexcept;

            /** Discard the latest \c snapshot and record the current state of the army. */
            void snapshot()
            {
                for (unit_group& g : this->m_groups) g.snapshot();
            }

            /** Metagroup masks (ordered). */
            const std::vector<mask_type>& metagroup_masks() const noexcept { return this->m_metagroup_masks; }

            /** Determines the defensive capabilities of the army. */
            detail::camp camp() const noexcept { return this->m_camp; }
            /** Determines the defensive capabilities of the army. */
            void set_camp(std::size_t value) noexcept { this->m_camp = value; }

            /** Increases (multiplicative) the attack damage of this army for every combat round past the first. */
            double frenzy_bonus() const noexcept { return this->m_frenzy_bonus; }
            /** @brief Increases (multiplicative) the attack damage of this army for every combat round past the first.
             *  @exception std::out_of_range \p value is not in the interval [0, 1].
             */
            void set_frenzy_bonus(double value)
            {
                if (value < 0.0) throw std::out_of_range("<value> must be non-negative.");
                this->m_frenzy_bonus = value;
            }

            /** Skills affecting various aspects of the battle. */
            const enum_array<battle_skill, std::size_t>& skills() const noexcept { return this->m_skills; }
            /** The level of the specified battle skill. */
            std::size_t level(battle_skill skill) const noexcept { return this->m_skills[skill]; }
            /** Skills affecting various aspects of the battle. */
            void set_level(battle_skill skill, std::size_t value) noexcept { this->m_skills[skill] = value; }

            /** Special traits that affect the entire battle. */
            const flags_t<battle_trait>& traits() const noexcept { return this->m_traits; }
            /** Special traits that affect the entire battle. */
            void set_trait(battle_trait trait, bool value) noexcept { this->m_traits[trait] = value; }
            /** Checks whether this army has the specified battle trait. */
            bool has(battle_trait trait) const noexcept { return this->m_traits.has(trait); }

            /** Unit types in each group (ordered by \c id). */
            std::vector<unit_type> types() const noexcept;

            /** Number of units in each group (ordered by \c id). */
            std::vector<std::size_t> counts_by_type() const noexcept;

            /** Total number of units in the army. */
            std::size_t count_units() const noexcept;

            /** Number of groups in the army. */
            std::size_t count_groups() const noexcept
            {
                return this->m_groups.size();
            }

            /** Access groups (ordered by \c id). */
            const unit_group& operator [](std::size_t index) const noexcept
            {
                return this->m_groups[index];
            }

            /** Original ordering of the units. */
            const aftermath::algebra::permutation& order_original() const noexcept { return this->m_order_original; }

            /** Ordering by unit id. */
            const aftermath::algebra::permutation& order_by_id() const noexcept { return this->m_order_by_id; }

            /** Ordering by unit hit points. */
            const aftermath::algebra::permutation& order_by_hp() const noexcept { return this->m_order_by_hp; }

            /** The mask, specific to this instance of \c army, indicating the surviving groups. */
            mask_type compute_alive_mask() const noexcept
            {
                return aftermath::algebra::elementwise::to_binary_mask(this->m_groups, [] (const unit_group& g) { return g.count() > 0; });
            }

            /** Returns only the unit groups masked by \p alive_mask. */
            std::vector<unit_group> by_mask(mask_type alive_mask) const noexcept
            {
                return aftermath::algebra::elementwise::from_binary_mask(this->m_groups, alive_mask);
            }

            /** Total number of metagroups (groups of unit groups). */
            std::size_t count_metagroups() const noexcept
            {
                return this->m_metagroup_masks.size();
            }

            /** Checks two armies for equality. */
            bool operator ==(const army& other) const noexcept
            {
                std::size_t count_groups = this->m_groups.size();
                if (count_groups != other.m_groups.size()) return false;
                // The groups are already properly sorted; no need to bother with ordering.
                for (std::size_t i = 0; i < count_groups; i++) if (this->m_groups[i] != other.m_groups[i]) return false;
                
                // Ignore metagroups.
                // std::size_t count_metagroups = this->m_metagroup_masks.size();
                // if (count_metagroups != other.m_metagroup_masks.size()) return false;
                // for (std::size_t i = 0; i < count_metagroups; i++) if (this->m_metagroup_masks[i] != other.m_metagroup_masks[i]) return false;
                
                return
                    this->m_camp == other.m_camp &&
                    this->m_frenzy_bonus == other.m_frenzy_bonus &&
                    this->m_skills == other.m_skills &&
                    this->m_traits == other.m_traits;
            }

            /** Checks two armies for inequality. */
            bool operator !=(const army& other) const noexcept
            {
                return !(this->operator ==(other));
            }

            friend std::ostream& operator <<(std::ostream& os, const army& self)
            {
                bool is_first = true;
                if (self.m_groups.empty()) os << "Empty";
                for (const unit_group& g : self.m_groups)
                {
                    if (is_first) is_first = false;
                    else os << " ";
                    os << g;
                }
                return os;
            }
         };

        army::army()
            : army(std::vector<unit_group>(0))
        {
        }

        army::army(std::initializer_list<unit_group> groups)
            : army(std::vector<unit_group>(groups))
        {
        }

        army::army(const std::vector<unit_group>& groups, std::size_t camp_hit_points)
            : m_groups(0), m_metagroup_masks(0), m_camp(camp_hit_points),
            m_order_by_id(groups.size()), m_order_by_hp(groups.size())
        {
            // ~~ Validation ~~
            if (groups.size() > army_capacity) throw std::out_of_range("<mask_type> not large enought to store this army.");

            // Check for duplicate id's.
            std::set<std::size_t> unit_ids = {};
            for (const unit_group& g : groups)
            {
                std::size_t id = g.type().id();
                bool has_inserted = unit_ids.insert(id).second;
                if (!has_inserted) throw std::logic_error("<groups> cannot have duplicate ids.");
            }

            // ~~ Construction ~~

            // Store groups ordered by id.
            aftermath::algebra::permutation order_by_id(groups.size());
            order_by_id.order_by(
                groups, [](const unit_group& x) { return x.type(); }, unit_type::compare_by_id);
            this->m_groups.reserve(groups.size());
            for (std::size_t i = 0; i < groups.size(); i++) this->m_groups.push_back(groups[order_by_id[i]]);
            this->m_groups.shrink_to_fit();

            // Store permutations.
            this->m_order_original = order_by_id.invert();
            this->m_order_by_hp.order_by(
                this->m_groups, [](const unit_group& x) { return x.type(); }, unit_type::compare_by_hit_points);

            // Count non-singleton metagroups, and build traits.
            std::unordered_map<std::int_fast32_t, std::size_t> metagroup_ids; // List of used metagroup ids and corresponding counts.
            std::size_t count_metagroups = 0; // Count metagroups that occur at least twice.
            for (const unit_group& g : this->m_groups)
            {
                this->m_traits |= g.type().traits(); // If a unit has a trait, the army has it.

                auto metagroup_it = metagroup_ids.find(g.metagroup_id()); // Find the current metagroup id in the list.
                if (metagroup_it == metagroup_ids.end()) metagroup_ids.emplace(g.metagroup_id(), 1); // Add to the list if it's not there.
                else
                {
                    std::size_t n = ++(metagroup_it->second); // If it's on the list already, increment the count.
                    if (n == 2) count_metagroups++; // If it is the second time the metagroup id has been observed, increment the non-singleton count.
                }
            }

            // Build metagroup masks.
            this->m_metagroup_masks.reserve(count_metagroups);
            // Remove singletons and trim.
            for (const auto& p : metagroup_ids)
            {
                if (p.second == 1) continue; // Skip singletons.
                mask_type metagroup_mask = aftermath::algebra::elementwise::to_binary_mask(
                    this->m_groups, [&](const unit_group& g) { return g.metagroup_id() == p.first; });
                this->m_metagroup_masks.push_back(metagroup_mask);
            }
            this->m_metagroup_masks.shrink_to_fit();
            // Sort masks.
            std::sort(this->m_metagroup_masks.begin(), this->m_metagroup_masks.end());
        }

        std::vector<unit_type> army::types() const noexcept
        {
            std::vector<unit_type> result = {};
            result.reserve(this->m_groups.size());
            for (const unit_group& g : this->m_groups) result.push_back(g.type());
            result.shrink_to_fit();
            return result;
        }

        std::vector<std::size_t> army::counts_by_type() const noexcept
        {
            std::vector<std::size_t> result = {};
            result.reserve(this->m_groups.size());
            for (const unit_group& g : this->m_groups) result.push_back(g.count());
            result.shrink_to_fit();
            return result;
        }

        std::size_t army::count_units() const noexcept
        {
            std::size_t value = 0;
            for (const unit_group& g : this->m_groups) value += g.count();
            return value;
        }

        bool army::has(unit_faction faction) const noexcept
        {
            for (const unit_group& g : this->m_groups) if (g.type().is(faction)) return true;
            return false;
        }

        bool army::has(unit_category category) const noexcept
        {
            for (const unit_group& g : this->m_groups) if (g.type().is(category)) return true;
            return false;
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::army>
    {
        typedef ropufu::settlers_online::army argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& x) const
        {
            std::hash<ropufu::settlers_online::unit_group> group_hash = {};
            std::hash<ropufu::settlers_online::mask_type> mask_hash = {};
            std::hash<ropufu::settlers_online::detail::camp> camp_hash = {};
            std::hash<ropufu::settlers_online::battle_skill> skill_hash = { };
            std::hash<ropufu::settlers_online::battle_trait> trait_hash = { };
            std::hash<std::size_t> size_hash = {};
            
            result_type result = 0;
            for (const ropufu::settlers_online::unit_group& g : x.groups()) result ^= group_hash(g);
            for (const ropufu::settlers_online::mask_type& m : x.metagroup_masks()) result ^= mask_hash(m);
            for (const auto& skill : x.skills()) result ^= (skill_hash(skill.first) ^ size_hash(skill.second));
            for (ropufu::settlers_online::battle_trait trait : x.traits()) result ^= trait_hash(trait);

            return
                camp_hash(x.camp()) ^
                result;
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_HPP_INCLUDED
