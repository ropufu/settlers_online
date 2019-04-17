
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include <ropufu/algorithm.hpp> // ropufu::aftermath::algorithm::matrix_pathfinder

#include "../algebra/blueprint_index.hpp"
#include "../algebra/blueprint_size.hpp"
#include "../map_value_iterator.hpp"
#include "blueprint.hpp"
#include "building.hpp"

#include <cstddef>  // std::size_t
#include <cstdint>  // std::int_fast32_t
#include <map>      // std::map
#include <system_error> // std::error_code, std::errc
#include <vector>   // std::vector

namespace ropufu::settlers_online
{
    struct island_map
    {
        using type = island_map;
        using building_key_type = std::size_t;

        using matrix_index_type = aftermath::algebra::matrix_index<std::size_t>;
        using footprint_matrix_type = typename footprint::matrix_type;
        using building_matrix_type = typename blueprint<std::size_t>::matrix_type;
        using const_building_pointer = const building*;
        using building_pointer = building*;
        using pathfinder_type = aftermath::algorithm::matrix_pathfinder;

    private:
        static constexpr std::size_t missing_key = 0;

        footprint m_layout = {};
        blueprint<std::size_t> m_building_layout = {};
        std::size_t m_next_key = type::missing_key + 1;
        std::map<std::size_t, building> m_buildings = {};
        // ~~ Cached values ~~
        face_size m_bounding_box = {};

    public:
        island_map() noexcept { }

        /** @brief Creates an empty island map. */
        island_map(std::size_t face_height, std::size_t face_width) noexcept
            : m_layout(face_height, face_width), m_building_layout(face_height, face_width), m_bounding_box(face_height, face_width)
        {
        } // island_map(...)

        std::size_t face_height() const noexcept { return this->m_layout.face_height(); }
        std::size_t face_width() const noexcept { return this->m_layout.face_width(); }

        // const std::map<std::size_t, building>& buildings() const noexcept { return this->m_buildings; }
        const_building_pointer building_at(const matrix_index_type& position) const noexcept
        {
            if (!geometry::is_inside(position, this->m_bounding_box)) return nullptr;

            std::size_t key = this->m_building_layout[position];
            auto search = this->m_buildings.find(key);
            if (search != this->m_buildings.end()) return &(search->second);
            return nullptr;
        } // building_at(...)

        auto begin() const noexcept { return make_value_iterator(this->m_buildings.begin()); }
        auto end() const noexcept { return make_value_iterator(this->m_buildings.end()); }
        auto cbegin() const noexcept { return make_value_iterator(this->m_buildings.cbegin()); }
        auto cend() const noexcept { return make_value_iterator(this->m_buildings.cend()); }

        bool can_be_built(const vertex_index& position, const building& item) const noexcept
        {
            const footprint_matrix_type& absolute = this->m_layout.cells();
            const footprint_matrix_type& relative = item.layout().cells();

            matrix_index_type top_left {}; // Absolute top left corner.
            matrix_index_type bottom_right {}; // Absolute bottom right corner.
            // Check if the bounding box vertex of the building should be on the map.
            if (!this->m_layout.fit(position, item.dimensions(), top_left, bottom_right)) return false;

            std::size_t local_i = 0;
            // Check faces.
            for (std::size_t i = top_left.row; i <= bottom_right.row; ++i)
            {
                std::size_t local_j = 0;
                for (std::size_t j = top_left.column; j <= bottom_right.column; ++j)
                {
                    // Disallow overlapping faces.
                    if (absolute(i, j) && relative(local_i, local_j)) return false;
                    ++local_j;
                } // for (...)
                ++local_i;
            } // for (...)
            
            return true;
        } // can_be_built(...)

        void build(const vertex_index& position, const building& item, std::error_code& ec) noexcept
        {
            if (!this->can_be_built(position, item))
            {
                aftermath::detail::on_error(ec, std::errc::operation_not_permitted, "Building cannot be build at this location.");
                return;
            } // if (...)

            footprint_matrix_type& absolute = this->m_layout.cells();
            const footprint_matrix_type& relative = item.layout().cells();

            matrix_index_type top_left {}; // Absolute top left corner.
            matrix_index_type bottom_right {}; // Absolute bottom right corner.
            // Initialize the corners.
            this->m_layout.fit(position, item.dimensions(), top_left, bottom_right);

            std::size_t local_i = 0;
            // Check faces.
            for (std::size_t i = top_left.row; i <= bottom_right.row; ++i)
            {
                std::size_t local_j = 0;
                for (std::size_t j = top_left.column; j <= bottom_right.column; ++j)
                {
                    // Disallow overlapping faces.
                    absolute(i, j) |= relative(local_i, local_j);
                    ++local_j;
                } // for (...)
                ++local_i;
            } // for (...)
        } // try_build(...)
    }; // struct island_map
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
