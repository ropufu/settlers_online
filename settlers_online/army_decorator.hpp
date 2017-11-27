
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_DECORATOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_DECORATOR_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <nlohmann/json.hpp>
#include "json.hpp"

#include "army.hpp"
#include "battle_skill.hpp"
#include "camp.hpp"
#include "char_string.hpp"
#include "enum_array.hpp"
#include "logger.hpp"

#include <cstddef> // std::size_t
#include <map> // std::map
#include <iostream> // std::cout
#include <ostream> // std::ostream, std::endl
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        struct army_decorator
        {
            using type = army_decorator;
            using skills_type = enum_array<battle_skill, std::size_t>;
            // ~~ Json names ~~
            static constexpr char camp_name[] = "camp";
            static constexpr char skills_name[] = "skills";

        private:
            detail::camp m_camp = { };
            std::map<std::string, skills_type> m_skills = { };

        public:
            const detail::camp& camp() const noexcept { return this->m_camp; }
            void set_camp(const detail::camp& value) noexcept { this->m_camp =  value; }

            const std::map<std::string, skills_type>& skills() const noexcept { return this->m_skills; }
            void set_skills(const std::map<std::string, skills_type>& value) noexcept { this->m_skills = value; }

            void decorate(army& a) const noexcept
            {
                detail::no_logger logger { };
                this->decorate(a, logger);
            } // decorate(...)
            
            template <typename t_logger_type>
            void decorate(army& a, t_logger_type& logger) const noexcept
            {
                a.set_camp(this->m_camp); // Overwrite camp.
                a.reset_skills(); // Reset skills.

                std::string prefix = ", ";
                for (const auto& pair : this->m_skills)
                {
                    bool is_match = false;
                    for (const unit_group& g : a.groups()) for (const std::string& name : g.unit().names()) if (name == pair.first) is_match = true;
                    if (!is_match) continue;
                    
                    std::vector<std::string> skill_names { };
                    for (const auto& skill_pair : pair.second)
                    {
                        if (skill_pair.second == 0) continue;
                        skill_names.push_back(detail::to_str(skill_pair.first) + " (" + std::to_string(skill_pair.second) + ")");
                        a.set_level(skill_pair.first, skill_pair.second);
                    }
                    if (!skill_names.empty()) logger.write(char_string::join(skill_names, prefix));
                }
            } // decorate(...)
            
            friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
            {
                os << type::camp_name << ": " << self.m_camp << std::endl;
                if (self.m_skills.empty())
                {
                    os << type::skills_name << ": none";
                    return os;
                }
                os << type::skills_name << ":";

                std::string prefix = "  |---- ";
                for (const auto& pair : self.m_skills)
                {
                    os << std::endl << '\t' << pair.first;
                    bool is_first = true;
                    for (const auto& skill_pair : pair.second)
                    {
                        if (skill_pair.second == 0) continue;
                        os << std::endl << '\t' << prefix << detail::to_str(skill_pair.first) << " (" << skill_pair.second << ")";
                        is_first = false;
                    }
                    if (is_first) os << std::endl << '\t' << prefix << "none";
                }
                return os;
            } // operator <<(...)
        }; // struct army_decorator

        // ~~ Json name definitions ~~
        constexpr char army_decorator::camp_name[];
        constexpr char army_decorator::skills_name[];
        
        void to_json(nlohmann::json& j, const army_decorator& x) noexcept
        {
            using type = army_decorator;

            j[type::camp_name] = x.camp();
            if (x.skills().empty())
                j[type::skills_name] = std::string("none");
            else
                j[type::skills_name] = x.skills();
        } // to_json(...)
    
        void from_json(const nlohmann::json& j, army_decorator& x) noexcept
        {
            using type = army_decorator;
            
            // Populate default values.
            detail::camp camp = x.camp();
            std::map<std::string, type::skills_type> skills = x.skills();

            // Parse json entries.
            if (quiet_json::is_missing(j, type::camp_name)) return;
            camp = j[type::camp_name];
            if (quiet_json::is_missing(j, type::skills_name)) return;
            const nlohmann::json& skills_json = j[type::skills_name];
            if (skills_json.is_string())
            {
                std::string value = skills_json;
                if (!(value == "none" || value == "empty"))
                {
                    aftermath::quiet_error::instance().push(
                        aftermath::not_an_error::runtime_error,
                        aftermath::severity_level::major,
                        "Skills not recognized", __FUNCTION__, __LINE__);
                    return;
                }
            }
            else if (skills_json.is_object())
            {
                for (auto it = skills_json.begin(); it != skills_json.end(); ++it)
                {
                    std::string key = it.key();
                    army_decorator::skills_type value = it.value(); 
                    skills.emplace(key, value);
                }
            }
            else 
            {
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::major,
                    "Skills not recognized", __FUNCTION__, __LINE__);
                return;
            }
            
            // Reconstruct the object.
            x.set_camp(camp);
            x.set_skills(skills);
        } // from_json(...)
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_DECORATOR_HPP_INCLUDED
