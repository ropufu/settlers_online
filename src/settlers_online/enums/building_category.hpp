
#ifndef ROPUFU_SETTLERS_ONLINE_BUILDING_CATEGORY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BUILDING_CATEGORY_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Named categories of buildings.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct building_category : char
    {
        unknown = 0,
        factory = 1,   // Buildings that produce goods on-site.
        harvester = 2, // Buildings that require a deposit.
        mine = 3,      // Special case of harvesters that either (i) provide their own deposits; or (ii) can only access a single deposit they are built upon.
        deposit = 4,   // Non-harvesting building with inner deposit available for external consumption.
        other = 5      // Everything else.
    }; // struct building_category
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::building_category x) noexcept
    {
        using argument_type = ropufu::settlers_online::building_category;
        switch (x)
        {
            case argument_type::unknown: return "unknown";
            case argument_type::factory: return "factory";
            case argument_type::harvester: return "harvester";
            case argument_type::mine: return "mine";
            case argument_type::deposit: return "deposit";
            case argument_type::other: return "other";
            default: return "unknown <building_category> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c building_category as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::building_category>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::building_category>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 6;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::building_category>
    {
        using enum_type = ropufu::settlers_online::building_category;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "unknown") { to = enum_type::unknown; return true; }
            if (from == "factory") { to = enum_type::factory; return true; }
            if (from == "harvester") { to = enum_type::harvester; return true; }
            if (from == "mine") { to = enum_type::mine; return true; }
            if (from == "deposit") { to = enum_type::deposit; return true; }
            if (from == "other") { to = enum_type::other; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_BUILDING_CATEGORY_HPP_INCLUDED
