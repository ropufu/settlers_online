
#ifndef ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED

#include <experimental/filesystem>
#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>

#include "../char_string.hpp"
#include "../name_database.hpp"

#include "../combat/unit_type.hpp"

#include <cstddef> // std::size_t
#include <fstream> // std::ifstream
#include <map> // std::map
#include <set> // std::set
#include <stdexcept>  // std::runtime_error
#include <string> // std::string
#include <type_traits> // std::decay
#include <utility> // std::declval
#include <vector> // std::vector

namespace ropufu::settlers_online
{
    /** @brief Class for accessing known units. */
    struct unit_database : public name_database<unit_database, unit_type, std::size_t>
    {
        using type = unit_database;
        using base_type = name_database<unit_database, unit_type, std::size_t>;
        using value_type = unit_type;
        using key_type = std::size_t; //std::decay_t<decltype(std::declval<unit_type>().id())>;
        using name_type = std::string;

        friend struct name_database<unit_database, unit_type, std::size_t>;

    protected:
        /** @brief Retrieves the database key. */
        key_type on_build_key(const value_type& unit) const noexcept
        {
            return unit.id();
        } // on_build_key(...)

        /** @brief Retrieves the names of the database entry. */
        std::set<name_type> on_build_names(const value_type& unit) const noexcept
        {
            std::set<name_type> names {};
            for (const name_type& x : unit.names()) names.insert(x);
            return names;
        } // on_build_names(...)

        // /** @brief Happens before \c relax(...) is called. */
        // void on_relaxing(const value_type& unit, bool& do_cancel) const noexcept
        // {
        // } // on_relaxing(...)

        /** @brief Happens after \c relax(...) has been called. */
        void on_relaxed(const value_type& unit, const std::set<name_type>& /*relaxed_names*/, std::set<name_type>& strict_names) const noexcept
        {
            for (const name_type& code : unit.codenames()) strict_names.insert(code);
        } // on_relaxed(...)

        // /** @brief Happens when \c clear() is called. */
        // void on_clear() noexcept
        // {
        // } // on_clear(...)

        // /** @brief Happens when a unit is about to be loaded. */
        // template <typename t_logger_type>
        // void on_loading(const value_type& unit, bool& do_cancel, t_logger_type& logger) const noexcept
        // {
        // } // on_loading(...)

        // /** @brief Happens when a unit has been loaded. */
        // void on_loaded(const value_type& unit) noexcept
        // {
        // } // on_loaded(...)

    public:
        unit_database() noexcept { }

        /** Load units from .json files in a specified folder. */
        template <typename t_logger_type>
        std::size_t load_from_folder(const std::string& folder_path, t_logger_type& logger) noexcept
        {
            std::size_t count = 0;
            try
            {
                for (const std::experimental::filesystem::directory_entry& p : std::experimental::filesystem::directory_iterator(folder_path))
                {
                    std::ifstream i(p.path()); // Try to open the file for reading.
                    if (!i.good()) continue; // Skip on failure.

                    try
                    {
                        nlohmann::json map {}; // map = nlohmann::json::parse(i);
                        i >> map;
                        if (map.count("units") != 0) // Check if there are any unit definitions in the file.
                        {
                            unit_type bad {};
                            for (const nlohmann::json& json_unit : map["units"])
                            {
                                unit_type unit = json_unit;
                                if (unit == bad) continue;

                                std::vector<std::string> names = unit.names();
                                for (std::string& name : names) name = char_string::deep_trim_copy(name);
                                unit.set_names(names);

                                if (this->add(unit, logger)) ++count;
                            }
                        } // if (...)
                    } // try
                    catch (const std::exception& /*e*/)
                    {
                        logger.write(std::string("Parsing error encountered in ") + p.path().string() + std::string("."));
                        continue;
                    } // catch (...)
                    catch (...)
                    {
                        logger.write(std::string("Something went very wrong with ") + p.path().string() + std::string("."));
                        continue;
                    } // catch (...)
                } // for (...)
            } // try
            catch (...)
            {
                logger.write(std::string("Reading ") + folder_path + std::string(" failed."));
                return 0;
            } // catch (...)
            return count;
        } // load_from_folder(...)
    }; // struct unit_database
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_UNIT_DATABASE_HPP_INCLUDED
