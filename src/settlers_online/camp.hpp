
#ifndef ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>

#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <ostream>    // std::ostream
#include <string>     // std::string

namespace ropufu::settlers_online
{
    namespace detail
    {
        /** Descriptor for army defence. */
        struct camp
        {
            using type = camp;
            // ~~ Json names ~~
            static constexpr char jstr_hit_points[] = "hit points";
            static constexpr char jstr_damage_reduction[] = "damage reduction";

        private:
            std::size_t m_hit_points = 0;  // Hit points.
            double m_damage_reduction = 0; // Damage reduction.

            void validate()
            {
                if (this->m_damage_reduction < 0 || this->m_damage_reduction > 1) throw std::logic_error("Damage reduction outside the interval [0, 1].");
            } // validate(...)

            void coerce() noexcept
            {
                if (this->m_damage_reduction < 0) this->m_damage_reduction = 0;
                if (this->m_damage_reduction > 1) this->m_damage_reduction = 1;
            } // coerce(...)

        public:
            /** @brief Defensive capabilities of the army. */
            camp() noexcept { }

            /** @brief Defensive capabilities of the army.
             *  @param hit_points Hit points of the camp.
             *  @param damage_reduction Damage reduction for units that have \c tower_bonus.
             *  @exception std::logic_error \p damage_reduction is outside the interval [0, 1].
             */
            camp(std::size_t hit_points, double damage_reduction = 0)
                : m_hit_points(hit_points), m_damage_reduction(damage_reduction)
            {
                this->validate();
            } // camp(...)

            /** @brief Defensive capabilities of the army.
             *  @param hit_points Hit points of the camp.
             *  @param damage_reduction Damage reduction for units that have \c tower_bonus.
             *  @exception std::logic_error \p damage_reduction is outside the interval [0, 1].
             */
            void reset(std::size_t hit_points, double damage_reduction)
            {
                this->m_hit_points = hit_points;
                this->m_damage_reduction = damage_reduction;
                this->validate();
            } // reset(...)
            
            /** Hit points of the camp. */
            std::size_t hit_points() const noexcept { return this->m_hit_points; }
            /** Hit points of the camp. */
            void set_hit_points(std::size_t value) noexcept { this->m_hit_points = value; }
            /** Damage reduction for units that have \c tower_bonus. */
            double damage_reduction() const noexcept { return this->m_damage_reduction; }
            /** Damage reduction for units that have \c tower_bonus. */
            void set_damage_reduction(double value)
            {
                this->m_damage_reduction = value; 
                this->validate();
            } // set_damage_reduction(...)
            
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

            friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
            {
                nlohmann::json j = self;
                return os << j;
            } // operator <<(...)
        }; // struct camp

        // ~~ Json name definitions ~~
        constexpr char camp::jstr_hit_points[];
        constexpr char camp::jstr_damage_reduction[];
        
        void to_json(nlohmann::json& j, const camp& x) noexcept
        {
            using type = camp;

            j = nlohmann::json{
                {type::jstr_hit_points, x.hit_points()},
                {type::jstr_damage_reduction, x.damage_reduction()}
            };
        } // to_json(...)
    
        void from_json(const nlohmann::json& j, camp& x)
        {
            using type = camp;

            // Special case.
            if (j.is_string())
            {
                std::string value = j;
                if (!(value == "none" || value == "empty")) throw std::runtime_error("Camp not recognized");
                return;
            } // if (...)

            // Populate default values.
            std::size_t hit_points = x.hit_points();
            double damage_reduction = x.damage_reduction();

            // Parse json entries.
            if (j.count(type::jstr_hit_points) > 0) hit_points = j[type::jstr_hit_points];
            if (j.count(type::jstr_hit_points) > 0) damage_reduction = j[type::jstr_damage_reduction];
            
            // Reconstruct the object.
            x.set_hit_points(hit_points);
            x.set_damage_reduction(damage_reduction);
        } // from_json(...)
    } // namespace detail
} // namespace ropufu::settlers_online

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
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED
