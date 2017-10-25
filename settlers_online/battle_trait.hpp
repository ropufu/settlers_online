
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED

#include "core.hpp"
#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
    {
        static constexpr std::size_t intercept_damage_percent = 95;

        /** @brief Traits that some units may have that modify the course of the entire battle.
         *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
         **/
        enum struct battle_trait : std::size_t
        {
            none = 0,
            dazzle = 1,              // Enemy accuracy is reduced to 0%.
            intercept = 2,           // Enemy units deal 5% less damage and their ability \c do_attack_weakest_target is ignored.
            explosive_ammunition = 3 // Ranged units get \c do_attack_weakest_target and 100\% \c splash_chance.
        }; // struct battle_trait

        /** Mark \c battle_trait as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<battle_trait>
        {
            /** The maximum value of \c battle_trait plus one. */
            static constexpr std::size_t value = 4;
        }; // struct enum_capacity

        template <>
        struct converter<battle_trait, std::string>
        {
            using from_type = battle_trait;
            using to_type = std::string;

            static to_type to(const from_type& from) noexcept
            {
                switch (from)
                {
                    case battle_trait::none: return "none";
                    case battle_trait::dazzle: return "dazzle";
                    case battle_trait::intercept: return "intercept";
                    case battle_trait::explosive_ammunition: return "explosive_ammunition";
                    default: return std::to_string(static_cast<std::size_t>(from));
                }
            } // to(...)

            static bool try_from(const to_type& from, from_type& to) noexcept
            {
                if (from == "none") { to = battle_trait::none; return true; }
                if (from == "dazzle") { to = battle_trait::dazzle; return true; }
                if (from == "intercept") { to = battle_trait::intercept; return true; }
                if (from == "explosive ammunition") { to = battle_trait::explosive_ammunition; return true; }
                return false;
            } // try_from(...)
        }; // struct converter
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_trait value) noexcept
    {
        return ropufu::settlers_online::to_str(value);
    } // to_string(...)
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_TRAIT_HPP_INCLUDED
