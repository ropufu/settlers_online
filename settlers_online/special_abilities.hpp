
#ifndef ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITIES_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITIES_HPP_INCLUDED

//#include <cstddef>
#include <cstdint>
#include <string>

namespace ropufu
{
    namespace settlers_online
    {
        enum class special_abilities : std::int32_t
        {
            none = 0x00,
            /// <summary>
            /// Get damage reduction in specific <c>camp_type</c>.
            /// </summary>
            tower_bonus = 0x04,
            /// <summary>
            /// Defender's <c>camp_type</c> does not affect inflicted damage.
            /// </summary>
            ignore_tower_bonus = 0x10,
			/// <summary>
			/// Is categorized as boss for some battle calculations.
			/// </summary>
			boss = 0x20,
			/// <summary>
			/// Attack twice per round. That second attack's initiative is "Last Strike".
			/// </summary>
			attack_twice = 0x100
        };
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::special_abilities value)
    {
        switch (value)
        {
        case ropufu::settlers_online::special_abilities::none: return "none";
        case ropufu::settlers_online::special_abilities::tower_bonus: return "tower bonus";
        case ropufu::settlers_online::special_abilities::ignore_tower_bonus: return "ignore tower bonus";
        case ropufu::settlers_online::special_abilities::boss: return "boss";
		case ropufu::settlers_online::special_abilities::attack_twice: return "attack twice";
        default: return std::to_string(static_cast<std::int32_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_SPECIAL_ABILITIES_HPP_INCLUDED
