
#ifndef ROPUFU_SETTLERS_ONLINE_ARITHMETIC_CORE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARITHMETIC_CORE_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <limits>  // std::numeric_limits::is_integer, std::numeric_limits::is_signed

namespace ropufu::settlers_online
{
    /** @brief Divides \p numerator by \p denominator and rounds toward zero. */
    template <typename t_integer_type>
    static t_integer_type fraction_floor(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Arguments must be integer-valued.");
        return numerator / denominator;
    } // fraction_floor(...)

    /** @brief Divides \p numerator by \p denominator and rounds away from zero.
     *  @param denominator Should be positive.
     *  @warning Checks on \p denominator being positive are not performed.
     */
    template <typename t_integer_type>
    static t_integer_type fraction_ceiling(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Arguments must be integer-valued.");
        if constexpr (std::numeric_limits<t_integer_type>::is_signed)
        {
            return numerator - fraction_floor(numerator * denominator - numerator, denominator);
        } // if constexpr
        else return fraction_floor(numerator + denominator - 1, denominator);
    } // fraction_ceiling(...)

    /** @brief Indicates if \p numerator divided by \p denominator is an integer or a fraction.
     *  @returns 0 if \p numerator is divisible by \p denominator; 1 otherwise.
     *  @param denominator Should be positive.
     *  @warning Checks on \p denominator being positive are not performed.
     */
    template <typename t_integer_type>
    static t_integer_type indicator_is_fractional(t_integer_type numerator, t_integer_type denominator) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Arguments must be integer-valued.");
        t_integer_type signed_indicator = fraction_ceiling(numerator % denominator, denominator);

        if constexpr (std::numeric_limits<t_integer_type>::is_signed)
        {
            return signed_indicator * signed_indicator;
        } // if constexpr
        else return signed_indicator;
    } // indicator_is_fractional(...)

    /** @brief Indicates if \p value is zero or not.
     *  @returns 0 if \p value equals 0; 1 otherwise.
     */
    template <typename t_integer_type>
    static t_integer_type indicator_is_non_zero(t_integer_type value) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Argument must be integer-valued.");
        if constexpr (std::numeric_limits<t_integer_type>::is_signed)
        {
            t_integer_type squared_value = value * value;
            return fraction_ceiling(squared_value, squared_value + 1);
        } // if constexpr
        else return fraction_ceiling(value, value + 1);
    } // indicator_is_non_zero(...)

    /** @brief Sign of \p value.
     *  @returns 0 if \p value equals 0; 1 if is \p value is positive; -1 if \p value is negative.
     */
    template <typename t_integer_type>
    static t_integer_type sign(t_integer_type value) noexcept
    {
        static_assert(std::numeric_limits<t_integer_type>::is_integer, "Argument must be integer-valued.");
        if constexpr (std::numeric_limits<t_integer_type>::is_signed)
        {
            return fraction_ceiling(value, value * value + 1);
        } // if constexpr
        else return fraction_ceiling(value, value + 1);
    } // indicator_is_non_zero(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARITHMETIC_CORE_HPP_INCLUDED
