
#ifndef ROPUFU_SETTLERS_ONLINE_ISLAND_VERTEX_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ISLAND_VERTEX_HPP_INCLUDED

namespace ropufu::settlers_online
{
    /** @brief Describes peroperties of a vertex on an island map. */
    struct island_vertex
    {
        using type = island_vertex;

    private:
        unsigned char m_flags = 0;

        static constexpr unsigned char f_not_walkable = 0x01; // Indicates if the vertex is walkable.
        static constexpr unsigned char f_prevents_building = 0x02; // Indicates if the vertex prevents building structures.
        static constexpr unsigned char f_entrance = 0x04; // Indicates if a vertex is an entrance to a building.

    public:
        island_vertex() noexcept { }

        /*implicit*/ island_vertex(bool walkable) noexcept
            : m_flags(walkable ? 0 : type::f_not_walkable)
        {
        } // island_vertex(...)

        bool empty() const noexcept { return this->m_flags == 0; }

        bool walkable() const noexcept { return (this->m_flags & type::f_not_walkable) == 0; }
        bool not_walkable() const noexcept { return (this->m_flags & type::f_not_walkable) != 0; }
        void set_walkable(bool is_walkable = true) noexcept
        {
            if (is_walkable) this->m_flags &= (~type::f_not_walkable);
            else this->m_flags |= type::f_not_walkable;
        } // set_walkable(...)

        bool permits_building() const noexcept { return (this->m_flags & type::f_prevents_building) == 0; }
        bool prevents_building() const noexcept { return (this->m_flags & type::f_prevents_building) != 0; }
        void set_prevents_building(bool does_prevent_building = true) noexcept
        {
            if (does_prevent_building) this->m_flags |= type::f_prevents_building;
            else this->m_flags &= (~type::f_prevents_building);
        } // set_prevents_building(...)

        bool not_entrance() const noexcept { return (this->m_flags & type::f_entrance) == 0; }
        bool entrance() const noexcept { return (this->m_flags & type::f_entrance) != 0; }
        void set_entrance(bool is_entrance = true) noexcept
        {
            if (is_entrance) this->m_flags |= type::f_entrance;
            else this->m_flags &= (~type::f_entrance);
        } // set_prevents_building(...)
    }; // struct island_vertex
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ISLAND_VERTEX_HPP_INCLUDED
