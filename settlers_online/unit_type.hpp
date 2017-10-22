
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED

#include "battle_phase.hpp"
#include "battle_trait.hpp"
#include "special_ability.hpp"
#include "unit_category.hpp"
#include "unit_faction.hpp"

#include "enum_array.hpp"
#include "damage.hpp"

#include <nlohmann/json.hpp>

#include <cstddef> // std::size_t
#include <functional> // std::hash
#include <ostream> // std::ostream
#include <stdexcept> // std::logic_error, out_of_range
#include <string> // std::string, std::to_string
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        /** Descriptor for unit types. */
        struct unit_type
        {
            using type = unit_type;

        private:
            // When a unit attacks an enemy army, there are two types of attack order.
            // Default is (smallest id -- ... -- highest id).
            // Alternative is (smallest hit points -- ... -- highest hit points) (not weak, smallest id -- ... -- not weak, highest id).
            std::size_t m_id = 0; // Used for attack order.
            std::vector<std::string> m_names = { };      // Names.
            flags_t<battle_phase> m_attack_phases = { }; // Determines phases (sub-rounds) when the unit attacks within each round.
            flags_t<special_ability> m_abilities = { };  // Special abilities.
            flags_t<battle_trait> m_traits = { };        // Special traits that affect the entire battle.
            
            unit_faction m_faction = unit_faction::non_player_adventure; // Unit faction.
            unit_category m_category = unit_category::unknown; // Unit category.
            std::size_t m_experience = 0;  // Experience gained by murderer when the unit is killed.
            std::size_t m_capacity = 0;    // The number of units this one can lead (typically used for "generals").

            std::size_t m_hit_points = 0;  // Health.
            detail::damage m_damage = { }; // Damage.

        public:
            /** Default constructor intended for initializing arrays etc. */
            unit_type() noexcept { }

            /** Detailed constructor. */
            unit_type(std::size_t id, const std::string& name, battle_phase initiative,
                unit_faction faction, unit_category category, std::size_t experience, std::size_t capacity,
                std::size_t hit_points, detail::damage damage)
                : m_id(id), m_names({ name }), m_attack_phases({ initiative }),
                m_faction(faction), m_category(category), m_experience(experience), m_capacity(capacity),
                m_hit_points(hit_points), m_damage(damage)
            {
            }

            /** Number to determine attack order. */
            std::size_t id() const noexcept { return this->m_id; }
            /** Number to determine attack order. */
            void set_id(std::size_t value) noexcept { this->m_id = value; }

            /** Name of the unit type. */
            const std::vector<std::string>& names() const noexcept { return this->m_names; }
            /** Name of the unit type. */
            std::vector<std::string>& names() noexcept { return this->m_names; }

            /** Number to determine which phase (sub-round) the unit attacks. */
            const flags_t<battle_phase>& attack_phases() const noexcept { return this->m_attack_phases; }
            /** Number to determine which phase (sub-round) the unit attacks. */
            void set_attack_phase(battle_phase phase, bool value) noexcept { this->m_attack_phases[phase] = value; }

            /** Special ability that affects this unit's performance in battle. */
            const flags_t<special_ability>& abilities() const noexcept { return this->m_abilities; }
            /** Special ability that affects this unit's performance in battle. */
            void set_ability(special_ability ability, bool value) noexcept { this->m_abilities[ability] = value; }

            /** Special traits that affect the entire battle. */
            const flags_t<battle_trait>& traits() const noexcept { return this->m_traits; }
            /** Special traits that affect the entire battle. */
            void set_trait(battle_trait trait, bool value) noexcept { this->m_traits[trait] = value; }
            
            /** Checks whether this unit type has the specified special ability. */
            bool has(special_ability ability) const noexcept { return this->m_abilities.has(ability); }
            /** Checks whether this unit type has the specified battle trait. */
            bool has(battle_trait trait) const noexcept { return this->m_traits.has(trait); }

            /** Unit faction (enemy / player-controlled). */
            unit_faction faction() const noexcept { return this->m_faction; }
            /** Unit faction (enemy / player-controlled). */
            void set_faction(unit_faction value) noexcept { this->m_faction = value; }

            /** Category (classification) of the unit. */
            unit_category category() const noexcept { return this->m_category; }
            /** Category (classification) of the unit. */
            void set_category(unit_category value) noexcept { this->m_category = value; }
            
            /** Checks whether this unit type has the specified special ability. */
            bool is(unit_faction faction) const noexcept { return this->m_faction == faction; }
            /** Checks whether this unit type has the specified battle trait. */
            bool is(unit_category category) const noexcept { return this->m_category == category; }

            /** Experience gained by attacker when this unit is defeated. */
            std::size_t experience() const noexcept { return this->m_experience; }
            /** Experience gained by attacker when this unit is defeated. */
            void set_experience(std::size_t value) noexcept { this->m_experience = value; }

            /** Number determining how many other units this one can lead (command). */
            std::size_t capacity() const noexcept { return this->m_capacity; }
            /** Number determining how many other units this one can lead (command). */
            void set_capacity(std::size_t value) noexcept { this->m_capacity = value; }

            /** Number of hit points (health) of unit type. */
            std::size_t hit_points() const noexcept { return this->m_hit_points; }
            /** Number of hit points (health) of unit type. */
            void set_hit_points(std::size_t value) noexcept { this->m_hit_points = value; }

            /** Offensive capabilities of the unit. */
            const detail::damage& damage() const noexcept { return this->m_damage; }
            /** Offensive capabilities of the unit. */
            void set_damage(const detail::damage& value) noexcept { this->m_damage = value; }

            /** Determines whether the two objects are in order by id. */
            static bool compare_by_id(const type& x, const type& y) noexcept
            {
                return x.m_id < y.m_id;
            }

            /** Determines whether the two objects are in order (by weakness, hit points, id). */
            static bool compare_by_hit_points(const type& x, const type& y) noexcept
            {
                bool is_x_not_weak = x.m_abilities.has(special_ability::not_weak);
                bool is_y_not_weak = y.m_abilities.has(special_ability::not_weak);
                // If one is weak, and another is not.
                if (is_x_not_weak ^ is_y_not_weak) return is_y_not_weak;
                // Otherwise do hit points comparison.
                if (x.m_hit_points == y.m_hit_points) return type::compare_by_id(x, y); // If hit points are the same, fall back to \c id comparison.
                return x.m_hit_points < y.m_hit_points;
            }

            /** Checks two types for equality; ignores names. */
            bool operator ==(const type& other) const noexcept
            {
                return
                    this->m_id == other.m_id &&
                    //this->m_names == other.m_names &&
                    this->m_attack_phases == other.m_attack_phases &&
                    this->m_abilities == other.m_abilities &&
                    this->m_traits == other.m_traits &&
                    this->m_faction == other.m_faction &&
                    this->m_category == other.m_category &&
                    this->m_experience == other.m_experience &&
                    this->m_capacity == other.m_capacity &&
                    this->m_hit_points == other.m_hit_points &&
                    this->m_damage == other.m_damage;
            }

            /** Checks two types for inequality. */
            bool operator !=(const type& other) const noexcept
            {
                return !(this->operator ==(other));
            }

            friend std::ostream& operator <<(std::ostream& os, const type& self)
            {
                nlohmann::json j = self;
                return os << j;
            }
        };

        void to_json(nlohmann::json& j, const unit_type& x)
        {
            std::vector<std::string> initiative;
            std::vector<std::string> abilities;
            std::vector<std::string> traits;
            std::vector<std::string> skills;

            for (battle_phase phase : x.attack_phases()) initiative.push_back(std::to_string(phase));
            for (special_ability ability : x.abilities()) abilities.push_back(std::to_string(ability));
            for (battle_trait trait : x.traits()) traits.push_back(std::to_string(trait));

            j = nlohmann::json{
                {"id", x.id()},
                {"names", x.names()},
                {"faction", std::to_string(x.faction())},
                {"class", std::to_string(x.category())},
                {"capacity", x.capacity()},
                {"hit points", x.hit_points()},
                {"low damage", x.damage().low()},
                {"high damage", x.damage().high()},
                {"accuracy", x.damage().accuracy()},
                {"splash chance", x.damage().splash_chance()},
                {"experience when killed", x.experience()}
            };

            if (initiative.size() == 1) j["initiative"] = initiative[0];
            else j["initiative"] = initiative;
            if (!abilities.empty()) j["special abilities"] = abilities;
            if (!traits.empty()) j["traits"] = traits;
        }
    
        void from_json(const nlohmann::json& j, unit_type& x)
        {
            // ~~ Names ~~
            std::vector<std::string> names = { };
            if (j.at("name").is_array()) names = j["name"].get<std::vector<std::string>>();
            else names = { j["name"].get<std::string>() };
            x.names() = names;

            // ~~ Faction~~
            unit_faction fac = unit_faction::non_player_adventure;
            if (j.count("faction") != 0) try_parse(j["faction"].get<std::string>(), fac);
            x.set_faction(fac);

            // ~~ Category~~
            unit_category cat = unit_category::unknown;
            if (j.count("class") != 0) try_parse(j["class"].get<std::string>(), cat);
            x.set_category(cat);

            // ~~ Experience ~~
            std::size_t experience = 0;
            if (j.count("experience when killed") != 0) experience = j["experience when killed"];
            else if (j.count("experience") != 0) experience = j["experience"];
            x.set_experience(experience);

            // ~~ Attack Phases ~~
            std::vector<std::string> phase_names = { };
            if (j.at("initiative").is_array()) phase_names = j["initiative"].get<std::vector<std::string>>();
            else phase_names = { j["initiative"].get<std::string>() };
            for (const std::string& value : phase_names)
            {
                battle_phase phase;
                if (try_parse(value, phase)) x.set_attack_phase(phase, true);
            }

            // ~~ Special Abilities ~~
            std::vector<std::string> ability_names = { };
            if (j.count("special abilities") != 0) ability_names = j["special abilities"].get<std::vector<std::string>>();
            for (const std::string& value : ability_names)
            {
                special_ability ability;
                if (try_parse(value, ability)) x.set_ability(ability, true);
            }

            // ~~ Traits ~~
            std::vector<std::string> trait_names = { };
            if (j.count("traits") != 0) trait_names = j["traits"].get<std::vector<std::string>>();
            for (const std::string& value : trait_names)
            {
                battle_trait trait;
                if (try_parse(value, trait)) x.set_trait(trait, true);
            }

            // ~~ Damage ~~
            std::size_t low_damage = j.at("low damage");
            std::size_t high_damage = j.at("high damage");
            double accuracy = j.at("accuracy");
            double splash_chance = 0;
            if (j.count("splash chance") != 0) splash_chance = j["splash chance"];
            x.set_damage(detail::damage(low_damage, high_damage, accuracy, splash_chance));

            // ~~ More ~~
            x.set_id(j.at("id").get<std::size_t>());
            if (j.count("capacity") != 0) x.set_capacity(j["capacity"].get<std::size_t>());
            x.set_hit_points(j.at("hit points").get<std::size_t>());
        } // from_json(...)
    }
}

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::unit_type>
    {
        using argument_type = ropufu::settlers_online::unit_type;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const
        {
            std::hash<std::size_t> size_hash = { };
            std::hash<ropufu::settlers_online::battle_phase> phase_hash = { };
            std::hash<ropufu::settlers_online::special_ability> ability_hash = { };
            std::hash<ropufu::settlers_online::battle_trait> trait_hash = { };
            std::hash<ropufu::settlers_online::unit_faction> fac_hash = { };
            std::hash<ropufu::settlers_online::unit_category> cat_hash = { };
            std::hash<ropufu::settlers_online::detail::damage> damage_hash = { };

            result_type result = 0;
            for (ropufu::settlers_online::battle_phase phase : x.attack_phases()) result ^= phase_hash(phase);
            for (ropufu::settlers_online::special_ability ability : x.abilities()) result ^= ability_hash(ability);
            for (ropufu::settlers_online::battle_trait trait : x.traits()) result ^= trait_hash(trait);

            return
                size_hash(x.id()) ^
                result ^
                fac_hash(x.faction()) ^
                cat_hash(x.category()) ^
                size_hash(x.experience()) ^
                size_hash(x.capacity()) ^
                size_hash(x.hit_points()) ^
                damage_hash(x.damage());
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
