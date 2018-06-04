
#ifndef ROPUFU_SETTLERS_ONLINE_BUILDING_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BUILDING_HPP_INCLUDED

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix_index

#include "blueprint.hpp"
#include "island_vertex.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    struct building : public blueprint<building, bool, island_vertex>
    {
        using type = building;
        using index_type = ropufu::aftermath::algebra::matrix_index<std::size_t>;
        
        friend struct island_map;

    private:
        // Cached position of entrance vertices.
        std::vector<index_type> m_entrance_indices = {};
        // Cached index in the island map building collection.
        std::size_t m_collection_index = 0;
        // Cached position on the island.
        index_type m_position = {};
        //// Pointer to the path leading away from the building. Used for production buildings.
        //std::size_t m_resource_path_index = 0;

    public:
        building() noexcept { }

        /** @brief Solid rectangular building of a given size. */
        building(std::size_t height, std::size_t width) noexcept
            : blueprint(height, width)
        {
            static const island_vertex s_exterior_vertex{}; // Default vertex is walkable.
            static const island_vertex s_interior_vertex{ false }; // Interior vertices are non-walkable.

            this->m_faces.fill(true); // Mark all faces as occupied.
            this->m_vertices.fill(s_interior_vertex); // Mark all vertices as non-walkable.

            // Mark border vertices as walkable.
            for (std::size_t i = 0; i <= height; ++i)
            {
                this->m_vertices(i, 0) = s_exterior_vertex; // Left border with corners.
                this->m_vertices(i, width) = s_exterior_vertex; // Right border with corners.
            } // for (...)
            for (std::size_t j = 1; j < width; ++j)
            {
                this->m_vertices(0, j) = s_exterior_vertex; // Top border without corners.
                this->m_vertices(height, j) = s_exterior_vertex; // Bottom border without corners.
            } // for (...)

            this->mark_as_entrance(height, width / 2);
        } // building(...)

        std::size_t height() const noexcept { return this->m_faces.height(); }
        std::size_t width() const noexcept { return this->m_faces.width(); }

        const std::vector<index_type>& entrance_indices() const noexcept { return this->m_entrance_indices; }

        std::size_t collection_index() const noexcept { return this->m_collection_index; }
        std::size_t row_index() const noexcept { return this->m_position.row; }
        std::size_t column_index() const noexcept { return this->m_position.column; }
        const index_type& position() const noexcept { return this->m_position; }

        void attach(std::size_t collection_index, const index_type& position) noexcept
        {
            this->m_collection_index = collection_index;
            this->m_position = position;
        } // attach(...)

        void attach(std::size_t collection_index, std::size_t row_index, std::size_t column_index) noexcept
        {
            this->m_collection_index = collection_index;
            this->m_position.row = row_index;
            this->m_position.column = column_index;
        } // attach(...)

        void mark_as_entrance(std::size_t row_index, std::size_t column_index) noexcept
        {
            if (row_index >= this->m_vertices.height()) return;
            if (column_index >= this->m_vertices.width()) return;

            this->m_vertices(row_index, column_index).set_entrance(true);
            this->m_entrance_indices.emplace_back(row_index, column_index);
        } // mark_as_entrance
    }; // struct building
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BUILDING_HPP_INCLUDED
