
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED

#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
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

        /** Mark \c unit_faction as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<unit_faction>
        {
            /** The maximum value of \c unit_faction plus one. */
            static constexpr std::size_t value = 6;
        }; // struct enum_capacity

        template <>
        struct converter<unit_faction, std::string>
        {
            using from_type = unit_faction;
            using to_type = std::string;

            static to_type to(const from_type& from) noexcept
            {
                switch (from)
                {
                    case unit_faction::non_player_adventure: return "non player adventure";
                    case unit_faction::non_player_expedition: return "non player expedition";
                    case unit_faction::general: return "general";
                    case unit_faction::expedition: return "expedition";
                    case unit_faction::common: return "common";
                    case unit_faction::elite: return "elite";
                    default: return std::to_string(static_cast<std::size_t>(from));
                }
            } // to(...)

            static bool try_from(const to_type& from, from_type& to) noexcept
            {
                if (from == "non player adventure") { to = unit_faction::non_player_adventure; return true; }
                if (from == "non-player adventure") { to = unit_faction::non_player_adventure; return true; }
                if (from == "non player expedition") { to = unit_faction::non_player_expedition; return true; }
                if (from == "non-player expedition") { to = unit_faction::non_player_expedition; return true; }
                if (from == "general") { to = unit_faction::general; return true; }
                if (from == "expedition") { to = unit_faction::expedition; return true; }
                if (from == "common") { to = unit_faction::common; return true; }
                if (from == "elite") { to = unit_faction::elite; return true; }
                return false;
            } // try_from(...)
        }; // struct converter
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_faction value) noexcept
    {
        return ropufu::settlers_online::to_str(value);
    } // to_string(...)
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED
