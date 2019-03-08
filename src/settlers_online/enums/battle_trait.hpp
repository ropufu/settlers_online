
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t, std::make_signed_t

namespace ropufu::settlers_online
{
    static constexpr std::make_signed_t<std::size_t> intercept_damage_percentage = -5;
    static constexpr std::make_signed_t<std::size_t> bombastic_damage_percentage = +100;

    /** @brief Traits that some units may have that modify the course of the entire battle.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct battle_trait : char
    {
        none = 0,
        dazzle = 1,               // Enemy accuracy is reduced to 0%.
        intercept = 2,            // Enemy units deal 5% less damage and their ability \c do_attack_weakest_target is ignored.
        explosive_ammunition = 3, // Ranged units get \c do_attack_weakest_target and 100\% \c splash_chance.
        bombastic = 4             // Doubles the damage of \c artillery units.
    }; // struct battle_trait
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_trait x) noexcept
    {
        using argument_type = ropufu::settlers_online::battle_trait;
        switch (x)
        {
            case argument_type::none: return "none";
            case argument_type::dazzle: return "dazzle";
            case argument_type::intercept: return "intercept";
            case argument_type::explosive_ammunition: return "explosive ammunition";
            case argument_type::bombastic: return "bombastic";
            default: return "unknown <battle_trait> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c battle_trait as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::battle_trait>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::battle_trait>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 5;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::battle_trait>
    {
        using enum_type = ropufu::settlers_online::battle_trait;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "none") { to = enum_type::none; return true; }
            if (from == "dazzle") { to = enum_type::dazzle; return true; }
            if (from == "intercept") { to = enum_type::intercept; return true; }
            if (from == "explosive ammunition") { to = enum_type::explosive_ammunition; return true; }
            if (from == "bombastic") { to = enum_type::bombastic; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED
