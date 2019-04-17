
#ifndef ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include "../algebra.hpp"

#include <cstddef>      // std::size_t
#include <functional>   // std::hash
#include <ostream>      // std::ostream
#include <stdexcept>    // std::runtime_error
#include <string>       // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>       // std::vector

namespace ropufu::settlers_online
{
    /** Descriptor for army defence. */
    struct camp;

    void to_json(nlohmann::json& j, const camp& x) noexcept;
    void from_json(const nlohmann::json& j, camp& x);

    /** Descriptor for army defence. */
    struct camp
    {
        using type = camp;
        using damage_percentage_type = typename damage_bonus_type::percentage_type;

        // ~~ Json names ~~
        static constexpr char jstr_names[] = "names";
        static constexpr char jstr_hit_points[] = "hit points";
        static constexpr char jstr_damage_reduction[] = "damage reduction";

    private:
        std::string m_display_name = "??";
        std::vector<std::string> m_names = {}; // Names.
        std::size_t m_hit_points = 0;  // Hit points.
        damage_percentage_type m_damage_reduction = {}; // Damage factor.

        bool validate(std::error_code& ec) const noexcept
        {
            if (this->m_damage_reduction.numerator() < 0 || this->m_damage_reduction.numerator() > 100) 
                return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Damage reduction must be between 0% and 100%.", false);
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            if (this->m_damage_reduction.numerator() < 0) this->m_damage_reduction.set_numerator(0);
            if (this->m_damage_reduction.numerator() > 100) this->m_damage_reduction.set_numerator(100);
            this->m_display_name = this->m_names.empty() ? "??" : this->m_names.front();
            this->m_names.shrink_to_fit();
        } // coerce(...)

    public:
        /** @brief Defensive capabilities of the army. */
        camp() noexcept { }

        /** @brief Defensive capabilities of the army.
         *  @param hit_points Hit points of the camp.
         */
        explicit camp(std::size_t hit_points) noexcept
            : m_hit_points(hit_points)
        {
        } // camp(...)

        /** @brief Defensive capabilities of the army.
         *  @param hit_points Hit points of the camp.
         *  @param damage_reduction Damage reduction for units that have \c tower_bonus.
         *  @param ec Set to std::errc::invalid_argument if \p damage_reduction is outside the interval [0, 1].
         */
        camp(std::size_t hit_points, const damage_percentage_type& damage_reduction, std::error_code& ec) noexcept
            : m_hit_points(hit_points), m_damage_reduction(damage_reduction)
        {
            this->validate(ec);
            this->coerce();
        } // camp(...)

        camp(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            // Special case.
            if (j.is_string())
            {
                std::string value = j.get<std::string>();
                if (!(value == "none" || value == "empty"))
                    aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, "Camp literal ot recognized.");
                return;
            } // if (...)
            
            // Parse json entries.
            aftermath::noexcept_json::optional(j, type::jstr_names, this->m_names, ec);
            aftermath::noexcept_json::optional(j, type::jstr_hit_points, this->m_hit_points, ec);
            aftermath::noexcept_json::optional(j, type::jstr_damage_reduction, this->m_damage_reduction, ec);

            this->validate(ec);
            this->coerce();
        } // camp(...)
        
        /** Hit points of the camp. */
        std::size_t hit_points() const noexcept { return this->m_hit_points; }
        /** Hit points of the camp. */
        void set_hit_points(std::size_t value) noexcept { this->m_hit_points = value; }

        /** Damage reduction for units that have \c tower_bonus. */
        const damage_percentage_type& damage_reduction() const noexcept { return this->m_damage_reduction; }
        /** Damage reduction for units that have \c tower_bonus. */
        void set_damage_reduction(const damage_percentage_type& value, std::error_code& ec) noexcept
        {
            this->m_damage_reduction = value;
            if (!this->validate(ec)) this->coerce();
        } // set_damage_reduction(...)

        /** Names of the unit type. */
        const std::vector<std::string>& names() const noexcept { return this->m_names; }
        /** Display name of the unit type. */
        const std::string& first_name() const noexcept { return this->m_display_name; }
        /** Names of the unit type. */
        void set_names(const std::vector<std::string>& value) noexcept
        {
            this->m_names = value;
            this->coerce();
        } // set_names(...)
        /** Name of the unit type. */
        void set_name(const std::string& value) noexcept
        {
            this->m_names.clear();
            this->m_names.reserve(1);
            this->m_names.push_back(value);
            this->m_names.shrink_to_fit();
            this->coerce();
        } // set_names(...)
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                //this->m_names == other.m_names &&
                this->m_hit_points == other.m_hit_points &&
                this->m_damage_reduction == other.m_damage_reduction;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct camp

    // ~~ Json name definitions ~~
    constexpr char camp::jstr_names[];
    constexpr char camp::jstr_hit_points[];
    constexpr char camp::jstr_damage_reduction[];
    
    void to_json(nlohmann::json& j, const camp& x) noexcept
    {
        using type = camp;

        j = nlohmann::json{
            {type::jstr_names, x.names()},
            {type::jstr_hit_points, x.hit_points()},
            {type::jstr_damage_reduction, x.damage_reduction()}
        };
    } // to_json(...)

    void from_json(const nlohmann::json& j, camp& x)
    {
        using type = camp;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::camp>
    {
        using argument_type = ropufu::settlers_online::camp;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = {};
            std::hash<typename ropufu::settlers_online::camp::damage_percentage_type> damage_reduction_hash = {};

            return
                size_hash(x.hit_points()) ^
                damage_reduction_hash(x.damage_reduction());
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_CAMP_HPP_INCLUDED
