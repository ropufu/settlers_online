
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED

#include "battle_phase.hpp"
#include "battle_trait.hpp"
#include "enum_array.hpp"
#include "special_ability.hpp"
#include "unit_category.hpp"
#include "unit_faction.hpp"

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
            flags_t<battle_trait> m_traits = { };        // Special traits that affect the entire battle.
            flags_t<special_ability> m_abilities = { };  // Special abilities.
            
            unit_faction m_faction = unit_faction::non_player_adventure; // Unit faction.
            unit_category m_category = unit_category::unknown; // Unit category.
            std::size_t m_experience = 0;  // Experience gained by murderer when the unit is killed.
            std::size_t m_capacity = 0;    // The number of units this one can lead (typically used for "generals").

            std::size_t m_hit_points = 0;  // Health.
            std::size_t m_low_damage = 0;  // Low damage.
            std::size_t m_high_damage = 0; // High damage.
            double m_accuracy = 0;         // Probability of high damage.
            double m_splash_chance = 0;    // Probability of dealing splash damage.

        public:
            /** Default constructor intended for initializing arrays etc. */
            unit_type() noexcept { }

            /** @brief Detailed constructor.
             *  @exception std::logic_error \p low_damage exceeds \p high_damage.
             *  @exception std::out_of_range \p accuracy is not in the interval [0, 1].
             *  @exception std::out_of_range \p splash_chance is not in the interval [0, 1].
             */
            unit_type(std::size_t id, const std::string& name, battle_phase initiative,
                unit_faction faction, unit_category category, std::size_t experience, std::size_t capacity,
                std::size_t hit_points, std::size_t low_damage, std::size_t high_damage, double accuracy, double splash_chance)
                : m_id(id), m_names({ name }), m_attack_phases({ initiative }),
                m_faction(faction), m_category(category), m_experience(experience), m_capacity(capacity),
                m_hit_points(hit_points)
            {
                this->set_damage(low_damage, high_damage, accuracy, splash_chance);
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
            void set_phase(battle_phase phase, bool value) noexcept { this->m_attack_phases[phase] = value; }

            /** Special traits that affect the entire battle. */
            const flags_t<battle_trait>& traits() const noexcept { return this->m_traits; }
            /** Special traits that affect the entire battle. */
            void set_trait(battle_trait trait, bool value) noexcept { this->m_traits[trait] = value; }

            /** Special ability that affects this unit's performance in battle. */
            const flags_t<special_ability>& abilities() const noexcept { return this->m_abilities; }
            /** Special ability that affects this unit's performance in battle. */
            void set_ability(special_ability ability, bool value) noexcept { this->m_abilities[ability] = value; }
            
            /** Checks whether this unit type has the specified battle trait. */
            bool has(battle_trait trait) const noexcept { return this->m_traits.has(trait); }
            /** Checks whether this unit type has the specified special ability. */
            bool has(special_ability ability) const noexcept { return this->m_abilities.has(ability); }

            /** Unit faction (enemy / player-controlled). */
            unit_faction faction() const noexcept { return this->m_faction; }
            /** Unit faction (enemy / player-controlled). */
            void set_faction(unit_faction value) noexcept { this->m_faction = value; }

            /** Category (classification) of the unit. */
            unit_category category() const noexcept { return this->m_category; }
            /** Category (classification) of the unit. */
            void set_category(unit_category value) noexcept { this->m_category = value; }

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

            /** Low damage. */
            std::size_t low_damage() const noexcept { return this->m_low_damage; }
            /** High damage. */
            std::size_t high_damage() const noexcept { return this->m_high_damage; }
            /** Probability of dealing high damage rather than low damage. */
            double accuracy() const noexcept { return this->m_accuracy; }
            /** Probability of dealing splash damage. */
            double splash_chance() const noexcept { return this->m_splash_chance; }

            /** @brief Offensive capabilities of the unit.
             *  @param low_damage Low damage.
             *  @param high_damage High damage.
             *  @exception std::logic_error \p low_damage exceeds \p high_damage.
             */
            void set_damage(std::size_t low_damage, std::size_t high_damage)
            {
                if (high_damage < low_damage) throw std::logic_error("<low_damage> cannot exceed <high_damage>.");

                this->m_low_damage = low_damage;
                this->m_high_damage = high_damage;
            }

            /** @brief Offensive capabilities of the unit.
             *  @param low_damage Low damage.
             *  @param high_damage High damage.
             *  @exception std::logic_error \p low_damage exceeds \p high_damage.
             *  @exception std::out_of_range \p accuracy is not in the interval [0, 1].
             */
            void set_damage(std::size_t low_damage, std::size_t high_damage, double accuracy)
            {
                if (high_damage < low_damage) throw std::logic_error("<low_damage> cannot exceed <high_damage>.");
                if (accuracy < 0.0 || accuracy > 1.0) throw std::out_of_range("<accuracy> must be in the range from 0 to 1.");

                this->m_low_damage = low_damage;
                this->m_high_damage = high_damage;
                this->m_accuracy = accuracy;
            }

            /** @brief Offensive capabilities of the unit.
             *  @param low_damage Low damage.
             *  @param high_damage High damage.
             *  @exception std::logic_error \p low_damage exceeds \p high_damage.
             *  @exception std::out_of_range \p accuracy is not in the interval [0, 1].
             *  @exception std::out_of_range \p splash_chance is not in the interval [0, 1].
             */
            void set_damage(std::size_t low_damage, std::size_t high_damage, double accuracy, double splash_chance)
            {
                if (high_damage < low_damage) throw std::logic_error("<low_damage> cannot exceed <high_damage>.");
                if (accuracy < 0.0 || accuracy > 1.0) throw std::out_of_range("<accuracy> must be in the range from 0 to 1.");
                if (splash_chance < 0.0 || splash_chance > 1.0) throw std::out_of_range("<splash_chance> must be in the range from 0 to 1.");

                this->m_low_damage = low_damage;
                this->m_high_damage = high_damage;
                this->m_accuracy = accuracy;
                this->m_splash_chance = splash_chance;
            }

            /** Determines whether the two objects are in order by id. */
            static bool compare_by_id(const type& x, const type& y) noexcept
            {
                return x.m_id < y.m_id;
            }

            /** Determines whether the two objects are in order (by weakness, hit points, id). */
            static bool compare_by_hit_points(const type& x, const type& y) noexcept
            {
                // Check whether the objects share the same \c is_not_weak property.
                if (x.m_abilities.has(special_ability::not_weak) ^ y.m_abilities.has(special_ability::not_weak))
                {
                    if (x.m_hit_points == y.m_hit_points) return type::compare_by_id(x, y); // If hit points are the same, fall back to \c id comparison.
                    return x.m_hit_points < y.m_hit_points;
                }
                return y.m_abilities.has(special_ability::not_weak);
            }

            /** Checks two types for equality; ignores names. */
            bool operator ==(const type& other) const noexcept
            {
                return
                    this->m_id == other.m_id &&
                    //this->m_names == other.m_names &&
                    this->m_attack_phases == other.m_attack_phases &&
                    this->m_traits == other.m_traits &&
                    this->m_abilities == other.m_abilities &&
                    this->m_faction == other.m_faction &&
                    this->m_category == other.m_category &&
                    this->m_experience == other.m_experience &&
                    this->m_capacity == other.m_capacity &&
                    this->m_hit_points == other.m_hit_points &&
                    this->m_low_damage == other.m_low_damage &&
                    this->m_high_damage == other.m_high_damage &&
                    this->m_accuracy == other.m_accuracy &&
                    this->m_splash_chance == other.m_splash_chance;
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

            for (battle_phase phase : x.attack_phases()) initiative.push_back(std::to_string(phase));
            for (battle_trait trait : x.traits()) traits.push_back(std::to_string(trait));
            for (special_ability ability : x.abilities()) abilities.push_back(std::to_string(ability));

            j = nlohmann::json{
                {"id", x.id()},
                {"names", x.names()},
                {"faction", std::to_string(x.faction())},
                {"class", std::to_string(x.category())},
                {"capacity", x.capacity()},
                {"hit points", x.hit_points()},
                {"low damage", x.low_damage()},
                {"high damage", x.high_damage()},
                {"accuracy", x.accuracy()},
                {"splash chance", x.splash_chance()},
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
                if (try_parse(value, phase)) x.set_phase(phase, true);
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
            x.set_damage(low_damage, high_damage, accuracy, splash_chance);

            // ~~ More ~~
            x.set_id(j.at("id").get<std::size_t>());
            if (j.count("capacity") != 0) x.set_capacity(j["capacity"].get<std::size_t>());
            x.set_hit_points(j.at("hit points").get<std::size_t>());
        }
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
            std::hash<double> double_hash = { };
            std::hash<ropufu::settlers_online::battle_phase> phase_hash = { };
            std::hash<ropufu::settlers_online::battle_trait> trait_hash = { };
            std::hash<ropufu::settlers_online::special_ability> ability_hash = { };
            std::hash<ropufu::settlers_online::unit_faction> fac_hash = { };
            std::hash<ropufu::settlers_online::unit_category> cat_hash = { };

            result_type result = 0;
            for (ropufu::settlers_online::battle_phase phase : x.attack_phases()) result ^= phase_hash(phase);
            for (ropufu::settlers_online::battle_trait trait : x.traits()) result ^= trait_hash(trait);
            for (ropufu::settlers_online::special_ability ability : x.abilities()) result ^= ability_hash(ability);

            return
                size_hash(x.id()) ^
                result ^
                fac_hash(x.faction()) ^
                cat_hash(x.category()) ^
                size_hash(x.experience()) ^
                size_hash(x.capacity()) ^
                size_hash(x.hit_points()) ^
                size_hash(x.low_damage()) ^
                size_hash(x.high_damage()) ^
                double_hash(x.accuracy()) ^
                double_hash(x.splash_chance());
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
