
#ifndef ROPUFU_SETTLERS_ONLINE_TYPEDEF_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TYPEDEF_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <cstdint> // std::uint_fast64_t

namespace ropufu
{
    namespace settlers_online
    {
        //typedef std::bitset<max_size> mask_type;
        using mask_type = std::uint_fast64_t;

        static constexpr std::size_t byte_size_in_bits = 8;
        // The maximum number of groups in an army.
        static constexpr std::size_t army_capacity = byte_size_in_bits * sizeof(mask_type);

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
            //static constexpr t_integer_type factor_upper_bound = 10;
            return (t_factor_upper_bound * value) - static_cast<t_integer_type>(value * (t_factor_upper_bound - factor)); // Rounds away from zero (equivalent to ceiling with positive numbers).
        } // product_ceiling(...)

        /** Adjusts \p damage by \p factor (multiplicative). */
        static std::size_t damage_cast(std::size_t damage, double factor)
        {
            return product_floor(damage, factor);
        } // damage_cast(...)

        /** Determines the smallest pure damage required to eliminate a unit with \p hit_points with (multiplicative) \p factor damage modifier. */
        static std::size_t inverse_damage_cast(std::size_t hit_points, double factor)
        {
            return product_ceiling(hit_points, 1.0 / factor);
        } // inverse_damage_cast(...)

        // /** Adjusts \p hit_points by \p factor (multiplicative). */
        // static std::size_t hit_points_cast(std::size_t hit_points, double factor)
        // {
        //     return product_ceiling(hit_points, factor);
        // } // hit_points_cast(...)
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_TYPEDEF_HPP_INCLUDED
