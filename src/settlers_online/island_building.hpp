
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_BUILDING_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_BUILDING_HPP_INCLUDED

#include <aftermath/algebra.hpp> // ropufu::aftermath::algebra::matrix

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    struct island_building
    {
        friend struct island_map;
        using container_type = ropufu::aftermath::algebra::matrix<bool>;

    private:
        container_type m_solid{ };
        container_type m_entrance{ };

    public:
        island_building() noexcept { }

        island_building(std::size_t height, std::size_t width) noexcept
            : m_solid(height, width, true), m_entrance(height, width, false)
        {
            if (height == 0 || width == 0) return;
            this->m_entrance.unchecked_at(height - 1, width / 2) = true;
        } // island_building(...)

        bool empty() const noexcept { return this->m_solid.empty(); }
    }; // struct island_building
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_BUILDING_HPP_INCLUDED
