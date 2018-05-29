
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_PATH_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_PATH_HPP_INCLUDED

#include <aftermath/algebra.hpp>   // ropufu::aftermath::algebra::matrix_index
#include <aftermath/algorithm.hpp> // ropufu::aftermath::algorithm::matrix_pathfinder

#include "building.hpp"

#include <cstddef> // std::size_t
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    namespace detail
    {
        struct path_anchor
        {
            using index_type = ropufu::aftermath::algebra::matrix_index;

        private:
            index_type m_position{};
            const building* m_building_ptr = nullptr;

        public:
            path_anchor() noexcept { }

            explicit path_anchor(index_type position) noexcept
                : m_position(position)
            {
            } // path_anchor(...)

            explicit path_anchor(const building* building_ptr) noexcept
                : m_building_ptr(building_ptr)
            {
            } // path_anchor(...)

            bool absolute() const noexcept { return this->m_building_ptr == nullptr; }
            bool relative() const noexcept { return this->m_building_ptr != nullptr; }

            bool is(const building* building_ptr) const noexcept { return this->m_building_ptr == building_ptr; }

            std::vector<index_type> unpack() const noexcept
            {
                if (this->m_building_ptr == nullptr) return { this->m_position };

                std::vector<index_type> result{};
                index_type shift = this->m_building_ptr->position();
                result.reserve(this->m_building_ptr->entrance_indices().size());
                for (index_type entrance_index : this->m_building_ptr->entrance_indices())
                {
                    entrance_index.offset(shift);
                    result.push_back(entrance_index);
                }
                result.shrink_to_fit();
                return result;
            } // unpack(...)
        }; // struct path_anchor
    } // namespace detail

    /** Provides optimized storage for path calculations originating at the same place. */
    struct forking_paths
    {
        using type = forking_paths;
        using index_type = ropufu::aftermath::algebra::matrix_index;
        using mask_type = ropufu::aftermath::algebra::matrix<bool>;
        using pathfinder_type = ropufu::aftermath::algorithm::matrix_pathfinder;

    private:
        detail::path_anchor m_from{};
        std::size_t m_capacity = 0; // Maximum path length.
        std::vector<pathfinder_type> m_tracers{}; // One tracer per each entrance.

    public:
        forking_paths() noexcept { }

        /** @brief A path between two buildings. */
        explicit forking_paths(const detail::path_anchor& from) noexcept : m_from(from) { }

        /** @brief An absolute path from one position to another. */
        explicit forking_paths(const index_type& from) noexcept : m_from(from) { }

        /** @brief A path between two buildings. */
        explicit forking_paths(const building& from) noexcept : m_from(&from) { }

        void invalidate(const mask_type& walkable_mask) noexcept
        {
            this->m_capacity = walkable_mask.size();
            this->m_tracers.clear();
            std::vector<index_type> sources = this->m_from.unpack();
            this->m_tracers.reserve(sources.size());

            for (const index_type& a : sources) this->m_tracers.emplace_back(walkable_mask, a);

            this->m_tracers.shrink_to_fit();
        } // initialize(...)

        /** @brief Develops the steps of the path. */
        bool develop(const detail::path_anchor& to, std::vector<index_type>& shortest) noexcept
        {
            shortest.clear();
            std::vector<index_type> candidate{};
            std::size_t length = this->m_capacity + 1;

            std::vector<index_type> targets = to.unpack();
            for (pathfinder_type& tracer : this->m_tracers)
            {
                for (const index_type& b : targets)
                {
                    candidate.clear();
                    if (!tracer.try_trace(b, candidate)) continue;
                    if (candidate.size() < length)
                    {
                        shortest = candidate;
                        length = candidate.size();
                    } // if (...)
                } // for (...)
            } // for (...)
            return !shortest.empty();
        } // develop(...)

        /** @brief Develops the steps of the path. */
        bool develop(const index_type& to, std::vector<index_type>& shortest) noexcept { return this->develop(detail::path_anchor{ to }, shortest); }
        /** @brief Develops the steps of the path. */
        bool develop(const building& to, std::vector<index_type>& shortest) noexcept { return this->develop(detail::path_anchor{ &to }, shortest); }
    }; // struct forking_paths

    struct island_path
    {
        using type = island_path;
        using index_type = ropufu::aftermath::algebra::matrix_index;
        using mask_type = ropufu::aftermath::algebra::matrix<bool>;
        
        friend struct island_map;

    private:
        std::vector<index_type> m_path{};
        detail::path_anchor m_from{};
        detail::path_anchor m_to{};

    public:
        island_path() noexcept { }

        /** @brief An absolute path from one position to another. */
        island_path(const index_type& from, const index_type& to) noexcept : m_from(from), m_to(to) { }

        /** @brief A path from a building to an absolute position on a map. */
        island_path(const building& from, const index_type& to) noexcept : m_from(&from), m_to(to) { }

        /** @brief A path from an absolute position on a map to a building. */
        island_path(const index_type& from, const building& to) noexcept : m_from(from), m_to(&to) { }

        /** @brief A path between two buildings. */
        island_path(const building& from, const building& to) noexcept : m_from(&from), m_to(&to) { }

        template <typename t_collection_type>
        bool validate(const t_collection_type& buildings) const noexcept
        {
            bool is_from_valid = this->m_from.absolute();
            bool is_to_valid = this->m_to.absolute();
            /** @todo Optimize! */
            for (const building& x : buildings)
            {
                if (this->m_from.is(&x)) is_from_valid = true;
                if (this->m_to.is(&x)) is_to_valid = true;
            }
            return is_from_valid && is_to_valid;
        } // validate(...)

        auto begin() const noexcept { return this->m_path.begin(); }
        auto end() const noexcept { return this->m_path.end(); }
        auto cbegin() const noexcept { return this->m_path.cbegin(); }
        auto cend() const noexcept { return this->m_path.cend(); }

        bool empty() const noexcept { return this->m_path.empty(); }

        const std::vector<index_type>& steps() const noexcept { return this->m_path; }

        /** Develops a path. Only recommended for one-time only use; otherwise consider \c forking_paths. */
        const std::vector<index_type>& develop(const mask_type& walkable_mask) noexcept
        {
            forking_paths garden{ this->m_from };
            garden.invalidate(walkable_mask);
            garden.develop(this->m_to, this->m_path);
            return this->m_path;
        } // develop(...)
    }; // struct island_path
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_PATH_HPP_INCLUDED
