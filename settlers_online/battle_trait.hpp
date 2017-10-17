
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED

#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Traits that some units may have that modify the course of the entire battle.
         *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
         **/
        enum class battle_trait : std::size_t
        {
            none = 0,
            dazzle = 1,
            intercept = 2,
            explosive_ammunition = 3
        };

        /** Mark \c battle_trait as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<battle_trait>
        {
            /** The maximum value of \c battle_trait plus one. */
            static constexpr std::size_t value = 4;
        };

        bool try_parse(const std::string& str, battle_trait& value)
        {
            if (str == "none") { value = battle_trait::none; return true; }
            if (str == "dazzle") { value = battle_trait::dazzle; return true; }
            if (str == "intercept") { value = battle_trait::intercept; return true; }
            if (str == "explosive ammunition") { value = battle_trait::explosive_ammunition; return true; }
            return false;
        }
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_trait value)
    {
        switch (value)
        {
        case ropufu::settlers_online::battle_trait::none: return "none";
        case ropufu::settlers_online::battle_trait::dazzle: return "dazzle";
        case ropufu::settlers_online::battle_trait::intercept: return "intercept";
        case ropufu::settlers_online::battle_trait::explosive_ammunition: return "explosive_ammunition";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED
