
#ifndef ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>

#include <nlohmann/json.hpp>

#include <cstddef> // std::size_t
#include <functional> // std::hash
#include <ostream> // std::ostream
#include <string> // std::string

namespace ropufu
{
    namespace settlers_online
    {
        namespace detail
        {
            /** Descriptor for army defence. */
            struct camp
            {
                using type = camp;

            private:
                bool m_is_quiet = false;       // Indicates if errors are to be pushed onto \c quiet_error when coercing occurs.
                std::size_t m_hit_points = 0;  // Hit points.
                double m_damage_reduction = 0; // Damage reduction.

                void coerce() noexcept
                {
                    if (this->m_damage_reduction < 0 || this->m_damage_reduction > 1)
                    {
                        if (this->m_damage_reduction < 0) this->m_damage_reduction = 0;
                        if (this->m_damage_reduction > 1) this->m_damage_reduction = 1;
                        if (!this->m_is_quiet) aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::logic_error,
                            aftermath::severity_level::minor,
                            "Damage reduction outside the interval [0, 1]. Damage reduction coerced.", __FUNCTION__, __LINE__);
                    }
                } // coerce(...)

            public:
                camp() noexcept { }

                /** @brief Defensive capabilities of the army.
                 *  @param is_quiet Indicates if errors are to be pushed onto \c quiet_error when coercing occurs.
                 */
                explicit camp(bool is_quiet) noexcept : m_is_quiet(is_quiet) { }

                /** @brief Defensive capabilities of the army.
                 *  @param hit_points Hit points of the camp.
                 *  @param damage_reduction Damage reduction for units that have \c tower_bonus.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p damage_reduction is outside the interval [0, 1].
                 */
                camp(std::size_t hit_points, double damage_reduction = 0) noexcept
                    : m_hit_points(hit_points), m_damage_reduction(damage_reduction)
                {
                    this->coerce();
                } // camp(...)

                /** Indicates if errors are to be pushed onto \c quiet_error when coercing occurs. */
                bool is_quiet() const noexcept { return this->m_is_quiet; }
                /** Indicates if errors are to be pushed onto \c quiet_error when coercing occurs. */
                void set_is_quiet(bool value) noexcept { this->m_is_quiet = value; }

                /** @brief Defensive capabilities of the army.
                 *  @param hit_points Hit points of the camp.
                 */
                void reset(std::size_t hit_points) noexcept
                {
                    this->m_hit_points = hit_points;
                    this->coerce();
                } // reset(...)

                /** @brief Defensive capabilities of the army.
                 *  @param hit_points Hit points of the camp.
                 *  @param damage_reduction Damage reduction for units that have \c tower_bonus.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p damage_reduction is outside the interval [0, 1].
                 */
                void reset(std::size_t hit_points, double damage_reduction) noexcept
                {
                    this->m_hit_points = hit_points;
                    this->m_damage_reduction = damage_reduction;
                    this->coerce();
                } // reset(...)
                
                /** Hit points of the camp. */
                std::size_t hit_points() const noexcept { return this->m_hit_points; }
                /** Hit points of the camp. */
                void set_hit_points(std::size_t value) noexcept { this->m_hit_points = value; }
                /** Damage reduction for units that have \c tower_bonus. */
                double damage_reduction() const noexcept { return this->m_damage_reduction; }
                /** Damage reduction for units that have \c tower_bonus. */
                void set_damage_reduction(double value) noexcept { this->m_damage_reduction = value; this->coerce(); }
                
                /** Checks two types for equality. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_hit_points == other.m_hit_points &&
                        this->m_damage_reduction == other.m_damage_reduction;
                } // operator ==(...)
    
                /** Checks two types for inequality. */
                bool operator !=(const type& other) const noexcept
                {
                    return !(this->operator ==(other));
                } // operator !=(...)

                /** Elementwise addition. */
                type& operator +=(const type& other) noexcept
                {
                    this->m_hit_points += other.m_hit_points;
                    this->m_damage_reduction += other.m_damage_reduction;
                    this->coerce();
                    return *this;
                } // operator +=(...)

                /** Elementwise addition. */
                type& operator -=(const type& other) noexcept
                {
                    this->m_hit_points = (this->m_hit_points < other.m_hit_points) ? 0 : (this->m_hit_points - other.m_hit_points);
                    this->m_damage_reduction -= other.m_damage_reduction;
                    this->coerce();
                    return *this;
                } // operator -=(...)

                /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
                friend type operator +(type left, const type& right) noexcept { left += right; return left; }
                friend type operator -(type left, const type& right) noexcept { left -= right; return left; }

                friend std::ostream& operator <<(std::ostream& os, const type& self)
                {
                    nlohmann::json j = self;
                    return os << j;
                }
            }; // struct camp
            
            void to_json(nlohmann::json& j, const camp& x)
            {
                j = nlohmann::json{
                    {"hit points", x.hit_points()},
                    {"damage reduction", x.damage_reduction()}
                };
            }
        
            void from_json(const nlohmann::json& j, camp& x)
            {
                if (j.is_string())
                {
                    std::string value = j;
                    if (!(value == "none" || value == "empty"))
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::major,
                            "Camp not recognized", __FUNCTION__, __LINE__);
                    }
                    return;
                }

                if (j.count("hit points") != 0) x.set_hit_points(j["hit points"].get<std::size_t>());
                if (j.count("damage reduction") != 0) x.set_damage_reduction(j["damage reduction"].get<double>());

            } // from_json(...)
        } // namespace detail
    } // namespace settlers_online
} // namespace ropufu

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::detail::camp>
    {
        using argument_type = ropufu::settlers_online::detail::camp;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = { };
            std::hash<double> double_hash = { };

            return
                size_hash(x.hit_points()) ^
                double_hash(x.damage_reduction());
        }
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED
