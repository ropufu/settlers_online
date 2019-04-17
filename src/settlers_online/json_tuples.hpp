
#ifndef ROPUFU_SETTLERS_ONLINE_JSON_TUPLES_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_JSON_TUPLES_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    namespace detail
    {
        template <std::size_t t_tuple_size>
        static void read_size_tuple(const nlohmann::json& j, std::array<std::size_t, t_tuple_size>& result, std::error_code& ec) noexcept
        {
            if (!j.is_array())
            {
                aftermath::detail::on_error(ec, std::errc::illegal_byte_sequence, "Array expected.");
                return;
            }
            
            std::size_t count = 0;
            for (const nlohmann::json& k : j)
            {
                if (count == t_tuple_size)
                {
                    aftermath::detail::on_error(ec, std::errc::invalid_argument, "Tuple not recognized.");
                    return;
                } // if (...)
                if (!k.is_number_unsigned())
                {
                    aftermath::detail::on_error(ec, std::errc::invalid_argument, "Tuple must be represented by non-negative integers.");
                    return;
                } // if (...)

                result[count] = k.get<std::size_t>();
                ++count;
            } // for (...)
        } // read_size_tuple(...)
    } // namespace detail
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_JSON_TUPLES_HPP_INCLUDED
