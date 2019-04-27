
#ifndef ROPUFU_SETTLERS_ONLINE_BUILDING_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BUILDING_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <ropufu/enum_array.hpp>

#include "../algebra/blueprint_index.hpp"
#include "../algebra/blueprint_size.hpp"
#include "../char_string.hpp" // char_string::to_duration<...>
#include "../enums.hpp"

#include "blueprint.hpp"
#include "dimension.hpp"
#include "resource_pair.hpp"

#include <chrono>     // std::chrono::seconds
#include <cstddef>    // std::size_t
#include <cstdint>    // std::int_fast32_t
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    /** Descriptor for buildings. */
    struct building;

    void to_json(nlohmann::json& j, const building& x) noexcept;
    void from_json(const nlohmann::json& j, building& x);

    /** Descriptor for buildings. */
    struct building
    {
        using type = building;
        using footprint_matrix_type = typename footprint::matrix_type;

        // ~~ Json names ~~
        static constexpr char jstr_names[] = "names";
        static constexpr char jstr_category[] = "category";
        static constexpr char jstr_attributes[] = "attributes";
        static constexpr char jstr_dimensions[] = "dimensions";
        static constexpr char jstr_layout[] = "layout";
        static constexpr char jstr_cycle_time[] = "cycle time";
        static constexpr char jstr_production[] = "production";
        static constexpr char jstr_required_deposit[] = "required deposit";
        static constexpr char jstr_inner_deposit[] = "inner deposit";

    private:
        std::string m_display_name = "??";
        std::vector<std::string> m_names = {};
        building_category m_category = building_category::unknown;
        aftermath::flags_t<building_attribute> m_attributes = {};
        dimension m_dimensions = {};
        footprint m_layout = {};
        bool m_is_explicit_layout = false;
        // Non-regular (production) buildings.
        std::chrono::seconds m_cycle_time = {};
        std::vector<resource_pair> m_production = {};
        // Harvesters and mines.
        std::string m_required_deposit = ""; // For harvesters.
        resource_pair m_inner_deposit = {};  // For mines.

        bool validate(std::error_code& ec) const noexcept
        {
            if (!this->m_dimensions.validate(ec)) return false;
            if (this->m_dimensions.bounding_box().height != this->m_layout.face_height()) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Bounding box should be aligned with layout.", false);
            if (this->m_dimensions.bounding_box().width != this->m_layout.face_width()) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Bounding box should be aligned with layout.", false);
            if (this->m_cycle_time.count() < 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Cycle time cannot be negative.", false);
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            std::error_code ec {};
            this->m_display_name = this->m_names.empty() ? "??" : this->m_names.front();

            face_size bounding_box = this->m_dimensions.bounding_box();
            if (this->m_dimensions.bounding_box().height != this->m_layout.face_height()) bounding_box.height = this->m_layout.face_height();
            if (this->m_dimensions.bounding_box().width != this->m_layout.face_width()) bounding_box.width = this->m_layout.face_width();
            this->m_dimensions.set_bounding_box(bounding_box, ec);

            if (this->m_cycle_time.count() < 0) this->m_cycle_time = -this->m_cycle_time;

            // ~~ Optimize ~~
            this->m_names.shrink_to_fit();
            this->m_production.shrink_to_fit();
        } // coerce(...)

        void default_layout() noexcept
        {
            footprint_matrix_type& cells = this->m_layout.cells();
            cells.fill(true);

            std::size_t m = cells.height();
            std::size_t n = cells.width();

            if (m == 0 || n == 0) return;
            for (std::size_t j = 0; j < n; ++j)
            {
                cells(0, j) = false; // Top row.
                cells(m - 1, j) = false; // Bottom row.            
            } // for (...)
            for (std::size_t i = 1; i < m - 1; ++i)
            {
                cells(i, 0) = false; // Left column.
                cells(i, n - 1) = false; // Right column.  
            } // for (...)
        } // default_layout(...)

    public:
        building() noexcept { }

        /** @brief Solid rectangular building of a given size. */
        building(const std::string& name, const dimension& dimensions, std::error_code& ec) noexcept
            : m_names({ name }), m_dimensions(dimensions), m_layout(dimensions.bounding_box().height, dimensions.bounding_box().width)
        {
            this->default_layout();
            this->validate(ec);
            this->coerce();
        } // building(...)

        /** @brief Building with explicit layout. */
        building(const std::string& name, const dimension& dimensions, const footprint& layout, std::error_code& ec) noexcept
            : m_names({ name }), m_dimensions(dimensions), m_layout(layout), m_is_explicit_layout(true)
        {
            this->validate(ec);
            this->coerce();
        } // building(...)

        building(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            // Auxiliary for enum struct.
            std::string category_str = std::to_string(this->m_category);
            std::string cycle_time_str = "";
            this->m_is_explicit_layout = j.count(type::jstr_layout) != 0;

            // Parse json entries.
            aftermath::noexcept_json::required(j, type::jstr_names, this->m_names, ec);
            aftermath::noexcept_json::required(j, type::jstr_category, category_str, ec);
            aftermath::noexcept_json::required(j, type::jstr_dimensions, this->m_dimensions, ec);

            aftermath::noexcept_json::optional(j, type::jstr_attributes, this->m_attributes, ec);
            aftermath::noexcept_json::optional(j, type::jstr_layout, this->m_layout, ec);

            if (!this->m_is_explicit_layout) this->default_layout();
            if (!aftermath::detail::try_parse_enum(category_str, this->m_category))
                aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, std::string("Category unrecognized: ") + category_str + std::string("."));

            switch (this->m_category)
            {
                case building_category::factory:
                    aftermath::noexcept_json::required(j, type::jstr_cycle_time, cycle_time_str, ec);
                    aftermath::noexcept_json::required(j, type::jstr_production, this->m_production, ec);
                    this->m_cycle_time = char_string::template to_duration<std::chrono::seconds>(cycle_time_str, ec);
                    break;
                case building_category::harvester:
                    aftermath::noexcept_json::required(j, type::jstr_cycle_time, cycle_time_str, ec);
                    aftermath::noexcept_json::required(j, type::jstr_production, this->m_production, ec);
                    aftermath::noexcept_json::required(j, type::jstr_required_deposit, this->m_required_deposit, ec);
                    this->m_cycle_time = char_string::template to_duration<std::chrono::seconds>(cycle_time_str, ec);
                    break;
                case building_category::mine:
                    aftermath::noexcept_json::required(j, type::jstr_cycle_time, cycle_time_str, ec);
                    aftermath::noexcept_json::required(j, type::jstr_production, this->m_production, ec);
                    aftermath::noexcept_json::required(j, type::jstr_inner_deposit, this->m_inner_deposit, ec);
                    this->m_cycle_time = char_string::template to_duration<std::chrono::seconds>(cycle_time_str, ec);
                    break;
                case building_category::deposit:
                    aftermath::noexcept_json::required(j, type::jstr_inner_deposit, this->m_inner_deposit, ec);
                    break;
                default: break;
            } // switch (...)

            this->validate(ec);
            this->coerce();
        } // building(...)

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
            this->coerce();
        } // set_names(...)

        building_category category() const noexcept { return this->m_category; }
        const aftermath::flags_t<building_attribute>& attributes() const noexcept { return this->m_attributes; }
        const dimension& dimensions() const noexcept { return this->m_dimensions; }
        const footprint& layout() const noexcept { return this->m_layout; }
        bool explicit_layout() const noexcept { return this->m_is_explicit_layout; }
        const std::chrono::seconds& cycle_time() const noexcept { return this->m_cycle_time; }
        const std::vector<resource_pair>& production() const noexcept { return this->m_production; }
        const std::string& required_deposit() const noexcept { return this->m_required_deposit; }
        const resource_pair& inner_deposit() const noexcept { return this->m_inner_deposit; }

        void mark_as_factory(const std::chrono::seconds& cycle_time, std::error_code& ec) noexcept
        {
            if (this->m_category != building_category::unknown)
            {
                aftermath::detail::on_error(ec, std::errc::not_supported, "Building category cannot be changed.");
                return;
            } // if (...)
            this->m_category = building_category::factory;
            this->m_cycle_time = cycle_time;

            if (!this->validate(ec)) this->coerce();
        } // mark_as_factory(...)

        void mark_as_harvester(const std::chrono::seconds& cycle_time, const std::string& required_deposit, std::error_code& ec) noexcept
        {
            if (this->m_category != building_category::unknown)
            {
                aftermath::detail::on_error(ec, std::errc::not_supported, "Building category cannot be changed.");
                return;
            } // if (...)
            this->m_category = building_category::harvester;
            this->m_cycle_time = cycle_time;
            this->m_required_deposit = required_deposit;
            
            if (!this->validate(ec)) this->coerce();
        } // mark_as_harvester(...)

        void mark_as_mine(const std::chrono::seconds& cycle_time, const resource_pair& inner_deposit, std::error_code& ec) noexcept
        {
            if (this->m_category != building_category::unknown)
            {
                aftermath::detail::on_error(ec, std::errc::not_supported, "Building category cannot be changed.");
                return;
            } // if (...)
            this->m_category = building_category::mine;
            this->m_cycle_time = cycle_time;
            this->m_inner_deposit = inner_deposit;
            
            if (!this->validate(ec)) this->coerce();
        } // mark_as_mine(...)

        void mark_as_deposit(const resource_pair& inner_deposit, std::error_code& ec) noexcept
        {
            if (this->m_category != building_category::unknown)
            {
                aftermath::detail::on_error(ec, std::errc::not_supported, "Building category cannot be changed.");
                return;
            } // if (...)
            this->m_category = building_category::deposit;
            this->m_inner_deposit = inner_deposit;
        } // mark_as_mine(...)

        void add_production(const resource_pair& item, std::error_code& ec) noexcept
        {
            if (this->m_cycle_time.count() == 0)
            {
                aftermath::detail::on_error(ec, std::errc::not_supported, "Production not supported on this building.");
                return;
            } // if (...)
            this->m_production.push_back(item);
        } // add_production(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct building

    // ~~ Json name definitions ~~
    constexpr char building::jstr_names[];
    constexpr char building::jstr_category[];
    constexpr char building::jstr_attributes[];
    constexpr char building::jstr_dimensions[];
    constexpr char building::jstr_layout[];
    constexpr char building::jstr_cycle_time[];
    constexpr char building::jstr_production[];
    constexpr char building::jstr_required_deposit[];
    constexpr char building::jstr_inner_deposit[];

    void to_json(nlohmann::json& j, const building& x) noexcept
    {
        using type = building;

        std::vector<std::string> attributes {};

        for (building_attribute attribute : x.attributes()) attributes.push_back(std::to_string(attribute));

        j = nlohmann::json{
            {type::jstr_names, x.names()},
            {type::jstr_category, std::to_string(x.category())},
            {type::jstr_dimensions, x.dimensions()}
        };

        if (!attributes.empty()) j[type::jstr_attributes] = attributes;
        if (!x.production().empty())
        {
            std::string cycle_time = char_string::from_duration(x.cycle_time());

            j[type::jstr_cycle_time] = cycle_time;
            j[type::jstr_production] = x.production();
        } // if (...)
        if (!x.inner_deposit().empty()) j[type::jstr_inner_deposit] = x.inner_deposit();
        else if (!x.required_deposit().empty()) j[type::jstr_required_deposit] = x.required_deposit();

        if (x.explicit_layout()) j[type::jstr_layout] = x.layout();
    } // to_json(...)

    void from_json(const nlohmann::json& j, building& x)
    {
        using type = building;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BUILDING_HPP_INCLUDED
