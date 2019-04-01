
#ifndef ROPUFU_SETTLERS_ONLINE_ARITHMETIC_CORE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARITHMETIC_CORE_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <limits>  // std::numeric_limits::is_integer

namespace ropufu::settlers_online
{
    /** Divides <numerator> by <denominator> and rounds toward zero. */
    template <typename t_integer_type>
    static t_integer_type fraction_floor(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Arguments must be integer-valued.");
        return numerator / denominator;
    } // fraction_floor(...)

    /** Divides <numerator> by <denominator> and rounds away from zero. */
    template <typename t_integer_type>
    static t_integer_type fraction_ceiling(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Arguments must be integer-valued.");
        return (numerator + denominator - 1) / denominator;
    } // fraction_ceiling(...)

    // /** Adjusts \p damage by \p factor (multiplicative). */
    // template <typename t_integer_type>
    // static t_integer_type damage_cast(t_integer_type damage, t_integer_type numerator, t_integer_type denominator) noexcept
    // {
    //     // static_assert(std::numeric_limits<t_integer_type>::is_integer, "Damage must be an integer.");
    //     return fraction_floor(damage * numerator, denominator);
    // } // damage_cast(...)

    // /** Determines the smallest pure damage required to eliminate a unit with \p hit_points with (multiplicative) \p factor damage modifier. */
    // template <typename t_integer_type>
    // static t_integer_type inverse_damage_cast(t_integer_type hit_points, t_integer_type numerator, t_integer_type denominator) noexcept
    // {
    //     // static_assert(std::numeric_limits<t_integer_type>::is_integer, "Hit points must be an integer.");
    //     return fraction_ceiling(hit_points * denominator, numerator);
    // } // inverse_damage_cast(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARITHMETIC_CORE_HPP_INCLUDED
