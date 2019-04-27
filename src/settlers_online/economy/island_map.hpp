
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include <ropufu/algorithm.hpp> // ropufu::aftermath::algorithm::pathfinder

#include "../algebra/blueprint_index.hpp"
#include "../algebra/blueprint_size.hpp"
#include "../indirect_vector.hpp"
#include "blueprint.hpp"
#include "blueprint_projector.hpp"
#include "building.hpp"

#include <cstddef>  // std::size_t
#include <optional> // std::optional
#include <system_error> // std::error_code, std::errc
#include <vector>   // std::vector

namespace ropufu::settlers_online
{
    struct island_building
    {
        using type = island_building;
        using building_type = settlers_online::building;

        building_type building;
        vertex_index position;
        std::size_t key;

        island_building() noexcept = default;

        island_building(const building_type& building, const vertex_index& position) noexcept
            : building(building), position(position)
        {
        } // island_building(...)

        island_building(const building_type& building, std::size_t vertex_row, std::size_t vertex_column) noexcept
            : building(building), position(vertex_row, vertex_column)
        {
        } // island_building(...)
    }; // struct island_building

    struct island_map
    {
        using type = island_map;

        using matrix_index_type = aftermath::algebra::matrix_index<std::size_t>;
        using footprint_matrix_type = typename footprint::matrix_type;
        using building_collection_type = indirect_vector<island_building>;
        using building_key_type = typename building_collection_type::key_type;

        using projector_type = footprint_projector;
        using pathfinder_type = aftermath::algorithm::pathfinder<projector_type>;

    private:
        footprint m_layout = {};
        projector_type m_projector = {};
        // ~~ Buildings ~~
        island_building m_invalid_building = {};
        blueprint<building_key_type> m_building_keys = {};
        building_collection_type m_buildings = building_collection_type(1);
        // ~~ Cached values ~~
        face_size m_bounding_box = {};

    public:
        island_map() noexcept { }

        /** @brief Creates an empty island map. */
        island_map(std::size_t face_height, std::size_t face_width) noexcept
            : m_layout(face_height, face_width), m_projector(this->m_layout),
            m_building_keys(face_height, face_width), m_bounding_box(face_height, face_width)
        {
        } // island_map(...)

        const footprint& layout() const noexcept { return this->m_layout; }

        std::size_t face_height() const noexcept { return this->m_layout.face_height(); }
        std::size_t face_width() const noexcept { return this->m_layout.face_width(); }

        bool contains(const island_building& building) const noexcept
        {
            if (!this->m_buildings.contains_key(building.key)) return false;
            return &building == &(this->m_buildings.by_key(building.key));
        } // contains(...)

        const building_collection_type& buildings() const noexcept { return this->m_buildings; }

        const island_building& at(const matrix_index_type& position, std::error_code& ec) const noexcept
        {
            if (!geometry::is_inside(position, this->m_bounding_box))
                return aftermath::detail::on_error(ec, std::errc::operation_not_permitted, "Position must be inside the bounding box.", this->m_invalid_building);
            building_key_type key = this->m_building_keys[position];
            return this->at(key, ec);
        } // building_at(...)

        const island_building& at(building_key_type key, std::error_code& ec) const noexcept
        {
            if (!this->m_buildings.contains_key(key))
                return aftermath::detail::on_error(ec, std::errc::argument_out_of_domain, "No building with provided key found.", this->m_invalid_building);

            return this->m_buildings.by_key(key);
        } // building_at(...)

        auto begin() const noexcept { return this->m_buildings.begin(); }
        auto end() const noexcept { return this->m_buildings.end(); }
        auto cbegin() const noexcept { return this->m_buildings.cbegin(); }
        auto cend() const noexcept { return this->m_buildings.cend(); }
        
        bool can_be_built(const building& item, const vertex_index& position) const noexcept
        {
            const footprint_matrix_type& absolute = this->m_layout.cells();
            const footprint_matrix_type& relative = item.layout().cells();

            matrix_index_type top_left {}; // Absolute top left corner.
            matrix_index_type bottom_right {}; // Absolute bottom right corner.
            // Check if the bounding box vertex of the building should be on the map.
            if (!this->m_layout.fit(position, item.dimensions(), top_left, bottom_right)) return false;

            std::size_t local_i = 0;
            // Check cells (faces/vertices/edges).
            for (std::size_t i = top_left.row; i <= bottom_right.row; ++i)
            {
                std::size_t local_j = 0;
                for (std::size_t j = top_left.column; j <= bottom_right.column; ++j)
                {
                    // Disallow overlapping cells.
                    if (absolute(i, j) && relative(local_i, local_j)) return false;
                    ++local_j;
                } // for (...)
                ++local_i;
            } // for (...)
            
            return true;
        } // can_be_built(...)

        const island_building& build(const building& item, const vertex_index& position, std::error_code& ec) noexcept
        {
            if (!this->can_be_built(item, position))
                return aftermath::detail::on_error(ec, std::errc::operation_not_permitted, "Building cannot be built at this location.", this->m_invalid_building);

            building_key_type key = this->m_buildings.emplace_back(item, position);
            island_building& result = this->m_buildings.back();
            result.key = key;

            typename blueprint<building_key_type>::matrix_type& keys = this->m_building_keys.cells();
            footprint_matrix_type& absolute = this->m_layout.cells();
            const footprint_matrix_type& relative = item.layout().cells();

            matrix_index_type top_left {}; // Absolute top left corner.
            matrix_index_type bottom_right {}; // Absolute bottom right corner.
            // Initialize the corners.
            this->m_layout.fit(position, item.dimensions(), top_left, bottom_right);

            std::size_t local_i = 0;
            for (std::size_t i = top_left.row; i <= bottom_right.row; ++i)
            {
                std::size_t local_j = 0;
                for (std::size_t j = top_left.column; j <= bottom_right.column; ++j)
                {
                    keys(i, j) = key;
                    absolute(i, j) |= relative(local_i, local_j);
                    ++local_j;
                } // for (...)
                ++local_i;
            } // for (...)

            // Update the projector for pathfinding.
            this->m_projector = projector_type(this->m_layout);

            return result;
        } // build(...)

        /** @brief Traces a path from \p from to \p to.
         *  @remark Unlike \p to, the entrance to \p from does not have to be walkable for the path to be laid out.
         */
        std::vector<vertex_index> path(const island_building& from, const island_building& to, std::error_code& ec) const noexcept
        {
            if (!this->contains(from) || !this->contains(to))
            {
                aftermath::detail::on_error(ec, std::errc::invalid_argument, "Buildings not recognized.");
                return {};
            } // if (...)

            vertex_index a = from.position;
            vertex_index b = to.position;

            a.row -= from.building.dimensions().anchor().row;
            a.column -= from.building.dimensions().anchor().column;

            b.row -= to.building.dimensions().anchor().row;
            b.column -= to.building.dimensions().anchor().column;
            
            a.offset(from.building.dimensions().entrance());
            b.offset(to.building.dimensions().entrance());

            return this->path(a, b, ec);
        } // path(...)

        /** @brief Traces a path from \p from to \p to.
         *  @remark Unlike \p to, the entrance to \p from does not have to be walkable for the path to be laid out.
         */
        std::vector<vertex_index> path(const vertex_index& from, const vertex_index& to, std::error_code& ec) const noexcept
        {
            if (!geometry::is_inside(from, this->m_bounding_box) || !geometry::is_inside(to, this->m_bounding_box))
            {
                aftermath::detail::on_error(ec, std::errc::invalid_argument, "Vertices must be inside the bounding box.");
                return {};
            } // if (...)

            matrix_index_type a {from.row, from.column};
            matrix_index_type b {to.row, to.column};
            pathfinder_type pathfinder {this->m_projector, from, ec};

            std::vector<matrix_index_type> path {};
            pathfinder.trace(to, path, ec);

            std::vector<vertex_index> result {};
            result.reserve(path.size());
            for (const matrix_index_type& index : path) result.emplace_back(index.row, index.column);
            result.shrink_to_fit();
            return result;
        } // path(...)
    }; // struct island_map
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
