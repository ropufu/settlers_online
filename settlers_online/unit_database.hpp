
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>

#include "string_more.hpp"
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

            std::size_t try_find(const skeleton_type& query, key_type& key) const noexcept
            {
                auto search = this->m_inverse_skeleton.find(query);
                if (search != this->m_inverse_skeleton.end())
                {
                    const std::set<key_type>& matches = search->second;
                    key = *(matches.begin());
                    return matches.size();
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
            using skeleton_type = std::string;

        private:
            std::map<key_type, unit_type> m_database = { }; // Primary key: shortest name.
            std::set<std::size_t> m_ids = { }; // Keep track of id's to prevent group collision in \army.
            lookup<key_type, skeleton_type> m_lowercase_name_lookup = { };

            /** Assuming the unit names have already been subject to \c deep_trim_copy. */
            void update_lookup(const key_type& key, const unit_type& unit)
            {
                std::set<skeleton_type> weak_names = { };
                for (const std::string& name : unit.names())
                {
                    std::string weak = name;
                    string_more::to_lower(weak);
                    weak_names.insert(weak);
                }
                this->m_lowercase_name_lookup.update(key, weak_names);
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
                this->m_lowercase_name_lookup.clear();
            }

            const unit_type& at(const key_type& key) const noexcept 
            {
                return this->m_database.at(key);
            }

            bool try_find(const key_type& query, unit_type& unit) const noexcept 
            {
                auto search = this->m_database.find(query);
                if (search != this->m_database.end())
                {
                    unit = search->second;
                    return true;
                }
                // Primary search failed. Secondary search: all lowercase!
                key_type key;
                std::string lowercase = query;
                string_more::to_lower(lowercase);
                std::size_t count_matches = this->m_lowercase_name_lookup.try_find(lowercase, key);
                if (count_matches >= 1)
                {
                    unit = this->m_database.at(key);
                    if (count_matches > 1)
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::minor,
                            "Multiple units match the specified name.", query, count_matches);
                        return false;
                    }
                    return true;
                }
                return false;
            }

            bool try_full_name(const std::string& name, unit_type& unit) const noexcept 
            {
                for (const auto& pair : this->m_database)
                {
                    bool is_match = false;
                    for (const std::string& s : pair.second.names()) if (s == name) is_match = true;
                    if (is_match)
                    {
                        unit = pair.second;
                        return true;
                    }
                }
                return false;
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
                                for (std::string& name : names) name = string_more::deep_trim_copy(name);

                                std::string key = names.front();
                                for (const std::string& name : names)
                                {
                                    // Take the shortest name as the key.
                                    if (name.length() < key.length()) key = name;
                                }
                                
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
