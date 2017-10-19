
#ifndef ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED

#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Enumerates some of the abilities for \c unit_type.
         *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
         **/
        enum class special_ability : std::size_t
        {
            none = 0,
            attack_weakest_target = 1, // When attacking enemies this unit will sort them by \c hit_points instead of \c id.
            not_weak = 2,              // This unit will be not be affected by attacker's \c do_attack_weakest_target, if any.
            tower_bonus = 3,           // This unit gets damage reduction in towers.
            ignore_tower_bonus = 4,    // When attacking, inflicted damage will not be affected by defenders' possible \c tower_bonus.
            boss = 5                   // Categorized as boss for some battle calculations.
        };

        /** Mark \c special_ability as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<special_ability>
        {
            /** The maximum value of \c special_ability plus one. */
            static constexpr std::size_t value = 6;
        };

        bool try_parse(const std::string& str, special_ability& value)
        {
            if (str == "none") { value = special_ability::none; return true; }
            if (str == "attack weakest target") { value = special_ability::attack_weakest_target; return true; }
            if (str == "not weak") { value = special_ability::not_weak; return true; }
            if (str == "tower bonus") { value = special_ability::tower_bonus; return true; }
            if (str == "ignore tower bonus") { value = special_ability::ignore_tower_bonus; return true; }
            if (str == "boss") { value = special_ability::boss; return true; }
            return false;
        }
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::special_ability value)
    {
        switch (value)
        {
        case ropufu::settlers_online::special_ability::none: return "none";
        case ropufu::settlers_online::special_ability::attack_weakest_target: return "attack weakest target";
        case ropufu::settlers_online::special_ability::not_weak: return "not weak";
        case ropufu::settlers_online::special_ability::tower_bonus: return "tower bonus";
        case ropufu::settlers_online::special_ability::ignore_tower_bonus: return "ignore tower bonus";
        case ropufu::settlers_online::special_ability::boss: return "boss";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED