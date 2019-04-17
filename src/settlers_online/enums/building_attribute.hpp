
#ifndef ROPUFU_SETTLERS_ONLINE_BUILDING_ATTRIBUTE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BUILDING_ATTRIBUTE_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Enumerates some of the building attributes.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct building_attribute : char
    {
        none = 0,
        cannot_be_moved = 1,
        cannot_be_buffed = 2,
        cannot_be_demolished = 3,
        placed_on_deposit = 4,
        storehouse = 5
    }; // struct building_attribute
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::building_attribute x) noexcept
    {
        using argument_type = ropufu::settlers_online::building_attribute;
        switch (x)
        {
            case argument_type::none: return "none";
            case argument_type::cannot_be_moved: return "cannot be moved";
            case argument_type::cannot_be_buffed: return "cannot be buffed";
            case argument_type::cannot_be_demolished: return "cannot be demolished";
            case argument_type::placed_on_deposit: return "placed on deposit";
            case argument_type::storehouse: return "storehouse";
            default: return "unknown <building_attribute> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c building_attribute as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::building_attribute>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::building_attribute>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 6;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::building_attribute>
    {
        using enum_type = ropufu::settlers_online::building_attribute;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "none") { to = enum_type::none; return true; }
            if (from == "cannot be moved") { to = enum_type::cannot_be_moved; return true; }
            if (from == "cannot be buffed") { to = enum_type::cannot_be_buffed; return true; }
            if (from == "cannot be demolished") { to = enum_type::cannot_be_demolished; return true; }
            if (from == "placed on deposit") { to = enum_type::placed_on_deposit; return true; }
            if (from == "storehouse") { to = enum_type::storehouse; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_BUILDING_ATTRIBUTE_HPP_INCLUDED
