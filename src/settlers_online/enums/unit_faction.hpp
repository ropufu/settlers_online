
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED

#include <aftermath/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Faction categories of units.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct unit_faction : std::size_t
    {
        non_player_adventure = 0,  // All adventure enemy units.
        non_player_expedition = 1, // All expedition enemy units.
        general = 2,    // Generals / champions.
        expedition = 3, // Combat academy (expedition) units.
        common = 4,     // Common barracks units.
        elite = 5       // Elite barracks units.
    }; // struct unit_faction
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_faction x) noexcept
    {
        using argument_type = ropufu::settlers_online::unit_faction;
        switch (x)
        {
            case argument_type::non_player_adventure: return "non player adventure";
            case argument_type::non_player_expedition: return "non player expedition";
            case argument_type::general: return "general";
            case argument_type::expedition: return "expedition";
            case argument_type::common: return "common";
            case argument_type::elite: return "elite";
            default: return "unknown <unit_faction> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c unit_faction as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::unit_faction>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::unit_faction>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 6;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::unit_faction>
    {
        using enum_type = ropufu::settlers_online::unit_faction;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "non player adventure") { to = enum_type::non_player_adventure; return true; }
            if (from == "non-player adventure") { to = enum_type::non_player_adventure; return true; }
            if (from == "non player expedition") { to = enum_type::non_player_expedition; return true; }
            if (from == "non-player expedition") { to = enum_type::non_player_expedition; return true; }
            if (from == "general") { to = enum_type::general; return true; }
            if (from == "expedition") { to = enum_type::expedition; return true; }
            if (from == "common") { to = enum_type::common; return true; }
            if (from == "elite") { to = enum_type::elite; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED
