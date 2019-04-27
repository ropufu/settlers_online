
#ifndef ROPUFU_SETTLERS_ONLINE_ALGEBRA_BLUEPRINT_POSITION_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ALGEBRA_BLUEPRINT_POSITION_HPP_INCLUDED

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix_index

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include "../enums/blueprint_cell.hpp"
#include "../json_tuples.hpp"
#include "blueprint_size.hpp"

#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <functional>   // std::hash
#include <ostream>      // std::ostream
#include <stdexcept>    // std::runtime_error
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    template <blueprint_cell t_cell>
    struct blueprint_index;

    using face_index = blueprint_index<blueprint_cell::face>;
    using vertex_index = blueprint_index<blueprint_cell::vertex>;
    using horizontal_edge_index = blueprint_index<blueprint_cell::horizontal_edge>;
    using vertical_edge_index = blueprint_index<blueprint_cell::vertical_edge>;

    template <blueprint_cell t_cell>
    void to_json(nlohmann::json& j, const blueprint_index<t_cell>& x) noexcept;
    template <blueprint_cell t_cell>
    void from_json(const nlohmann::json& j, blueprint_index<t_cell>& x);

    template <blueprint_cell t_cell>
    struct blueprint_index : public aftermath::algebra::matrix_index<std::size_t> 
    {
        using type = blueprint_index<t_cell>;
        using base_type = aftermath::algebra::matrix_index<std::size_t>;
        static constexpr blueprint_cell cell = t_cell;

        using base_type::matrix_index; // Inherit constructor.

        blueprint_index(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            std::array<std::size_t, 2> dims {};
            detail::read_size_tuple(j, dims, ec);
            if (ec.value() == 0)
            {
                this->row = dims[0];
                this->column = dims[1];
            } // if (...)
        } // blueprint_index(...)

        bool inside(const blueprint_size<type::cell>& box) const noexcept { return this->row < box.height && this->column < box.width; }
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return this->row == other.row && this->column == other.column;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        /** Re-scaling. */
        type& operator *=(std::size_t scalar) noexcept
        {
            this->row *= scalar;
            this->column *= scalar;
            return *this;
        } // operator *=(...)

        friend type operator *(type left, std::size_t scalar) noexcept { left *= scalar; return left; }
        friend type operator *(std::size_t scalar, type right) noexcept { right *= scalar; return right; }

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct blueprint_index
    
    template <blueprint_cell t_cell>
    void to_json(nlohmann::json& j, const blueprint_index<t_cell>& x) noexcept
    {
        std::array<std::size_t, 2> dims = { x.row, x.column };
        j = dims;
    } // to_json(...)

    template <blueprint_cell t_cell>
    void from_json(const nlohmann::json& j, blueprint_index<t_cell>& x)
    {
        using type = blueprint_index<t_cell>;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)

    struct geometry
    {
        /** Checks if the \p index is inside its matrix of size \p box. */
        template <blueprint_cell t_cell>
        static bool is_inside(const blueprint_index<t_cell>& index, const blueprint_size<t_cell>& box) noexcept
        {
            return index.inside(box);
        } // is_inside(...)

        /** Vertex matrix is exatly one row and one column larget than the face matrix. */
        static bool is_inside(const vertex_index& index, const face_size& box) noexcept
        {
            return index.row <= box.height && index.column <= box.width;
        } // is_inside(...)

        /** Ensures vertex is inside the box. */
        static void force_inside(vertex_index& index, const face_size& box) noexcept
        {
            if (index.row > box.height) index.row = box.height;
            if (index.column > box.width) index.column = box.width;
        } // force_inside(...)

        /** Horizontal edge matrix is exatly one row larger that the face matrix, but has the same numer of columns. */
        static bool is_inside(const horizontal_edge_index& index, const face_size& box) noexcept
        {
            return index.row <= box.height && index.column < box.width;
        } // is_inside(...)

        /** Vertical edge matrix is exatly one column larger that the face matrix, but has the same numer of rows. */
        static bool is_inside(const vertical_edge_index& index, const face_size& box) noexcept
        {
            return index.row < box.height && index.column <= box.width;
        } // is_inside(...)

        /** Checks if the the absolute \p index is inside the face matrix of size \p box. */
        static bool is_inside(const aftermath::algebra::matrix_index<std::size_t>& index, const face_size& box) noexcept
        {
            return index.row <= 2 * box.height && index.column <= 2 * box.width;
        } // is_inside(...)

        static aftermath::algebra::matrix_index<std::size_t> to_absolute(const face_index& index) noexcept
        {
            return { 2 * index.row + 1, 2 * index.column + 1 };
        } // to_absolute(...)

        static aftermath::algebra::matrix_index<std::size_t> to_absolute(const vertex_index& index) noexcept
        {
            return { 2 * index.row, 2 * index.column };
        } // to_absolute(...)

        static aftermath::algebra::matrix_index<std::size_t> to_absolute(const horizontal_edge_index& index) noexcept
        {
            return { 2 * index.row, 2 * index.column + 1 };
        } // to_absolute(...)

        static aftermath::algebra::matrix_index<std::size_t> to_absolute(const vertical_edge_index& index) noexcept
        {
            return { 2 * index.row + 1, 2 * index.column };
        } // to_absolute(...)
    }; // struct geometry

} // namespace ropufu::settlers_online

namespace std
{
    template <ropufu::settlers_online::blueprint_cell t_cell>
    struct hash<ropufu::settlers_online::blueprint_index<t_cell>>
    {
        using argument_type = ropufu::settlers_online::blueprint_index<t_cell>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::size_t> size_hash = {};

            return size_hash(x.row << 8) ^ size_hash(x.column);
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_ALGEBRA_BLUEPRINT_POSITION_HPP_INCLUDED
