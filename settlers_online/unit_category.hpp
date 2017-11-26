
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED

#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Named categories of units.
         *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
         **/
        enum struct unit_category : std::size_t
        {
            unknown = 0,
            melee = 1,
            ranged = 2,
            cavalry = 3,
            artillery = 4,
            elite = 5
        }; // struct unit_category

        /** Mark \c unit_category as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<unit_category>
        {
            /** The maximum value of \c unit_category plus one. */
            static constexpr std::size_t value = 6;
        }; // struct enum_capacity

        template <>
        struct converter<unit_category, std::string>
        {
            using from_type = unit_category;
            using to_type = std::string;

            static to_type to(const from_type& from) noexcept
            {
                switch (from)
                {
                    case unit_category::unknown: return "unknown";
                    case unit_category::melee: return "melee";
                    case unit_category::ranged: return "ranged";
                    case unit_category::cavalry: return "cavalry";
                    case unit_category::artillery: return "artillery";
                    case unit_category::elite: return "elite";
                    default: return std::to_string(static_cast<std::size_t>(from));
                }
            } // to(...)

            static bool try_from(const to_type& from, from_type& to) noexcept
            {
                if (from == "none") { to = unit_category::unknown; return true; }
                if (from == "unknown") { to = unit_category::unknown; return true; }
                if (from == "melee") { to = unit_category::melee; return true; }
                if (from == "ranged") { to = unit_category::ranged; return true; }
                if (from == "cavalry") { to = unit_category::cavalry; return true; }
                if (from == "artillery") { to = unit_category::artillery; return true; }
                if (from == "elite") { to = unit_category::elite; return true; }
                return false;
            } // try_from(...)
        }; // struct converter
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_category value) noexcept
    {
        return ropufu::settlers_online::detail::to_str(value);
    } // to_string(...)
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
