
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Named categories of units.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct unit_category : char
    {
        unknown = 0,
        melee = 1,
        ranged = 2,
        cavalry = 3,
        artillery = 4,
        elite = 5
    }; // struct unit_category
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_category x) noexcept
    {
        using argument_type = ropufu::settlers_online::unit_category;
        switch (x)
        {
            case argument_type::unknown: return "unknown";
            case argument_type::melee: return "melee";
            case argument_type::ranged: return "ranged";
            case argument_type::cavalry: return "cavalry";
            case argument_type::artillery: return "artillery";
            case argument_type::elite: return "elite";
            default: return "unknown <unit_category> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c unit_category as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::unit_category>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::unit_category>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 6;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::unit_category>
    {
        using enum_type = ropufu::settlers_online::unit_category;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "none") { to = enum_type::unknown; return true; }
            if (from == "unknown") { to = enum_type::unknown; return true; }
            if (from == "melee") { to = enum_type::melee; return true; }
            if (from == "ranged") { to = enum_type::ranged; return true; }
            if (from == "cavalry") { to = enum_type::cavalry; return true; }
            if (from == "artillery") { to = enum_type::artillery; return true; }
            if (from == "elite") { to = enum_type::elite; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
