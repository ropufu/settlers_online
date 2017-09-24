
#ifndef ROPUFU_SETTLERS_ONLINE_TYPEDEF_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TYPEDEF_HPP_INCLUDED

//#include <aftermath/algebra/fraction.hpp>

//#include <bitset>
#include <cstddef>
#include <cstdint>

namespace ropufu
{
	namespace settlers_online
	{
        //typedef aftermath::algebra::fraction<std::size_t> non_integer_type;
		//typedef std::bitset<max_size> mask_type;
		typedef std::uint_fast64_t mask_type;

        /** Divides <numerator> by <denominator> and rounds down. Both arguments should be positive. */
        template <typename t_integer_type>
        static t_integer_type fraction_floor(t_integer_type numerator, t_integer_type denominator) noexcept
        {
            return numerator / denominator;
        }

        /** Divides <numerator> by <denominator> and rounds up. Both arguments should be positive. */
        template <typename t_integer_type>
        static t_integer_type fraction_ceiling(t_integer_type numerator, t_integer_type denominator) noexcept
        {
            return (numerator + denominator - 1) / denominator;
        }

        /** Multiplies <value> by <factor> and rounds down. Both arguments should be positive. */
        template <typename t_integer_type>
        static t_integer_type product_floor(t_integer_type value, double factor) noexcept
        {
            return static_cast<t_integer_type>(value * factor); // Rounds toward zero (equivalent to floor with positive numbers).
        }

        /** Multiplies <value> by <factor> and rounds up. Both arguments should be positive. */
        template <typename t_integer_type, t_integer_type t_factor_upper_bound = 10>
        static t_integer_type product_ceiling(t_integer_type value, double factor) noexcept
        {
            //static constexpr t_integer_type factor_upper_bound = 10;
            return (t_factor_upper_bound * value) - static_cast<t_integer_type>(value * (t_factor_upper_bound - factor)); // Rounds away from zero (equivalent to ceiling with positive numbers).
        }
	}
}

#endif // ROPUFU_SETTLERS_ONLINE_TYPEDEF_HPP_INCLUDED
