
#ifndef ROPUFU_SETTLERS_ONLINE_BALCK_MARSH_CONFIG_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BALCK_MARSH_CONFIG_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include "../settlers_online/army.hpp"
#include "../settlers_online/army_decorator.hpp"
#include "../settlers_online/report_entry.hpp"
#include "../settlers_online/warnings.hpp"

#include <cstdint> // std::uint8_t
#include <fstream> // std::ifstream, std::ofstream
#include <iostream> // std::ostream
#include <string> // std::string
#include <vector> // std::vector

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
                // ~~ General configuration ~~
                bool m_is_good = false;
                bool m_has_changed = false;
                std::string m_filename = ""; // Where the configuration was loaded from.
                std::string m_cbor_filename = "";
                nlohmann::json m_json = { }; // Raw configuration json.
                // ~~ Specific properties ~~
                std::string m_maps_path = "./maps/";
                std::string m_faces_path = "./faces/";
                std::size_t m_count_combat_sims = 10'000;
                std::size_t m_count_destruct_sims_per_combat = 50;
                std::size_t m_count_threads = 1;
                army_decorator m_left = { };
                army_decorator m_right = { };

                bool has(const std::string& name) const noexcept { return this->m_json.count(name) != 0; }

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

                const army_decorator& left() const noexcept { return this->m_left; }
                army_decorator& left() noexcept { return this->m_left; }

                const army_decorator& right() const noexcept { return this->m_right; }
                army_decorator& right() noexcept { return this->m_right; }

                const std::string& maps_path() const noexcept { return this->m_maps_path; }
                void set_maps_path(const std::string& value) noexcept { this->m_maps_path = value; }
                
                const std::string& faces_path() const noexcept { return this->m_faces_path; }
                void set_faces_path(const std::string& value) noexcept { this->m_faces_path = value; }

                std::size_t simulation_count() const noexcept { return this->m_count_combat_sims; }
                void set_simulation_count(std::size_t value) noexcept { this->m_count_combat_sims = value; this->m_has_changed = true; }

                std::size_t destruction_count() const noexcept { return this->m_count_destruct_sims_per_combat; }
                void set_destruction_count(std::size_t value) noexcept { this->m_count_destruct_sims_per_combat = value; this->m_has_changed = true; }

                std::size_t threads() const noexcept { return this->m_count_threads; }
                void set_threads(std::size_t value) noexcept { this->m_count_threads = value; this->m_has_changed = true; }

                /** Read the configuration from a file. */
                bool read(const std::string& filename = "./black_marsh.config") noexcept
                {
                    std::ifstream i(filename); // Try to open the file for reading.
                    if (!i.good()) return false; // Stop on failure.

                    try
                    {
                        this->m_filename = filename; // Remember the filename.
                        // Replace extension with .cbor.
                        std::size_t index_of_dot = filename.rfind(".");
                        if (index_of_dot == std::string::npos) this->m_cbor_filename = filename + ".cbor";
                        else this->m_cbor_filename = filename.substr(0, index_of_dot) + ".cbor";

                        i >> this->m_json;
                        this->m_is_good = true;

                        // ~~ Parse json ~~
                        std::string maps_path = this->m_maps_path;
                        std::string faces_path = this->m_faces_path;
                        std::size_t n1 = this->m_count_combat_sims;
                        std::size_t n2 = this->m_count_destruct_sims_per_combat;
                        std::size_t count_threads = this->m_count_threads;
                        army_decorator left = this->m_left;
                        army_decorator right = this->m_right;
                        
                        this->maybe("maps folder", maps_path);
                        this->maybe("faces folder", faces_path);
                        this->maybe("simulations", n1);
                        this->maybe("destructions per sim", n2);
                        this->maybe("threads", count_threads);
                        this->maybe("left", left);
                        this->maybe("right", right);

                        this->m_maps_path = maps_path;
                        this->m_faces_path = faces_path;
                        this->m_count_combat_sims = n1;
                        this->m_count_destruct_sims_per_combat = n2;
                        this->m_count_threads = count_threads;
                        this->m_left = left;
                        this->m_right = right;

                        return true;
                    }
                    catch (const std::exception& /*e*/)
                    {
                        this->m_is_good = false;
                        return false;
                    }
                } // read(...)
                
                /** Write the configuration to a file. */
                bool write() noexcept { return this->write(this->m_filename); }

                /** Write the configuration to a file. */
                bool write(const std::string& filename) noexcept
                {
                    if (!this->m_has_changed) return true;
                    std::ofstream o(filename); // Try to open the file for writing.
                    if (!o.good()) return false; // Stop on failure.

                    this->m_json["maps folder"] = this->m_maps_path;
                    this->m_json["faces folder"] = this->m_faces_path;
                    this->m_json["simulations"] = this->m_count_combat_sims;
                    this->m_json["destructions per sim"] = this->m_count_destruct_sims_per_combat;
                    this->m_json["threads"] = this->m_count_threads;
                    this->m_json["left"] = this->m_left;
                    this->m_json["right"] = this->m_right;

                    o << std::setw(4) << this->m_json << std::endl;
                    this->m_has_changed = false;
                    return true;
                } // write(...)
                
                /** Write .cbor output. */
                bool to_cbor(const std::vector<report_entry>& entries) const noexcept
                {
                    return this->to_cbor(entries, this->m_cbor_filename);
                } // to_cbor(...)

                /** Write .cbor output. */
                bool to_cbor(const std::vector<report_entry>& entries, const std::string& cbor_filename) const noexcept
                {
                    std::ofstream o(cbor_filename, std::ios::binary); // Try to open the file for writing.
                    if (!o.good()) return false; // Stop on failure.

                    nlohmann::json j { };
                    j["report"] = entries;
                    std::vector<std::uint8_t> v_cbor = nlohmann::json::to_cbor(j);
                    o.write(reinterpret_cast<char*>(v_cbor.data()), v_cbor.size() * sizeof(std::uint8_t));
                    return true;
                } // to_cbor(...)

                /** The only instance of this type. */
                static type& instance()
                {
                    // Since it's a static variable, if the class has already been created, it won't be created again.
                    // Note: it is thread-safe in C++11.
                    static type s_instance { };
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
