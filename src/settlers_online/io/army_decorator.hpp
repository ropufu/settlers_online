
#ifndef ROPUFU_SETTLERS_ONLINE_ARMY_DECORATOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ARMY_DECORATOR_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>

#include "../char_string.hpp"
#include "../enums.hpp"
#include "../logger.hpp"

#include "../combat/army.hpp"
#include "../combat/camp.hpp"

#include <cstddef> // std::size_t
#include <map> // std::map
#include <iostream> // std::cout
#include <ostream> // std::ostream, std::endl
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu::settlers_online
{
    struct army_decorator
    {
        using type = army_decorator;
        using skills_type = aftermath::enum_array<battle_skill, std::size_t>;
        using camp_type = settlers_online::camp;
        // ~~ Json names ~~
        static constexpr char jstr_camp[] = "camp";
        static constexpr char jstr_skill_map[] = "skills";

    private:
        camp_type m_camp = {};
        std::map<std::string, skills_type> m_skills = {};

    public:
        const camp_type& camp() const noexcept { return this->m_camp; }
        void set_camp(const camp_type& value) noexcept { this->m_camp = value; }

        const std::map<std::string, skills_type>& skills() const noexcept { return this->m_skills; }
        void set_skills(const std::map<std::string, skills_type>& value) noexcept { this->m_skills = value; }

        void decorate(army& a) const noexcept
        {
            detail::no_logger logger {};
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
                for (const unit_group& g : a.groups()) if (g.count_as_attacker() > 0) for (const std::string& name : g.unit().names()) if (name == pair.first) is_match = true;
                if (!is_match) continue;
                
                std::vector<std::string> skill_names {};
                for (const auto& skill_pair : pair.second)
                {
                    if (skill_pair.value() == 0) continue;
                    skill_names.push_back(std::to_string(skill_pair.key()) + " (" + std::to_string(skill_pair.value()) + ")");
                    a.set_level(skill_pair.key(), skill_pair.value());
                }
                if (!skill_names.empty()) logger.write(char_string::join(skill_names, prefix));
            }
        } // decorate(...)
        
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            os << type::jstr_camp << ": " << self.m_camp << std::endl;
            if (self.m_skills.empty())
            {
                os << type::jstr_skill_map << ": none";
                return os;
            }
            os << type::jstr_skill_map << ":";

            std::string prefix = "  |---- ";
            for (const auto& pair : self.m_skills)
            {
                os << std::endl << '\t' << pair.first;
                bool is_first = true;
                for (const auto& skill_pair : pair.second)
                {
                    if (skill_pair.value() == 0) continue;
                    os << std::endl << '\t' << prefix << std::to_string(skill_pair.key()) << " (" << skill_pair.value() << ")";
                    is_first = false;
                }
                if (is_first) os << std::endl << '\t' << prefix << "none";
            }
            return os;
        } // operator <<(...)
    }; // struct army_decorator

    // ~~ Json name definitions ~~
    constexpr char army_decorator::jstr_camp[];
    constexpr char army_decorator::jstr_skill_map[];
    
    void to_json(nlohmann::json& j, const army_decorator& x) noexcept
    {
        using type = army_decorator;

        j = nlohmann::json{
            {type::jstr_camp, x.camp()}
        };

        if (x.skills().empty()) j[type::jstr_skill_map] = std::string("none");
        else j[type::jstr_skill_map] = x.skills();
    } // to_json(...)

    void from_json(const nlohmann::json& j, army_decorator& x)
    {
        using type = army_decorator;
        
        // Populate default values.
        settlers_online::camp camp = x.camp();
        std::map<std::string, type::skills_type> skills = x.skills();

        // Parse json entries.
        camp = j[type::jstr_camp];
        const nlohmann::json& skills_json = j[type::jstr_skill_map];
        if (skills_json.is_string())
        {
            std::string value = skills_json;
            if (!(value == "none" || value == "empty")) throw std::runtime_error("Skills not recognized");
        } // if (...)
        else skills = skills_json.get<std::map<std::string, type::skills_type>>();
        
        // Reconstruct the object.
        x.set_camp(camp);
        x.set_skills(skills);
    } // from_json(...)
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ARMY_DECORATOR_HPP_INCLUDED
