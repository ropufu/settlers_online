
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
        enum class unit_faction : std::size_t
        {
            non_player_adventure = 0,  // All adventure enemy units.
            non_player_expedition = 1, // All expedition enemy units.
            general = 2,    // Generals / champions.
            expedition = 3, // Combat academy (expedition) units.
            common = 4,     // Common barracks units.
            elite = 5,      // Elite barracks units.
        };

        /** Mark \c unit_faction as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<unit_faction>
        {
            /** The maximum value of \c unit_faction plus one. */
            static constexpr std::size_t value = 6;
        };

        bool try_parse(const std::string& str, unit_faction& value)
        {
            if (str == "non player adventure") { value = unit_faction::non_player_adventure; return true; }
            if (str == "non-player adventure") { value = unit_faction::non_player_adventure; return true; }
            if (str == "non player expedition") { value = unit_faction::non_player_expedition; return true; }
            if (str == "non-player expedition") { value = unit_faction::non_player_expedition; return true; }
            if (str == "general") { value = unit_faction::general; return true; }
            if (str == "expedition") { value = unit_faction::expedition; return true; }
            if (str == "common") { value = unit_faction::common; return true; }
            if (str == "elite") { value = unit_faction::elite; return true; }
            return false;
        }
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_faction value)
    {
        switch (value)
        {
        case ropufu::settlers_online::unit_faction::non_player_adventure: return "non player adventure";
        case ropufu::settlers_online::unit_faction::non_player_expedition: return "non player expedition";
        case ropufu::settlers_online::unit_faction::general: return "general";
        case ropufu::settlers_online::unit_faction::expedition: return "expedition";
        case ropufu::settlers_online::unit_faction::common: return "common";
        case ropufu::settlers_online::unit_faction::elite: return "elite";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_FACTION_HPP_INCLUDED
