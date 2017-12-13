
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>

#include "char_string.hpp"
#include "prefix_database.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <fstream> // std::ifstream
#include <map> // std::map
#include <set> // std::set
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Auxiliary class for \c prefix_database specializations. */
        template <>
        struct prefix_builder<unit_type>
        {
            using value_type = unit_type;
            using key_type = std::string;

            static key_type build_key(const value_type& unit) noexcept
            {
                key_type key = unit.names().front();
                for (const std::string& name :  unit.names())
                {
                    // Take the shortest name as the key.
                    if (name.length() < key.length()) key = name;
                }
                return key;
            } // build_key(...)

            static key_type build_primary_name(const value_type& unit) noexcept
            {
                key_type key = unit.names().front();
                char_string::to_lower(key);
                return key;
            } // build_primary_name(...)

            /** @brief Collection of names associated with the \p unit.
             *  @remark Return type has to support range-based loops.
             */
            static const std::vector<key_type>& names(const value_type& unit) noexcept
            {
                return unit.names();
            } // names(...)
        }; // struct prefix_builder

        /** @brief Class for accessing known units.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        struct unit_database : public prefix_database<unit_database, unit_type>
        {
            using type = unit_database;
            using base_type = prefix_database<unit_database, unit_type>;
            using value_type = unit_type;
            using key_type = std::string;

            friend struct prefix_database<unit_database, unit_type>;

        private:
            std::set<std::size_t> m_ids = { }; // Keep track of id's to prevent group collision in \army.

        protected:
            unit_database() noexcept { }
            ~unit_database() noexcept { }

            /** @brief Happens when \c clear() is called. */
            void on_clear() noexcept
            {
                this->m_ids.clear();
            } // on_clear(...)

            /** @brief Happens when a unit is about to be loaded. */
            template <typename t_logger_type>
            void on_loading(const value_type& unit, bool& do_cancel, t_logger_type& logger) const noexcept
            {
                if (this->m_ids.count(unit.id()) != 0)
                {
                    logger.write(std::string("Unit with id ") + std::to_string(unit.id()) + std::string(" already exists."));
                    do_cancel = true;
                }
            } // on_loading(...)

            /** @brief Happens when a unit has been loaded. */
            void on_loaded(const value_type& unit) noexcept
            {
                this->m_ids.insert(unit.id());
            } // on_loaded(...)

        public:
            /** Load units from .json files in a specified folder. */
            template <typename t_logger_type>
            std::size_t load_from_folder(const std::string& folder_path, t_logger_type& logger) noexcept
            {
                std::size_t count = 0;
                for (const std::experimental::filesystem::directory_entry& p : std::experimental::filesystem::directory_iterator(folder_path))
                {
                    std::ifstream i(p.path()); // Try to open the file for reading.
                    if (!i.good()) continue; // Skip on failure.

                    // @todo Get rid of the try/catch statement.
                    try
                    {
                        nlohmann::json map { };// = nlohmann::json::parse(i);
                        i >> map;
                        if (map.count("units") != 0) // Check if there are any unit definitions in the file.
                        {
                            unit_type bad { };
                            for (const nlohmann::json& json_unit : map["units"])
                            {
                                unit_type unit = json_unit;
                                if (unit == bad) continue;

                                std::vector<std::string> names = unit.names();
                                for (std::string& name : names) name = char_string::deep_trim_copy(name);
                                unit.set_names(names);

                                if (this->add(unit, logger)) count++;
                            }
                        }
                    }
                    catch (const std::exception& /*e*/)
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::minor,
                            std::string("Parsing error encountered in ") + p.path().string() + std::string("."), __FUNCTION__, __LINE__);
                        continue;
                    }
                    catch (...)
                    {
                        aftermath::quiet_error::instance().push(
                            aftermath::not_an_error::runtime_error,
                            aftermath::severity_level::fatal,
                            std::string("Something went very wrong.") + p.path().string() + std::string("."), __FUNCTION__, __LINE__);
                        continue;
                    }
                }
                return count;
            } // load_from_folder(...)

            /** The only instance of this type. */
            static type& instance() noexcept
            {
                // Since it's a static variable, if the class has already been created, it won't be created again.
                // Note: it is thread-safe in C++11.
                static type s_instance { };
                // Return a reference to our instance.
                return s_instance;
            } // instance(...)

            // ~~ Delete copy and move constructors and assign operators ~~
            unit_database(const type&) = delete;    // Copy constructor.
            unit_database(type&&)      = delete;    // Move constructor.
            type& operator =(const type&) = delete; // Copy assign.
            type& operator =(type&&)      = delete; // Move assign.
        }; // struct unit_database
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
