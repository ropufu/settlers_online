
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED

#include <aftermath/algebra.hpp> // ropufu::aftermath::algebra::matrix

#include "island_building.hpp"
#include "island_cell.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    /** Structure describing cells and paths on a rectangular island map.
     *  Any "cell" element is always adorned by "paths". The figure below
     *  outlines the structure; '#' represents a cell, 'o'-s represent paths.
     * 
     *     o---o---o
     *     | # | # |
     *     o---o---o
     *     | # | # |
     *     o---o---o
     *
     *  @remark If a map consists of m-by-n cells, this structure can be
     *          represented by a (2m + 1)-by-(2n + 1) matrix.
     */
    struct island_map
    {
        using type = island_map;

    private:
        ropufu::aftermath::algebra::matrix<island_cell> m_cells{0, 0}; // Cells of the map.
        ropufu::aftermath::algebra::matrix<bool> m_paths{1, 1}; // Paths of the map.
        std::vector<island_building> m_buildings{ }; // Buildings.
        island_cell m_invalid_cell{ };
        island_building m_invalid_building{ };

        // void translate() noexcept
        // {
            // // Translate coordinates.
            // row_index -= this->m_corner_row_coordinate;
            // column_index -= this->m_corner_column_coordinate;
            // bool is_row_inside = (row_index >= 0) && (row_index < this->m_blocks.height());
            // bool is_column_inside = (column_index >= 0) && (column_index < this->m_blocks.width());
        // } // translate(...)

    public:
        island_map() noexcept { }

        /** @brief Creates an empty island map where each cell is surrounded by walkable paths. */
        island_map(std::size_t height, std::size_t width) noexcept
            : m_cells(height, width), m_paths(height + 1, width + 1, true)
        {
        } // island_map(...)

        std::size_t height() const noexcept { return this->m_cells.height(); }
        std::size_t width() const noexcept { return this->m_cells.width(); }

        const island_cell& cell(std::size_t row_index, std::size_t column_index) const noexcept
        {
            if (row_index >= this->m_cells.height()) return this->m_invalid_cell;
            if (column_index >= this->m_cells.width()) return this->m_invalid_cell;
            return this->m_cells.unchecked_at(row_index, column_index);
        } // at(...)

        bool build_solid(std::size_t row_index, std::size_t column_index, std::size_t solid_height, std::size_t solid_width) noexcept
        {
            if (solid_height == 0 || solid_width == 0) return true;

            std::size_t past_the_last_row_index = row_index + solid_height;
            std::size_t past_the_last_column_index = column_index + solid_width;

            if (past_the_last_row_index >= this->m_cells.height()) past_the_last_row_index = this->m_cells.height();
            if (past_the_last_column_index >= this->m_cells.width()) past_the_last_column_index = this->m_cells.width();

            // Check cells.
            for (std::size_t i = row_index; i < past_the_last_row_index; ++i)
                for (std::size_t j = column_index; j < past_the_last_column_index; ++j)
                    if (!this->m_cells.unchecked_at(i, j).is_available()) return false;

            this->m_buildings.emplace_back(solid_height, solid_width);
            // Fill cells.
            for (std::size_t i = row_index; i < past_the_last_row_index; ++i)
                for (std::size_t j = column_index; j < past_the_last_column_index; ++j)
                    this->m_cells.unchecked_at(i, j).m_building_pointer = &this->m_buildings.back();
            // Fill paths.
            for (std::size_t i = row_index + 1; i < past_the_last_row_index; ++i)
                for (std::size_t j = column_index + 1; j < past_the_last_column_index; ++j)
                    this->m_paths.unchecked_at(i, j) = false;

            return true;
        } // build_solid(...)
    }; // struct island_map
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
