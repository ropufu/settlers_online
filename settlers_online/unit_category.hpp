
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>

namespace ropufu
{
    namespace settlers_online
    {
		// The maximum value of <unit_category> + 1.
		static constexpr std::size_t category_capacity = 5;

		// Names categories of units. Used internally as an indexer for <std::array>, so don't go too high.
        enum class unit_category : std::int32_t
        {
            none = 0,
            melee = 1,
            ranged = 2,
            cavalry = 3,
            artillery = 4
        };
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::unit_category value)
    {
        switch (value)
        {
        case ropufu::settlers_online::unit_category::none: return "none";
        case ropufu::settlers_online::unit_category::melee: return "melee";
        case ropufu::settlers_online::unit_category::ranged: return "ranged";
        case ropufu::settlers_online::unit_category::cavalry: return "cavalry";
        case ropufu::settlers_online::unit_category::artillery: return "artillery";
        default: return std::to_string(static_cast<std::int32_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_CATEGORY_HPP_INCLUDED
