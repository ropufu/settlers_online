
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Names categories of units.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct battle_phase : std::size_t
    {
        first_strike = 0,
        normal = 1,
        last_strike = 2
    }; // struct battle_phase
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_phase x) noexcept
    {
        using argument_type = ropufu::settlers_online::battle_phase;
        switch (x)
        {
            case argument_type::first_strike: return "first strike";
            case argument_type::normal: return "normal";
            case argument_type::last_strike: return "last strike";
            default: return "unknown <battle_phase> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c battle_phase as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::battle_phase>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::battle_phase>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 3;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::battle_phase>
    {
        using enum_type = ropufu::settlers_online::battle_phase;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "first strike") { to = enum_type::first_strike; return true; }
            if (from == "normal") { to = enum_type::normal; return true; }
            if (from == "last strike") { to = enum_type::last_strike; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED
