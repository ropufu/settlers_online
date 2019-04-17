
#ifndef ROPUFU_SETTLERS_ONLINE_CHAR_STRING_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_CHAR_STRING_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include <cctype>   // std::isspace, std::tolower
#include <chrono>   // std::chrono::duration
#include <cstddef>  // std::size_t
#include <iomanip>  // std::defaultfloat
#include <iterator> // std::ostream_iterator
#include <sstream>  // std::ostringstream
#include <string>   // std::string, std::stol, std::stod
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::is_same_v
#include <vector>   // std::vector

using namespace std::literals::chrono_literals;

namespace ropufu::settlers_online
{
    /** Class for trivial one-char-per-symbol string manipulation. */
    struct char_string
    {
        static bool is_space(char c) noexcept { return std::isspace(static_cast<unsigned char>(c)); }
        static bool is_digit(char c) noexcept { return std::isdigit(static_cast<unsigned char>(c)); }
        static bool is_letter(char c) noexcept { return std::isalpha(static_cast<unsigned char>(c)); }

        static char to_lower(char c) noexcept
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } // to_lower(...)

        static void to_lower(std::string& value) noexcept
        {
            for (char& c : value) c = char_string::to_lower(c);
        } // to_lower(...)

        static void space_to_whitespace(std::string& value) noexcept
        {
            for (char& c : value) if (char_string::is_space(c)) c = ' ';
        } // space_to_whitespace(...)

        static void remove_repeated_whitespace(std::string& value) noexcept
        {
            std::size_t index = value.find("  ");
            while (index != std::string::npos)
            {
                value.erase(index, 1);
                index = value.find("  "); // Move on to the next repeated double space.
            }
        } // remove_repeated_whitespace(...)

        /** @brief Locates all the occurences of \p what in \p value in a non-overlapping fashion; then replaces each with \p with.
         *  @example If in "xxxxx" one would ask to replace "xx" with "yx" one would get "yxyxx".
         */
        static void replace(std::string& value, const std::string& what, const std::string& with) noexcept
        {
            if (what.length() == 0) return;
            std::vector<std::size_t> indices {};
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
        } // replace(...)

        static bool starts_with(const std::string& value, const std::string& beginning) noexcept
        {
            if (beginning == "") return true;
            if (beginning.length() > value.length()) return false;
            if (beginning.length() == beginning.length()) return value == beginning;

            return (value.substr(0, beginning.length()) == beginning);
        } // starts_with(...)

        static bool ends_with(const std::string& value, const std::string& ending) noexcept
        {
            if (ending == "") return true;
            if (ending.length() > value.length()) return false;
            if (ending.length() == value.length()) return value == ending;

            std::size_t index_from = value.length() - ending.length();
            return (value.substr(index_from, ending.length()) == ending);
        } // ends_with(...)

        static std::string trim_copy(const std::string& value) noexcept
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
        } // trim_copy(...)

        /** Trims the string, replaces spaces with whitespaces, and removes repeated whitespaces. */
        static std::string deep_trim_copy(const std::string& value) noexcept
        {
            std::string result = char_string::trim_copy(value);
            char_string::space_to_whitespace(result);
            char_string::remove_repeated_whitespace(result);
            return result;
        } // deep_trim_copy(...)

        template <typename t_string_collection_type>
        static std::string join(const t_string_collection_type& values, const std::string& delimiter) noexcept
        {
            std::ostringstream s {};
            bool is_first = true;
            for (const std::string& x : values)
            {
                if (!is_first) s << delimiter;
                s << x;
                is_first = false;
            }
            return s.str();
        } // join(...)

        static std::vector<std::string> split(const std::string& value, const std::string& delimiter) noexcept
        {
            if (delimiter.empty()) return {};

            std::vector<std::string> result {};
            std::size_t offset = 0;
            std::size_t index_of_delimiter = value.find(delimiter, offset);
            while (index_of_delimiter != std::string::npos) 
            {
                // ---  .  ---  .  ---  #  --- # --- . ---  
                //      ^               ^                   
                //      offset          index               
                result.push_back(value.substr(offset, index_of_delimiter - offset));
                offset = index_of_delimiter + delimiter.size();
                // ---  .  ---  .  ---  #  --- # --- . ---  
                //                                   ^      
                //                                   offset 
                index_of_delimiter = value.find(delimiter, offset);
            }
            result.push_back(value.substr(offset));
            return result;
        } // split(...)

        template <typename t_rep_type, typename t_period_type>
        static std::string from_duration(std::chrono::duration<t_rep_type, t_period_type> time_interval) noexcept
        {
            using time_type = std::chrono::milliseconds;
            using count_type = typename time_type::rep;

            time_type t = std::chrono::duration_cast<time_type>(time_interval);
            time_type one_hour = 1h;
            time_type one_minute = 1min;
            time_type one_second = 1s;

            if (t.count() == 0) return "0s";
            std::ostringstream output {};
            output << std::defaultfloat;

            if (t.count() < 0)
            {
                output << '-';
                t = -t;
            } // if (...)

            count_type whole_hours = t.count() / one_hour.count();
            t -= whole_hours * one_hour;
            count_type whole_minutes = t.count() / one_minute.count();
            t -= whole_minutes * one_minute;
            double seconds = static_cast<double>(t.count()) / one_second.count();

            std::string separator = "";
            if (whole_hours != 0) { output << separator << whole_hours << "h"; separator = " "; }
            if (whole_minutes != 0) { output << separator << whole_minutes << "min"; separator = " "; }
            output << separator << seconds << "s";

            return output.str();
        } // from_duration(...)

        template <typename t_duration_type>
        static auto to_duration(const std::string& time_str, std::error_code& ec) noexcept
            -> std::chrono::duration<typename t_duration_type::rep, typename t_duration_type::period>
        {
            using count_type = typename t_duration_type::rep;
            using period_type = typename t_duration_type::period;
            using duration_type = std::chrono::duration<count_type, period_type>;

            using hours_period = typename std::chrono::hours::period;
            using minutes_period = typename std::chrono::minutes::period;
            using seconds_period = typename std::chrono::seconds::period;

            static_assert(std::is_same_v<t_duration_type, duration_type>, "std::chrono::duration type expected.");

            duration_type result {};
            std::vector<std::string> str_blocks {};
            std::vector<char> current_block {};
            str_blocks.reserve(time_str.length());
            current_block.reserve(time_str.length());

            bool is_negative = false;
            bool has_started = false;
            bool is_digit_block_floating = false;
            bool is_digit_block = false;
            bool is_letter_block = false;
            for (char c : time_str)
            {
                bool is_space = char_string::is_space(c);
                bool is_digit = char_string::is_digit(c);
                bool is_letter = char_string::is_letter(c);
                bool is_period = (c == '.');

                // Check the sign.
                if (!has_started && c == '-')
                {
                    is_negative = true;
                    has_started = true;
                    continue;
                } // if (...)

                // Ignore spaces.
                if (is_space) continue;
\
                has_started = true;
                if (is_digit || is_period)
                {
                    is_digit_block = true;
                    if (is_letter_block) // Flush the preceeding letter block.
                    {
                        is_letter_block = false;
                        str_blocks.emplace_back(current_block.data(), current_block.size());
                        current_block.clear();
                    } // if (...)

                    // Make sure there is only one period in each digit block.
                    if (is_digit_block_floating && is_period) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Time string not recognized.", result);
                    if (is_period) is_digit_block_floating = true;
                    current_block.push_back(c);
                } // if (...)
                else if (is_letter)
                {
                    is_letter_block = true;
                    if (is_digit_block) // Flush the preceeding digit block.
                    {
                        is_digit_block = false;
                        is_digit_block_floating = false;
                        str_blocks.emplace_back(current_block.data(), current_block.size());
                        current_block.clear();
                    } // if (...)

                    // Make sure letter block is not the first block.
                    if (str_blocks.empty()) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Time string not recognized.", result);
                    current_block.push_back(c);
                } // else if (...)
                else return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Time string not recognized.", result);
            } // for (...)

            // Final block.
            str_blocks.emplace_back(current_block.data(), current_block.size());
            current_block.clear();

            // Make sure there is an even number of blocks (every numeric block should be followed by a letter block).
            if ((str_blocks.size() & 0x01) != 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Time string not recognized.", result);
            for (std::size_t i = 0; i < str_blocks.size(); i += 2)
            {
                const std::string& number_str = str_blocks[i];
                const std::string& unit_str = str_blocks[i + 1];

                bool is_hours = unit_str == "h";
                bool is_minutes = (unit_str == "min") || (unit_str == "m");
                bool is_seconds = unit_str == "s";
                if (!(is_hours || is_minutes || is_seconds))
                    return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Time unit not recognized.", result);

                if (i == str_blocks.size() - 2) // Last pair: use floating point arithmetic.
                {
                    double number = std::stod(number_str);
                    if (is_hours)
                    {
                        std::chrono::duration<double, hours_period> dt { number };
                        result += std::chrono::duration_cast<duration_type>(dt);
                    } // if (...)
                    if (is_minutes)
                    {
                        std::chrono::duration<double, minutes_period> dt { number };
                        result += std::chrono::duration_cast<duration_type>(dt);
                    } // if (...)
                    if (is_seconds)
                    {
                        std::chrono::duration<double, seconds_period> dt { number };
                        result += std::chrono::duration_cast<duration_type>(dt);
                    } // if (...)
                } // if (...)
                else
                {
                    long long number = std::stoll(number_str);
                    if (is_hours)
                    {
                        std::chrono::duration<long long, hours_period> dt { number };
                        result += std::chrono::duration_cast<duration_type>(dt);
                    } // if (...)
                    if (is_minutes)
                    {
                        std::chrono::duration<long long, minutes_period> dt { number };
                        result += std::chrono::duration_cast<duration_type>(dt);
                    } // if (...)
                    if (is_seconds)
                    {
                        std::chrono::duration<long long, seconds_period> dt { number };
                        result += std::chrono::duration_cast<duration_type>(dt);
                    } // if (...)
                } // else (...)
            } // for (...)

            return is_negative ? -result : result;
        } // to_duration(...)

    }; // struct char_string
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_CHAR_STRING_HPP_INCLUDED
