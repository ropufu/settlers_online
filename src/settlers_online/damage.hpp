
#ifndef ROPUFU_SETTLERS_ONLINE_DAMAGE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_DAMAGE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <aftermath/quiet_json.hpp>
#include <aftermath/not_an_error.hpp>

#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <ostream>    // std::ostream
#include <string>     // std::string

namespace ropufu::settlers_online
{
    namespace detail
    {
        /** Descriptor for unit types. */
        struct damage
        {
            using type = damage;
            // ~~ Json names ~~
            static constexpr char jstr_low[] = "low";
            static constexpr char jstr_high[] = "high";
            static constexpr char jstr_accuracy[] = "accuracy";
            static constexpr char jstr_splash_chance[] = "splash chance";

        private:

            bool m_is_quiet = false;    // Indicates if errors are to be pushed onto \c quiet_error when coercing occurs.
            std::size_t m_low = 0;      // Low damage.
            std::size_t m_high = 0;     // High damage.
            double m_accuracy = 0;      // Probability of high damage.
            double m_splash_chance = 0; // Probability of dealing splash damage.

            void coerce() noexcept
            {
                if (this->m_low > this->m_high)
                {
                    this->m_low = this->m_high;
                    if (!this->m_is_quiet) aftermath::quiet_error::instance().push(
                        aftermath::not_an_error::logic_error,
                        aftermath::severity_level::minor,
                        "Low damage exceeds high damage. Capped low damage to match high.", __FUNCTION__, __LINE__);
                }
                if (this->m_accuracy < 0 || this->m_accuracy > 1)
                {
                    if (this->m_accuracy < 0) this->m_accuracy = 0;
                    if (this->m_accuracy > 1) this->m_accuracy = 1;
                    if (!this->m_is_quiet) aftermath::quiet_error::instance().push(
                        aftermath::not_an_error::logic_error,
                        aftermath::severity_level::minor,
                        "Accuracy outside the interval [0, 1]. Accuracy coerced.", __FUNCTION__, __LINE__);
                }
                if (this->m_splash_chance < 0 || this->m_splash_chance > 1)
                {
                    if (this->m_splash_chance < 0) this->m_splash_chance = 0;
                    if (this->m_splash_chance > 1) this->m_splash_chance = 1;
                    if (!this->m_is_quiet) aftermath::quiet_error::instance().push(
                        aftermath::not_an_error::logic_error,
                        aftermath::severity_level::minor,
                        "Splash chance outside the interval [0, 1]. Splash chance coerced.", __FUNCTION__, __LINE__);
                }
            } // coerce(...)

        public:
            damage() noexcept { }

            /** @brief Offensive capabilities of the unit.
             *  @param is_quiet Indicates if errors are to be pushed onto \c quiet_error when coercing occurs.
             */
            explicit damage(bool is_quiet) noexcept : m_is_quiet(is_quiet) { }

            /** @brief Offensive capabilities of the unit.
             *  @param low Low damage.
             *  @param high High damage.
             *  @param accuracy Probability of dealing high damage.
             *  @param splash_chance Probability of dealing splash damage.
             *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p low exceeds \p high.
             *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p accuracy is outside the interval [0, 1].
             *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p splash_chance is outside the interval [0, 1].
             */
            damage(std::size_t low, std::size_t high, double accuracy, double splash_chance) noexcept
                : m_low(low), m_high(high), m_accuracy(accuracy), m_splash_chance(splash_chance)
            {
                this->coerce();
            } // damage(...)

            /** Indicates if errors are to be pushed onto \c quiet_error when coercing occurs. */
            bool is_quiet() const noexcept { return this->m_is_quiet; }
            /** Indicates if errors are to be pushed onto \c quiet_error when coercing occurs. */
            void set_is_quiet(bool value) noexcept { this->m_is_quiet = value; }

            /** @brief Offensive capabilities of the unit.
             *  @param low Low damage.
             *  @param high High damage.
             *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p low exceeds \p high.
             */
            void reset(std::size_t low, std::size_t high) noexcept
            {
                this->m_low = low;
                this->m_high = high;
                this->coerce();
            } // reset(...)

            /** @brief Offensive capabilities of the unit.
             *  @param low Low damage.
             *  @param high High damage.
             *  @param accuracy Probability of dealing high damage.
             *  @param splash_chance Probability of dealing splash damage.
             *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p low exceeds \p high.
             */
            void reset(std::size_t low, std::size_t high, double accuracy, double splash_chance) noexcept
            {
                this->m_low = low;
                this->m_high = high;
                this->m_accuracy = accuracy;
                this->m_splash_chance = splash_chance;
                this->coerce();
            } // reset(...)
            
            /** Low damage. */
            std::size_t low() const noexcept { return this->m_low; }
            /** Low damage. */
            void set_low(std::size_t value) noexcept { this->m_low = value; this->coerce(); }
            /** High damage. */
            std::size_t high() const noexcept { return this->m_high; }
            /** High damage. */
            void set_high(std::size_t value) noexcept { this->m_high = value; this->coerce(); }
            /** Probability of dealing high damage. */
            double accuracy() const noexcept { return this->m_accuracy; }
            /** Probability of dealing high damage. */
            void set_accuracy(double value) noexcept { this->m_accuracy = value; this->coerce(); }
            /** Probability of dealing splash damage. */
            double splash_chance() const noexcept { return this->m_splash_chance; }
            /** Probability of dealing splash damage. */
            void set_splash_chance(double value) noexcept { this->m_splash_chance = value; this->coerce(); }
            
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

            /** Elementwise addition. */
            type& operator +=(const type& other) noexcept
            {
                this->m_low += other.m_low;
                this->m_high += other.m_high;
                this->m_accuracy += other.m_accuracy;
                this->m_splash_chance += other.m_splash_chance;
                this->coerce();
                return *this;
            } // operator +=(...)

            /** Elementwise addition. */
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
    
        void from_json(const nlohmann::json& j, damage& x) noexcept
        {
            ropufu::aftermath::quiet_json q(j);
            using type = damage;

            // Populate default values.
            std::size_t low = x.low();
            std::size_t high = x.high();
            double accuracy = x.accuracy();
            double splash_chance = x.splash_chance();

            // Parse json entries.
            q.optional(type::jstr_low, low);
            q.optional(type::jstr_high, high);
            q.optional(type::jstr_accuracy, accuracy);
            q.optional(type::jstr_splash_chance, splash_chance);
            
            // Reconstruct the object.
            if (!q.good())
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::major, 
                    q.message(), __FUNCTION__, __LINE__);
                return;
            } // if (...)
            x.reset(low, high, accuracy, splash_chance);
        } // from_json(...)
    } // namespace detail
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::detail::damage>
    {
        using argument_type = ropufu::settlers_online::detail::damage;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = { };
            std::hash<double> double_hash = { };

            return
                size_hash(x.low()) ^
                size_hash(x.high()) ^
                double_hash(x.accuracy()) ^
                double_hash(x.splash_chance());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_DAMAGE_HPP_INCLUDED
