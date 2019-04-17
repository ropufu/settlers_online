
#ifndef ROPUFU_SETTLERS_ONLINE_ECONOMY_BLUEPRINT_PARSER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ECONOMY_BLUEPRINT_PARSER_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <ropufu/algebra.hpp> // ropufu::aftermath::algebra::matrix

#include "../algebra/blueprint_index.hpp" // vertex_index
#include "../algebra/blueprint_size.hpp"  // face_size
#include "../enums/blueprint_cell.hpp"    // blueprint_cell

#include <cstddef>      // std::size_t
#include <string>       // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>       // std::vector

namespace ropufu::settlers_online
{
    template <typename t_value_type>
    struct blueprint_parser
    {
        using type = blueprint_parser<t_value_type>;
        using value_type = t_value_type;
        using matrix_type = aftermath::algebra::matrix<t_value_type>;

        static constexpr bool specialized = false;

        // static void from_json(const nlohmann::json& j, matrix_type& value, std::error_code& ec) noexcept;
        // static void to_json(nlohmann::json& j, const matrix_type& value, std::error_code& ec) noexcept;
    }; // struct blueprint_parser

    template <>
    struct blueprint_parser<bool>
    {
        using type = blueprint_parser<bool>;
        using value_type = bool;
        using matrix_type = aftermath::algebra::matrix<bool>;

        static constexpr bool specialized = true;

    private:
        static bool read(char c, std::error_code& ec) noexcept
        {
            switch (c)
            {
                case free_vertex_chr:
                case free_horizontal_edge_chr:
                case free_vertical_edge_chr:
                case free_face_chr:
                    return false;
                case occupied_chr:
                    return true;
                default:
                    return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Blueprint not recognized.", false);
            } // switch (...)
            return false;
        } // parse(...)

        static char write(blueprint_cell cell, bool value, std::error_code& /*ec*/) noexcept
        {
            switch (cell)
            {
                case blueprint_cell::vertex:
                    return value ? occupied_chr : free_vertex_chr;
                case blueprint_cell::horizontal_edge:
                    return value ? occupied_chr : free_horizontal_edge_chr;
                case blueprint_cell::vertical_edge:
                    return value ? occupied_chr : free_vertical_edge_chr;
                case blueprint_cell::face:
                    return value ? occupied_chr : free_face_chr;
                default:
                    return '\0';
            } // switch (...)
            return '\0';
        } // write(...)

    public:
        static void from_json(const nlohmann::json& j, matrix_type& value, std::error_code& ec) noexcept
        {
            if (!j.is_array())
            {
                aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, "Array expected.");
                return;
            } // if (...)
        
            std::size_t m = 0;
            std::size_t n = 0;
            std::vector<std::vector<bool>> occupied {};
            for (const nlohmann::json& k : j)
            {
                if (!k.is_string())
                {
                    aftermath::detail::on_error(ec, std::errc::invalid_argument, "Blueprint must be represented by strings.");
                    return;
                } // if (...)

                std::string line = k.get<std::string>();
                if (m == 0) n = line.length();
                else if (line.length() != n)
                {
                    aftermath::detail::on_error(ec, std::errc::invalid_argument, "Blueprint lines must have the same length.");
                    return;
                } // else (...)

                std::vector<bool> flags {};
                flags.reserve(n);
                for (char c : line)
                {
                    flags.push_back(type::read(c, ec));
                    if (ec.value() != 0) return;
                } // for (...)

                occupied.push_back(flags);
                ++m;
            } // for (...)

            value = {m, n};
            for (std::size_t i = 0; i < m; ++i) for (std::size_t j = 0; j < n; ++j) value(i, j) = occupied[i][j];
        } // from_json(...)
    
        static void to_json(nlohmann::json& j, const matrix_type& value, std::error_code& ec) noexcept
        {
            std::error_code ec {};
            std::size_t m = value.height();
            std::size_t n = value.width();
            std::vector<std::string> lines {};
            lines.reserve(m);
            for (std::size_t i = 0; i < m; ++i)
            {
                std::string line(n, '\0');
                for (std::size_t j = 0; j < n; ++j)
                {
                    line[j] = type::write(blueprint_cell_traits::which(i, j), value(i, j), ec);
                } // for (...)
                lines.push_back(line);
            } // for (...)
            j = lines;
        } // to_json(...)
    }; // struct blueprint_parser<...>
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ECONOMY_BLUEPRINT_PARSER_HPP_INCLUDED
