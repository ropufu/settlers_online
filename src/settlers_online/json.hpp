
#ifndef ROPUFU_SETTLERS_ONLINE_JSON_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_JSON_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <nlohmann/json.hpp>

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t, std::int_fast64_t
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu
{
    namespace detail
    {
        template <typename t_value_type>
        struct json_type_check
        {
            static bool is(const nlohmann::json& entry) noexcept;
        }; // struct json_type_check

        template <>
        struct json_type_check<bool>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_boolean(); }
        }; // struct json_type_check

        template <>
        struct json_type_check<float>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_number(); }
        }; // struct json_type_check

        template <>
        struct json_type_check<double>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_number(); }
        }; // struct json_type_check

        template <>
        struct json_type_check<std::int32_t>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_number_integer(); }
        }; // struct json_type_check

        template <>
        struct json_type_check<std::int64_t>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_number_integer(); }
        }; // struct json_type_check

        template <>
        struct json_type_check<std::size_t>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_number_unsigned(); }
        }; // struct json_type_check

        template <>
        struct json_type_check<std::string>
        {
            static bool is(const nlohmann::json& entry) noexcept { return entry.is_string(); }
        }; // struct json_type_check
    } // namespace detail

    /** Extra functionality for \c nlohmann::json. */
    struct quiet_json
    {
        using type = quiet_json;

        /** @brief Checks if a required element is missing in json.
         *  @return True if the element is missing.
         *  @exception not_an_error::runtime_error This error is pushed to \c quiet_error if the element is required but missing.
         */
        static bool is_missing(const nlohmann::json& container, const std::string& key, bool is_optional = false) noexcept
        {
            if (container.count(key) > 0) return false;

            if (!is_optional) aftermath::quiet_error::instance().push(
                aftermath::not_an_error::runtime_error,
                aftermath::severity_level::major,
                std::string("Missing required value for ") + key + std::string("."), __FUNCTION__, __LINE__);
            return true;
        } // is_missing(...)

        /** @brief Tries to parse \p entry as a specific type.
         *  @return True if \p entry has been parsed.
         *  @remark \p value is overwritten only if \p entry is properly formed.
         *  @exception not_an_error::domain_error This error is pushed to \c quiet_error if \p entry is of a wrong type (incompatible).
         */
        template <typename t_value_type>
        static bool try_parse(const nlohmann::json& entry, const std::string& name, t_value_type& value) noexcept
        {
            bool is_good = detail::json_type_check<t_value_type>::is(entry);
            if (!is_good)
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::domain_error,
                    aftermath::severity_level::major,
                    std::string("JSON representation for ") + name + std::string(" malformed."), __FUNCTION__, __LINE__);
                return false;
            }

            value = entry.get<t_value_type>();
            return true;
        } // try_parse(...)

        /** @brief Tries to parse \p entry as a vector of specific element type.
         *  @return True if \p entry has been parsed.
         *  @remark \p value is overwritten only if \p entry is an array with compatible element type.
         *  @exception not_an_error::domain_error This error is pushed to \c quiet_error if \p entry is either not an array.
         *  @exception not_an_error::domain_error This error is pushed to \c quiet_error if one of the elements is of a wrong type (incompatible).
         */
        template <typename t_value_type>
        static bool try_parse(const nlohmann::json& entry, const std::string& name, std::vector<t_value_type>& value, bool do_allow_singletons = false) noexcept
        {
            // Identify <x> with singleton set { <x> }.
            if (do_allow_singletons)
            {
                bool is_singular = detail::json_type_check<t_value_type>::is(entry);
                if (is_singular)
                {
                    value.clear();
                    value.push_back(entry.get<t_value_type>());
                    return true;
                }
            }

            // Otherwise require array notation [ ... ].
            bool is_good = entry.is_array();
            if (!is_good)
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::domain_error,
                    aftermath::severity_level::major,
                    std::string("JSON representation for ") + name + std::string(" should be an array."), __FUNCTION__, __LINE__);
                return false;
            }

            // Make sure the element types withing the array are consistent with <t_value_type>.
            for (const nlohmann::json& sub_entry : entry)
            {
                bool is_element_good = detail::json_type_check<t_value_type>::is(sub_entry);
                if (is_element_good) continue;
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::domain_error,
                    aftermath::severity_level::major,
                    std::string("JSON representation for one of ") + name + std::string(" elements malformed."), __FUNCTION__, __LINE__);
                return false;
            }

            // Now that all the checks have been made, populate the vector.
            value.clear();
            for (const nlohmann::json& sub_entry : entry) value.push_back(sub_entry.get<t_value_type>());
            return true;
        } // try_parse(...)

        /** @brief Tries to read a required or optional record from json.
         *  @return True if the record was acceptable.
         *  @remark \p value is overwritten only if the record was found and was properly formed.
         *  @exception not_an_error::runtime_error This error is pushed to \c quiet_error if the required record in \p j is missing.
         *  @exception not_an_error::domain_error This error is pushed to \c quiet_error if the record in \p j is of a wrong type or is malformed.
         */
        template <typename t_value_type>
        static bool required(const nlohmann::json& container, const std::string& key, t_value_type& value, bool is_optional = false) noexcept
        {
            if (type::is_missing(container, key, is_optional)) return is_optional; // Allow missing keys for optional values.
            return type::try_parse(container[key], key, value);
        } // required(...)

        /** @brief Tries to read a required or optional value from json.
         *  @return True if the record was acceptable.
         *  @remark \p value is overwritten only if the record was found and was properly formed.
         *  @exception not_an_error::runtime_error This error is pushed to \c quiet_error if the required record in \p j is missing.
         *  @exception not_an_error::domain_error This error is pushed to \c quiet_error if the record in \p j is of a wrong type or is malformed.
         */
        template <typename t_value_type>
        static bool required(const nlohmann::json& container, const std::string& key, std::vector<t_value_type>& value, bool is_optional = false) noexcept
        {
            if (type::is_missing(container, key, is_optional)) return is_optional; // Allow missing keys for optional values.
            return type::try_parse(container[key], key, value);
        } // required(...)
        
        template <typename t_value_type>
        static bool optional(const nlohmann::json& container, const std::string& key, t_value_type& value) noexcept
        {
            return type::required(container, key, value, true);
        } // optional(...)
        
        template <typename t_value_type>
        static bool optional(const nlohmann::json& container, const std::string& key, std::vector<t_value_type>& value) noexcept
        {
            return type::required(container, key, value, true);
        } // optional(...)
    }; // struct quiet_json
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_JSON_HPP_INCLUDED
