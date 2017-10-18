
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>

#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <fstream> // std::ifstream
#include <map> // std::map
#include <set> // std::set
#include <string> // std::string

#include <experimental/filesystem>
#include <nlohmann/json.hpp>

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Class for accessing known units.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        struct unit_database
        {
            using type = unit_database;
            using key_type = std::string;
            using weak_key_type = std::string;

        private:
            std::map<key_type, unit_type> m_database = { }; // Primary key: shortest name.
            std::set<std::size_t> m_ids = { }; // Keep track of id's to prevent group collision in \army.
            std::map<weak_key_type, std::vector<key_type>> m_multiple_relaxed = { }; // Keeps track of which lowercase-version keys have multiple occurrences.

            weak_key_type relax(const key_type& key)
            {
                weak_key_type weak_key = key;
                return weak_key;
            }

            void build_weak_keys(const key_type& key, const unit_type& unit)
            {

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
                this->m_multiple_relaxed.clear();
            }

            const unit_type& at(const std::string& key) const noexcept 
            {
                return this->m_database.at(key);
            }

            bool try_find(const std::string& key, unit_type& unit) const noexcept 
            {
                auto search = this->m_database.find(key);
                if (search != this->m_database.end())
                {
                    unit = search->second;
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
                                std::string key = u.names().front();
                                for (const std::string& name : u.names())
                                {
                                    if (name.find("  ") != std::string::npos) 
                                        aftermath::quiet_error::instance().push(
                                            aftermath::not_an_error::all_good,
                                            aftermath::severity_level::not_at_all,
                                            "Name has repeated whitespace.", p.path().string(), __LINE__);
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
                                    this->build_weak_keys(key, u);
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
