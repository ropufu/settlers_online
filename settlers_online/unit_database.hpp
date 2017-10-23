
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>

#include "char_string.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <fstream> // std::ifstream
#include <map> // std::map
#include <set> // std::set
#include <string> // std::string
#include <vector> // std::vector

#include <experimental/filesystem>
#include <nlohmann/json.hpp>

namespace ropufu
{
    namespace settlers_online
    {
        template <typename t_key_type, typename t_skeleton_type = t_key_type>
        struct lookup
        {
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
            }

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
            }

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
            }
        };

        /** @brief Class for accessing known units.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        struct unit_database
        {
            using type = unit_database;
            using key_type = std::string;

        private:
            std::map<key_type, unit_type> m_database = { }; // Primary key: shortest name.
            std::set<std::size_t> m_ids = { }; // Keep track of id's to prevent group collision in \army.
            lookup<key_type, key_type> m_lowercase_lookup = { }; // Lookup for lowercase names.
            lookup<key_type, key_type> m_misspelled_lookup = { }; // Lookup for misspelled names.

            /** Relaxed lookup stage 1. */
            static key_type relax_to_lowercase(const key_type& query) noexcept
            {
                key_type relaxed = query;
                char_string::to_lower(relaxed);
                return relaxed;
            }

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
            }

            /** Assuming the unit names have already been subject to \c deep_trim_copy. */
            void update_lookup(const key_type& key, const unit_type& unit) noexcept
            {
                std::set<key_type> relaxed_names = { };
                std::set<key_type> misspelled_names = { };
                for (const std::string& name : unit.names())
                {
                    key_type stage1 = unit_database::relax_to_lowercase(name);
                    key_type stage2 = unit_database::relax_spelling(stage1);
                    relaxed_names.insert(stage1);
                    misspelled_names.insert(stage2);
                }

                this->m_lowercase_lookup.update(key, relaxed_names);
                this->m_misspelled_lookup.update(key, misspelled_names);
            }

        protected:
            unit_database() noexcept { }
            ~unit_database() noexcept { }

        public:
            /** Clears the contents of the database. */
            void clear() noexcept
            {
                this->m_database.clear();
                this->m_ids.clear();
                this->m_lowercase_lookup.clear();
                this->m_misspelled_lookup.clear();
            }

            const std::map<key_type, unit_type>& data() const noexcept { return this->m_database; }

            const unit_type& at(const key_type& key) const noexcept 
            {
                return this->m_database.at(key);
            }

            template <typename t_predicate_type>
            bool try_find(const key_type& query, unit_type& unit, const t_predicate_type& filter) const noexcept 
            {
                auto search = this->m_database.find(query);
                if (search != this->m_database.end())
                {
                    unit = search->second;
                    if (filter(unit)) return true;
                }

                // Primary search failed. Secondary search: all lowercase!
                key_type key;
                key_type lowercase = unit_database::relax_to_lowercase(query);
                key_type misspelled = unit_database::relax_spelling(lowercase);
                std::size_t count_matches;
                count_matches = this->m_lowercase_lookup.try_find(lowercase, key, [&] (const key_type& maybe) { return filter(this->m_database.at(maybe)); });
                if (count_matches >= 1)
                {
                    unit = this->m_database.at(key);
                    if (count_matches == 1) return true;
                    aftermath::quiet_error::instance().push(aftermath::not_an_error::runtime_error, aftermath::severity_level::minor, "Multiple units match the specified query.", lowercase, count_matches);
                    return false;
                }
                count_matches = this->m_misspelled_lookup.try_find(misspelled, key, [&] (const key_type& maybe) { return filter(this->m_database.at(maybe)); });
                if (count_matches >= 1)
                {
                    unit = this->m_database.at(key);
                    if (count_matches == 1) return true;
                    aftermath::quiet_error::instance().push(aftermath::not_an_error::runtime_error, aftermath::severity_level::minor, "Multiple units match the specified query.", misspelled, count_matches);
                    return false;
                }
                return false;
            }

            key_type build_key(const unit_type& unit) const noexcept
            {
                key_type key = unit.names().front();
                for (const std::string& name :  unit.names())
                {
                    // Take the shortest name as the key.
                    if (name.length() < key.length()) key = name;
                }
                return key;
            }

            /** Load units from .json files in a specified folder. */
            std::size_t load_from_folder(const std::string& folder_path)
            {
                std::size_t count = 0;
                for (const std::experimental::filesystem::directory_entry& p : std::experimental::filesystem::directory_iterator(folder_path))
                {
                    std::ifstream i(p.path()); // Try to open the file for reading.
                    if (!i.good()) continue; // Skip on failure.

                    try
                    {
                        nlohmann::json map;// = nlohmann::json::parse(i);
                        i >> map;
                        if (map.count("units") != 0) // Check if there are any unit definitions in the file.
                        {
                            for (const nlohmann::json& unit : map["units"])
                            {
                                unit_type u = unit;
                                std::vector<std::string>& names = u.names();
                                for (std::string& name : names) name = char_string::deep_trim_copy(name);

                                key_type key = this->build_key(u);
                                
                                if (this->m_database.count(key) != 0)
                                {
                                    aftermath::quiet_error::instance().push(
                                        aftermath::not_an_error::logic_error,
                                        aftermath::severity_level::negligible,
                                        "Unit with same name already exists.", p.path().string(), __LINE__);
                                }
                                else if (this->m_ids.count(u.id()) != 0)
                                {
                                    aftermath::quiet_error::instance().push(
                                        aftermath::not_an_error::logic_error,
                                        aftermath::severity_level::negligible,
                                        "Unit with same id already exists.", p.path().string(), __LINE__);
                                }
                                else
                                {
                                    this->m_database.emplace(key, u);
                                    this->m_ids.insert(u.id());
                                    this->update_lookup(key, u);
                                    count++;
                                }
                            }
                        }
                    }
                    catch (const std::exception& /*e*/)
                    {
                        //std::cout << "Failed while reading " << p.path() << ": " << e.what() << std::endl;
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::minor,
                            "Parsing error encountered.", p.path().string(), __LINE__);
                        continue;
                    }
                    catch (...)
                    {
                        aftermath::quiet_error::instance().push("Something went wrong.");
                        continue;
                    }
                }
                return count;
            }

            /** The only instance of this type. */
            static type& instance()
            {
                // Since it's a static variable, if the class has already been created, it won't be created again.
                // Note: it is thread-safe in C++11.
                static type s_instance;
                // Return a reference to our instance.
                return s_instance;
            }

            // ~~ Delete copy and move constructors and assign operators ~~
            unit_database(const type&) = delete;    // Copy constructor.
            unit_database(type&&)      = delete;    // Move constructor.
            type& operator =(const type&) = delete; // Copy assign.
            type& operator =(type&&)      = delete; // Move assign.
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
