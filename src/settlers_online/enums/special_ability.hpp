
#ifndef ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Enumerates some of the abilities for \c unit_type.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct special_ability : char
    {
        none = 0,
        attack_weakest_target = 1, // When attacking enemies this unit will sort them by \c hit_points instead of \c id.
        not_weak = 2,              // This unit will be not be affected by attacker's \c do_attack_weakest_target, if any.
        tower_bonus = 3,           // This unit gets damage reduction in towers.
        ignore_tower_bonus = 4,    // When attacking, inflicted damage will not be affected by defenders' possible \c tower_bonus.
        archer = 5,                // Indicates that this unis is affected by friendly skill \c battle_skill::rapid_fire.
        sniper = 6,                // Indicates that this unis is affected by friendly skill \c battle_skill::sniper_training.
        butcher = 7                // Indicates that this unis is affected by friendly skill \c battle_skill::cleave.
    }; // struct special_ability
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::special_ability x) noexcept
    {
        using argument_type = ropufu::settlers_online::special_ability;
        switch (x)
        {
            case argument_type::none: return "none";
            case argument_type::attack_weakest_target: return "attack weakest target";
            case argument_type::not_weak: return "not weak";
            case argument_type::tower_bonus: return "tower bonus";
            case argument_type::ignore_tower_bonus: return "ignore tower bonus";
            case argument_type::archer: return "archer";
            case argument_type::sniper: return "sniper";
            case argument_type::butcher: return "butcher";
            default: return "unknown <special_ability> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c special_ability as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::special_ability>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::special_ability>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 8;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::special_ability>
    {
        using enum_type = ropufu::settlers_online::special_ability;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "none") { to = enum_type::none; return true; }
            if (from == "attack weakest target" || from == "flanking") { to = enum_type::attack_weakest_target; return true; }
            if (from == "not weak") { to = enum_type::not_weak; return true; }
            if (from == "tower bonus") { to = enum_type::tower_bonus; return true; }
            if (from == "ignore tower bonus") { to = enum_type::ignore_tower_bonus; return true; }
            if (from == "archer") { to = enum_type::archer; return true; }
            if (from == "sniper") { to = enum_type::sniper; return true; }
            if (from == "butcher") { to = enum_type::butcher; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITY_HPP_INCLUDED
