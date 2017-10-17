
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED

#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Names categories of units.
         *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
         **/
        enum class battle_phase : std::size_t
        {
            first_strike = 0,
            normal = 1,
            last_strike = 2
        };

        /** Mark \c battle_phase as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<battle_phase>
        {
            /** The maximum value of \c battle_phase plus one. */
            static constexpr std::size_t value = 3;
        };

        bool try_parse(const std::string& str, battle_phase& value)
        {
            if (str == "first strike") { value = battle_phase::first_strike; return true; }
            if (str == "normal") { value = battle_phase::normal; return true; }
            if (str == "last strike") { value = battle_phase::last_strike; return true; }
            return false;
        }
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_phase value)
    {
        switch (value)
        {
        case ropufu::settlers_online::battle_phase::first_strike: return "first strike";
        case ropufu::settlers_online::battle_phase::normal: return "normal";
        case ropufu::settlers_online::battle_phase::last_strike: return "last strike";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED
