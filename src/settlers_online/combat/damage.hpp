
#ifndef ROPUFU_SETTLERS_ONLINE_DAMAGE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_DAMAGE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <ropufu/algebra.hpp> // aftermath::algebra::fraction

#include <cstddef>    // std::size_t, std::nullptr_t
#include <functional> // std::hash
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    struct unit_type;

    namespace detail
    {
        /** @warning Please use responsibly: no checks are made on \c indicator_high which should be either 0 or 1. */
        struct overshoot_damage
        {
            using type = overshoot_damage;
            using fraction_type = aftermath::algebra::fraction<std::size_t>;

            std::size_t low;
            std::size_t high;
            std::size_t indicator_high;

            bool empty() const noexcept { return this->total() == 0; }

            std::size_t total() const noexcept { return this->low + this->high; }

            void clear() noexcept
            {
                this->low = 0;
                this->high = 0;
            } // clear(...)

            void adjust(const fraction_type& low_factor, const fraction_type& high_factor) noexcept
            {
                this->low = this->low * low_factor.numerator() / low_factor.denominator();
                this->high = this->high * high_factor.numerator() / high_factor.denominator();
            } // adjust(...)

            /** @warning No underflow checks are performed. */
            type& operator -=(std::size_t value) noexcept
            {
                this->low -= (1 - this->indicator_high) * value;
                this->high -= (this->indicator_high) * value;
                return *this;
            } // operator -=(...)

            /** @warning No overflow checks are performed. */
            type& operator +=(std::size_t value) noexcept
            {
                this->low += (1 - this->indicator_high) * value;
                this->high += (this->indicator_high) * value;
                return *this;
            } // operator +=(...)

            friend type operator -(type left, std::size_t right) noexcept { left -= right; return left; }
            friend type operator +(type left, std::size_t right) noexcept { left += right; return left; }
        }; // struct damage_pair
    } // namespace detail

    /** Descriptor for unit types. */
    struct damage;

    void to_json(nlohmann::json& j, const damage& x) noexcept;
    void from_json(const nlohmann::json& j, damage& x);

    /** Descriptor for unit types. */
    struct damage
    {
        using type = damage;
        // ~~ Json names ~~
        static constexpr char jstr_low[] = "low";
        static constexpr char jstr_high[] = "high";
        static constexpr char jstr_accuracy[] = "accuracy";
        static constexpr char jstr_splash_chance[] = "splash chance";

        friend unit_type;

    private:
        std::size_t m_low = 0;
        std::size_t m_high = 0;
        double m_accuracy = 0;      // Probability of high damage.
        double m_splash_chance = 0; // Probability of dealing splash damage.

        bool validate(std::error_code& ec) const noexcept
        {
            if (this->m_accuracy < 0 || this->m_accuracy > 1) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Accuracy must be between zero and one.", false);
            if (this->m_splash_chance < 0 || this->m_splash_chance > 1) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Splash chance must be between zero and one.", false);
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            if (this->m_accuracy < 0) this->m_accuracy = 0;
            if (this->m_accuracy > 1) this->m_accuracy = 1;
            if (this->m_splash_chance < 0) this->m_splash_chance = 0;
            if (this->m_splash_chance > 1) this->m_splash_chance = 1;
        } // coerce(...)

        /** Unchecked constructor. */
        damage(std::nullptr_t, std::size_t low, std::size_t high, double accuracy, double splash_chance) noexcept
            : m_low(low), m_high(high), m_accuracy(accuracy), m_splash_chance(splash_chance)
        {
        } // damage(...)

    public:
        /** @brief Offensive capabilities of the unit. */
        damage() noexcept { }

        /** @brief Offensive capabilities of the unit.
         *  @param low Low damage.
         *  @param high High damage.
         *  @param accuracy Probability of dealing high damage.
         *  @param splash_chance Probability of dealing splash damage.
         *  @param ec Set to std::errc::invalid_argument if \p accuracy is outside the interval [0, 1].
         *  @param ec Set to std::errc::invalid_argument if \p splash_chance is outside the interval [0, 1].
         */
        damage(std::size_t low, std::size_t high, double accuracy, double splash_chance, std::error_code& ec) noexcept
            : m_low(low), m_high(high), m_accuracy(accuracy), m_splash_chance(splash_chance)
        {
            if (!this->validate(ec)) this->coerce();
        } // damage(...)

        damage(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            // Parse json entries.
            aftermath::noexcept_json::optional(j, type::jstr_low, this->m_low, ec);
            aftermath::noexcept_json::optional(j, type::jstr_high, this->m_high, ec);
            aftermath::noexcept_json::optional(j, type::jstr_accuracy, this->m_accuracy, ec);
            aftermath::noexcept_json::optional(j, type::jstr_splash_chance, this->m_splash_chance, ec);

            if (!this->validate(ec)) this->coerce();
        } // damage(...)

        /** @brief Offensive capabilities of the unit.
         *  @param low Low damage.
         *  @param high High damage.
         */
        void reset(std::size_t low, std::size_t high) noexcept
        {
            this->m_low = low;
            this->m_high = high;
        } // reset(...)

        /** Low damage. */
        std::size_t low() const noexcept { return this->m_low; }
        /** Low damage. */
        void set_low(std::size_t value) noexcept { this->m_low = value; }

        /** High damage. */
        std::size_t high() const noexcept { return this->m_high; }
        /** High damage. */
        void set_high(std::size_t value) noexcept { this->m_high = value; }

        /** Probability of dealing high damage. */
        double accuracy() const noexcept { return this->m_accuracy; }
        /** Probability of dealing high damage. */
        void set_accuracy(double value, std::error_code& ec) noexcept
        {
            this->m_accuracy = value;
            if (!this->validate(ec)) this->coerce();
        } // set_accuracy(...)

        /** Probability of dealing splash damage. */
        double splash_chance() const noexcept { return this->m_splash_chance; }
        /** Probability of dealing splash damage. */
        void set_splash_chance(double value, std::error_code& ec) noexcept
        {
            this->m_splash_chance = value;
            if (!this->validate(ec)) this->coerce();
        } // set_accuracy(...)
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_low == other.m_low &&
                this->m_high == other.m_high &&
                this->m_accuracy == other.m_accuracy &&
                this->m_splash_chance == other.m_splash_chance;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        /** Component-wise addition. */
        type& operator +=(const type& other) noexcept
        {
            this->m_low += other.m_low;
            this->m_high += other.m_high;
            this->m_accuracy += other.m_accuracy;
            this->m_splash_chance += other.m_splash_chance;
            this->coerce();
            return *this;
        } // operator +=(...)

        /** Component-wise addition. */
        type& operator -=(const type& other) noexcept
        {
            this->m_low = (this->m_low < other.m_low) ? 0 : (this->m_low - other.m_low);
            this->m_high = (this->m_high < other.m_high) ? 0 : (this->m_high - other.m_high);
            this->m_accuracy -= other.m_accuracy;
            this->m_splash_chance -= other.m_splash_chance;
            this->coerce();
            return *this;
        } // operator -=(...)

        /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
        friend type operator +(type left, const type& right) noexcept { left += right; return left; }
        friend type operator -(type left, const type& right) noexcept { left -= right; return left; }

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct damage

    // ~~ Json name definitions ~~
    constexpr char damage::jstr_low[];
    constexpr char damage::jstr_high[];
    constexpr char damage::jstr_accuracy[];
    constexpr char damage::jstr_splash_chance[];

    void to_json(nlohmann::json& j, const damage& x) noexcept
    {
        using type = damage;

        j = nlohmann::json{
            {type::jstr_low, x.low()},
            {type::jstr_high, x.high()},
            {type::jstr_accuracy, x.accuracy()},
            {type::jstr_splash_chance, x.splash_chance()}
        };
    } // to_json(...)

    void from_json(const nlohmann::json& j, damage& x)
    {
        using type = damage;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::damage>
    {
        using argument_type = ropufu::settlers_online::damage;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = {};
            std::hash<double> double_hash = {};

            return
                size_hash(x.low()) ^
                size_hash(x.high()) ^
                double_hash(x.accuracy()) ^
                double_hash(x.splash_chance());
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_DAMAGE_HPP_INCLUDED
