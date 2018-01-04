
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
        enum struct special_ability : std::size_t
        {
            none = 0,
            attack_weakest_target = 1, // When attacking enemies this unit will sort them by \c hit_points instead of \c id.
            not_weak = 2,              // This unit will be not be affected by attacker's \c do_attack_weakest_target, if any.
            tower_bonus = 3,           // This unit gets damage reduction in towers.
            ignore_tower_bonus = 4,    // When attacking, inflicted damage will not be affected by defenders' possible \c tower_bonus.
            rapid_fire = 5,            // Indicates that this unis is affected by friendly skill \c battle_skill::rapid_fire.
            sniper_training = 6,       // Indicates that this unis is affected by friendly skill \c battle_skill::sniper_training.
            cleave = 7,                // Indicates that this unis is affected by friendly skill \c battle_skill::cleave.
            overrun = 8                // Indicates that this unis is affected by enemy skill \c battle_skill::overrun.
        }; // struct special_ability

        /** Mark \c special_ability as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<special_ability>
        {
            /** The maximum value of \c special_ability plus one. */
            static constexpr std::size_t value = 9;
        }; // struct enum_capacity

        template <>
        struct converter<special_ability, std::string>
        {
            using from_type = special_ability;
            using to_type = std::string;

            static to_type to(const from_type& from) noexcept
            {
                switch (from)
                {
                    case special_ability::none: return "none";
                    case special_ability::attack_weakest_target: return "attack weakest target";
                    case special_ability::not_weak: return "not weak";
                    case special_ability::tower_bonus: return "tower bonus";
                    case special_ability::ignore_tower_bonus: return "ignore tower bonus";
                    case special_ability::rapid_fire: return "rapid fire";
                    case special_ability::sniper_training: return "sniper training";
                    case special_ability::cleave: return "cleave";
                    case special_ability::overrun: return "overrun";
                    default: return std::to_string(static_cast<std::size_t>(from));
                }
            } // to(...)

            static bool try_from(const to_type& from, from_type& to) noexcept
            {
                if (from == "none") { to = special_ability::none; return true; }
                if (from == "attack weakest target") { to = special_ability::attack_weakest_target; return true; }
                if (from == "not weak") { to = special_ability::not_weak; return true; }
                if (from == "tower bonus") { to = special_ability::tower_bonus; return true; }
                if (from == "ignore tower bonus") { to = special_ability::ignore_tower_bonus; return true; }
                if (from == "rapid fire") { to = special_ability::rapid_fire; return true; }
                if (from == "sniper training") { to = special_ability::sniper_training; return true; }
                if (from == "cleave") { to = special_ability::cleave; return true; }
                if (from == "overrun") { to = special_ability::overrun; return true; }
                if (from == "boss") { to = special_ability::overrun; return true; }
                return false;
            } // try_from(...)
        }; // struct converter
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    std::string to_string(ropufu::settlers_online::special_ability value) noexcept
    {
        return ropufu::settlers_online::detail::to_str(value);
    } // to_string(...)
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED
