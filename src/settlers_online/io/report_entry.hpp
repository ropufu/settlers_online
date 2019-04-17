
#ifndef ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>

#include <ropufu/probability.hpp>

#include "../combat/unit_group.hpp"

#include <cstddef> // std::size_t
#include <stdexcept>  // std::runtime_error
#include <string>  // std::string
#include <ostream> // std::ostream, std::endl
#include <system_error> // std:error_code
#include <utility> // std:pair, std::make_pair
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    struct report_entry
    {
        using type = report_entry;
        using empirical_measure_type = aftermath::probability::empirical_measure<std::size_t, std::size_t, double>;
        // ~~ Json names ~~
        static constexpr char jstr_is_header[] = "header";
        static constexpr char jstr_caption[] = "caption";
        static constexpr char jstr_details[] = "details";
        static constexpr char jstr_clipboard_text[] = "clipboard text";
        static constexpr char jstr_unit_name[] = "unit name";
        static constexpr char jstr_lower_bound[] = "lower bound";
        static constexpr char jstr_upper_bound[] = "upper bound";
        static constexpr char jstr_observed_values[] = "observed values";
        static constexpr char jstr_observed_counts[] = "observed counts";

    private:
        bool m_is_header = false;
        std::string m_caption = "";
        std::string m_details = "";
        std::string m_clipboard_text = "";
        std::string m_unit_name = "";
        std::pair<std::size_t, bool> m_lower_bound = std::make_pair(0, false);
        std::pair<std::size_t, bool> m_upper_bound = std::make_pair(0, false);
        empirical_measure_type m_observations = {};

    public:
        report_entry() noexcept { }

        explicit report_entry(const std::string& caption, const std::string& details = "", const std::string& clipboard_text = "") noexcept 
            : m_is_header(true), m_caption(caption), m_details(details), m_clipboard_text(clipboard_text)
        {
        } // report_entry
        
        explicit report_entry(const std::string& caption, const empirical_measure_type& observations) noexcept
            : m_caption(caption), m_observations(observations)
        {
        } // report_entry
        
        explicit report_entry(const unit_group& group, const empirical_measure_type& observations) noexcept
            : m_unit_name(group.unit().first_name()), m_observations(observations)
        {
        } // report_entry

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
        
        const empirical_measure_type& observations() const noexcept { return this->m_observations; }
        void set_observations(const empirical_measure_type& value) noexcept { this->m_observations = value; }

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
    constexpr char report_entry::jstr_is_header[];
    constexpr char report_entry::jstr_caption[];
    constexpr char report_entry::jstr_details[];
    constexpr char report_entry::jstr_clipboard_text[];
    constexpr char report_entry::jstr_unit_name[];
    constexpr char report_entry::jstr_lower_bound[];
    constexpr char report_entry::jstr_upper_bound[];
    constexpr char report_entry::jstr_observed_values[];
    constexpr char report_entry::jstr_observed_counts[];
    
    void to_json(nlohmann::json& j, const report_entry& x) noexcept
    {
        using type = report_entry;

        j = nlohmann::json{
            {type::jstr_is_header, x.is_header()},
            {type::jstr_caption, x.caption()}
        };

        if (!x.details().empty()) j[type::jstr_details] = x.details();
        if (!x.clipboard_text().empty()) j[type::jstr_clipboard_text] = x.clipboard_text();
        if (!x.unit_name().empty()) j[type::jstr_unit_name] = x.unit_name();
        if (x.has_lower_bound()) j[type::jstr_lower_bound] = x.lower_bound();
        if (x.has_upper_bound()) j[type::jstr_upper_bound] = x.upper_bound();
        if (!x.observations().empty())
        {
            std::size_t size = x.observations().data().size();
            std::vector<std::size_t> values {};
            std::vector<std::size_t> counts {};
            values.reserve(size);
            counts.reserve(size);
            for (const auto& item : x.observations().data())
            {
                values.push_back(item.first);
                counts.push_back(item.second);
            } // for (...)
            j[type::jstr_observed_values] = values;
            j[type::jstr_observed_counts] = counts;
        } // if (...)
    } // to_json(...)

    void from_json(const nlohmann::json& j, report_entry& x)
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
        typename type::empirical_measure_type observations = x.observations();
        std::vector<std::size_t> values = {};
        std::vector<std::size_t> counts = {};

        // Parse json entries.
        is_header = j[type::jstr_is_header];
        caption = j[type::jstr_caption].get<std::string>();
        if (j.count(type::jstr_details) > 0) details = j[type::jstr_details].get<std::string>();
        if (j.count(type::jstr_clipboard_text) > 0) clipboard_text = j[type::jstr_clipboard_text].get<std::string>();
        if (j.count(type::jstr_unit_name) > 0) unit_name = j[type::jstr_unit_name].get<std::string>();
        if (j.count(type::jstr_observed_values) > 0) values = j[type::jstr_observed_values].get<std::vector<std::size_t>>();
        if (j.count(type::jstr_observed_counts) > 0) counts = j[type::jstr_observed_counts].get<std::vector<std::size_t>>();

        // More entries.
        if (j.count(type::jstr_lower_bound) > 0)
        {
            if (!j[type::jstr_lower_bound].is_null())
            {
                has_lower_bound = true;
                lower_bound = j[type::jstr_lower_bound];
            } // if (...)
        } // if (...)
        if (j.count(type::jstr_upper_bound) > 0)
        {
            if (!j[type::jstr_upper_bound].is_null())
            {
                has_upper_bound = true;
                upper_bound = j[type::jstr_upper_bound];
            } // if (...)
        } // if (...)

        // Reconstruct the object.
        if (values.size() != counts.size()) throw std::runtime_error("Observations size mismatch.");
        std::error_code ec {};
        observations = type::empirical_measure_type(values, counts, ec);
        if (ec) throw std::runtime_error("Empirical measure reconstruction error.");

        x.set_is_header(is_header);
        x.set_text(caption, details, clipboard_text);
        x.set_unit_name(unit_name);
        if (has_lower_bound) x.set_lower_bound(lower_bound);
        if (has_upper_bound) x.set_upper_bound(upper_bound);
        x.set_observations(observations);
    } // from_json(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED
