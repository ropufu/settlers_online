
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <nlohmann/json.hpp>

// ~~ Enumerations ~~
#include "battle_phase.hpp"
#include "battle_trait.hpp"
#include "special_ability.hpp"
#include "unit_category.hpp"
#include "unit_faction.hpp"
// ~~ Basic structures ~~
#include "damage.hpp"
// ~~ Misc ~~
#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <functional> // std::hash
#include <ostream> // std::ostream
#include <stdexcept> // std::domain_error
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
            std::vector<std::string> m_names = { "" };   // Names.
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
                std::size_t hit_points, detail::damage damage) noexcept
                : m_id(id), m_names({ name }), m_attack_phases({ initiative }),
                m_faction(faction), m_category(category), m_experience(experience), m_capacity(capacity),
                m_hit_points(hit_points), m_damage(damage)
            {
            } // unit_type(...)

            /** Number to determine attack order. */
            std::size_t id() const noexcept { return this->m_id; }
            /** Number to determine attack order. */
            void set_id(std::size_t value) noexcept { this->m_id = value; }

            /** Name of the unit type. */
            const std::vector<std::string>& names() const noexcept { return this->m_names; }
            /** Name of the unit type. */
            void set_names(const std::vector<std::string>& value) noexcept
            {
                this->m_names = value;
                if (this->m_names.empty()) this->m_names.emplace_back("");
            }

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
            } // compare_by_id(...)

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
            } // compare_by_hit_points(...)

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
            } // operator ==(...)

            /** Checks two types for inequality. */
            bool operator !=(const type& other) const noexcept
            {
                return !(this->operator ==(other));
            } // operator !=(...)

            friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
            {
                nlohmann::json j = self;
                return os << j;
            } // operator <<(...)
        }; // struct unit_type

        void to_json(nlohmann::json& j, const unit_type& x) noexcept
        {
            std::vector<std::string> initiative { };
            std::vector<std::string> abilities { };
            std::vector<std::string> traits { };

            for (battle_phase phase : x.attack_phases()) initiative.push_back(std::to_string(phase));
            for (special_ability ability : x.abilities()) abilities.push_back(std::to_string(ability));
            for (battle_trait trait : x.traits()) traits.push_back(std::to_string(trait));

            j = nlohmann::json{
                {"id", x.id()},
                {"name", x.names()},
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

            if (initiative.size() != 1) j["initiative"] = initiative;
            else
            {
                std::string single = initiative[0];
                j["initiative"] = single;
            }
            if (!abilities.empty()) j["special abilities"] = abilities;
            if (!traits.empty()) j["traits"] = traits;
        } // to_json(...)
    
        void from_json(const nlohmann::json& j, unit_type& x) noexcept
        {
            std::string missing_property { };
            // Required properties.
            if (j.count("id") == 0) missing_property = "Missing required property: id.";
            if (j.count("hit points") == 0) missing_property = "Missing required property: hit points.";
            if (j.count("low damage") == 0) missing_property = "Missing required property: low damage.";
            if (j.count("high damage") == 0) missing_property = "Missing required property: high damage.";
            if (j.count("accuracy") == 0) missing_property = "Missing required property: accuracy.";
            if (j.count("name") == 0) missing_property = "Missing required property: name.";
            if (j.count("initiative") == 0) missing_property = "Missing required property: initiative.";
            if (!missing_property.empty())
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::major,
                    missing_property, __FUNCTION__, __LINE__);
                x = { };
                return;
            }

            try
            {
                // ~~ Id: required ~~
                std::size_t id = j["id"];
                x.set_id(id);
                // ~~ Hit Points: required ~~
                std::size_t hit_points = j["hit points"];
                x.set_hit_points(hit_points);
                // ~~ Low Damage: required ~~
                std::size_t low_damage = j["low damage"];
                // ~~ High Damage: required ~~
                std::size_t high_damage = j["high damage"];
                // ~~ Accuracy: required ~~
                double accuracy = j["accuracy"];
                // ~~ Splash Chance: optional ~~
                double splash_chance = 0;
                if (j.count("splash chance") != 0) splash_chance = j["splash chance"];
                x.set_damage(detail::damage(low_damage, high_damage, accuracy, splash_chance));
                // ~~ Capacity: optional ~~
                std::size_t capacity = 0;
                if (j.count("capacity") != 0) capacity = j["capacity"];
                x.set_capacity(capacity);

                // ~~ Names: required ~~
                std::vector<std::string> names { };
                if (j["name"].is_array()) names = j["name"].get<std::vector<std::string>>();
                else names = { j["name"].get<std::string>() };
                x.set_names(names);

                // ~~ Faction: optional ~~
                unit_faction fac = unit_faction::non_player_adventure;
                if (j.count("faction") != 0) try_parse(j["faction"].get<std::string>(), fac);
                x.set_faction(fac);

                // ~~ Category: optional ~~
                unit_category cat = unit_category::unknown;
                if (j.count("class") != 0) try_parse(j["class"].get<std::string>(), cat);
                x.set_category(cat);

                // ~~ Experience: optional ~~
                std::size_t experience = 0;
                if (j.count("experience when killed") != 0) experience = j["experience when killed"];
                else if (j.count("experience") != 0) experience = j["experience"];
                x.set_experience(experience);

                // ~~ Attack Phases: required ~~
                std::vector<std::string> phase_names { };
                if (j["initiative"].is_array()) phase_names = j["initiative"].get<std::vector<std::string>>();
                else phase_names = { j["initiative"].get<std::string>() };
                for (const std::string& value : phase_names)
                {
                    battle_phase y;
                    if (try_parse(value, y)) x.set_attack_phase(y, true);
                    else
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::negligible,
                            "Skipping unrecognized battle phase.", __FUNCTION__, __LINE__);
                    }
                }

                // ~~ Special Abilities: optional ~~
                std::vector<std::string> ability_names { };
                if (j.count("special abilities") != 0) ability_names = j["special abilities"].get<std::vector<std::string>>();
                for (const std::string& value : ability_names)
                {
                    special_ability y;
                    if (try_parse(value, y)) x.set_ability(y, true);
                    else
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::negligible,
                            "Skipping unrecognized special ability.", __FUNCTION__, __LINE__);
                    }
                }

                // ~~ Traits: optional ~~
                std::vector<std::string> trait_names { };
                if (j.count("traits") != 0) trait_names = j["traits"].get<std::vector<std::string>>();
                for (const std::string& value : trait_names)
                {
                    battle_trait y;
                    if (try_parse(value, y)) x.set_trait(y, true);
                    else
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::negligible,
                            "Skipping unrecognized battle trait.", __FUNCTION__, __LINE__);
                    }
                }
            }
            catch (std::domain_error)
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::domain_error,
                    aftermath::severity_level::major,
                    "JSON unit representation malformed. Using default instead.", __FUNCTION__, __LINE__);
                x = { };
            }
        } // from_json(...)
    } // namespace settlers_online
} // namespace ropufu

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
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
