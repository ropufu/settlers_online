
#ifndef ROPUFU_SETTLERS_ONLINE_BONUS_MODIFIER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BONUS_MODIFIER_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include "arithmetic.hpp"

#include <cstddef>      // std::size_t
#include <cstdint>      // std::size_t
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::make_signed_t

namespace ropufu::settlers_online
{
    struct unit_type;

    namespace detail
    {
        struct bonus_modifier
        {
            using type = bonus_modifier;
            using signed_size_t = std::make_signed_t<std::size_t>;
            using modifier_type = signed_size_t;
            static constexpr signed_size_t hundred = 100;

            friend unit_type;

        private:
            signed_size_t m_additive_bonus = 0; // Additive bonus (in absolute value).
            signed_size_t m_rate_bonus = 0; // Multiplicative modifier (in percent). Rates themselves stack additively: two 10% bonuses will result in 20% rather than 21% = (1 + 10%)(1 + 10%) - 100%.

            bool validate(std::error_code& ec) const noexcept
            {
                if (this->m_rate_bonus < -100) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Rate modifier cannot be less than -100%.", false);
                return true;
            } // validate(...)

            void coerce() noexcept
            {
                if (this->m_rate_bonus < -100) this->m_rate_bonus = -100;
            } // coerce(...)

        public:
            bonus_modifier() noexcept { }

            void append_additive(signed_size_t modifier) noexcept
            {
                this->m_additive_bonus += modifier;
            } // append_additive(...)

            /** @remark \p modifier should be a percentage. */
            void append_rate(signed_size_t modifier) noexcept
            {
                this->m_rate_bonus += modifier;
            } // append_additive(...)

            //template <bool t_do_check_overflow = false>
            std::size_t apply_to(std::size_t value) const noexcept
            {
                signed_size_t base_value = static_cast<signed_size_t>(value);
                // Multiplicative bonus: affects base hit points.
                signed_size_t bonus = fraction_floor(base_value * this->m_rate_bonus, type::hundred);
                // Additive bonus comes next.
                bonus += this->m_additive_bonus;
                // Apply the bonus to base hit points.
                base_value += bonus;
                // if constexpr (t_do_check_overflow)
                // {
                //     if (base_value < 0) base_value = 0;
                // } // if constexpr (...)
                return static_cast<std::size_t>(base_value);
            } // modify(...)
        }; // struct bonus_modifier
    } // namespace detail
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_SETTLERS_ONLINE_BONUS_MODIFIER_HPP_INCLUDED
