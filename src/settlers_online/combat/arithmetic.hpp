
#ifndef ROPUFU_SETTLERS_ONLINE_ARITHMETIC_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARITHMETIC_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <limits>  // std::numeric_limits::is_integer

namespace ropufu::settlers_online
{
    /** Divides <numerator> by <denominator> and rounds down. Both arguments should be positive. */
    template <typename t_integer_type>
    static t_integer_type fraction_floor(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        return numerator / denominator;
    } // fraction_floor(...)

    /** Divides <numerator> by <denominator> and rounds up. Both arguments should be positive. */
    template <typename t_integer_type>
    static t_integer_type fraction_ceiling(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        return (numerator + denominator - 1) / denominator;
    } // fraction_ceiling(...)

    /** Multiplies <value> by <factor> and rounds down. Both arguments should be positive. */
    template <typename t_integer_type>
    static t_integer_type product_floor(t_integer_type value, double factor) noexcept
    {
        return static_cast<t_integer_type>(value * factor); // Rounds toward zero (equivalent to floor with positive numbers).
    } // product_floor(...)

    /** Multiplies <value> by <factor> and rounds up. Both arguments should be positive. */
    template <typename t_integer_type, t_integer_type t_factor_upper_bound = 10>
    static t_integer_type product_ceiling(t_integer_type value, double factor) noexcept
    {
        //static constexpr t_integer_type factor_upper_bound = t_factor_upper_bound;
        return (t_factor_upper_bound * value) - static_cast<t_integer_type>(value * (t_factor_upper_bound - factor)); // Rounds away from zero (equivalent to ceiling with positive numbers).
    } // product_ceiling(...)

    /** Adjusts \p damage by \p factor (multiplicative). */
    template <typename t_integer_type>
    static t_integer_type damage_cast(t_integer_type damage, double factor) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Damage must be an integer.");
        return product_floor(damage, factor);
    } // damage_cast(...)

    /** Determines the smallest pure damage required to eliminate a unit with \p hit_points with (multiplicative) \p factor damage modifier. */
    template <typename t_integer_type>
    static t_integer_type inverse_damage_cast(t_integer_type hit_points, double factor) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Hit points must be an integer.");
        return product_ceiling(hit_points, 1.0 / factor);
    } // inverse_damage_cast(...)

    /** Adjusts \p hit_points by \p factor (multiplicative). */
    template <typename t_integer_type>
    static t_integer_type hit_points_cast(t_integer_type hit_points, double factor) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Hit points must be an integer.");
        return product_ceiling(hit_points, factor);
    } // hit_points_cast(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARITHMETIC_HPP_INCLUDED
