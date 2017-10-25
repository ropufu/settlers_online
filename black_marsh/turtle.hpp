
#ifndef ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <aftermath/probability.hpp>
#include <nlohmann/json.hpp>

#include "config.hpp"

#include "../settlers_online/unit_database.hpp"
#include "../settlers_online/army_parser.hpp"
#include "../settlers_online/army.hpp"
#include "../settlers_online/battle_skill.hpp"
#include "../settlers_online/binomial_pool.hpp"
#include "../settlers_online/camp.hpp"
#include "../settlers_online/char_string.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/combat_result.hpp"
#include "../settlers_online/conditioned_army.hpp"
#include "../settlers_online/randomized_attack_sequence.hpp"
#include "../settlers_online/trivial_attack_sequence.hpp"
#include "../settlers_online/unit_faction.hpp"
#include "../settlers_online/unit_group.hpp"
#include "../settlers_online/unit_type.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <stdexcept> // std::domain_error
#include <iostream> // std::cout, std::endl
#include <map> // std::map
#include <set> // std::set
#include <string> // std::string, std::to_string
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        namespace black_marsh
        {
            /** The main struct of the simulator. Responsible for handling parsing / construction / simulations / etc. */
            struct turtle
            {
                using type = turtle;
                
                using sequencer_type = randomized_attack_sequence<>;
                using sequencer_type_low = trivial_attack_sequence<false>;
                using sequencer_type_high = trivial_attack_sequence<true>;
                
                using empirical_measure = aftermath::probability::empirical_measure<std::size_t, std::size_t, double>;
                using skills_type = enum_array<battle_skill, std::size_t>;

                static bool is_config_valid() noexcept
                {
                    config& c = config::instance();
                    if (!c.good()) return false;

                    return true;
                } // is_config_valid(...)

                template <typename t_action_type>
                static double elapsed_seconds(t_action_type action) noexcept
                {
                    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                    action();
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                
                    double elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1'000.0;
                    return elapsed_seconds;
                } // elapsed_seconds(...)
                
                template <typename t_predicate_type>
                static bool try_parse(const std::string& str, army& value, const t_predicate_type& predicate, bool is_quiet = false) noexcept
                {
                    const unit_database& db = unit_database::instance();

                    army_parser parser = str;
                    if (!parser.good())
                    {
                        if (!is_quiet) std::cout << "Parsing army \"" << str << "\" failed." << std::endl;
                        return false;
                    }
                    if (!parser.try_build(db, value, predicate)) 
                    {
                        if (!is_quiet) std::cout << "Reconstructing army \"" << str << "\" from database failed." << std::endl;
                        return false;
                    }
                    return true;
                } // try_parse(...)

                static bool try_parse_skills(const nlohmann::json& what, std::map<std::string, skills_type>& where) noexcept
                {
                    if (!what.is_object()) return false;

                    for (auto it = what.begin(); it != what.end(); ++it)
                    {
                        skills_type skills = it.value();
                        where.emplace(it.key(), skills);
                    }
                    return aftermath::quiet_error::instance().good();
                } // parse_skills(...)
                
                static void print_skills(const std::map<std::string, skills_type>& skills) noexcept
                {
                    std::string prefix = "  |---- ";
                    bool is_empty = true;
                    for (const auto& pair : skills)
                    {
                        is_empty = false;
                        std::cout << '\t' << pair.first << std::endl;
                        bool is_first = true;
                        for (const auto& skill_pair : pair.second)
                        {
                            if (skill_pair.second == 0) continue;
                            std::cout << '\t' << prefix << std::to_string(skill_pair.first) << " (" << skill_pair.second << ")" << std::endl;
                            is_first = false;
                        }
                        if (is_first) std::cout << '\t' << prefix << "none" << std::endl;
                        std::cout << std::endl;
                    }
                    if (is_empty) std::cout << "None" << std::endl;
                } // print_skills(...)
                
                static bool try_apply_skills(army& a, const std::map<std::string, skills_type>& skills) noexcept
                {
                    std::string prefix = "  |---- ";
                    for (const auto& pair : skills)
                    {
                        bool is_match = false;
                        for (const unit_group& g : a.groups()) for (const std::string& name : g.unit().names()) if (name == pair.first) is_match = true;
                        if (!is_match) continue;
                        
                        std::cout << "Applying skills:" << std::endl;
                        bool is_first = true;
                        for (const auto& skill_pair : pair.second)
                        {
                            if (skill_pair.second == 0) continue;
                            std::cout << '\t' << prefix << std::to_string(skill_pair.first) << " (" << skill_pair.second << ")" << std::endl;
                            a.set_level(skill_pair.first, skill_pair.second);
                            is_first = false;
                        }
                        if (is_first) std::cout << '\t' << prefix << "none" << std::endl;
                        std::cout << std::endl;
                        return true;
                    }
                    return false;
                } // try_apply_skills(...)

                static void unwind_errors(bool do_skip_log, bool is_quiet = false) noexcept
                {
                    aftermath::quiet_error& err = aftermath::quiet_error::instance();
                
                    if (!is_quiet)
                    {
                        if (!err.good()) std::cout << "~~ Oh no! Errors encoutered: ~~" << std::endl;
                        else if (!err.empty()) std::cout << "~~ Something to keep in mind: ~~" << std::endl;
                    }
                    while (!err.empty())
                    {
                        aftermath::quiet_error_descriptor desc = err.pop();
                        if (is_quiet) continue;
                        if (do_skip_log && desc.severity() == aftermath::severity_level::not_at_all) continue; // Skip log messages.
                
                        std::cout << '\t' <<
                            " " << std::to_string(desc.severity()) <<
                            " " << std::to_string(desc.error_code()) <<
                            " on line " << desc.caller_line_number() <<
                            " of <" << desc.caller_function_name() << ">: " << desc.description() << std::endl;
                    }
                } // unwind_errors(...)

            private:
                army m_left = { };
                army m_right = { };
                detail::camp m_left_camp = { };
                detail::camp m_right_camp = detail::camp(250, 0);
                std::map<std::string, skills_type> m_left_skills = { };
                std::map<std::string, skills_type> m_right_skills = { };

                std::size_t m_count_combat_sims = 10'000;
                std::size_t m_count_destruct_sims_per_combat = 50;

            public:
                turtle() noexcept
                {
                    this->reload();
                } // turtle(...)

                void reload() noexcept
                {
                    config& c = config::instance();
                    unit_database& db = unit_database::instance();

                    c.read();
                    if (!c.good())
                    {
                        std::cout << "Failed to read config file." << std::endl;
                        return;
                    }

                    std::string maps_path = "./maps/";
                    std::size_t n1 = this->m_count_combat_sims;
                    std::size_t n2 = this->m_count_destruct_sims_per_combat;
                    detail::camp left_camp = this->m_left_camp;
                    detail::camp right_camp = this->m_right_camp;

                    c.maybe("maps folder", maps_path);
                    c.maybe("simulations", n1);
                    c.maybe("destructions per sim", n2);
                    c.maybe("left camp", left_camp);
                    c.maybe("right camp", right_camp);

                    if (c.has("left skills")) type::try_parse_skills(c["left skills"], this->m_left_skills);
                    if (c.has("right skills")) type::try_parse_skills(c["right skills"], this->m_right_skills);

                    this->m_count_combat_sims = n1;
                    this->m_count_destruct_sims_per_combat = n2;
                    this->m_left_camp = left_camp;
                    this->m_right_camp = right_camp;

                    // ~~ Build unit database ~~
                    std::cout << "Loaded " << db.load_from_folder(maps_path) << " units." << std::endl;
                    this->m_left = { };
                    this->m_right = { };
                    type::unwind_errors(false);
                } // reload(...)

                void print_left_skills() const noexcept { type::print_skills(this->m_left_skills); }
                void print_right_skills() const noexcept { type::print_skills(this->m_right_skills); }

                const army& left() const noexcept { return this->m_left; }
                const army& right() const noexcept { return this->m_right; }

                const detail::camp& left_camp() const noexcept { return this->m_left_camp; }
                const detail::camp& right_camp() const noexcept { return this->m_right_camp; }

                void set_left_camp(const detail::camp& value) noexcept { this->m_left_camp = value; }
                void set_right_camp(const detail::camp& value) noexcept { this->m_right_camp = value; }

                std::size_t simulation_count() const noexcept { return this->m_count_combat_sims; }
                void set_simulation_count(std::size_t value) noexcept
                {
                    this->m_count_combat_sims = value;
                    config::instance()["simulations"] = value;
                    config::instance().write();
                } // set_simulation_count(...)

                void parse_left(const std::string& army_string, bool is_strict = false) noexcept
                {
                    type::try_parse(army_string, this->m_left, [] (const unit_type& /**u*/) { return true; });
                    type::unwind_errors(false);
                    
                    // Check for missing generals.
                    if (!(this->m_left.empty() || this->m_left.has(unit_faction::general))) std::cout << "Warning! Left army does not have any generals. Type \"units general\" for suggestions." << std::endl;

                    // Check for multiple factions.
                    std::set<unit_faction> left_factions { };
                    for (const auto& g : this->m_left.groups()) left_factions.insert(g.unit().faction());
                    left_factions.erase(unit_faction::general); // Generals do not count.

                    std::size_t count_options = 0;
                    if (left_factions.size() > 1)
                    {
                        std::cout << "Warning! There is more than one faction in your army." << std::endl;
                        army a { };
                        for (unit_faction f : left_factions)
                        {
                            // Try to re-build this army assuming only fraction <f> is allowed (or generals).
                            if (type::try_parse(army_string, a, [&] (const unit_type& u) { return u.is(f) || u.is(unit_faction::general); }, true))
                            {
                                ++count_options;
                                std::cout << "Did you mean: ";
                                bool is_first = true;
                                for (const unit_group& g : a.groups())
                                {
                                    if (!is_first) std::cout << " ";
                                    std::cout << g.count() << unit_database::build_key(g.unit());
                                    is_first = false;
                                }
                                std::cout << "?" << std::endl;
                            }
                            type::unwind_errors(true, true);
                        }
                        // If there is only one alternative with single faction, take it!
                        if (!is_strict && count_options == 1)
                        {
                            std::cout << "Assuming yes." << std::endl;
                            this->m_left = a;
                        }
                    }
                } // parse_left(...)

                void parse_right(const std::string& army_string) noexcept
                {
                    type::try_parse(army_string, this->m_right, [] (const unit_type& /**u*/) { return true; });
                    type::unwind_errors(false);
                } // parse_right(...)

                void run(bool is_log = false) noexcept
                {
                    unwind_errors(false);
                    if (this->m_left.count_groups() == 0 || this->m_right.count_groups() == 0)
                    {
                        std::cout << "Armies (left and right) have to be set prior to execution." << std::endl;
                        return;
                    }

                    this->m_left.set_camp(this->m_left_camp);
                    this->m_right.set_camp(this->m_right_camp);
                    
                    this->m_left.reset_skills();
                    this->m_right.reset_skills();

                    type::try_apply_skills(this->m_left, this->m_left_skills);
                    type::try_apply_skills(this->m_right, this->m_right_skills);
                
                    // ~~ Combat phase ~~
                    ropufu::settlers_online::combat_mechanics combat(this->m_left, this->m_right);
                    ropufu::settlers_online::combat_mechanics snapshot = combat;
                
                    //std::cout << "Building left cache..." << std::endl;
                    sequencer_type::pool_type::instance().cache(combat.left().underlying());
                    unwind_errors(true);
                    //std::cout << "Building right cache..." << std::endl;
                    sequencer_type::pool_type::instance().cache(combat.right().underlying());
                    unwind_errors(true);
                
                    // ~~ Choose sequencers ~~
                    sequencer_type left_seq { };
                    sequencer_type right_seq { };
                
                    std::cout << this->m_left << " vs. " << this->m_right << std::endl;
                    if (is_log)
                    {
                        combat.set_do_log(true);
                        combat.execute(left_seq, right_seq);
                        return;
                    }
                    
                    empirical_measure rounds { };
                    std::vector<empirical_measure> left_losses(this->m_left.count_groups());
                    for (std::size_t i = 0; i < this->m_count_combat_sims; ++i)
                    {
                        std::size_t combat_rounds = combat.execute(left_seq, right_seq);
                        //const ropufu::settlers_online::combat_result& result = combat.outcome();

                        std::vector<std::size_t> x = combat.left().calculate_losses();
                        for (std::size_t k = 0; k < x.size(); k++) left_losses[k].observe(x[k]);
                
                        for (std::size_t j = 0; j < this->m_count_destruct_sims_per_combat; ++j)
                        {
                            std::size_t destruction_rounds = combat.destruct(left_seq, right_seq);
                            rounds.observe(combat_rounds + destruction_rounds);
                        }
                        combat = snapshot;
                    }
                    
                    std::cout << "Rounds:" << std::endl << rounds << std::endl;
                    for (std::size_t k = 0; k < this->m_left.count_groups(); k++)
                    {
                        std::cout << "Losses in " << this->m_left[k].unit().names().front() << ":" << std::endl;
                        std::cout << left_losses[k] << std::endl;
                    }
                
                    unwind_errors(false);
                } // run(...)
            }; // struct turtle
        } // namespace black_marsh
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
