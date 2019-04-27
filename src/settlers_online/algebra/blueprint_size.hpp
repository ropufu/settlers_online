
#ifndef ROPUFU_SETTLERS_ONLINE_ALGEBRA_BLUEPRINT_SIZE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ALGEBRA_BLUEPRINT_SIZE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include "../enums/blueprint_cell.hpp"
#include "../json_tuples.hpp"

#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <functional>   // std::hash
#include <ostream>      // std::ostream
#include <stdexcept>    // std::runtime_error
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    template <blueprint_cell t_cell>
    struct blueprint_size;

    using face_size = blueprint_size<blueprint_cell::face>;
    using vertex_size = blueprint_size<blueprint_cell::vertex>;
    using horizontal_edge_size = blueprint_size<blueprint_cell::horizontal_edge>;
    using vertical_edge_size = blueprint_size<blueprint_cell::vertical_edge>;

    template <blueprint_cell t_cell>
    void to_json(nlohmann::json& j, const blueprint_size<t_cell>& x) noexcept;
    template <blueprint_cell t_cell>
    void from_json(const nlohmann::json& j, blueprint_size<t_cell>& x);

    template <blueprint_cell t_cell>
    struct blueprint_size
    {
        using type = blueprint_size<t_cell>;
        static constexpr blueprint_cell cell = t_cell;
        
        std::size_t height = 0; // Number of rows.
        std::size_t width = 0; // Number of columns.

        blueprint_size() noexcept { }

        blueprint_size(std::size_t count_rows, std::size_t count_columns) noexcept
            : height(count_rows), width(count_columns)
        {
        } // blueprint_size(...)

        blueprint_size(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            std::array<std::size_t, 2> dims {};
            detail::read_size_tuple(j, dims, ec);
            if (ec.value() == 0)
            {
                this->height = dims[0];
                this->width = dims[1];
            } // if (...)
        } // blueprint_size(...)

        bool empty() const noexcept { return this->height == 0 || this->width == 0; }
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return this->height == other.height && this->width == other.width;
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
    }; // struct blueprint_size
    
    template <blueprint_cell t_cell>
    void to_json(nlohmann::json& j, const blueprint_size<t_cell>& x) noexcept
    {
        std::array<std::size_t, 2> dims = { x.height, x.width };
        j = dims;
    } // to_json(...)

    template <blueprint_cell t_cell>
    void from_json(const nlohmann::json& j, blueprint_size<t_cell>& x)
    {
        using type = blueprint_size<t_cell>;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <ropufu::settlers_online::blueprint_cell t_cell>
    struct hash<ropufu::settlers_online::blueprint_size<t_cell>>
    {
        using argument_type = ropufu::settlers_online::blueprint_size<t_cell>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = {};

            return size_hash(x.height << 8) ^ size_hash(x.width);
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_ALGEBRA_BLUEPRINT_SIZE_HPP_INCLUDED
