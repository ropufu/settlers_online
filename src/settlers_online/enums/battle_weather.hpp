
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_WEATHER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_WEATHER_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include "../algebra.hpp"

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    static constexpr typename hit_points_bonus_type::percentage_type sunshine_hit_points_percentage { +20 };
    static constexpr typename damage_bonus_type::percentage_type hurricane_damage_percentage { +20 };

    /** @brief Changes battle_weather conditions affecting the entire adventure. **/
    enum struct battle_weather : char
    {
        none = 0,
        hard_frost = 1,         // All \c melee units (including enemy \c melee units) gain splash damage.
        bright_sunshine = 2,    // The HP of all units (including enemy units) are increased by 20%.
        heavy_fog = 3,          // All units (including enemy units) gain \c do_attack_weakest_target.
        hurricane = 4           // The minimum and maximum damage of all units (including enemy units) is increased by 20%.
    }; // struct battle_weather
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_weather x) noexcept
    {
        using argument_type = ropufu::settlers_online::battle_weather;
        switch (x)
        {
            case argument_type::none: return "none";
            case argument_type::hard_frost: return "hard frost";
            case argument_type::bright_sunshine: return "bright sunshine";
            case argument_type::heavy_fog: return "heavy fog";
            case argument_type::hurricane: return "hurricane";
            default: return "unknown <battle_weather> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c battle_skill as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::battle_weather>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::battle_weather>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 5;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::battle_weather>
    {
        using enum_type = ropufu::settlers_online::battle_weather;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "none") { to = enum_type::none; return true; }
            if (from == "hard frost" || from == "frost") { to = enum_type::hard_frost; return true; }
            if (from == "bright sunshine" || from == "sunshine" || from == "sun" || from == "bright sun") { to = enum_type::bright_sunshine; return true; }
            if (from == "heavy fog" || from == "fog") { to = enum_type::heavy_fog; return true; }
            if (from == "hurricane") { to = enum_type::hurricane; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_WEATHER_HPP_INCLUDED
