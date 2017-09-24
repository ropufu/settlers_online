
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED

#include <settlers_online/flags_type.hpp>
#include <settlers_online/special_abilities.hpp>
#include <settlers_online/unit_category.hpp>

#include <cstdint>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <string>

namespace ropufu
{
    namespace settlers_online
    {
        /** Descriptor for unit types. */
        struct unit_type
        {
        private:
            // Default attack ordering:
            // (smallest id -- ... -- highest id)
            std::int_fast32_t m_id = 0;         // Used for attack order.
			std::int_fast32_t m_initiative = 0; // Determines phases (sub-rounds) within each round.

            std::string m_name;            // Name (TODO: think list of names, separated by, say, comma?)
            std::size_t m_hit_points = {}; // Health.
			std::size_t m_experience = {}; // Experience gained by murderer when the unit is killed.
            std::size_t m_capacity = {};   // The number of units this one can lead (typically used for "generals").

            std::size_t m_min_damage = {}; // Low damage.
            std::size_t m_max_damage = {}; // High damage.
            double m_accuracy = 0;         // Probability of high damage.
			double m_splash_chance = 0;    // Probability of dealing splash damage.

            // Alternative attack ordering:
            // (smallest hit points -- ... -- highest hit points) (not weak, smallest id -- ... -- not weak, highest id)
            bool m_do_attack_weakest_target = false; // If true, when attacking enemies this unit will sort them by \c hit_points instead of \c id.
            bool m_is_not_weak = false;              // If true, this unit will be not be affected by attacker's \c do_attack_weakest_target, if any.

			unit_category m_category = unit_category::none;
			flags_type<special_abilities> m_special_abilities = special_abilities::none;

        public:
            /** Default constructor intended for initializing arrays etc. */
            unit_type() noexcept { }

            /** Detailed constructor. */
            unit_type(std::int_fast32_t id, std::int_fast32_t initiative,
				std::string name, std::size_t hit_points, std::size_t experience, std::size_t capacity,
				std::size_t min_damage, std::size_t max_damage, double accuracy, double splash_chance,
				unit_category category, flags_type<special_abilities> abilities) noexcept
                : m_id(id), m_initiative(initiative), 
				m_name(name), m_hit_points(hit_points), m_experience(experience), m_capacity(capacity),
				m_category(category), m_special_abilities(abilities)
            {
				this->set_damage(min_damage, max_damage, accuracy, splash_chance);
            }

            /** Number to determine attack order. */
            std::int_fast32_t id() const noexcept { return this->m_id; }
            /** Number to determine attack order. */
            void set_id(std::int_fast32_t value) noexcept { this->m_id = value; }

            /** Number to determine which phase (sub-round) the unit attacks. */
			std::int_fast32_t initiative() const noexcept { return this->m_initiative; }
            /** Number to determine which phase (sub-round) the unit attacks. */
			void set_initiative(std::int_fast32_t value) noexcept { this->m_initiative = value; }

            /** Name of the unit type. */
            const std::string& name() const noexcept { return this->m_name; }
            /** Name of the unit type. */
			void set_name(const std::string& value) noexcept { this->m_name = value; }

            /** Number of hit points (health) of unit type. */
            std::size_t hit_points() const noexcept { return this->m_hit_points; }
            /** Number of hit points (health) of unit type. */
			void set_hit_points(std::size_t value) noexcept { this->m_hit_points = value; }

            /** Experience gained by attacker when this unit is defeated. */
			std::size_t experience() const noexcept { return this->m_experience; }
            /** Experience gained by attacker when this unit is defeated. */
			void set_experience(std::size_t value) noexcept { this->m_experience = value; }

            /** Number determining how many other units this one can lead (command). */
            std::size_t capacity() const noexcept { return this->m_capacity; }
            /** Number determining how many other units this one can lead (command). */
            void set_capacity(std::size_t value) noexcept { this->m_capacity = value; }

			/** Low damage. */
            std::size_t min_damage() const noexcept { return this->m_min_damage; }
            /** High damage. */
            std::size_t max_damage() const noexcept { return this->m_max_damage; }
            /** Probability of dealing high damage rather than low damage. */
            double accuracy() const noexcept { return this->m_accuracy; }
            /** Probability of dealing splash damage. */
			double splash_chance() const noexcept { return this->m_splash_chance; }

            /** If true, this unit will sort the enemies by \c hit_points instead of \c id. */
            bool do_attack_weakest_target() const noexcept { return this->m_do_attack_weakest_target; }
            /** If true, this unit will sort the enemies by \c hit_points instead of \c id. */
            void set_do_attack_weakest_target(bool value) noexcept { this->m_do_attack_weakest_target = value; }

            /** If true, this unit will be not be affected by attacker's \c do_attack_weakest_target. */
            bool is_not_weak() const noexcept { return this->m_is_not_weak; }
            /** If true, this unit will be not be affected by attacker's \c do_attack_weakest_target. */
            void set_is_not_weak(bool value) noexcept { this->m_is_not_weak = value; }

            /** @brief Offensive capabilities of the unit.
             *  @param min_damage Low damage.
             *  @param max_damage High damage.
             *  @exception std::logic_error \p min_damage exceeds \p max_damage.
             */
			void set_damage(std::size_t min_damage, std::size_t max_damage)
			{
				if (max_damage < min_damage) throw std::logic_error("<min_damage> cannot exceed <max_damage>");

				this->m_min_damage = min_damage;
				this->m_max_damage = max_damage;
			}

            /** @brief Offensive capabilities of the unit.
             *  @param min_damage Low damage.
             *  @param max_damage High damage.
             *  @exception std::logic_error \p min_damage exceeds \p max_damage.
             *  @exception std::out_of_range \p accuracy is not in the interval [0, 1].
             */
			void set_damage(std::size_t min_damage, std::size_t max_damage, double accuracy)
			{
				if (max_damage < min_damage) throw std::logic_error("<min_damage> cannot exceed <max_damage>");
				if (accuracy < 0.0 || accuracy > 1.0) throw std::out_of_range("<accuracy> must be in the range from 0 to 1");

				this->m_min_damage = min_damage;
				this->m_max_damage = max_damage;
				this->m_accuracy = accuracy;
			}

            /** @brief Offensive capabilities of the unit.
            *  @param min_damage Low damage.
            *  @param max_damage High damage.
            *  @exception std::logic_error \p min_damage exceeds \p max_damage.
            *  @exception std::out_of_range \p accuracy is not in the interval [0, 1].
            *  @exception std::out_of_range \p splash_chance is not in the interval [0, 1].
            */
			void set_damage(std::size_t min_damage, std::size_t max_damage, double accuracy, double splash_chance)
			{
				if (max_damage < min_damage) throw std::logic_error("<min_damage> cannot exceed <max_damage>");
				if (accuracy < 0.0 || accuracy > 1.0) throw std::out_of_range("<accuracy> must be in the range from 0 to 1");
				if (splash_chance < 0.0 || splash_chance > 1.0) throw std::out_of_range("<splash_chance> must be in the range from 0 to 1");

				this->m_min_damage = min_damage;
				this->m_max_damage = max_damage;
				this->m_accuracy = accuracy;
				this->m_splash_chance = splash_chance;
			}

            /** Category (classification) of the unit. */
			unit_category category() const noexcept { return this->m_category; }
            /** Category (classification) of the unit. */
			void set_category(unit_category value) noexcept { this->m_category = value; }

            /** Special abilities of the unit. */
			const flags_type<special_abilities>& abilities() const noexcept { return this->m_special_abilities; }
            /** Special abilities of the unit. */
			void set_abilities(flags_type<special_abilities> value) noexcept { this->m_special_abilities = value; }

            /** Checks whether this unit type has the specified ability. */
			bool has(special_abilities ability) const noexcept
			{
				return this->m_special_abilities.has(ability);
			}

            /** Determines whether the two objects are in order by id. */
            static bool compare_by_id(const unit_type& x, const unit_type& y) noexcept
            {
                return x.m_id < y.m_id;
            }

            /** Determines whether the two objects are in order by initiative, then id. */
            static bool compare_by_initiative(const unit_type& x, const unit_type& y) noexcept
            {
                if (x.m_initiative == y.m_initiative) return unit_type::compare_by_id(x, y); // If initiative is the same, fall back to \c id comparison.
                return x.m_initiative < y.m_initiative;
            }

            /** Determines whether the two objects are in order (by weakness, hit points, id). */
            static bool compare_by_hit_points(const unit_type& x, const unit_type& y) noexcept
            {
                // Check whether the objects share the same \c is_not_weak property.
                if (x.m_is_not_weak ^ y.m_is_not_weak)
                {
                    if (x.m_hit_points == y.m_hit_points) return unit_type::compare_by_id(x, y); // If hit points are the same, fall back to \c id comparison.
                    return x.m_hit_points < y.m_hit_points;
                }
                return y.m_is_not_weak;
            }

            /** Checks two types for equality. */
            bool operator ==(const unit_type& other) const noexcept
            {
				return
					this->m_id == other.m_id &&
					this->m_initiative == other.m_initiative &&
                    //this->m_name == other.m_name &&
					this->m_hit_points == other.m_hit_points &&
					this->m_experience == other.m_experience &&
                    this->m_capacity == other.m_capacity &&
					this->m_min_damage == other.m_min_damage &&
					this->m_max_damage == other.m_max_damage &&
					this->m_accuracy == other.m_accuracy &&
					this->m_splash_chance == other.m_splash_chance &&
                    this->m_do_attack_weakest_target == other.m_do_attack_weakest_target &&
                    this->m_is_not_weak == other.m_is_not_weak &&
					this->m_category == other.m_category &&
					this->m_special_abilities == other.m_special_abilities;
            }

            /** Checks two types for inequality. */
            bool operator !=(const unit_type& other) const noexcept
            {
                return !(this->operator ==(other));
            }

            friend std::ostream& operator <<(std::ostream& os, const unit_type& that)
            {
				os <<
					"id = " << that.m_id <<
					" initiative = " << that.m_initiative <<
					" hp = " << that.m_hit_points <<
					" experience = " << that.m_experience <<
					" min damage = " << that.m_min_damage <<
					" max damage = " << that.m_max_damage <<
					" accuracy = " << that.m_accuracy <<
					" splash = " << that.m_splash_chance <<
					" category = " << std::to_string(that.m_category) <<
					" special = " << that.m_special_abilities.value();
                return os;
            }
        };
    }
}

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::unit_type>
    {
        typedef ropufu::settlers_online::unit_type argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& x) const
        {
            std::hash<std::int32_t> int32_hash = {};
			std::hash<std::int_fast32_t> int_fast32_hash = {};
            std::hash<std::size_t> size_hash = {};
			std::hash<double> double_hash = {};
            std::hash<bool> bool_hash = {};

			return
				int_fast32_hash(x.id()) ^
				int_fast32_hash(x.initiative()) ^
				size_hash(x.hit_points()) ^
				size_hash(x.experience()) ^
                size_hash(x.capacity()) ^
				size_hash(x.min_damage()) ^
				size_hash(x.max_damage()) ^
				double_hash(x.accuracy()) ^
				double_hash(x.splash_chance()) ^
                bool_hash(x.do_attack_weakest_target()) ^
                bool_hash(x.is_not_weak()) ^
				int32_hash(static_cast<std::int32_t>(x.category())) ^
				int32_hash(static_cast<std::int32_t>(x.abilities().value()));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
