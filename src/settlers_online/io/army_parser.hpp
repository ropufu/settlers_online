
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_PARSER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_PARSER_HPP_INCLUDED

#include "../char_string.hpp"

#include "../combat/army.hpp"
#include "../combat/unit_group.hpp"
#include "../combat/unit_type.hpp"

#include "unit_database.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string
#include <system_error> // std::error_code, std::errc
#include <utility> // std::pair, std::make_pair
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Class for parsing armies. */
        struct army_parser
        {
            using type = army_parser;

        private:
            std::string m_value = "";
            bool m_is_valid = false;
            std::vector<std::pair<std::size_t, std::string>> m_army_blueprint = {};

            /** Parses the provided \p value. It is assumed that the string has already been trimmed, and all spaces replaced with whitespaces. */
            void parse_blueprint(const std::string& value) noexcept
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
                }
                this->m_is_valid = true;
            } // parse_blueprint(...)

        public:
            army_parser(const std::string& value) noexcept
                : m_value(char_string::deep_trim_copy(value))
            {
                this->parse_blueprint(this->m_value);
                if (!this->m_is_valid) this->m_army_blueprint.clear();
            } // army_parser(...)

            const std::string& value() const noexcept { return this->m_value; }

            bool good() const noexcept { return this->m_is_valid; }

            std::size_t size() const noexcept { return this->m_army_blueprint.size(); }

            // bool try_build(army& a, const unit_database& db) const noexcept
            // {
            //     //return this->try_build(a, db, [] (const unit_type& /**u*/) { return true; });
            //     return this->try_build(a, db, nullptr);
            // } // try_build(...)

            template <typename t_predicate_type>
            bool try_build(army& a, const unit_database& db, const t_predicate_type& filter) const noexcept
            {
                if (!this->m_is_valid) return false;

                std::vector<unit_group> groups {};
                groups.reserve(this->m_army_blueprint.size());
                for (const auto& pair : this->m_army_blueprint)
                {
                    // unit_type u {};
                    // if (!db.try_find(pair.second, u, filter)) return false;
                    const unit_type& u = db.find(pair.second, filter);
                    if (!db.is_valid(u)) return false;

                    if (pair.first == 0) continue; // Skip empty groups.
                    groups.emplace_back(u, pair.first);
                }

                std::error_code ec {};
                camp c {};
                a = army(groups, c, ec);
                return true;
            } // try_build(...)
            
            template <typename t_logger_type>
            army build(const unit_database& db, t_logger_type& logger, bool do_check_generals = false, bool do_coerce_factions = false, bool is_strict = false) const noexcept
            {
                army a {};
                if (!this->m_is_valid)
                {
                    logger.write("Parsing army failed.");
                    return a;
                }

                //if (!this->try_build(a, db, [] (const unit_type& /**u*/) { return true; }))
                if (!this->try_build(a, db, nullptr))
                {
                    logger.write("Reconstructing army from database failed.");
                    return a;
                }

                if (a.empty()) return a; // There is nothing more to be done if the army is empty.
                
                // Check for missing generals.
                if (do_check_generals)
                {
                    if (!a.has(unit_faction::general)) logger.write("Army does not have any generals.");
                }

                // Check for multiple factions.
                if (do_coerce_factions)
                {
                    const auto& format_compact = [&] (const unit_type& u) { return " " + u.first_name(); }; // TODO: replace with codenames where possible.

                    std::set<unit_faction> factions {};
                    for (const auto& g : a.groups()) factions.insert(g.unit().faction());
                    factions.erase(unit_faction::general); // Generals do not count.

                    std::size_t count_options = 0;
                    if (factions.size() > 1)
                    {
                        logger.write("There is more than one faction in the army.");
                        army b {};
                        for (unit_faction f : factions)
                        {
                            // Try to re-build this army assuming only fraction <f> is allowed (or generals).
                            if (this->try_build(b, db, [&] (const unit_type& u) { return u.is(f) || u.is(unit_faction::general); }))
                            {
                                ++count_options;
                                logger.write("Did you mean: " + b.to_string(format_compact) + "?");
                            }
                        }
                        // If there is only one alternative with single faction, take it!
                        if (count_options == 1 && !is_strict)
                        {
                            logger.write("Assuming yes.");
                            a = b;
                        }
                    }
                }
                return a;
            } // build(...)
        }; // struct army_parser
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_PARSER_HPP_INCLUDED
