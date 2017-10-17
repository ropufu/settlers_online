
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED

#include <aftermath/algebra.hpp>

#include "typedef.hpp"
#include "unit_type.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace ropufu
{
    namespace settlers_online
    {
        /** Descriptor for groups of units. */
        struct unit_group
        {
            friend struct combat_mechanics;

        private:
            unit_type m_type = {};
            std::size_t m_count = 0;             // Count of living units in the group.
            std::size_t m_count_at_snapshot = 0; // Count of living units in the group at the moment of latest \c snapshot.
            std::size_t m_damage_taken = 0;             // Damage taken by the top unit.
            std::size_t m_damage_taken_at_snapshot = 0; // Damage taken by the top unit at the moment of latest \c snapshot.
            std::int_fast32_t m_metagroup_id = 0; // An index for (hyper-)grouping purposes.

        public:
            /** Default constructor intended for initializing arrays etc. */
            unit_group() noexcept { }

            /** Detailed constructor. */
            unit_group(unit_type type, std::size_t count, std::int_fast32_t metagroup_id = 0) noexcept
                : m_type(type), m_count(count), m_count_at_snapshot(count), m_metagroup_id(metagroup_id)
            {
            }

            /** Common unit type for the group. */
            const unit_type& type() const noexcept { return this->m_type; }
            /** Common unit type for the group. */
            void set_type(const unit_type& value) noexcept { this->m_type = value; }

            /** Indicates if the group is empty. */
            bool empty() const noexcept { return this->m_count == 0; }

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
            }

            /** Undo the changes since the latest \c snapshot. */
            void reset() noexcept
            {
                this->m_count = this->m_count_at_snapshot;
                this->m_damage_taken = this->m_damage_taken_at_snapshot;
            }

            /** Discard the latest \c snapshot and record the current state of the group. */
            void snapshot() noexcept
            {
                this->m_count_at_snapshot = this->m_count;
                this->m_damage_taken_at_snapshot = this->m_damage_taken;
            }

            /** The total amount of hit points in the group. */
            std::size_t total_hit_points() const noexcept
            {
                return (this->m_count * this->m_type.hit_points()) - this->m_damage_taken;
            }

            /** @brief Hit points of the top unit.
             *  @remark Returns 0 if the group is empty.
             */
            std::size_t top_hit_points() const noexcept
            {
                if (this->m_count == 0) return 0;
                return this->m_type.hit_points() - this->m_damage_taken;
            }

            /** Total amount of hit points in the group. */
            void set_total_hit_points(std::size_t value) noexcept
            {
                this->m_count = value / this->m_type.hit_points();
                this->m_damage_taken = value % this->m_type.hit_points();
            }

            /** @brief Indicates if the top unit is damaged.
             *  @remark Returns false if the group is empty.
             */
            bool is_top_damaged() const noexcept
            {
                return this->m_damage_taken != 0;
            }

            /** @brief Heals the top unit.
             *  @remark Does nothing if the group is empty.
             */
            void heal_top() noexcept
            {
                this->m_damage_taken = 0;
            }

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
            }

            /** @brief Kill the top unit in the group.
             *  @remark Does nothing if the group is empty.
             */
            void kill_top() noexcept
            {
                if (this->m_count == 0) return;

                this->m_damage_taken = 0;
                this->m_count--;
            }

            /** Kill all units in the group. */
            void kill_all() noexcept
            {
                this->m_damage_taken = 0;
                this->m_count = 0;
            }

            /** @brief Kills \p count units (top unit being the first to die).
             *  @remark Equivalent to \c kill_all if \p count exceeds the number of units.
             */
            void kill(std::size_t count) noexcept
            {
                if (count == 0) return;
                if (count > this->m_count) count = this->m_count;

                this->m_damage_taken = 0;
                this->m_count -= count;
            }
            
            /** @brief Checks two groups for equality.
             *  @remark Ignores \c snapshots.
             */
            bool operator ==(const unit_group& other) const noexcept
            {
                return 
                    this->m_type == other.m_type &&
                    this->m_count == other.m_count &&
                    this->m_damage_taken == other.m_damage_taken &&
                    this->m_metagroup_id == other.m_metagroup_id;
            }
            
            /** @brief Checks two groups for inequality.
             *  @remark Ignores \c snapshots.
             */
            bool operator !=(const unit_group& other) const noexcept
            {
                return !(this->operator ==(other));
            }

            friend std::ostream& operator <<(std::ostream& os, const unit_group& that)
            {
                os << that.m_count << " " << that.m_type.names().front();
                return os;
            }
        };
    }
}

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::unit_group>
    {
        typedef ropufu::settlers_online::unit_group argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& x) const
        {
            std::hash<std::int_fast32_t> int_fast32_hash = {};
            std::hash<std::size_t> size_hash = {};

            return
                std::hash<ropufu::settlers_online::unit_type>()(x.type()) ^
                size_hash(x.count()) ^
                size_hash(x.damage_taken()) ^
                int_fast32_hash(x.metagroup_id());
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_GROUP_HPP_INCLUDED
