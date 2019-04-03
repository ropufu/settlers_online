
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <ropufu/enum_array.hpp>

#include "../arithmetic.hpp"
#include "../enums.hpp"

#include "damage.hpp"

#include <array>      // std::array
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string, std::to_string
#include <system_error> // std::error_code, std::errc
#include <vector>     // std::vector

namespace ropufu::settlers_online
{
    /** Descriptor for unit types. */
    struct unit_type;

    void to_json(nlohmann::json& j, const unit_type& x) noexcept;
    void from_json(const nlohmann::json& j, unit_type& x);

    /** Descriptor for unit types. */
    struct unit_type
    {
        using type = unit_type;
        using damage_type = ropufu::settlers_online::damage;
        using damage_percentage_type = typename damage_bonus_type::percentage_type;
        using hit_points_percentage_type = typename hit_points_bonus_type::percentage_type;
        using experience_percentage_type = typename experience_bonus_type::percentage_type;

        // ~~ Json names ~~
        static constexpr char jstr_id[] = "id";
        static constexpr char jstr_names[] = "names";
        static constexpr char jstr_codenames[] = "codenames";
        static constexpr char jstr_faction[] = "faction";
        static constexpr char jstr_category[] = "category";
        static constexpr char jstr_attack_phases[] = "phases";
        static constexpr char jstr_capacity[] = "capacity";
        static constexpr char jstr_hit_points[] = "hit points";
        static constexpr char jstr_damage[] = "damage";
        static constexpr char jstr_experience[] = "experience when killed";
        static constexpr char jstr_abilities[] = "special abilities";
        static constexpr char jstr_traits[] = "traits";

    private:
        // When a unit attacks an enemy army, there are two types of attack order.
        // Default is (smallest id -- ... -- highest id).
        // Alternative is (smallest hit points -- ... -- highest hit points) (not weak, smallest id -- ... -- not weak, highest id).
        std::size_t m_id = 0; // Used for attack order.
        std::string m_display_name = "??";
        std::vector<std::string> m_names = {}; // Names.
        std::vector<std::string> m_codenames = {}; // Shorthand names.
        unit_faction m_faction = unit_faction::non_player_adventure; // Unit faction.
        unit_category m_category = unit_category::unknown;           // Unit category.

        std::size_t m_capacity = 0;   // The number of units this one can lead (typically used for "generals").
        std::size_t m_hit_points = 1; // Health.
        damage_type m_damage = {};    // Damage.
        std::size_t m_experience = 0; // Experience gained by murderer when the unit is killed.

        aftermath::flags_t<battle_phase> m_attack_phases = {}; // Determines phases (sub-rounds) when the unit attacks within each round.
        aftermath::flags_t<special_ability> m_abilities = {};  // Special abilities.
        aftermath::flags_t<battle_trait> m_traits = {};        // Special traits that affect the entire battle.

        hit_points_bonus_type m_hit_points_bonus = {};
        damage_bonus_type m_low_damage_bonus = {};
        damage_bonus_type m_high_damage_bonus = {};
        experience_bonus_type m_experience_bonus = {};

        bool validate(std::error_code& ec) const noexcept
        {
            if (this->m_hit_points == 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Hit points cannot be zero.", false);
            if (!this->m_damage.validate(ec)) return false;
            
            if (!this->m_hit_points_bonus.validate(ec)) return false;
            if (!this->m_low_damage_bonus.validate(ec)) return false;
            if (!this->m_high_damage_bonus.validate(ec)) return false;
            if (!this->m_experience_bonus.validate(ec)) return false;
            
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            if (this->m_hit_points == 0) this->m_hit_points = 1;
            this->m_damage.coerce();
            this->m_display_name = this->m_names.empty() ? "??" : this->m_names.front();

            this->m_hit_points_bonus.coerce();
            this->m_low_damage_bonus.coerce();
            this->m_high_damage_bonus.coerce();
            this->m_experience_bonus.coerce();

            // ~~ Optimize ~~
            this->m_names.shrink_to_fit();
            this->m_codenames.shrink_to_fit();
        } // coerce(...)

    public:
        /** Default constructor intended for initializing arrays etc. */
        unit_type() noexcept { }

        /** @brief Detailed constructor.
         *  @param ec Set to std::errc::invalid_argument if \p hit_points is zero.
         */
        unit_type(std::size_t id, battle_phase initiative, std::size_t hit_points, damage_type damage, std::error_code& ec) noexcept
            : m_id(id), m_hit_points(hit_points), m_damage(damage), m_attack_phases({ initiative })
        {
            this->validate(ec);
            this->coerce();
        } // unit_type(...)

        unit_type(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            // Auxiliary for enum struct.
            std::string faction_str = std::to_string(this->m_faction); 
            std::string category_str = std::to_string(this->m_category);

            // Parse json entries.
            aftermath::noexcept_json::required(j, type::jstr_id, this->m_id, ec);
            aftermath::noexcept_json::required(j, type::jstr_names, this->m_names, ec);
            aftermath::noexcept_json::required(j, type::jstr_hit_points, this->m_hit_points, ec);
            aftermath::noexcept_json::required(j, type::jstr_damage, this->m_damage, ec);
            aftermath::noexcept_json::required(j, type::jstr_attack_phases, this->m_attack_phases, ec);

            aftermath::noexcept_json::optional(j, type::jstr_codenames, this->m_codenames, ec);
            aftermath::noexcept_json::optional(j, type::jstr_faction, faction_str, ec);
            aftermath::noexcept_json::optional(j, type::jstr_category, category_str, ec);
            aftermath::noexcept_json::optional(j, type::jstr_capacity, this->m_capacity, ec);
            aftermath::noexcept_json::optional(j, type::jstr_experience, this->m_experience, ec);
            aftermath::noexcept_json::optional(j, type::jstr_abilities, this->m_abilities, ec);
            aftermath::noexcept_json::optional(j, type::jstr_traits, this->m_traits, ec);

            // Enum structures etc.
            this->validate(ec);
            this->coerce();
            if (!aftermath::detail::try_parse_enum(faction_str, this->m_faction))
                aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, std::string("Faction unrecognized: ") + faction_str + std::string("."));
            if (!aftermath::detail::try_parse_enum(category_str, this->m_category))
                aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, std::string("Category unrecognized: ") + category_str + std::string("."));
        } // unit_type(...)

        /** Checks two types for equality; ignores names. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_id == other.m_id &&
                //this->m_names == other.m_names &&
                //this->m_codenames == other.m_codenames &&
                this->m_faction == other.m_faction &&
                this->m_category == other.m_category &&
                this->m_capacity == other.m_capacity &&
                this->m_hit_points == other.m_hit_points &&
                this->m_damage == other.m_damage &&
                this->m_experience == other.m_experience &&
                this->m_attack_phases == other.m_attack_phases &&
                this->m_abilities == other.m_abilities &&
                this->m_traits == other.m_traits;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        /** Number to determine attack order. */
        std::size_t id() const noexcept { return this->m_id; }
        /** Number to determine attack order. */
        void set_id(std::size_t value) noexcept { this->m_id = value; }

        /** Names of the unit type. */
        const std::vector<std::string>& names() const noexcept { return this->m_names; }
        /** Display name of the unit type. */
        const std::string& first_name() const noexcept { return this->m_display_name; }
        /** Names of the unit type. */
        void set_names(const std::vector<std::string>& value) noexcept
        {
            this->m_names = value;
            this->coerce();
        } // set_names(...)
        /** Name of the unit type. */
        void set_name(const std::string& value) noexcept
        {
            this->m_names.clear();
            this->m_names.reserve(1);
            this->m_names.push_back(value);
            this->coerce();
        } // set_names(...)

        /** Number to determine attack order. */
        const std::vector<std::string>& codenames() const noexcept { return this->m_codenames; }
        /** Number to determine attack order. */
        void set_codenames(const std::vector<std::string>& value) noexcept { this->m_codenames = value; }

        /** Number to determine which phase (sub-round) the unit attacks. */
        const aftermath::flags_t<battle_phase>& attack_phases() const noexcept { return this->m_attack_phases; }
        /** Number to determine which phase (sub-round) the unit attacks. */
        void set_attack_phase(battle_phase phase, bool value) { this->m_attack_phases[phase] = value; }

        /** Special ability that affects this unit's performance in battle. */
        const aftermath::flags_t<special_ability>& abilities() const noexcept { return this->m_abilities; }
        /** Special ability that affects this unit's performance in battle. */
        void set_ability(special_ability ability, bool value) { this->m_abilities[ability] = value; }

        /** Special traits that affect the entire battle. */
        const aftermath::flags_t<battle_trait>& traits() const noexcept { return this->m_traits; }
        /** Special traits that affect the entire battle. */
        void set_trait(battle_trait trait, bool value) { this->m_traits[trait] = value; }
        
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

        // /** Experience gained by attacker when a unit group is defeated. */
        // std::size_t experience(std::size_t group_size) const noexcept { return group_size * this->m_experience; }
        /** Effective experience gained by attacker when a unit group  is defeated. */
        std::size_t experience(std::size_t group_size) const noexcept { return this->m_experience_bonus.apply_to(group_size * this->m_experience); }
        /** Experience gained by attacker when one unit is defeated. */
        void set_experience(std::size_t value) noexcept { this->m_experience = value; }

        /** Number determining how many other units this one can lead (command). */
        std::size_t capacity() const noexcept { return this->m_capacity; }
        /** Number determining how many other units this one can lead (command). */
        void set_capacity(std::size_t value) noexcept { this->m_capacity = value; }

        // /** Number of hit points (health) of unit type. */
        // std::size_t hit_points() const noexcept { return this->m_hit_points; }
        /** Effective number hit points (health) of unit type. */
        std::size_t hit_points() const noexcept { return this->m_hit_points_bonus.apply_to(this->m_hit_points); }
        /** Number of hit points (health) of unit type. */
        void set_hit_points(std::size_t value, std::error_code& ec) noexcept
        {
            this->m_hit_points = value;
            if (!this->validate(ec)) this->coerce();
        } // set_hit_points(...)

        /** @brief Basic offensive capabilities of the unit, disregarding possible bonus modifiers. */
        const damage_type& base_damage() const noexcept { return this->m_damage; }
        bool never_splash() const noexcept { return this->m_damage.splash_chance() == 0; }
        bool always_splash() const noexcept { return this->m_damage.splash_chance() == 1; }
        /** @brief Effective offensive capabilities of the unit. */
        damage_type damage(const damage_percentage_type& modifier) const noexcept
        {
            damage_bonus_type low_bonus = this->m_low_damage_bonus;
            damage_bonus_type high_bonus = this->m_high_damage_bonus;
            low_bonus.append_rate(modifier);
            high_bonus.append_rate(modifier);
            return { nullptr,
                low_bonus.apply_to(this->m_damage.low()),
                high_bonus.apply_to(this->m_damage.high()),
                this->m_damage.accuracy(),
                this->m_damage.splash_chance() };
        } // damage(...)
        /** Offensive capabilities of the unit. */
        void set_damage(const damage_type& value, std::error_code& ec) noexcept
        {
            this->m_damage = value;
            if (!this->validate(ec)) this->coerce();
        } // set_damage(...)

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
            if (x.hit_points() == y.hit_points()) return type::compare_by_id(x, y); // If hit points are the same, fall back to \c id comparison.
            return x.hit_points() < y.hit_points();
        } // compare_by_hit_points(...)

        void apply_weather(battle_weather weather) noexcept
        {
            std::error_code ec {};
            switch (weather)
            {
                case battle_weather::hard_frost:
                    if (this->is(unit_category::melee)) this->m_damage.set_splash_chance(1, ec);
                    break;
                case battle_weather::bright_sunshine:
                    this->m_hit_points_bonus.append_rate(sunshine_hit_points_percentage);
                    break;
                case battle_weather::heavy_fog:
                    this->set_ability(special_ability::attack_weakest_target, true);
                    break;
                case battle_weather::hurricane:
                    this->m_low_damage_bonus.append_rate(hurricane_damage_percentage);
                    this->m_high_damage_bonus.append_rate(hurricane_damage_percentage);
                    break;
                default: break;
            } // switch (...)
        } // apply_weather(...)

        void apply_friendly_trait(battle_trait trait) noexcept
        {
            std::error_code ec {};
            switch (trait)
            {
                case battle_trait::explosive_ammunition:
                    if (this->is(unit_category::ranged))
                    {
                        this->set_ability(special_ability::attack_weakest_target, true);
                        this->m_damage.set_splash_chance(1, ec);
                    } // if (...)
                    break;
                case battle_trait::bombastic:
                    if (this->is(unit_category::artillery))
                    {
                        this->m_low_damage_bonus.append_rate(bombastic_damage_percentage);
                        this->m_high_damage_bonus.append_rate(bombastic_damage_percentage);
                    } // if (...)
                    break;
                case battle_trait::astute_strategist:
                    if (this->is(unit_category::cavalry))
                    {
                        this->m_low_damage_bonus.append_rate(strategist_friendly_damage_percentage);
                        this->m_high_damage_bonus.append_rate(strategist_friendly_damage_percentage);
                    } // if (...)
                    break;
                default: break;
            } // switch (...)
        } // apply_friendly_trait(...)

        void apply_enemy_trait(battle_trait trait) noexcept
        {
            std::error_code ec {};
            switch (trait)
            {
                case battle_trait::intercept:
                    this->set_ability(special_ability::attack_weakest_target, false);
                    this->m_low_damage_bonus.append_rate(intercept_damage_percentage);
                    this->m_high_damage_bonus.append_rate(intercept_damage_percentage);
                    break;
                case battle_trait::dazzle:
                    this->m_damage.set_accuracy(0, ec);
                    break;
                case battle_trait::astute_strategist:
                    this->m_experience_bonus.append_rate(strategist_xp_percentage);
                    if (this->is(unit_category::cavalry))
                    {
                        this->m_low_damage_bonus.append_rate(strategist_enemy_damage_percentage);
                        this->m_high_damage_bonus.append_rate(strategist_enemy_damage_percentage);
                    } // if (...)
                    break;
                default: break;
            } // switch (...)
        } // apply_enemy_trait(...)

        /** @brief Apply the firendly army's skill to a unit.
         *  @param level The number of books invested in this skill.
         */
        void apply_friendly_skill(battle_skill skill, std::size_t level) noexcept
        {
            if (level == 0) return;
            if (level > 3) level = 3;

            double accuracy_bonus = 0;
            double splash_bonus = 0;

            // Level      0  1   2   3    4    ...
            // Thirds, %  0  33  66  100  133  ...
            constexpr std::size_t three = 3;
            const double thirds = fraction_floor(100 * level, three) / 100.0;

            // Misc.
            constexpr std::array<typename damage_bonus_type::integer_type, 4> sniper_bonus_table { 0, 45, 85, 130 };
            const damage_percentage_type sniper_low_bonus { sniper_bonus_table[level] };
            const damage_percentage_type sniper_high_bonus { static_cast<typename damage_bonus_type::integer_type>(5 * level) };

            switch (skill)
            {
                case battle_skill::juggernaut: // Increases the general's (faction: general) attack damage by 20/40/60. These attacks have a 33/66/100% chance of dealing splash damage.
                    if (!this->is(unit_faction::general)) return;
                    this->m_low_damage_bonus.append_additive(20 * level);
                    this->m_high_damage_bonus.append_additive(20 * level);
                    splash_bonus = thirds;
                    break;
                case battle_skill::garrison_annex: // Increases the unit capacity (faction: general) by 5/10/15.
                    if (!this->is(unit_faction::general)) return;
                    this->m_capacity += 5 * level;
                    break;
                case battle_skill::lightning_slash: // The general (faction: general) attacks twice per round. That second attack's initiative is \c last_strike.
                    if (!this->is(unit_faction::general)) return;
                    this->m_attack_phases[battle_phase::last_strike] = true;
                    break;
                case battle_skill::unstoppable_charge: // Increases the maximum attack damage of your swift units (faction: cavalry) by 1/2/3 and their attacks have a 33/66/100% chance of dealing splash damage.
                    if (!this->is(unit_category::cavalry)) return;
                    this->m_high_damage_bonus.append_additive(level);
                    splash_bonus = thirds;
                    break;
                case battle_skill::weekly_maintenance: // Increases the attack damage of your heavy units (faction: artillery) by 10/20/30.
                    if (!this->is(unit_category::artillery)) return;
                    this->m_low_damage_bonus.append_additive(10 * level);
                    this->m_high_damage_bonus.append_additive(10 * level);
                    break;
                case battle_skill::master_planner: // Adds 10% to this army's accuracy.
                    accuracy_bonus = 0.1;
                    break;
                case battle_skill::rapid_fire: // Increases the maximum attack damage of your Bowmen by 5/10/15.
                    if (!this->has(special_ability::archer)) return;
                    this->m_high_damage_bonus.append_additive(5 * level);
                    break;
                case battle_skill::sniper_training: // Increases your Longbowmen's and regular Marksmen's minimum attack damage by 45/85/130% and the maximum by 5/10/15%.
                    if (!this->has(special_ability::sniper)) return;
                    this->m_low_damage_bonus.append_rate(sniper_low_bonus);
                    this->m_high_damage_bonus.append_rate(sniper_high_bonus);
                    break;
                case battle_skill::cleave: // Increases the attack damage of Elite Soldiers by 4/8/12 and their attacks have a 33/66/100% chance of dealing splash damage.
                    if (!this->has(special_ability::butcher)) return;
                    this->m_low_damage_bonus.append_additive(4 * level);
                    this->m_high_damage_bonus.append_additive(4 * level);
                    splash_bonus = thirds;
                    break;
                default: break;
            } // switch(...)

            double new_accuracy = this->m_damage.accuracy() + accuracy_bonus;
            double new_splash = this->m_damage.splash_chance() + splash_bonus;
            
            // Apply new damage.
            std::error_code ec {};
            this->m_damage.set_accuracy(new_accuracy, ec);
            this->m_damage.set_splash_chance(new_splash, ec);
        } // apply_friendly_skill(...)

        /** @brief Apply the enemy army's skill to a unit.
         *  @param level The number of books invested in this skill.
         */
        void apply_enemy_skill(battle_skill skill, std::size_t level) noexcept
        {
            if (level == 0) return;
            if (level > 3) level = 3;

            // Misc.
            constexpr std::array<typename hit_points_bonus_type::integer_type, 4> overrun_rate_table { 0, -8, -16, -25 };
            const hit_points_percentage_type overrun_rate { overrun_rate_table[level] };
            const experience_percentage_type experience_bonus { static_cast<typename experience_percentage_type::integer_type>(10 * level) };

            switch (skill)
            {
                case battle_skill::fast_learner: // Increases the XP gained from enemy units defeated by this army by 10/20/30%.
                    this->m_experience_bonus.append_rate(experience_bonus);
                    break;
                case battle_skill::overrun: // Decreases the HP of enemy bosses by 8/16/25%.
                    if (!this->is(unit_category::boss)) return;
                    this->m_hit_points_bonus.append_rate(overrun_rate);
                    break;
                default: break;
            } // switch (...)
        } // apply_enemy_skill(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self)
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct unit_type

    // ~~ Json name definitions ~~
    constexpr char unit_type::jstr_id[];
    constexpr char unit_type::jstr_names[];
    constexpr char unit_type::jstr_codenames[];
    constexpr char unit_type::jstr_faction[];
    constexpr char unit_type::jstr_category[];
    constexpr char unit_type::jstr_attack_phases[];
    constexpr char unit_type::jstr_capacity[];
    constexpr char unit_type::jstr_hit_points[];
    constexpr char unit_type::jstr_damage[];
    constexpr char unit_type::jstr_experience[];
    constexpr char unit_type::jstr_abilities[];
    constexpr char unit_type::jstr_traits[];

    void to_json(nlohmann::json& j, const unit_type& x) noexcept
    {
        using type = unit_type;

        std::vector<std::string> attack_phases {};
        std::vector<std::string> abilities {};
        std::vector<std::string> traits {};

        for (battle_phase phase : x.attack_phases()) attack_phases.push_back(std::to_string(phase));
        for (special_ability ability : x.abilities()) abilities.push_back(std::to_string(ability));
        for (battle_trait trait : x.traits()) traits.push_back(std::to_string(trait));

        j = nlohmann::json{
            {type::jstr_id, x.id()},
            {type::jstr_names, x.names()},
            {type::jstr_codenames, x.codenames()},
            {type::jstr_faction, std::to_string(x.faction())},
            {type::jstr_category, std::to_string(x.category())},
            {type::jstr_attack_phases, attack_phases},
            {type::jstr_capacity, x.capacity()},
            {type::jstr_hit_points, x.hit_points()},
            {type::jstr_damage, x.damage({})},
            {type::jstr_experience, x.experience(1)}
        };

        if (!abilities.empty()) j[type::jstr_abilities] = abilities;
        if (!traits.empty()) j[type::jstr_traits] = traits;
    } // to_json(...)

    void from_json(const nlohmann::json& j, unit_type& x)
    {
        using type = unit_type;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::unit_type>
    {
        using argument_type = ropufu::settlers_online::unit_type;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = {};
            std::hash<ropufu::settlers_online::battle_phase> phase_hash = {};
            std::hash<ropufu::settlers_online::special_ability> ability_hash = {};
            std::hash<ropufu::settlers_online::battle_trait> trait_hash = {};
            std::hash<ropufu::settlers_online::unit_faction> fac_hash = {};
            std::hash<ropufu::settlers_online::unit_category> cat_hash = {};
            std::hash<ropufu::settlers_online::damage> damage_hash = {};

            result_type result = 0;
            for (ropufu::settlers_online::battle_phase phase : x.attack_phases()) result ^= phase_hash(phase);
            for (ropufu::settlers_online::special_ability ability : x.abilities()) result ^= ability_hash(ability);
            for (ropufu::settlers_online::battle_trait trait : x.traits()) result ^= trait_hash(trait);

            return
                result ^
                size_hash(x.id()) ^
                fac_hash(x.faction()) ^
                cat_hash(x.category()) ^
                size_hash(x.experience(1)) ^
                size_hash(x.capacity()) ^
                size_hash(x.hit_points()) ^
                damage_hash(x.damage({}));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_TYPE_HPP_INCLUDED
