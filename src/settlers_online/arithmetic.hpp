
#ifndef ROPUFU_SETTLERS_ONLINE_ARITHMETIC_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARITHMETIC_HPP_INCLUDED

#include "arithmetic/bonus_modifier.hpp"
#include "arithmetic/core.hpp"
#include "arithmetic/percentage.hpp"

namespace ropufu::settlers_online
{
    using damage_bonus_type = bonus_modifier<std::make_signed_t<std::size_t>, cast_direction::away_from_zero>; // Damage modifiers are rounded up.
    using hit_points_bonus_type = bonus_modifier<std::make_signed_t<std::size_t>, cast_direction::toward_zero>; // Hit points modifiers are rounded down.
    using experience_bonus_type = bonus_modifier<std::make_signed_t<std::size_t>, cast_direction::toward_zero>; // Experience points modifiers are rounded down.
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARITHMETIC_HPP_INCLUDED
