
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED

#include "../algebra.hpp"

#include "unit_type.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int_fast32_t
#include <functional> // std::hash
#include <ostream>    // std::ostream

namespace ropufu::settlers_online
{
    /** Descriptor for groups of units. */
    struct unit_group
    {
        using type = unit_group;

    private:
        unit_type m_type = {};
        std::int_fast32_t m_metagroup_id = 0; // An index for (hyper-)grouping purposes.
        std::size_t m_count_at_snapshot = 0;  // Number of units in the group at the moment of latest snapshot.
        std::size_t m_current_hit_points = 0; // Current overall hit points of the group.
        // ~~ Caching ~~
        std::size_t m_unit_hit_points = 0;

    public:
        /** Default constructor intended for initializing arrays etc. */
        unit_group() noexcept { }

        /** Detailed constructor. */
        unit_group(const unit_type& type, std::size_t count, std::int_fast32_t metagroup_id = 0) noexcept
            : m_type(type), m_metagroup_id(metagroup_id),
            m_count_at_snapshot(count),
            m_current_hit_points(count * type.hit_points()),
            m_unit_hit_points(type.hit_points())
        {
        } // unit_group(...)

        /** Common unit type for the group. */
        const unit_type& unit() const noexcept { return this->m_type; }
        /** Common unit type for the group. */
        void set_unit(const unit_type& value) noexcept
        {
            std::size_t damage_taken = this->m_count_at_snapshot * this->m_type.hit_points() - this->m_current_hit_points;
            std::size_t updated_hit_points = this->m_count_at_snapshot * value.hit_points();
            if (damage_taken > updated_hit_points) updated_hit_points = 0;
            else updated_hit_points -= damage_taken;

            this->m_type = value;
            this->m_current_hit_points = updated_hit_points;
            this->m_unit_hit_points = value.hit_points();
        } // set_unit(...)

        /** Index of the sub-group the group is in. */
        std::int_fast32_t metagroup_id() const noexcept { return this->m_metagroup_id; }

        /** Number of units capable of attacking. */
        std::size_t count_as_attacker() const noexcept { return this->m_count_at_snapshot; }

        /** Number of units capable of defending. */
        std::size_t count_as_defender() const noexcept
        {
            const std::size_t h = this->m_unit_hit_points;
            return fraction_ceiling(this->m_current_hit_points, h);
        } // count_defender(...)

        bool alive_as_attacker() const noexcept { return this->m_count_at_snapshot > 0; }
        bool alive_as_defender() const noexcept { return this->m_current_hit_points > 0; }

        /** Discard the latest \c snapshot and record the current state of the group. */
        void snapshot() noexcept { this->m_count_at_snapshot = this->count_as_defender(); }

        /* Total number of hit points left in the group. */
        std::size_t total_hit_points_defender() const noexcept { return this->m_current_hit_points; }

        /* Number of hit points of the top unit in the group (zero if the group has been eliminated). */
        std::size_t top_hit_points_defender() const noexcept
        {
            const std::size_t h = this->m_unit_hit_points;
            std::size_t trivial_indicator = indicator_is_non_zero(this->m_current_hit_points); // 0 if the group has been killed, 1 otherwise.
            std::size_t fractional_indicator = indicator_is_fractional(this->m_current_hit_points, h); // 0 if all units have full hp, 1 otherwise.
            std::size_t damage_cap = (fractional_indicator) * (this->m_current_hit_points % h) + (1 - fractional_indicator) * h;
            damage_cap *= trivial_indicator;
            return damage_cap;
        } // top_hit_points_defender(...)

        /** Kill all units in the group. */
        void kill_all_defender() noexcept { this->m_current_hit_points = 0; }

        /** Kill the top unit in the group. */
        void kill_top_defender() noexcept { this->m_current_hit_points -= this->top_hit_points_defender(); }

        /** Resets the number of defending units in the group. */
        void reset_count_defender(std::size_t count) noexcept
        {
            const std::size_t h = this->m_unit_hit_points;
            this->m_current_hit_points = count * h;
        } // reset_count_defender(...)

        /** Heal the top unit in the group. */
        void heal_top_defender() noexcept
        {
            const std::size_t h = this->m_unit_hit_points;
            std::size_t count = fraction_ceiling(this->m_current_hit_points, h);
            this->m_current_hit_points = count * h;
        } // heal_top_defender(...)

        /** @brief Damages the top unit without splash. */
        void damage_no_splash(std::size_t damage) noexcept
        {
            std::size_t damage_cap = this->top_hit_points_defender();
            this->m_current_hit_points -= (damage > damage_cap ? damage_cap : damage);
        } // damage_no_splash(...)

        /** @brief Deals pure splash damage. */
        std::size_t damage_pure_splash(std::size_t damage) noexcept
        {
            if (damage > this->m_current_hit_points)
            {
                damage -= this->m_current_hit_points;
                this->m_current_hit_points = 0;
                return damage;
            } // if (...)
            this->m_current_hit_points -= damage;
            return 0;
        } // damage_pure_splash(...)

        static bool compare_by_id(const type& x, const type& y) noexcept { return unit_type::compare_by_id(x.unit(), y.unit()); }
        static bool compare_by_hit_points(const type& x, const type& y) noexcept { return unit_type::compare_by_hit_points(x.unit(), y.unit()); }
        
        /** @brief Checks two groups for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return 
                this->m_type == other.m_type &&
                this->m_metagroup_id == other.m_metagroup_id &&
                this->m_count_at_snapshot == other.m_count_at_snapshot &&
                this->m_current_hit_points == other.m_current_hit_points;
        } // operator ==(...)
        
        /** @brief Checks two groups for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self)
        {
            os << self.m_count_at_snapshot << " " << self.m_type.first_name();
            return os;
        } // operator <<(...)
    }; // struct unit_group
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::unit_group>
    {
        using argument_type = ropufu::settlers_online::unit_group;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::int_fast32_t> int_fast32_hash = {};
            std::hash<std::size_t> size_hash = {};

            return
                int_fast32_hash(x.metagroup_id()) ^
                std::hash<ropufu::settlers_online::unit_type>()(x.unit()) ^
                size_hash(x.count_as_attacker());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED
