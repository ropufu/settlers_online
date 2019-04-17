
#ifndef ROPUFU_SETTLERS_ONLINE_BLUEPRINT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLUEPRINT_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix

#include "../algebra/blueprint_index.hpp" // vertex_index
#include "../algebra/blueprint_size.hpp"  // face_size
#include "../enums/blueprint_cell.hpp"    // blueprint_cell
#include "blueprint_parser.hpp"
#include "dimension.hpp"

#include <cstddef>      // std::size_t
#include <cstdint>      // std::int_fast32_t
#include <stdexcept>    // std::runtime_error
#include <string>       // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>       // std::vector

namespace ropufu::settlers_online
{
    namespace detail
    {
        template <typename t_derived_type, typename t_parser_type, bool t_is_enabled = t_parser_type::specialized>
        struct blueprint_parsing_module { };

        template <typename t_derived_type, typename t_parser_type>
        struct blueprint_parsing_module<t_derived_type, t_parser_type, true>
        {
            using parser_type = t_parser_type;
            using derived_type = t_derived_type;
            using matrix_type = typename parser_type::matrix_type;

            void from_json(const nlohmann::json& j, std::error_code& ec) noexcept
            {
                derived_type* that = static_cast<derived_type*>(this);
                parser_type::from_json(j, that->m_cells, ec);
            } // from_json(...)

            void to_json(nlohmann::json& j, std::error_code& ec) const noexcept
            {
                const derived_type* that = static_cast<const derived_type*>(this);
                parser_type::to_json(j, that->m_cells, ec);
            } // to_json(...)
        }; // struct blueprint_parsing_module<...>
    } // namespace detail

    template <typename t_value_type, typename t_parser_type = blueprint_parser<t_value_type>>
    struct blueprint;

    template <typename t_parser_type = blueprint_parser<bool>>
    using footprint_t = blueprint<bool, t_parser_type>;

    using footprint = footprint_t<>;

    void to_json(nlohmann::json& j, const footprint& x) noexcept;
    void from_json(const nlohmann::json& j, footprint& x);

    /** Core functionality for structural elements of an island map; or the
     *  map itself. Can be represented by a graph, where each face has four
     *  incident vertices and four incident edges.
     *  In the following figure each '#' denotes a face, 'o'---a vertex.
     *     0 1 2 3 4 ...
     *  0  o---o---o
     *  1  | # | # |
     *  2  o---o---o
     *  3  | # | # |
     *  4  o---o---o
     *
     *  Vertically (row-counting):
     *  -- number of vertices = number of faces + 1;
     *  -- number of horizontal edges = number of faces + 1;
     *  -- number of vertical edges = number of faces.
     * 
     *  Horizontally (column-counting):
     *  -- number of vertices = number of faces + 1;
     *  -- number of horizontal edges = number of faces;
     *  -- number of vertical edges = number of faces + 1.
     */
    template <typename t_value_type, typename t_parser_type>
    struct blueprint
        : public detail::blueprint_parsing_module<blueprint<t_value_type, t_parser_type>, t_parser_type>
    {
        using type = blueprint<t_value_type, t_parser_type>;
        using value_type = t_value_type;
        using parser_type = t_parser_type;

        using matrix_index_type = aftermath::algebra::matrix_index<std::size_t>;
        using matrix_type = aftermath::algebra::matrix<value_type>;

        friend detail::blueprint_parsing_module<type, parser_type>;

    private:
        matrix_type m_cells = {1, 1};

    public:
        blueprint() noexcept
            : blueprint(0, 0)
        {
        } // blueprint(...)

        blueprint(std::size_t face_height, std::size_t face_width) noexcept
            : m_cells(2 * face_height + 1, 2 * face_width + 1)
        {
        } // blueprint(...)

        /** Number of faces in the vertical direction. */
        std::size_t face_height() const noexcept { return this->m_cells.height() / 2; }
        /** Number of faces in the horizontal direction. */
        std::size_t face_width() const noexcept { return this->m_cells.width() / 2; }

        /** Offsets \p index if it is within current blueprint's dimensions. */
        bool fit(const vertex_index& position, const dimension& dimensions, matrix_index_type& top_left, matrix_index_type& bottom_right) const noexcept
        {
            vertex_index top_left_corner = position;
            const vertex_index& offset = dimensions.anchor();
            const face_size& bounding_box = dimensions.bounding_box();
            // Top left corner vertex of the building should be on the map.
            if (offset.row > top_left_corner.row || offset.column > top_left_corner.column) return false;

            top_left_corner.row -= offset.row;
            top_left_corner.column -= offset.column;

            vertex_index bottom_right_corner = top_left_corner;
            bottom_right_corner.row += bounding_box.height;
            bottom_right_corner.column += bounding_box.width;

            if (bottom_right_corner.row > this->face_height()) return false;
            if (bottom_right_corner.column > this->face_width()) return false;

            top_left = geometry::to_absolute(bottom_right_corner);
            bottom_right = geometry::to_absolute(bottom_right_corner);

            return true;
        } // fit(...)

        const matrix_type& cells() const noexcept { return this->m_cells; }
        matrix_type& cells() noexcept { return this->m_cells; }

        const value_type& operator [](const matrix_index_type& index) const { return this->m_cells[index]; }
        value_type& operator [](const matrix_index_type& index) { this->m_cells[index]; }
    }; // struct blueprint

    void to_json(nlohmann::json& j, const footprint& x) noexcept
    {
        using type = footprint;
        std::error_code ec {};
        x.to_json(j, ec);
    } // to_json(...)

    void from_json(const nlohmann::json& j, footprint& x)
    {
        using type = footprint;
        std::error_code ec {};
        x.from_json(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BLUEPRINT_HPP_INCLUDED
