
#ifndef ROPUFU_SETTLERS_ONLINE_BLUEPRINT_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLUEPRINT_HPP_INCLUDED

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    /** Core functionality for structural elements of an island map; or the
     *  map itself. Can be represented by a graph, where each face has four
     *  incident vertices and four incident edges.
     *  In the following figure each '#' denotes a face, 'o'---a vertex.
     * 
     *     o---o---o
     *     | # | # |
     *     o---o---o
     *     | # | # |
     *     o---o---o
     *
     */
    template <typename t_derived_type, typename t_face_type, typename t_vertex_type>
    struct blueprint
    {
        using type = blueprint<t_derived_type, t_face_type, t_vertex_type>;
        using derived_type = t_derived_type;
        using face_type = t_face_type;
        using vertex_type = t_vertex_type;

        template <typename t_value_type>
        using container_t = ropufu::aftermath::algebra::matrix<t_value_type>;

    protected:
        container_t<face_type> m_faces{ 0, 0 };
        container_t<vertex_type> m_vertices{ 1, 1 };

    public:
        blueprint() noexcept { }

        blueprint(std::size_t height, std::size_t width) noexcept
            : m_faces(height, width), m_vertices(height + 1, width + 1)
        {
        } // blueprint(...)

        // Indicates if the building consists of only one vertex.
        bool empty() const noexcept { return this->m_faces.empty(); }

        const container_t<face_type>& faces() const noexcept { return this->m_faces; }
        const container_t<vertex_type>& vertices() const noexcept { return this->m_vertices; }
    }; // struct blueprint
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BLUEPRINT_HPP_INCLUDED
