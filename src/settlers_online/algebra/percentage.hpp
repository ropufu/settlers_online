
#ifndef ROPUFU_SETTLERS_ONLINE_ALGEBRA_PERCENTAGE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ALGEBRA_PERCENTAGE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include "arithmetic.hpp"

#include <cstddef>      // std::size_t
#include <cmath>        // std::round
#include <functional>   // std::hash
#include <limits>       // std::numeric_limits<...>::is_integer
#include <stdexcept>    // std::runtime_error
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::make_signed_t

namespace ropufu::settlers_online
{
    enum struct cast_direction : bool
    {
        toward_zero = false,
        away_from_zero = true
    }; // enum struct cast_direction

    template <typename t_integer_type, cast_direction t_direction>
    struct percentage;

    template <typename t_integer_type, cast_direction t_direction>
    void to_json(nlohmann::json& j, const percentage<t_integer_type, t_direction>& x) noexcept;
    template <typename t_integer_type, cast_direction t_direction>
    void from_json(const nlohmann::json& j, percentage<t_integer_type, t_direction>& x);

    template <typename t_integer_type, cast_direction t_direction>
    struct percentage
    {
        using type = percentage<t_integer_type, t_direction>;
        using integer_type = t_integer_type;

        static constexpr cast_direction direction = t_direction;

    private:
        integer_type m_numerator = 0;
        static constexpr integer_type denominator = 100;

        static constexpr void traits_check()
        {
            static_assert(std::numeric_limits<integer_type>::is_integer, "Percentage points must be integer-valued.");
        } // traits_check(...)

        /** Applies the percentage to \p base_value and rounds down to the nearest integer. */
        integer_type apply_and_floor(integer_type base_value) const noexcept
        {
            return fraction_floor(base_value * this->m_numerator, type::denominator);
        } // floor(...)

        /** Applies the percentage to \p base_value and rounds up to the nearest integer. */
        integer_type apply_and_ceiling(integer_type base_value) const noexcept
        {
            return fraction_ceiling(base_value * this->m_numerator, type::denominator);
        } // floor(...)

    public:
        constexpr percentage() noexcept { type::traits_check(); }

        explicit constexpr percentage(integer_type value) noexcept
            : m_numerator(value)
        {
            type::traits_check();
        } // percentage(...)

        percentage(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            type::traits_check();
            if (!j.is_number_float()) aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, "Explicit proportion expected.");
            else
            {
                double proportion = j.get<double>();
                this->m_numerator = static_cast<integer_type>(std::round(proportion * type::denominator));
            } // if (...)
        } // percentage(...)

        static type from_proportion(float proportion) noexcept
        {
            type result {};
            result.m_numerator = static_cast<integer_type>(std::round(proportion * type::denominator));
            return result;
        } // from_proportion(...)

        static type from_proportion(double proportion) noexcept
        {
            type result {};
            result.m_numerator = static_cast<integer_type>(std::round(proportion * type::denominator));
            return result;
        } // from_proportion(...)

        double to_double() const noexcept
        {
            return static_cast<double>(this->m_numerator) / static_cast<double>(type::denominator);
        } // to_double(...)

        float to_float() const noexcept
        {
            return static_cast<float>(this->m_numerator) / static_cast<float>(type::denominator);
        } // to_float(...)

        operator double() const noexcept { return this->to_double(); }

        operator float() const noexcept { return this->to_float(); }

        integer_type numerator() const noexcept { return this->m_numerator; }
        void set_numerator(integer_type value) noexcept { this->m_numerator = value; }

        /** Applies the percentage to \p base_value and rounds up to the nearest integer. */
        integer_type of(integer_type base_value) const noexcept
        {
            if constexpr (type::direction == cast_direction::away_from_zero) return this->apply_and_ceiling(base_value);
            if constexpr (type::direction == cast_direction::toward_zero) return this->apply_and_floor(base_value);
        } // floor(...)
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return this->m_numerator == other.m_numerator;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        /** Re-scaling. */
        type& operator *=(integer_type scalar) noexcept
        {
            this->m_numerator *= scalar;
            return *this;
        } // operator *=(...)

        /** Component-wise addition. */
        type& operator +=(const type& other) noexcept
        {
            this->m_numerator += other.m_numerator;
            return *this;
        } // operator +=(...)

        /** Component-wise addition. */
        type& operator -=(const type& other) noexcept
        {
            this->m_numerator -= other.m_numerator;
            return *this;
        } // operator -=(...)

        friend type operator *(type left, integer_type scalar) noexcept { left *= scalar; return left; }
        friend type operator *(integer_type scalar, type right) noexcept { right *= scalar; return right; }
        
        /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
        friend type operator +(type left, const type& right) noexcept { left += right; return left; }
        friend type operator -(type left, const type& right) noexcept { left -= right; return left; }
    }; // struct percentage
    
    template <typename t_integer_type, cast_direction t_direction>
    void to_json(nlohmann::json& j, const percentage<t_integer_type, t_direction>& x) noexcept
    {
        // using type = percentage<t_integer_type, t_direction>;

        j = x.to_double();
    } // to_json(...)

    template <typename t_integer_type, cast_direction t_direction>
    void from_json(const nlohmann::json& j, percentage<t_integer_type, t_direction>& x)
    {
        using type = percentage<t_integer_type, t_direction>;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <typename t_integer_type, ropufu::settlers_online::cast_direction t_direction>
    struct hash<ropufu::settlers_online::percentage<t_integer_type, t_direction>>
    {
        using argument_type = ropufu::settlers_online::percentage<t_integer_type, t_direction>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<t_integer_type> numerator_hash = {};

            return numerator_hash(x.numerator());
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_ALGEBRA_PERCENTAGE_HPP_INCLUDED
