
#ifndef ROPUFU_SETTLERS_ONLINE_ALGEBRA_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ALGEBRA_HPP_INCLUDED

#include "algebra/arithmetic.hpp"
#include "algebra/blueprint_index.hpp"
#include "algebra/blueprint_size.hpp"
#include "algebra/bonus_modifier.hpp"
#include "algebra/percentage.hpp"

namespace ropufu::settlers_online
{
    using damage_bonus_type = bonus_modifier<std::make_signed_t<std::size_t>, cast_direction::away_from_zero>; // Damage modifiers are rounded up.
    using hit_points_bonus_type = bonus_modifier<std::make_signed_t<std::size_t>, cast_direction::toward_zero>; // Hit points modifiers are rounded down.
    using experience_bonus_type = bonus_modifier<std::make_signed_t<std::size_t>, cast_direction::toward_zero>; // Experience points modifiers are rounded down.
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ALGEBRA_HPP_INCLUDED
