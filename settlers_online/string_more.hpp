
#ifndef ROPUFU_SETTLERS_ONLINE_STRING_MORE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_STRING_MORE_HPP_INCLUDED

#include <algorithm> // std::transform
#include <cctype> // std::isspace, std::tolower
#include <cstddef> // std::size_t
#include <string> // std::string

namespace ropufu
{
    namespace settlers_online
    {
        /** Class for trivial string manipulation. */
        struct string_more
        {
            static bool is_space(char c) noexcept
            {
                return std::isspace(static_cast<unsigned char>(c));
            }

            static char to_lower(char c) noexcept
            {
                return std::tolower(static_cast<unsigned char>(c));
            }

            static void to_lower(std::string& value)
            {
                std::transform(
                    value.begin(), value.end(), value.begin(), 
                    [] (unsigned char c) { return std::tolower(c); });
            }    

            static void space_to_whitespace(std::string& value)
            {
                for (std::size_t i = 0; i < value.size(); ++i) if (is_space(value[i])) value[i] = ' ';
            }

            static void remove_repeated_whitespace(std::string& value)
            {
                std::size_t index = value.find("  ");
                while (index != std::string::npos)
                {
                    value.erase(index, 1);
                    index = value.find("  "); // Move on to the next repeated double space.
                }
            }

            static std::string trim_copy(const std::string& value)
            {
                std::size_t count = value.size();
                if (count == 0) return "";

                std::size_t first_index = 0;
                for (; first_index < count; ++first_index) if (!is_space(value[first_index])) break; // Skip the leading spaces.
                if (first_index == count) return "";
                
                std::size_t last_index = count - 1;
                // First index points to an element that is not a space; so last index is bount to hit it.
                for (;; --last_index) if (!is_space(value[last_index])) break; // Skip the trailing spaces.
                return value.substr(first_index, last_index - first_index + 1);
            }

            /** Trims the string, replaces spaces with whitespaces, and removes repeated whitespaces. */
            static std::string deep_trim_copy(const std::string& value)
            {
                std::string result = trim_copy(value);
                space_to_whitespace(result);
                remove_repeated_whitespace(result);
                return result;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_STRING_MORE_HPP_INCLUDED
