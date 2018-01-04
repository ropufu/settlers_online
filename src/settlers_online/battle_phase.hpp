
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED

#include "core.hpp"
#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
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

        /** Mark \c battle_phase as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<battle_phase>
        {
            /** The maximum value of \c battle_phase plus one. */
            static constexpr std::size_t value = 3;
        }; // struct enum_capacity

        template <>
        struct converter<battle_phase, std::string>
        {
            using from_type = battle_phase;
            using to_type = std::string;

            static to_type to(const from_type& from) noexcept
            {
                switch (from)
                {
                    case battle_phase::first_strike: return "first strike";
                    case battle_phase::normal: return "normal";
                    case battle_phase::last_strike: return "last strike";
                    default: return std::to_string(static_cast<std::size_t>(from));
                }
            } // to(...)

            static bool try_from(const to_type& from, from_type& to) noexcept
            {
                if (from == "first strike") { to = battle_phase::first_strike; return true; }
                if (from == "normal") { to = battle_phase::normal; return true; }
                if (from == "last strike") { to = battle_phase::last_strike; return true; }
                return false;
            } // try_from(...)
        }; // struct converter
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_phase value) noexcept
    {
        return ropufu::settlers_online::detail::to_str(value);
    } // to_string(...)
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_PHASE_HPP_INCLUDED
