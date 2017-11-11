
#ifndef ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <aftermath/probability.hpp>
#include <nlohmann/json.hpp>

#include "config.hpp"

#include "../settlers_online/unit_database.hpp"
#include "../settlers_online/army_parser.hpp"
#include "../settlers_online/army.hpp"
#include "../settlers_online/army_decorator.hpp"
#include "../settlers_online/battle_skill.hpp"
#include "../settlers_online/binomial_pool.hpp"
#include "../settlers_online/camp.hpp"
#include "../settlers_online/char_string.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/combat_result.hpp"
#include "../settlers_online/conditioned_army.hpp"
#include "../settlers_online/randomized_attack_sequence.hpp"
#include "../settlers_online/report_entry.hpp"
#include "../settlers_online/trivial_attack_sequence.hpp"
#include "../settlers_online/unit_faction.hpp"
#include "../settlers_online/unit_group.hpp"
#include "../settlers_online/unit_type.hpp"
#include "../settlers_online/warnings.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
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

                static void present_losses(std::vector<report_entry>& report, const army& a, const std::vector<empirical_measure>& losses) noexcept
                {
                    for (std::size_t k = 0; k < a.count_groups(); k++)
                    {
                        report_entry entry(a[k], losses[k]);
                        entry.set_text("Losses in " + a[k].unit().names().front());
                        report.push_back(entry);
                    }
                } // present_losses(...)

            private:
                settlers_online::warnings m_warnings = { };
                army m_left = { };
                army m_right = { };

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
                        this->m_warnings.push_back("Failed to read config file.");
                        return;
                    }

                    // ~~ Build unit database ~~
                    std::size_t count_units = db.load_from_folder(c.maps_path());
                    this->m_warnings.push_back("Loaded " + std::to_string(count_units) + " units.");
                    this->m_left = { };
                    this->m_right = { };
                } // reload(...)

                const settlers_online::warnings& warnings() const noexcept { return this->m_warnings; }
                settlers_online::warnings& warnings() noexcept { return this->m_warnings; }

                const army& left() const noexcept { return this->m_left; }
                const army& right() const noexcept { return this->m_right; }

                void parse_left(const std::string& army_string) noexcept
                {
                    army_parser parser = army_string;
                    this->m_left = parser.build(this->m_warnings, true, true, false);
                } // parse_left(...)

                void parse_right(const std::string& army_string) noexcept
                {
                    army_parser parser = army_string;
                    this->m_right = parser.build(this->m_warnings);
                } // parse_right(...)

                std::vector<report_entry> log() noexcept { return this->run(true); }

                std::vector<report_entry> run(bool is_log = false) noexcept
                {
                    std::vector<report_entry> report { };
                    
                    const config& c = config::instance();
                    if (this->m_left.count_groups() == 0 || this->m_right.count_groups() == 0)
                    {
                        this->m_warnings.push_back("Armies (left and right) have to be set prior to execution.");
                        return report;
                    }
                    
                    settlers_online::warnings left_decorator_warnings { };
                    settlers_online::warnings right_decorator_warnings { };
                    c.left().decorate(this->m_left, left_decorator_warnings);
                    c.right().decorate(this->m_right, right_decorator_warnings);
                    
                    // ~~ Combat phase ~~
                    ropufu::settlers_online::combat_mechanics combat(this->m_left, this->m_right);
                    ropufu::settlers_online::combat_mechanics snapshot = combat;
                
                    //std::cout << "Building left cache..." << std::endl;
                    sequencer_type::pool_type::instance().cache(combat.left().underlying());
                    //std::cout << "Building right cache..." << std::endl;
                    sequencer_type::pool_type::instance().cache(combat.right().underlying());

                    auto army_format = [] (const unit_type& u) { return " " + u.names().front(); };
                    std::string left_army_string = this->m_left.to_string(army_format);
                    std::string right_army_string = this->m_right.to_string(army_format);
                
                    // ~~ Choose sequencers ~~
                    sequencer_type left_seq { };
                    sequencer_type right_seq { };
                
                    report.emplace_back(left_army_string + " vs. " + right_army_string);
                    if (is_log)
                    {
                        combat.set_do_log(true);
                        combat.execute(left_seq, right_seq);
                        return report;
                    }
                    
                    empirical_measure rounds { };
                    std::vector<empirical_measure> left_losses(this->m_left.count_groups());
                    std::vector<empirical_measure> right_losses(this->m_right.count_groups());
                    for (std::size_t i = 0; i < c.simulation_count(); ++i)
                    {
                        std::size_t combat_rounds = combat.execute(left_seq, right_seq);
                        //const ropufu::settlers_online::combat_result& result = combat.outcome();

                        std::vector<std::size_t> x = combat.left().calculate_losses();
                        std::vector<std::size_t> y = combat.right().calculate_losses();
                        for (std::size_t k = 0; k < x.size(); k++) left_losses[k].observe(x[k]);
                        for (std::size_t k = 0; k < y.size(); k++) right_losses[k].observe(y[k]);
                
                        for (std::size_t j = 0; j < c.destruction_count(); ++j)
                        {
                            std::size_t destruction_rounds = combat.destruct(left_seq, right_seq);
                            rounds.observe(combat_rounds + destruction_rounds);
                        }
                        combat = snapshot;
                    }
                    
                    report.emplace_back("Rounds", rounds);

                    report.emplace_back("Left army", left_army_string);
                    if (!left_decorator_warnings.empty()) left_decorator_warnings.unwind([&] (const std::string& w) { report.emplace_back("Skills", w); });
                    type::present_losses(report, this->m_left, left_losses);

                    report.emplace_back("Right army", left_army_string);
                    if (!right_decorator_warnings.empty()) right_decorator_warnings.unwind([&] (const std::string& w) { report.emplace_back("Skills", w); });
                    type::present_losses(report, this->m_right, right_losses);
                
                    c.to_cbor(report);
                    return report;
                } // run(...)
            }; // struct turtle
        } // namespace black_marsh
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
