
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
        enum class unit_category : std::size_t
        {
            unknown = 0,
            melee = 1,
            ranged = 2,
            cavalry = 3,
            artillery = 4,
            elite = 5
        };

        /** Mark \c unit_category as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<unit_category>
        {
            /** The maximum value of \c unit_category plus one. */
            static constexpr std::size_t value = 6;
        };

        bool try_parse(const std::string& str, unit_category& value)
        {
            if (str == "none") { value = unit_category::unknown; return true; }
            if (str == "unknown") { value = unit_category::unknown; return true; }
            if (str == "melee") { value = unit_category::melee; return true; }
            if (str == "ranged") { value = unit_category::ranged; return true; }
            if (str == "cavalry") { value = unit_category::cavalry; return true; }
            if (str == "artillery") { value = unit_category::artillery; return true; }
            if (str == "elite") { value = unit_category::elite; return true; }
            return false;
        }
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_category value)
    {
        switch (value)
        {
        case ropufu::settlers_online::unit_category::unknown: return "unknown";
        case ropufu::settlers_online::unit_category::melee: return "melee";
        case ropufu::settlers_online::unit_category::ranged: return "ranged";
        case ropufu::settlers_online::unit_category::cavalry: return "cavalry";
        case ropufu::settlers_online::unit_category::artillery: return "artillery";
        case ropufu::settlers_online::unit_category::elite: return "elite";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
