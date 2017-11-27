
#ifndef ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <aftermath/probability.hpp>
#include <nlohmann/json.hpp>
#include "json.hpp"

#include "unit_group.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string
#include <ostream> // std::ostream, std::endl
#include <utility> // std:pair, std::make_pair
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        struct report_entry
        {
            using type = report_entry;
            using empirical_measure = aftermath::probability::empirical_measure<std::size_t, std::size_t, double>;
            // ~~ Json names ~~
            static constexpr char is_header_name[] = "header";
            static constexpr char caption_name[] = "caption";
            static constexpr char details_name[] = "details";
            static constexpr char clipboard_text_name[] = "clipboard text";
            static constexpr char unit_name_name[] = "unit name"; // orz
            static constexpr char lower_bound_name[] = "lower bound";
            static constexpr char upper_bound_name[] = "upper bound";
            static constexpr char observed_values_name[] = "observed values";
            static constexpr char observed_counts_name[] = "observed counts";

        private:
            bool m_is_header = false;
            std::string m_caption = "";
            std::string m_details = "";
            std::string m_clipboard_text = "";
            std::string m_unit_name = "";
            std::pair<std::size_t, bool> m_lower_bound = std::make_pair(0, false);
            std::pair<std::size_t, bool> m_upper_bound = std::make_pair(0, false);
            empirical_measure m_observations = { };

        public:
            report_entry() noexcept { }

            explicit report_entry(const std::string& caption, const std::string& details = "", const std::string& clipboard_text = "") noexcept 
                : m_is_header(true), m_caption(caption), m_details(details), m_clipboard_text(clipboard_text)
            { }
            
            explicit report_entry(const std::string& caption, const empirical_measure& observations) noexcept
                : m_caption(caption), m_observations(observations)
            { }
            
            explicit report_entry(const unit_group& group, const empirical_measure& observations) noexcept
                : m_unit_name(group.unit().names().front()), m_observations(observations)
            { }

            bool is_header() const noexcept { return this->m_is_header; }
            void set_is_header(bool value) noexcept { this->m_is_header = value; }

            const std::string& caption() const noexcept { return this->m_caption; }
            const std::string& details() const noexcept { return this->m_details; }
            const std::string& clipboard_text() const noexcept { return this->m_clipboard_text; }
            void set_text(const std::string& caption, const std::string& details = "", const std::string& clipboard_text = "") noexcept 
            {
                this->m_caption = caption; 
                this->m_details = details; 
                this->m_clipboard_text = clipboard_text; 
            }

            const std::string& unit_name() const noexcept { return this->m_unit_name; }
            void set_unit_name(const std::string& value) noexcept { this->m_unit_name = value; }

            bool has_lower_bound() const noexcept { return this->m_lower_bound.second; }
            bool has_upper_bound() const noexcept { return this->m_upper_bound.second; }
            std::size_t lower_bound() const noexcept { return this->m_lower_bound.first; }
            std::size_t upper_bound() const noexcept { return this->m_upper_bound.first; }
            void set_lower_bound(std::size_t value) noexcept { this->m_lower_bound = std::make_pair(value, true); }
            void set_upper_bound(std::size_t value) noexcept { this->m_upper_bound = std::make_pair(value, true); }
            void clear_lower_bound() noexcept { this->m_lower_bound = std::make_pair(0, false); }
            void clear_upper_bound() noexcept { this->m_upper_bound = std::make_pair(0, false); }
            
            const empirical_measure& observations() const noexcept { return this->m_observations; }
            void set_observations(const empirical_measure& value) noexcept { this->m_observations = value; }

            friend std::ostream& operator <<(std::ostream& os, const type& self)
            {
                os << self.m_caption;
                if (!self.m_details.empty()) os << " (" << self.m_details << ")";
                if (self.m_observations.empty()) os << ".";
                else
                {
                    os << ":" << std::endl;
                    os << self.m_observations;
                }
                return os;
            } // operator <<(...)
        }; // struct report_entry

        // ~~ Json name definitions ~~
        constexpr char report_entry::is_header_name[];
        constexpr char report_entry::caption_name[];
        constexpr char report_entry::details_name[];
        constexpr char report_entry::clipboard_text_name[];
        constexpr char report_entry::unit_name_name[];
        constexpr char report_entry::lower_bound_name[];
        constexpr char report_entry::upper_bound_name[];
        constexpr char report_entry::observed_values_name[];
        constexpr char report_entry::observed_counts_name[];
        
        void to_json(nlohmann::json& j, const report_entry& x) noexcept
        {
            using type = report_entry;

            j[type::is_header_name] = x.is_header();
            j[type::caption_name] = x.caption();
            if (!x.details().empty()) j[type::details_name] = x.details();
            if (!x.clipboard_text().empty()) j[type::clipboard_text_name] = x.clipboard_text();
            if (!x.unit_name().empty()) j[type::unit_name_name] = x.unit_name();
            if (x.has_lower_bound()) j[type::lower_bound_name] = x.lower_bound();
            if (x.has_upper_bound()) j[type::upper_bound_name] = x.upper_bound();
            if (!x.observations().empty())
            {
                std::size_t size = x.observations().data().size();
                std::vector<std::size_t> values(size);
                std::vector<std::size_t> counts(size);
                x.observations().copy_to(values.data(), counts.data());
                j[type::observed_values_name] = values;
                j[type::observed_counts_name] = counts;
            }
        } // to_json(...)
    
        void from_json(const nlohmann::json& j, report_entry& x) noexcept
        {
            using type = report_entry;

            // Populate default values.
            bool is_header = x.is_header();
            std::string caption = x.caption();
            std::string details = x.details();
            std::string clipboard_text = x.clipboard_text();
            std::string unit_name = x.unit_name();
            bool has_lower_bound = x.has_lower_bound();
            bool has_upper_bound = x.has_upper_bound();
            std::size_t lower_bound = x.lower_bound();
            std::size_t upper_bound = x.upper_bound();
            report_entry::empirical_measure observations = x.observations();
            std::vector<std::size_t> values = { };
            std::vector<std::size_t> counts = { };

            // Parse json entries.
            if (!quiet_json::required(j, type::is_header_name, is_header)) return;
            if (!quiet_json::required(j, type::caption_name, caption)) return;
            if (!quiet_json::optional(j, type::details_name, details)) return;
            if (!quiet_json::optional(j, type::clipboard_text_name, clipboard_text)) return;
            if (!quiet_json::optional(j, type::unit_name_name, unit_name)) return;
            if (!quiet_json::is_missing(j, type::lower_bound_name))
            {
                if (!j[type::lower_bound_name].is_null())
                {
                    has_lower_bound = true;
                    quiet_json::required(j, type::lower_bound_name, lower_bound);
                }
            }
            if (!quiet_json::is_missing(j, type::upper_bound_name))
            {
                if (!j[type::upper_bound_name].is_null())
                {
                    has_upper_bound = true;
                    quiet_json::required(j, type::upper_bound_name, upper_bound);
                }
            }
            if (!quiet_json::optional(j, type::observed_values_name, values)) return;
            if (!quiet_json::optional(j, type::observed_counts_name, counts)) return;
            if (values.size() != counts.size())
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::major,
                    "Observations size mismatch.", __FUNCTION__, __LINE__);
                return;
            }
            observations = report_entry::empirical_measure(values, counts);
            
            // Reconstruct the object.
            x.set_is_header(is_header);
            x.set_text(caption, details, clipboard_text);
            x.set_unit_name(unit_name);
            if (has_lower_bound) x.set_lower_bound(lower_bound);
            if (has_upper_bound) x.set_upper_bound(upper_bound);
            x.set_observations(observations);
        } // from_json(...)
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED
