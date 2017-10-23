
#ifndef ROPUFU_SETTLERS_ONLINE_BALCK_MARSH_CONFIG_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BALCK_MARSH_CONFIG_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include <fstream> // std::ifstream, std::ofstream
#include <exception> // std::exception
#include <iostream> // std::ostream
#include <string> // std::string

namespace ropufu
{
    namespace settlers_online
    {
        namespace black_marsh
        {
            /** @brief Class for reading and writing configurtation setting.
                *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
                */
            struct config
            {
                using type = config;

            private:
                bool m_is_good = false;
                bool m_has_changed = false;
                std::string m_filename = "";
                nlohmann::json m_json = { };

            protected:
                // Always try to read the default configuration on construction.
                config() noexcept { this->read(); }
                // Always try to save the configuration on exit.
                ~config() noexcept { this->write(); }

            public:
                /** Output configuration parameters. */
                friend std::ostream& operator <<(std::ostream& os, const type& self)
                {
                    os << self.m_json;
                    return os;
                }

                bool good() const noexcept { return this->m_is_good; }

                /** Read the configuration from a file. */
                bool read(std::string filename = "./black_marsh.config") noexcept
                {
                    std::ifstream i(filename); // Try to open the file for reading.
                    if (!i.good()) return false; // Stop on failure.

                    try
                    {
                        this->m_filename = filename; // Remember the filename.
                        i >> this->m_json;
                        this->m_is_good = true;
                        return true;
                    }
                    catch (const std::exception& /*e*/)
                    {
                        this->m_is_good = false;
                        return false;
                    }
                }
                
                /** Write the configuration to a file. */
                bool write() noexcept
                {
                    if (!this->m_has_changed) return true;
                    if (this->write(this->m_filename))
                    {
                        this->m_has_changed = false;
                        return true;
                    }
                    return false;
                }

                /** Write the configuration to a file. */
                bool write(const std::string& filename) const noexcept
                {
                    std::ofstream o(filename); // Try to open the file for writing.
                    if (!o.good()) return false; // Stop on failure.

                    o << std::setw(4) << this->m_json << std::endl;
                    return true;
                }

                /** Expose all the settings. */
                const nlohmann::json& json() const noexcept { return this->m_json;  }

                /** Expose specific settings. */
                const nlohmann::json& operator [](const std::string& parameter_name) const { return this->m_json[parameter_name]; }
                /** Expose specific settings. */
                nlohmann::json& operator [](const std::string& parameter_name) { return this->m_json[parameter_name]; }

                template <typename t_result_type>
                bool maybe(const std::string& name, t_result_type& result) const noexcept
                {
                    if (this->m_json.count(name) != 0)
                    {
                        result = this->m_json[name].get<t_result_type>();
                        return true;
                    }
                    return false;
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
                config(const type&) = delete; // Copy constructor.
                config(type&&)      = delete; // Move constructor.
                type& operator =(const type&) = delete; // Copy assign.
                type& operator =(type&&)      = delete; // Move assign.
            };
        }
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_BALCK_MARSH_CONFIG_HPP_INCLUDED
