
#ifndef ROPUFU_SETTLERS_ONLINE_BLUEPRINT_PROJECTOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLUEPRINT_PROJECTOR_HPP_INCLUDED

#include <ropufu/algebra.hpp>
#include <ropufu/algorithm.hpp>

#include "../algebra/blueprint_index.hpp"
#include "blueprint.hpp"

#include <cstddef> // std::size_t
#include <type_traits> // std::is_same_v
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    template <typename t_value_type>
    struct blueprint_projector;

    using footprint_projector = blueprint_projector<bool>;

    template <typename t_value_type>
    struct blueprint_projector : public aftermath::algorithm::projector<
        blueprint_projector<t_value_type>,
        blueprint<t_value_type>,
        std::size_t>
    {
        using type = blueprint_projector<t_value_type>;
        using surface_type = blueprint<t_value_type>;
        using cost_type = std::size_t;

        using base_type = aftermath::algorithm::projector<type, surface_type, cost_type>;
        using value_type = t_value_type;

        using matrix_type = typename surface_type::matrix_type;
        using index_type = aftermath::algebra::matrix_index<std::size_t>;
        using cell_comparer_type = aftermath::algorithm::detail::inequality_comparer<value_type>;
        using pair_type = aftermath::algorithm::index_cost_pair<std::size_t, cost_type>;

        friend base_type;
        static constexpr bool blocked_indicator = true;

    protected:
        std::size_t height_override() const noexcept 
        {
            return this->surface().vertex_height();
        } // height_override(...)

        std::size_t width_override() const noexcept 
        {
            return this->surface().vertex_width();
        } // width_override(...)

        /** L1 distance between two indices. */
        cost_type distance_override(const index_type& a, const index_type& b) const noexcept
        {
            cost_type dx = (a.column < b.column) ? (b.column - a.column) : (a.column - b.column);
            cost_type dy = (a.row < b.row) ? (b.row - a.row) : (a.row - b.row);
            return dx + dy;
        } // distance_override(...)

        void neighbors_override(const index_type& source, std::vector<pair_type>& projected_neighbors) const noexcept
        {
            const matrix_type& cells = this->surface().cells();
            index_type absolute {2 * source.row, 2 * source.column};

            projected_neighbors.clear();
            projected_neighbors.reserve(4);

            if (source.row != 0)
            {
                index_type neighbor_index = source;
                --neighbor_index.row;
                bool is_edge_blocked = cell_comparer_type::good(cells(absolute.row - 1, absolute.column), type::blocked_indicator);
                bool is_vertex_blocked = cell_comparer_type::good(cells(absolute.row - 2, absolute.column), type::blocked_indicator);
                if (!(is_edge_blocked || is_vertex_blocked)) projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
            if (source.column != this->width() - 1)
            {
                index_type neighbor_index = source;
                ++neighbor_index.column;
                bool is_edge_blocked = cell_comparer_type::good(cells(absolute.row, absolute.column + 1), type::blocked_indicator);
                bool is_vertex_blocked = cell_comparer_type::good(cells(absolute.row, absolute.column + 2), type::blocked_indicator);
                if (!(is_edge_blocked || is_vertex_blocked)) projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
            if (source.row != this->height() - 1)
            {
                index_type neighbor_index = source;
                ++neighbor_index.row;
                bool is_edge_blocked = cell_comparer_type::good(cells(absolute.row + 1, absolute.column), type::blocked_indicator);
                bool is_vertex_blocked = cell_comparer_type::good(cells(absolute.row + 2, absolute.column), type::blocked_indicator);
                if (!(is_edge_blocked || is_vertex_blocked)) projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
            if (source.column != 0)
            {
                index_type neighbor_index = source;
                --neighbor_index.column;
                bool is_edge_blocked = cell_comparer_type::good(cells(absolute.row, absolute.column - 1), type::blocked_indicator);
                bool is_vertex_blocked = cell_comparer_type::good(cells(absolute.row, absolute.column - 2), type::blocked_indicator);
                if (!(is_edge_blocked || is_vertex_blocked)) projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
        } // neighbors_override(...)

    public:
        using base_type::projector; // Inherit constructors.
    }; // struct blueprint_projector
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BLUEPRINT_PROJECTOR_HPP_INCLUDED
