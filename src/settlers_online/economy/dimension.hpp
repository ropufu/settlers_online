
#ifndef ROPUFU_SETTLERS_ONLINE_DIMENSIONS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_DIMENSIONS_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include "../algebra/blueprint_index.hpp" // vertex_index
#include "../algebra/blueprint_size.hpp"  // face_size
#include "../enums/blueprint_cell.hpp"    // blueprint_cell

#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    struct building;

    /** Descriptor for building dimensions. */
    struct dimension;

    void to_json(nlohmann::json& j, const dimension& x) noexcept;
    void from_json(const nlohmann::json& j, dimension& x);

    /** @brief Descriptor for building dimensions.
     *  In the following figure each '#' denotes a face, 'o'---a vertex.
     *     0 1 2 3 4 ...
     *  0  o---o---o
     *  1  | # | # |
     *  2  o---o---o
     *  3  | # | # |
     *  4  o---o---o
     *  Bounding box counts the number of faces.
     *  Anchor and entrance are vertices.
     */
    struct dimension
    {
        using type = dimension;
        // ~~ Json names ~~
        static constexpr char jstr_bounding_box[] = "bounding box";
        static constexpr char jstr_anchor[] = "anchor";
        static constexpr char jstr_entrance[] = "entrance";

        friend building;

    private:
        face_size m_bounding_box = {};
        vertex_index m_anchor = {};
        vertex_index m_entrance = {};

        bool validate(std::error_code& ec) const noexcept
        {
            if (!geometry::is_inside(this->m_anchor, this->m_bounding_box)) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Anchor must be inside the bounding box.", false);
            if (!geometry::is_inside(this->m_entrance, this->m_bounding_box)) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Entrance must be inside the bounding box.", false);
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            geometry::force_inside(this->m_anchor, this->m_bounding_box);
            geometry::force_inside(this->m_entrance, this->m_bounding_box);
        } // coerce(...)

    public:
        /** @brief Dimensions of a building. */
        dimension() noexcept { }

        /** @brief Dimensions of a building.
         *  @param ec Set to std::errc::invalid_argument if \p bounding_box is trivial.
         *  @param ec Set to std::errc::invalid_argument if \p anchor or \p entrance is outside the \p bounding box.
         */
        dimension(const face_size& bounding_box, const vertex_index& anchor, const vertex_index& entrance, std::error_code& ec) noexcept
            : m_bounding_box(bounding_box), m_anchor(anchor), m_entrance(entrance)
        {
            if (!this->validate(ec)) this->coerce();
        } // dimension(...)

        dimension(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            // Parse json entries.
            aftermath::noexcept_json::required(j, type::jstr_bounding_box, this->m_bounding_box, ec);
            aftermath::noexcept_json::required(j, type::jstr_anchor, this->m_anchor, ec);
            aftermath::noexcept_json::required(j, type::jstr_entrance, this->m_entrance, ec);

            if (!this->validate(ec)) this->coerce();
        } // dimension(...)

        /** Bounding box of the building. */
        const face_size& bounding_box() const noexcept { return this->m_bounding_box; }
        /** Anchor of the building. */
        void set_bounding_box(const face_size& value, std::error_code& ec) noexcept
        {
            this->m_bounding_box = value;
            if (!this->validate(ec)) this->coerce();
        } // set_anchor(...)

        /** Anchor of the building. */
        const vertex_index& anchor() const noexcept { return this->m_anchor; }
        /** Anchor of the building. */
        void set_anchor(const vertex_index& value, std::error_code& ec) noexcept
        {
            this->m_anchor = value;
            if (!this->validate(ec)) this->coerce();
        } // set_anchor(...)

        /** Entrance to the building. */
        const vertex_index& entrance() const noexcept { return this->m_entrance; }
        /** Entrance to the building. */
        void set_entrance(const vertex_index& value, std::error_code& ec) noexcept
        {
            this->m_entrance = value;
            if (!this->validate(ec)) this->coerce();
        } // set_anchor(...)
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_bounding_box == other.m_bounding_box &&
                this->m_anchor == other.m_anchor &&
                this->m_entrance == other.m_entrance;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)
    }; // struct dimension

    // ~~ Json name definitions ~~
    constexpr char dimension::jstr_bounding_box[];
    constexpr char dimension::jstr_anchor[];
    constexpr char dimension::jstr_entrance[];

    void to_json(nlohmann::json& j, const dimension& x) noexcept
    {
        using type = dimension;

        j = nlohmann::json{
            {type::jstr_bounding_box, x.bounding_box()},
            {type::jstr_anchor, x.anchor()},
            {type::jstr_entrance, x.entrance()}
        };
    } // to_json(...)

    void from_json(const nlohmann::json& j, dimension& x)
    {
        using type = dimension;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::dimension>
    {
        using argument_type = ropufu::settlers_online::dimension;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<ropufu::settlers_online::face_size> matrix_size_hash = {};
            std::hash<ropufu::settlers_online::vertex_index> matrix_index_hash = {};

            return
                matrix_size_hash(x.bounding_box()) ^
                matrix_index_hash(x.anchor()) ^
                matrix_index_hash(x.entrance());
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_DIMENSIONS_HPP_INCLUDED
