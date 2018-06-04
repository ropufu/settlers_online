
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix

#include "blueprint.hpp"
#include "building.hpp"
#include "island_vertex.hpp"
#include "island_path.hpp"

#include <cstddef>  // std::size_t
#include <cstdint>  // std::int_fast32_t
#include <optional> // std::optional
#include <vector>   // std::vector

namespace ropufu::settlers_online
{
    struct island_map : public blueprint<island_map, std::optional<std::size_t>, island_vertex>
    {
        using type = island_map;
        using index_type = ropufu::aftermath::algebra::matrix_index<std::size_t>;
        using mask_type = ropufu::aftermath::algebra::matrix<bool>;

    private:
        island_vertex m_invalid_vertex = {};
        building m_invalid_building = {};
        island_path m_invalid_path = {};
        std::vector<building> m_buildings = {}; // Buildings.
        std::vector<island_path> m_paths = {}; // Paths.
        // ~~ Cached values ~~
        mask_type m_walkable = {};

    public:
        island_map() noexcept { }

        /** @brief Creates an empty island map where each cell is surrounded by walkable paths. */
        island_map(std::size_t height, std::size_t width) noexcept
            : blueprint(height, width), m_walkable(height + 1, width + 1, true)
        {
            //this->m_faces.fill(nullptr); // Unnecessary: nullptr corresponds to 0-ed out memory region.
            //this->m_vertices.fill(island_vertex{}); // Unnecessary: default vertex corresponds to 0-ed out memory region.
        } // island_map(...)

        std::size_t height() const noexcept { return this->m_faces.height(); }
        std::size_t width() const noexcept { return this->m_faces.width(); }

        const std::vector<building>& buildings() const noexcept { return this->m_buildings; }
        const building& buildings(std::size_t index) const noexcept { return index < this->m_buildings.size() ? this->m_buildings[index] : this->m_invalid_building; }

        const std::vector<island_path>& paths() const noexcept { return this->m_paths; }
        const island_path& paths(std::size_t index) const noexcept { return index < this->m_paths.size() ? this->m_paths[index] : this->m_invalid_path; }

        const mask_type& walkable() const noexcept { return this->m_walkable; }

        auto begin() const noexcept { return this->m_buildings.begin(); }
        auto end() const noexcept { return this->m_buildings.end(); }
        auto cbegin() const noexcept { return this->m_buildings.cbegin(); }
        auto cend() const noexcept { return this->m_buildings.cend(); }

        void invalidate_paths() noexcept
        {
            const std::size_t m = this->m_vertices.height();
            const std::size_t n = this->m_vertices.width();

            for (std::size_t i = 0; i < m; ++i)
                for (std::size_t j = 0; j < n; ++j)
                    this->m_walkable(i, j) = this->m_vertices(i, j).walkable();
        
            for (island_path& path : this->m_paths) path.develop(this->m_walkable);
        } // invalidate_paths(...)

        /** Develops a path as if it belonged to the \c paths collection. */
        void develop(island_path& path) const noexcept
        {
            path.develop(this->m_walkable);
        } // develop(...)

        /** @brief Adds a path to the \c paths collection without developing. */
        bool try_anchor(const island_path& path) noexcept
        {
            if (!path.validate(this->m_buildings)) return false;
            this->m_paths.push_back(path);
            return true;
        } // sketch_path

        bool can_be_built(std::size_t row_index, std::size_t column_index, const building& blueprint) const noexcept
        {
            // Check faces.
            for (std::size_t i = 0; i < blueprint.faces().height(); ++i)
            {
                std::size_t global_i = row_index + i;
                for (std::size_t j = 0; j < blueprint.faces().width(); ++j)
                {
                    std::size_t global_j = column_index + j;
                    // Unset blueprint faces don't prevent building.
                    if (blueprint.faces()(i, j) == false) continue;
                    // Check map bounds.
                    if (global_i >= this->m_faces.height() || global_j >= this->m_faces.width()) return false;
                    // Disallow overlapping faces.
                    if (this->m_faces(global_i, global_j).has_value()) return false;
                } // for (...)
            } // for (...)

            // Check vertices.
            for (std::size_t i = 0; i < blueprint.vertices().height(); ++i)
            {
                std::size_t global_i = row_index + i;
                for (std::size_t j = 0; j < blueprint.vertices().width(); ++j)
                {
                    std::size_t global_j = column_index + j;
                    // Walkable blueprint vertices don't prevent building.
                    if (blueprint.vertices()(i, j).walkable()) continue;
                    // Check map bounds.
                    if (global_i >= this->m_vertices.height() || global_j >= this->m_vertices.width()) return false;
                    // Unset vertices on the map don't prevent building.
                    const island_vertex& map_vertex = this->m_vertices(global_i, global_j);
                    if (map_vertex.not_walkable()) return false;
                    if (map_vertex.prevents_building()) return false;
                } // for (...)
            } // for (...)
            
            return true;
        } // can_be_built(...)

        bool try_build(std::size_t row_index, std::size_t column_index, const building& blueprint) noexcept
        {
            if (!this->can_be_built(row_index, column_index, blueprint)) return false;

            std::size_t collection_index = this->m_buildings.size();
            this->m_buildings.push_back(blueprint);
            building& just_built = this->m_buildings.back();
            just_built.attach(collection_index, row_index, column_index);
            std::optional<std::size_t> face_value{ collection_index };

            // Set faces.
            for (std::size_t i = 0; i < blueprint.faces().height(); ++i)
            {
                for (std::size_t j = 0; j < blueprint.faces().width(); ++j)
                {
                    if (blueprint.faces()(i, j) == true)
                    {
                        this->m_faces(row_index + i, column_index + j) = face_value;
                    }; // if (...)
                } // for (...)
            } // for (...)

            // Set vertices.
            for (std::size_t i = 0; i < blueprint.vertices().height(); ++i)
            {
                for (std::size_t j = 0; j < blueprint.vertices().width(); ++j)
                {
                    const island_vertex& blueprint_vertex = blueprint.vertices()(i, j);
                    if (!blueprint_vertex.empty()) // If the blueprint vertex has not been set, continue.
                    {
                        this->m_vertices(row_index + i, column_index + j) = blueprint_vertex;
                    }; // if (...)
                } // for (...)
            } // for (...)

            this->invalidate_paths();
            return true;
        } // try_build(...)
    }; // struct island_map
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_MAP_HPP_INCLUDED
