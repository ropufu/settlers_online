
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_CELL_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_CELL_HPP_INCLUDED

#include "island_building.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    struct island_cell
    {
        friend struct island_map;

    private:
        bool m_does_permit_building{ true };
        island_building* m_building_pointer{ nullptr };

    public:
        island_cell() noexcept { }
        
        /** @brief Indicates if any buildings can be potentially built on this cell. */
        bool does_permit_building() const noexcept { return this->m_does_permit_building; }
        /** @brief Indicates if any building has already been built on this cell. */
        bool is_occupied() const noexcept { return this->m_building_pointer != nullptr; }
        /** @brief Indicates if no buildings have been built on this cell. */
        bool is_unoccupied() const noexcept { return this->m_building_pointer == nullptr; }
        /** @brief Indicates if a building can be placed on this cell in its current state. */
        bool is_available() const noexcept { return this->does_permit_building() && this->is_unoccupied(); }
    }; // struct island_cell
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_CELL_HPP_INCLUDED
