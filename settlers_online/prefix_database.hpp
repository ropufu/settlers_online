
#ifndef ROPUFU_SETTLERS_ONLINE_PREFIX_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_PREFIX_DATABASE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>

#include "char_string.hpp"
#include "logger.hpp"
#include "prefix_tree.hpp"

#include <cstddef> // std::size_t
#include <fstream> // std::ifstream
#include <map> // std::map
#include <set> // std::set
#include <string> // std::string
#include <type_traits> // std::is_same
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        template <typename t_key_type, typename t_skeleton_type = t_key_type>
        struct lookup
        {
            using type = lookup<t_key_type, t_skeleton_type>;
            using key_type = t_key_type;
            using skeleton_type = t_skeleton_type;

        private:
            std::map<key_type, std::set<skeleton_type>> m_skeleton = { }; // For eack key, keeps track of all associated relaxed names.
            std::map<skeleton_type, std::set<key_type>> m_inverse_skeleton = { }; // For reach relaxed name, keeps track of all associated keys.

        public:
            void clear() noexcept
            {
                this->m_skeleton.clear();
                this->m_inverse_skeleton.clear();
            } // clear(...)

            void update(const key_type& key, const std::set<skeleton_type>& values) noexcept
            {
                // Record the values associated with the given key.
                this->m_skeleton.emplace(key, values);

                // Now the reverse: find all keys associated with values.
                for (const skeleton_type& weak : values)
                {
                    auto search = this->m_inverse_skeleton.find(weak);
                    if (search != this->m_inverse_skeleton.end()) search->second.insert(key);
                    else
                    {
                        std::set<key_type> matches { key }; // This is the only key that matches <weak>, since otherwise it would've been present in the inverse skeleton.
                        this->m_inverse_skeleton.emplace(weak, matches);
                    }
                }
            } // update(...)

            template <typename t_predicate_type>
            std::size_t try_find(const skeleton_type& query, key_type& key, const t_predicate_type& filter) const noexcept
            {
                auto search = this->m_inverse_skeleton.find(query);
                if (search != this->m_inverse_skeleton.end())
                {
                    const std::set<key_type>& matches = search->second;
                    std::size_t count = 0;
                    for (const key_type& maybe : matches)
                    {
                        if (filter(maybe)) { key = maybe; count++; }
                    }
                    return count;
                }
                return 0;
            } // try_find(...)
        }; // struct lookup

        /** @brief Auxiliary class for \c prefix_database specializations. */
        template <typename t_value_type>
        struct prefix_builder
        {
            using value_type = t_value_type;
            using key_type = std::string;

            static key_type build_key(const value_type& unit) noexcept
            {
                static_assert(false, "<prefix_builder> not specialized.");
            } // build_key(...)

            static key_type build_primary_name(const value_type& unit) noexcept
            {
                static_assert(false, "<prefix_builder> not specialized.");
            } // build_primary_name(...)

            /** @brief Collection of names associated with the \p unit.
             *  @remark Return type has to support range-based loops.
             */
            static std::vector<key_type> names(const value_type& unit) noexcept
            {
                static_assert(false, "<prefix_builder> not specialized.");
            } // names(...)
        }; // struct prefix_builder

        /** @brief Class for accessing known units.
         *  @remark This is an abstract class (CRTP). For more information on CRTP see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
         *  @remark The class has to implement the following functions:
         *          static key_type build_key(const value_type& unit) const noexcept
         */
        template <typename t_derived_type, typename t_value_type>
        struct prefix_database
        {
            using type = prefix_database<t_derived_type, t_value_type>;
            using derived_type = t_derived_type;
            using value_type = t_value_type;
            using key_type = std::string;

        private:
            using builder_type = prefix_builder<value_type>;

            value_type m_invalid = { }; // Used to indicates invalid return result.
            std::map<key_type, value_type> m_database = { }; // Primary key: shortest name.
            std::set<key_type> m_primary_names = { }; // Keep track of primary names to allow for fast prefix search.
            char_tree m_primary_name_tree = { }; // Fast prefix search by primary name.
            lookup<key_type, key_type> m_lowercase_lookup = { }; // Lookup for lowercase names.
            lookup<key_type, key_type> m_misspelled_lookup = { }; // Lookup for misspelled names.

            /** Relaxed lookup stage 1. */
            static key_type relax_to_lowercase(const key_type& query) noexcept
            {
                key_type relaxed = query;
                char_string::to_lower(relaxed);
                return relaxed;
            } // relax_to_lowercase(...)

            /** Relaxed lookup stage 2. Assuming stage 1 has already been applied. */
            static key_type relax_spelling(const key_type& query) noexcept
            {
                key_type relaxed = query;
                // Plural: man -> men.
                char_string::replace(relaxed, "men", "man");
                // Plural: ...es or ...s.
                if (char_string::ends_with(relaxed, "es")) relaxed = relaxed.substr(0, relaxed.length() - 2);
                else if (char_string::ends_with(relaxed, "s")) relaxed = relaxed.substr(0, relaxed.length() - 1);

                // Collapse all repeated letters for "reasonably" long words.
                if (relaxed.length() > 4)
                {
                    char previous = relaxed[0];
                    for (std::size_t i = 1; i < relaxed.size(); ++i)
                    {
                        if (relaxed[i] == previous) relaxed.erase(i, 1);
                        else previous = relaxed[i];
                    }
                }
                
                return relaxed;
            } // relax_spelling(...)

            /** Assuming the unit names have already been subject to \c deep_trim_copy. */
            void update_lookup(const key_type& key, const value_type& unit) noexcept
            {
                std::set<key_type> relaxed_names { };
                std::set<key_type> misspelled_names { };
                for (const key_type& name : builder_type::names(unit))
                {
                    key_type stage1 = type::relax_to_lowercase(name);
                    key_type stage2 = type::relax_spelling(stage1);
                    relaxed_names.insert(stage1);
                    misspelled_names.insert(stage2);
                }

                this->m_lowercase_lookup.update(key, relaxed_names);
                this->m_misspelled_lookup.update(key, misspelled_names);
            } // update_lookup(...)

        protected:
            /** @brief Happens when \c clear() is called. */
            void on_clear() noexcept
            {
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::on_clear), 
                    decltype(&type::on_clear)>::value;
                if (!is_overwritten) return; // Do nothing if the function has not been overridden.

                derived_type* that = static_cast<derived_type*>(this);
                that->on_clear();
            } // on_clear(...)

            /** @brief Happens when a unit is about to be loaded. */
            template <typename t_logger_type>
            void on_loading(const value_type& unit, bool& do_cancel, t_logger_type& logger) const noexcept
            {
                do_cancel = false;
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::template on_loading<t_logger_type>), 
                    decltype(&type::template on_loading<t_logger_type>)>::value;
                if (!is_overwritten) return; // Do nothing if the function has not been overridden.

                const derived_type* that = static_cast<const derived_type*>(this);
                that->on_loading(unit, do_cancel, logger);
            } // on_loading(...)

            /** @brief Happens when a unit has been loaded. */
            void on_loaded(const value_type& unit) noexcept
            {
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::on_loaded), 
                    decltype(&type::on_loaded)>::value;
                if (!is_overwritten) return; // Do nothing if the function has not been overridden.

                derived_type* that = static_cast<derived_type*>(this);
                that->on_loaded(unit);
            } // on_loaded(...)

        public:
            prefix_database() noexcept { }

            /** Clears the contents of the database. */
            void clear() noexcept
            {
                this->m_database.clear();
                this->m_primary_names.clear();
                this->m_primary_name_tree.clear();
                this->m_lowercase_lookup.clear();
                this->m_misspelled_lookup.clear();
                this->on_clear();
            } // clear(...)

            const std::map<key_type, value_type>& data() const noexcept { return this->m_database; }

            /** @brief Access elements by key.
             *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if specified \p key is not in the database.
             */
            const value_type& at(const key_type& key) const noexcept 
            {
                auto search = this->m_database.find(key);
                if (search != this->m_database.end()) return search->second;

                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::out_of_range,
                    aftermath::severity_level::fatal,
                    "<key> is not present in the database.", __FUNCTION__, __LINE__);
                return this->m_invalid;
            } // at(...)

            template <typename t_predicate_type>
            bool try_find(const key_type& query, value_type& unit, const t_predicate_type& filter) const noexcept 
            {
                detail::no_logger logger { };
                return this->try_find(query, unit, filter, logger);
            } // try_find(...)

            template <typename t_predicate_type, typename t_logger_type>
            bool try_find(const key_type& query, value_type& unit, const t_predicate_type& filter, t_logger_type& logger) const noexcept 
            {
                auto search = this->m_database.find(query);
                if (search != this->m_database.end())
                {
                    unit = search->second;
                    if (filter(unit)) return true;
                }

                // Primary search failed. Secondary search: all lowercase!
                key_type key { };
                key_type lowercase = unit_database::relax_to_lowercase(query);
                key_type misspelled = unit_database::relax_spelling(lowercase);
                std::size_t count_matches = 0;

                // Stage 0: prefix tree search.
                bool is_single = false;
                key_type first_prefix_match = this->m_primary_name_tree.first(lowercase, is_single);
                //count_matches = this->m_primary_name_tree.count(lowercase);
                // Only one terminus matches the prefix.
                if (is_single) lowercase = first_prefix_match; // Overwrite the search query.

                // Stage 1: lowercase lookup.
                count_matches = this->m_lowercase_lookup.try_find(lowercase, key, [&] (const key_type& maybe) { return filter(this->m_database.at(maybe)); });
                if (count_matches >= 1)
                {
                    unit = this->m_database.at(key);
                    if (count_matches == 1) return true;
                    logger.write(std::string("Multiple units match the specified query: ") + lowercase + std::string("."));
                    return false;
                }

                // Stage 2: misspelled lookup.
                count_matches = this->m_misspelled_lookup.try_find(misspelled, key, [&] (const key_type& maybe) { return filter(this->m_database.at(maybe)); });
                if (count_matches >= 1)
                {
                    unit = this->m_database.at(key);
                    if (count_matches == 1) return true;
                    logger.write(std::string("Multiple units match the specified query: ") + misspelled + std::string("."));
                    return false;
                }
                return false;
            } // try_find(...)

            /** Load units from .json files in a specified folder. */
            template <typename t_logger_type>
            bool add(const value_type& unit, t_logger_type& logger) noexcept
            {
                bool do_cancel = false;
                this->on_loading(unit, do_cancel, logger);
                if (do_cancel) return false;

                key_type key = builder_type::build_key(unit);
                key_type primary_name = builder_type::build_primary_name(unit);

                if (this->m_database.count(key) != 0)
                {
                    logger.write(std::string("Unit with name ") + key + std::string(" already exists."));
                    return false;
                }
                else if (this->m_primary_names.count(primary_name) != 0)
                {
                    logger.write(std::string("Unit with a similar primary name ") + primary_name + std::string(" already exists."));
                    return false;
                }
                else
                {
                    this->m_database.emplace(key, unit);
                    this->m_primary_names.insert(primary_name);
                    this->m_primary_name_tree.add(primary_name);
                    this->update_lookup(key, unit);
                    this->on_loaded(unit);
                    return true;
                }
            } // add(...)
        }; // struct prefix_database
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_PREFIX_DATABASE_HPP_INCLUDED
