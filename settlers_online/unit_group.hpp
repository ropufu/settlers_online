
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED

// ~~ Misc ~~
#include "typedef.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <functional> // std::hash
#include <ostream> // std::ostream

namespace ropufu
{
    namespace settlers_online
    {
        /** Descriptor for groups of units. */
        struct unit_group
        {
            using type = unit_group;

        private:
            unit_type m_type = { };
            std::size_t m_count = 0;             // Count of living units in the group.
            std::size_t m_count_at_snapshot = 0; // Count of living units in the group at the moment of latest \c snapshot.
            std::size_t m_damage_taken = 0;             // Damage taken by the top unit.
            std::size_t m_damage_taken_at_snapshot = 0; // Damage taken by the top unit at the moment of latest \c snapshot.
            std::int_fast32_t m_metagroup_id = 0; // An index for (hyper-)grouping purposes.

        public:
            /** Default constructor intended for initializing arrays etc. */
            unit_group() noexcept { }

            /** Detailed constructor. */
            unit_group(const unit_type& type, std::size_t count, std::int_fast32_t metagroup_id = 0) noexcept
                : m_type(type), m_count(count), m_count_at_snapshot(count), m_metagroup_id(metagroup_id)
            {
            } // unit_group(...)

            /** Common unit type for the group. */
            const unit_type& unit() const noexcept { return this->m_type; }
            /** Common unit type for the group. */
            void set_unit(const unit_type& value) noexcept { this->m_type = value; }

            /** Indicates if the group is empty. */
            bool empty() const noexcept { return this->m_count == 0; }

            /** Indicates if the group is empty at the moment of latest \c snapshot. */
            bool empty_at_snapshot() const noexcept { return this->m_count_at_snapshot == 0; }

            /** Number of units in the group. */
            std::size_t count() const noexcept { return this->m_count; }

            /** Number of units in the group at the moment of latest \c snapshot. */
            std::size_t count_at_snapshot() const noexcept { return this->m_count_at_snapshot; }

            /** Damage taken by the top unit. */
            std::size_t damage_taken() const noexcept { return this->m_damage_taken; }

            /** Damage taken by the top unit at the moment of latest \c snapshot. */
            std::size_t damage_taken_at_snapshot() const noexcept { return this->m_damage_taken_at_snapshot; }

            /** Index of the sub-group the group is in. */
            std::int_fast32_t metagroup_id() const noexcept { return this->m_metagroup_id; }

            /** Indicates whether the group has been changed since the latest \c snapshot. */
            bool is_dirty() const noexcept
            {
                return !((this->m_count == this->m_count_at_snapshot) && (this->m_damage_taken == this->m_damage_taken_at_snapshot));
            } // is_dirty(...)

            /** Undo the changes since the latest \c snapshot. */
            void reset() noexcept
            {
                this->m_count = this->m_count_at_snapshot;
                this->m_damage_taken = this->m_damage_taken_at_snapshot;
            } // reset(...)

            /** Discard the latest \c snapshot and record the current state of the group. */
            void snapshot() noexcept
            {
                this->m_count_at_snapshot = this->m_count;
                this->m_damage_taken_at_snapshot = this->m_damage_taken;
            } // snapshot(...)

            /** The total amount of hit points in the group. */
            std::size_t total_hit_points() const noexcept
            {
                /** @todo */
                // debug_assert(this->m_count > 0 || this->m_damage_taken == 0);
                // debug_assert(this->m_count * this->m_type.hit_points() >= this->m_damage_taken);
                return (this->m_count * this->m_type.hit_points()) - this->m_damage_taken;
            } // total_hit_points(...)

            /** @brief Hit points of the top unit.
             *  @remark Returns 0 if the group is empty.
             */
            std::size_t top_hit_points() const noexcept
            {
                if (this->m_count == 0) return 0;
                /** @todo */
                // debug_assert(this->m_type.hit_points() >= this->m_damage_taken);
                return this->m_type.hit_points() - this->m_damage_taken;
            } // top_hit_points(...)

            /** Total amount of hit points in the group. */
            void set_total_hit_points(std::size_t value) noexcept
            {
                this->m_count = fraction_ceiling(value, this->m_type.hit_points());
                this->m_damage_taken = (this->m_count * this->m_type.hit_points()) - value;
            } // set_total_hit_points(...)

            /** @brief Indicates if the top unit is damaged.
             *  @remark Returns false if the group is empty.
             */
            bool is_top_damaged() const noexcept
            {
                return this->m_damage_taken != 0;
            } // is_top_damaged(...)

            /** @brief Heals the top unit.
             *  @remark Does nothing if the group is empty.
             */
            void heal_top() noexcept
            {
                this->m_damage_taken = 0;
            } // heal_top(...)

            /** @brief Damages the top unit without splash.
             *  @remark Excess damage will be ignored.
             *  @remark Does nothing if the group is empty.
             */
            bool try_kill_top(std::size_t no_splash_damage) noexcept
            {
                if (this->m_count == 0) return false;

                this->m_damage_taken += no_splash_damage;
                if (this->m_damage_taken >= this->m_type.hit_points())
                {
                    this->m_damage_taken = 0;
                    this->m_count--;
                    return true;
                }
                return false;
            } // try_kill_top(...)

            /** @brief Kill the top unit in the group.
             *  @remark Does nothing if the group is empty.
             */
            void kill_top() noexcept
            {
                if (this->m_count == 0) return;

                this->m_damage_taken = 0;
                this->m_count--;
            } // kill_top(...)

            /** Kill all units in the group. */
            void kill_all() noexcept
            {
                this->m_damage_taken = 0;
                this->m_count = 0;
            } // kill_all(...)

            /** @brief Kills \p count units (top unit being the first to die).
             *  @remark Equivalent to \c kill_all if \p count exceeds the number of units.
             */
            void kill(std::size_t count) noexcept
            {
                if (count == 0) return;
                if (count > this->m_count) count = this->m_count;

                this->m_damage_taken = 0;
                this->m_count -= count;
            } // kill(...)
            
            /** @brief Checks two groups for equality.
             *  @remark Ignores \c snapshots.
             */
            bool operator ==(const type& other) const noexcept
            {
                return 
                    this->m_type == other.m_type &&
                    this->m_count == other.m_count &&
                    this->m_damage_taken == other.m_damage_taken &&
                    this->m_metagroup_id == other.m_metagroup_id;
            } // operator ==(...)
            
            /** @brief Checks two groups for inequality.
             *  @remark Ignores \c snapshots.
             */
            bool operator !=(const type& other) const noexcept
            {
                return !(this->operator ==(other));
            }

            friend std::ostream& operator <<(std::ostream& os, const type& that) noexcept
            {
                os << that.m_count << " " << that.m_type.names().front();
                return os;
            }
        }; // struct unit_group
    } // namespace settlers_online
} // namespace ropufu

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
                std::hash<ropufu::settlers_online::unit_type>()(x.unit()) ^
                size_hash(x.count()) ^
                size_hash(x.damage_taken()) ^
                int_fast32_hash(x.metagroup_id());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED
