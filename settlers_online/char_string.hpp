
#ifndef ROPUFU_SETTLERS_ONLINE_CHAR_STRING_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CHAR_STRING_HPP_INCLUDED

#include <cctype> // std::isspace, std::tolower
#include <cstddef> // std::size_t
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        /** Class for trivial one-char-per-symbol string manipulation. */
        struct char_string
        {
            static bool is_space(char c) noexcept
            {
                return std::isspace(static_cast<unsigned char>(c));
            }

            static char to_lower(char c) noexcept
            {
                return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }

            static void to_lower(std::string& value)
            {
                for (char& c : value) c = char_string::to_lower(c);
            }

            static void space_to_whitespace(std::string& value)
            {
                for (char& c : value) if (char_string::is_space(c)) c = ' ';
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

            /** @brief Locates all the occurences of \p what in \p value in a non-overlapping fashion; then replaces each with \p with.
             *  @example If in "xxxxx" one would ask to replace "xx" with "yx" one would get "yxyxx".
             */
            static void replace(std::string& value, const std::string& what, const std::string& with)
            {
                if (what.length() == 0) return;
                std::vector<std::size_t> indices { };
                std::size_t index = value.find(what);
                while (index != std::string::npos)
                {
                    indices.push_back(index);
                    // .  .  .  w  h  a  t  .  .  . 
                    //          ^           ^       
                    //        index    next search 
                    index = value.find(what, index + what.length());
                }
                while (indices.size() > 0)
                {
                    index = indices.back();
                    value.replace(index, what.length(), with);
                    indices.pop_back();
                }
            }

            static bool ends_with(const std::string& value, const std::string& ending)
            {
                if (ending == "") return true;
                if (ending.length() > value.length()) return false;
                if (ending.length() == value.length()) return value == ending;

                std::size_t index_from = value.length() - ending.length();
                return (value.substr(index_from, ending.length()) == ending);
            }

            static std::string trim_copy(const std::string& value)
            {
                std::size_t count = value.size();
                if (count == 0) return "";

                std::size_t first_index = 0;
                for (; first_index < count; ++first_index) if (!char_string::is_space(value[first_index])) break; // Skip the leading spaces.
                if (first_index == count) return "";
                
                std::size_t last_index = count - 1;
                // First index points to an element that is not a space; so last index is bount to hit it.
                for (;; --last_index) if (!char_string::is_space(value[last_index])) break; // Skip the trailing spaces.
                return value.substr(first_index, last_index - first_index + 1);
            }

            /** Trims the string, replaces spaces with whitespaces, and removes repeated whitespaces. */
            static std::string deep_trim_copy(const std::string& value)
            {
                std::string result = char_string::trim_copy(value);
                char_string::space_to_whitespace(result);
                char_string::remove_repeated_whitespace(result);
                return result;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_CHAR_STRING_HPP_INCLUDED