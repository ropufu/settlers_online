
#ifndef ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <aftermath/quiet_json.hpp>

#include <aftermath/not_an_error.hpp>
#include <aftermath/probability.hpp>

#include "unit_group.hpp"

#include <cstddef> // std::size_t
#include <string>  // std::string
#include <ostream> // std::ostream, std::endl
#include <utility> // std:pair, std::make_pair
#include <vector>  // std::vector

namespace ropufu::settlers_online
{
    struct report_entry
    {
        using type = report_entry;
        using empirical_measure = aftermath::probability::empirical_measure<std::size_t, std::size_t, double>;
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
            std::vector<std::size_t> values(size);
            std::vector<std::size_t> counts(size);
            x.observations().copy_to(values.data(), counts.data());
            j[type::jstr_observed_values] = values;
            j[type::jstr_observed_counts] = counts;
        } // if (...)
    } // to_json(...)

    void from_json(const nlohmann::json& j, report_entry& x) noexcept
    {
        ropufu::aftermath::quiet_json q(j);
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
        q.required(type::jstr_is_header, is_header);
        q.required(type::jstr_caption, caption);
        q.optional(type::jstr_details, details);
        q.optional(type::jstr_clipboard_text, clipboard_text);
        q.optional(type::jstr_unit_name, unit_name);
        q.optional(type::jstr_observed_values, values);
        q.optional(type::jstr_observed_counts, counts);

        // More entries.
        if (j.count(type::jstr_lower_bound) > 0)
        {
            if (!j[type::jstr_lower_bound].is_null())
            {
                has_lower_bound = true;
                q.required(type::jstr_lower_bound, lower_bound);
            } // if (...)
        } // if (...)
        if (j.count(type::jstr_upper_bound) > 0)
        {
            if (!j[type::jstr_upper_bound].is_null())
            {
                has_upper_bound = true;
                q.required(type::jstr_upper_bound, upper_bound);
            } // if (...)
        } // if (...)

        // Reconstruct the object.
        if (!q.good())
        {
            aftermath::quiet_error::instance().push(
                aftermath::not_an_error::runtime_error,
                aftermath::severity_level::major, 
                q.message(), __FUNCTION__, __LINE__);
            return;
        } // if (...)
        if (values.size() != counts.size())
        {
            aftermath::quiet_error::instance().push(
                aftermath::not_an_error::runtime_error,
                aftermath::severity_level::major,
                "Observations size mismatch.", __FUNCTION__, __LINE__);
            return;
        } // if (...)
        x.set_is_header(is_header);
        x.set_text(caption, details, clipboard_text);
        x.set_unit_name(unit_name);
        if (has_lower_bound) x.set_lower_bound(lower_bound);
        if (has_upper_bound) x.set_upper_bound(upper_bound);
        observations = report_entry::empirical_measure(values, counts);
        x.set_observations(observations);
    } // from_json(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_REPORT_ENTRY_HPP_INCLUDED
