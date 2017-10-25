
#ifndef ROPUFU_SETTLERS_ONLINE_CORE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CORE_HPP_INCLUDED

#include <string> // std::string

namespace ropufu
{
    namespace settlers_online
    {
        /** Handles conversion between differenct types. */
        template <typename t_from_type, typename t_to_type = std::string>
        struct converter
        {
            using from_type = t_from_type;
            using to_type = t_to_type;

            static to_type to(const from_type& from) noexcept;

            static bool try_from(const to_type& from, from_type& to) noexcept;
        }; // struct converter

        template <typename t_type>
        std::string to_str(const t_type& from) noexcept
        {
            return converter<t_type, std::string>::to(from);
        } // to_str(...)

        template <typename t_type>
        bool try_parse_str(const std::string& from, t_type& to) noexcept
        {
            return converter<t_type, std::string>::try_from(from, to);
        } // try_parse_str(...)
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_CORE_HPP_INCLUDED
