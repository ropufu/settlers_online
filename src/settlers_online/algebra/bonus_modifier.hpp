
#ifndef ROPUFU_SETTLERS_ONLINE_ALGEBRA_BONUS_MODIFIER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ALGEBRA_BONUS_MODIFIER_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include "percentage.hpp"

#include <cstddef>      // std::size_t
#include <limits>       // std::numeric_limits<...>::is_integer
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::make_signed_t

namespace ropufu::settlers_online
{
    struct unit_type;
    
    template <typename t_integer_type, cast_direction t_direction>
    struct bonus_modifier;

    template <typename t_integer_type, cast_direction t_direction>
    struct bonus_modifier
    {
        using type = bonus_modifier<t_integer_type, t_direction>;
        using integer_type = t_integer_type;
        using percentage_type = percentage<integer_type, t_direction>;

        static constexpr cast_direction direction = t_direction;

        friend unit_type;

    private:
        integer_type m_additive_bonus = 0; // Additive bonus (in absolute value).
        percentage_type m_rate_bonus = {}; // Multiplicative modifier (in percent). Rates themselves stack additively: two 10% bonuses will result in 20% rather than 21% = (1 + 10%)(1 + 10%) - 100%.

        bool validate(std::error_code& ec) const noexcept
        {
            if (this->m_rate_bonus.numerator() < -100) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Rate modifier cannot be less than -100%.", false);
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            if (this->m_rate_bonus.numerator() < -100) this->m_rate_bonus.set_numerator(-100);
        } // coerce(...)

    public:
        constexpr bonus_modifier() noexcept { }

        void append_additive(integer_type modifier) noexcept
        {
            this->m_additive_bonus += modifier;
        } // append_additive(...)

        /** @remark \p modifier should be a percentage. */
        void append_rate(const percentage_type& modifier) noexcept
        {
            this->m_rate_bonus += modifier;
        } // append_additive(...)

        template <typename t_other_integer_type>
        t_other_integer_type apply_to(t_other_integer_type value) const noexcept
        {
            static_assert(std::numeric_limits<integer_type>::is_integer, "Base value must be integer-valued.");
            integer_type base_value = static_cast<integer_type>(value);
            // Multiplicative bonus: affects base hit points.
            integer_type bonus = this->m_rate_bonus.of(base_value);
            // Additive bonus comes next.
            bonus += this->m_additive_bonus;
            // Apply the bonus to base hit points.
            base_value += bonus;
            return static_cast<t_other_integer_type>(base_value);
        } // modify(...)
    }; // struct bonus_modifier
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ALGEBRA_BONUS_MODIFIER_HPP_INCLUDED
