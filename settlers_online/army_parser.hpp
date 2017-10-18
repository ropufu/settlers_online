
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_PARSER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_PARSER_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>

#include "string_more.hpp"
#include "unit_database.hpp"
#include "unit_type.hpp"
#include "unit_group.hpp"
#include "army.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string
#include <utility> // std::pair, std::make_pair
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Class for parsing armies.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        struct army_parser
        {
            using type = army_parser;

        private:
            bool m_is_valid = false;
            std::vector<std::pair<std::size_t, std::string>> m_army_blueprint = { };

            /** Parses the provided \p value. It is assumed that the string has already been trimmed, and all spaces replaced with whitespaces. */
            void blueprint(std::string&& value)
            {
                constexpr std::size_t zero = static_cast<std::size_t>('0');
                this->m_is_valid = false;
                std::size_t count = 0;
                std::string key = "";

                std::size_t position = 0;
                std::size_t first_index = 0;
                std::size_t last_index = 0;
                while (position < value.size())
                {
                    // Scan mode: we are building a number.
                    count = 0;
                    first_index = position;
                    for (; position < value.size(); ++position)
                    {
                        char c = value[position];
                        if (c < '0' || c > '9') break; // Non-digit encountered!

                        count *= 10;
                        count += (static_cast<std::size_t>(c) - zero);
                    }
                    if (first_index == position) return; // The first non-whitespace character was not a digit: invalid format.
                    // aftermath::quiet_error::instance().push(
                    //     aftermath::not_an_error::all_good,
                    //     aftermath::severity_level::not_at_all,
                    //     "Group size aknowledged.", value, count);

                    for (; position < value.size(); ++position) if (value[position] != ' ') break; // Skip the leading whitespaces (at most one---cf. assumptions).

                    // Scan mode: we are now building a name.
                    first_index = position;
                    last_index = 0;
                    for (; position < value.size(); ++position)
                    {
                        char c = value[position];
                        if (c >= '0' && c <= '9') break; // Digit encountered!
                        if (c != ' ') last_index = position; // Skip the trailing whitespaces.
                    }
                    if (last_index == 0) return; // The first non-whitespace character was a digit (not a letter!): invalid format.
                    key = value.substr(first_index, last_index - first_index + 1);
                    this->m_army_blueprint.emplace_back(count, key);
                    
                    // aftermath::quiet_error::instance().push(
                    //     aftermath::not_an_error::all_good,
                    //     aftermath::severity_level::not_at_all,
                    //     "Group parsed.", key, count);
                }
                this->m_is_valid = true;
            }

        public:
            /** Clears the contents of the database. */
            army_parser(const std::string& value) noexcept
            {
                this->blueprint(string_more::deep_trim_copy(value));
                if (!this->m_is_valid) this->m_army_blueprint.clear();
            }

            bool good() const noexcept { return this->m_is_valid; }

            std::size_t size() const noexcept { return this->m_army_blueprint.size(); }

            bool try_build_fast(const unit_database& db, army& a) noexcept
            {
                if (!this->m_is_valid) return false;

                std::vector<unit_group> groups = { };
                groups.reserve(this->m_army_blueprint.size());
                for (const auto& pair : this->m_army_blueprint)
                {
                    unit_type u;
                    if (!db.try_find(pair.second, u)) return false;
                    if (pair.first == 0) continue; // Skip empty groups.

                    groups.emplace_back(u, pair.first);
                }
                a = groups;
                return true;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_PARSER_HPP_INCLUDED
