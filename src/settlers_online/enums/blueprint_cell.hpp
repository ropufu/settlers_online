
#ifndef ROPUFU_SETTLERS_ONLINE_BLUEPRINT_CELL_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLUEPRINT_CELL_HPP_INCLUDED

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix_index

namespace ropufu::settlers_online
{
    static constexpr char free_vertex_chr = '.';
    static constexpr char free_horizontal_edge_chr = '-';
    static constexpr char free_vertical_edge_chr = '|';
    static constexpr char free_face_chr = ' ';
    static constexpr char occupied_chr = '#';

    /** @remark Values are chosen so that the first (least significant) bit indicates
     *  the parity (0 for even, 1 for odd) of the column index, and the second bit
     *  indicates the parity of the row index.
     */
    enum struct blueprint_cell : char
    {
        vertex = 0b00, // Vertices have an even row index and an even column index.
        horizontal_edge = 0b01, // Horizontal edges have an even row index and an odd column index.
        vertical_edge = 0b10,   // Vertical edges have an odd row index and an even column index.
        face = 0b11    // Faces have an odd row index and an odd column index.
    }; // enum struct blueprint_cell

    struct blueprint_cell_traits
    {
        static constexpr blueprint_cell which(const aftermath::algebra::matrix_index<std::size_t>& position) noexcept { return which(position.row, position.column); }
        static constexpr blueprint_cell which(std::size_t row, std::size_t column) noexcept
        {
            return static_cast<blueprint_cell>(((row & 0x01) << 1) | (column & 0x01));
        } // which(...)

        static constexpr bool is_face(const aftermath::algebra::matrix_index<std::size_t>& position) noexcept { return is_face(position.row, position.column); }
        static constexpr bool is_face(std::size_t row, std::size_t column) noexcept
        {
            return ((row & 0x01) == 1) && ((column & 0x01) == 1);
        } // is_face(...)

        static constexpr bool is_vertex(const aftermath::algebra::matrix_index<std::size_t>& position) noexcept { return is_vertex(position.row, position.column); }
        static constexpr bool is_vertex(std::size_t row, std::size_t column) noexcept
        {
            return ((row & 0x01) == 0) && ((column & 0x01) == 0);
        } // is_vertex(...)

        static constexpr bool is_horizontal_edge(const aftermath::algebra::matrix_index<std::size_t>& position) noexcept { return is_horizontal_edge(position.row, position.column); }
        static constexpr bool is_horizontal_edge(std::size_t row, std::size_t column) noexcept
        {
            return ((row & 0x01) == 0) && ((column & 0x01) == 1);
        } // is_horizontal_edge(...)

        static constexpr bool is_vertical_edge(const aftermath::algebra::matrix_index<std::size_t>& position) noexcept { return is_vertical_edge(position.row, position.column); }
        static constexpr bool is_vertical_edge(std::size_t row, std::size_t column) noexcept
        {
            return ((row & 0x01) == 1) && ((column & 0x01) == 0);
        } // is_vertical_edge(...)
    }; // struct blueprint_cell_traits
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BLUEPRINT_CELL_HPP_INCLUDED
