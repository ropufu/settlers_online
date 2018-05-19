
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
#include "../settlers_online/binomial_pool.hpp"
#include "../settlers_online/camp.hpp"
#include "../settlers_online/char_string.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/combat_result.hpp"
#include "../settlers_online/conditioned_army.hpp"
#include "../settlers_online/enums.hpp"
#include "../settlers_online/logger.hpp"
#include "../settlers_online/randomized_attack_sequence.hpp"
#include "../settlers_online/report_entry.hpp"
#include "../settlers_online/trivial_attack_sequence.hpp"
#include "../settlers_online/unit_group.hpp"
#include "../settlers_online/unit_type.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t, nullptr
#include <cstdint> // std::int32_t
#include <deque> // std::deque
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
                using logger_type = ropufu::settlers_online::detail::logger;
                using no_logger_type = ropufu::settlers_online::detail::no_logger;

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

                /** Adds information about losses to the report, and calculates the upper and lower bounds on the surviving army. */
                static void present_losses(std::vector<report_entry>& report, const army& a, 
                    const std::vector<std::size_t>& losses_lower_bound, const std::vector<empirical_measure>& losses, const std::vector<std::size_t>& losses_upper_bound,
                    army& worst_case, army& best_case) noexcept
                {
                    worst_case = a;
                    best_case = a;
                    for (std::size_t k = 0; k < a.count_groups(); k++)
                    {
                        report_entry entry(a[k], losses[k]);
                        entry.set_lower_bound(losses_lower_bound[k]);
                        entry.set_upper_bound(losses_upper_bound[k]);
                        entry.set_text("Losses in " + a[k].unit().names().front());
                        report.push_back(entry);

                        worst_case[k].kill(losses[k].max());
                        best_case[k].kill(losses[k].min());
                    }
                    worst_case.snapshot();
                    best_case.snapshot();
                } // present_losses(...)

            private:
                logger_type m_logger = { };
                std::deque<army> m_left_sequence = { };
                std::deque<army> m_right_sequence = { };

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
                        this->m_logger.write("Failed to read config file.");
                        return;
                    }

                    // ~~ Build unit database ~~
                    std::size_t count_units = db.load_from_folder(c.maps_path(), this->m_logger);
                    this->m_logger << "Loaded " << count_units << " units." << nullptr;
                    this->m_left_sequence = { };
                    this->m_right_sequence = { };
                } // reload(...)

                const logger_type& logger() const noexcept { return this->m_logger; }
                logger_type& logger() noexcept { return this->m_logger; }

                const std::deque<army>& left_sequence() const noexcept { return this->m_left_sequence; }
                const std::deque<army>& right_sequence() const noexcept { return this->m_right_sequence; }

                void parse_left(const std::string& army_list_string) noexcept
                {
                    this->m_left_sequence.clear();
                    for (const std::string& army_string : char_string::split(army_list_string, "+"))
                    {
                        army_parser parser = army_string;
                        this->m_left_sequence.emplace_back(parser.build(this->m_logger, true, true, false));
                    }
                } // parse_left(...)

                void parse_right(const std::string& army_list_string) noexcept
                {
                    this->m_right_sequence.clear();
                    for (const std::string& army_string : char_string::split(army_list_string, "+"))
                    {
                        army_parser parser = army_string;
                        this->m_right_sequence.emplace_back(parser.build(this->m_logger));
                    }
                } // parse_right(...)

                std::vector<report_entry> log() noexcept 
                {
                    const config& c = config::instance();
                    logger_type logger { };
                    std::vector<report_entry> report = this->run(c.left(), c.right(), 1, 1, logger);
                    logger.unwind();
                    return report;
                }

                std::vector<report_entry> run() noexcept 
                {
                    const config& c = config::instance();
                    no_logger_type logger { };
                    return this->run(c.left(), c.right(), c.simulation_count(), c.destruction_count(), logger);
                }
                
            private:
                template <typename t_logger_type>
                std::vector<report_entry> run(const army_decorator& left_decorator, const army_decorator& right_decorator, std::size_t simulation_count, std::size_t destruction_count, t_logger_type& logger) noexcept
                {
                    // @todo Erase old .cbor output.
                    std::vector<report_entry> report { };
                    if (this->m_left_sequence.empty() || this->m_right_sequence.empty())
                    {
                        this->m_logger.write("Armies (left and right) have to be set prior to execution.");
                        return report;
                    }

                    // Make copies of army sequences for the simulation.
                    std::deque<army> attacker_sequence = this->m_left_sequence;
                    std::deque<army> defender_sequence = this->m_right_sequence;
                    for (army& next_left : attacker_sequence) left_decorator.decorate(next_left);
                    for (army& next_right : defender_sequence) right_decorator.decorate(next_right);

                    // Goal: eliminate all right (defender) waves.
                    for (army& next_right : defender_sequence)
                    {
                        // As long as the defender (next_right) is alive, keep throwing attackers at them.
                        for (army& next_left : attacker_sequence)
                        {
                            if (next_left.count_units() == 0) continue; // If the attacker wave has been defeated, continue to the next one.
                            type::run(next_left, next_right, report, simulation_count, destruction_count, logger);
                        }
                    }

                    config::instance().to_cbor(report);
                    return report;
                } // run(...)

                template <typename t_logger_type>
                static void run(army& left, army& right, std::vector<report_entry>& report, std::size_t simulation_count, std::size_t destruction_count, t_logger_type& logger) noexcept
                {
                    no_logger_type no_logger { };
                    army next_left = left;
                    // ~~ Combat phase ~~
                    ropufu::settlers_online::combat_mechanics combat(left, right);
                    ropufu::settlers_online::combat_mechanics snapshot = combat;
                
                    sequencer_type::pool_type::instance().cache(combat.left().underlying());
                    sequencer_type::pool_type::instance().cache(combat.right().underlying());

                    auto army_format = [] (const unit_type& u) { return " " + u.names().front(); };
                    auto compact_format = [] (const unit_type& u) { return prefix_builder<unit_type>::build_key(u); };
                    std::string left_army_compact_string = left.to_string(compact_format);
                    std::string left_army_string = left.to_string(army_format);
                    std::string right_army_string = right.to_string(army_format);
                
                    // ~~ Choose sequencers ~~
                    sequencer_type left_seq { };
                    sequencer_type right_seq { };

                    report.emplace_back(left_army_string + " vs. " + right_army_string);

                    // ~~ Bounds ~~
                    sequencer_type_low weak_seq { };
                    sequencer_type_high strong_seq { };

                    combat.execute(weak_seq, strong_seq, no_logger);
                    std::vector<std::size_t> left_losses_upper_bound = combat.left().calculate_losses();
                    std::vector<std::size_t> right_losses_lower_bound = combat.right().calculate_losses();
                    combat = snapshot; // Reset combat.
                    combat.execute(strong_seq, weak_seq, no_logger);
                    std::vector<std::size_t> left_losses_lower_bound = combat.left().calculate_losses();
                    std::vector<std::size_t> right_losses_upper_bound = combat.right().calculate_losses();
                    combat = snapshot; // Reset combat.
                    
                    empirical_measure combat_rounds { };
                    empirical_measure destruction_rounds { };
                    empirical_measure total_rounds { };
                    std::vector<empirical_measure> left_losses(left.count_groups());
                    std::vector<empirical_measure> right_losses(right.count_groups());
                    for (std::size_t i = 0; i < simulation_count; ++i)
                    {
                        std::size_t count_combat_rounds = combat.execute(left_seq, right_seq, logger);
                        //const ropufu::settlers_online::combat_result& result = combat.outcome();

                        std::vector<std::size_t> x = combat.left().calculate_losses();
                        std::vector<std::size_t> y = combat.right().calculate_losses();
                        for (std::size_t k = 0; k < x.size(); k++) left_losses[k].observe(x[k]);
                        for (std::size_t k = 0; k < y.size(); k++) right_losses[k].observe(y[k]);
                
                        combat_rounds.observe(count_combat_rounds);
                        for (std::size_t j = 0; j < destruction_count; ++j)
                        {
                            std::size_t count_destruction_rounds = combat.destruct(left_seq, right_seq);
                            destruction_rounds.observe(count_destruction_rounds);
                            total_rounds.observe(count_combat_rounds + count_destruction_rounds);
                        }
                        combat = snapshot; // Reset combat.
                    }
                    
                    report.emplace_back("Rounds", total_rounds);
                    if (!destruction_rounds.empty() && destruction_rounds.max() != 0)
                    {
                        report.emplace_back("Combat Rounds", combat_rounds);
                        report.emplace_back("Destruction Rounds", destruction_rounds);
                    }

                    army dummy { };
                    army worst_case_left { };
                    army best_case_right { };

                    report.emplace_back("Left army", left_army_string, left_army_compact_string);
                    if (!left.skills().empty()) report.emplace_back("Skills", left.skills_string());
                    if (!left.traits().empty()) report.emplace_back("Traits", left.traits_string());
                    type::present_losses(report, left, left_losses_lower_bound, left_losses, left_losses_upper_bound, worst_case_left, dummy);

                    report.emplace_back("Right army", right_army_string, right_army_string);
                    if (!right.skills().empty()) report.emplace_back("Skills", right.skills_string());
                    if (!right.traits().empty()) report.emplace_back("Traits", right.traits_string());
                    type::present_losses(report, right, right_losses_lower_bound, right_losses, right_losses_upper_bound, dummy, best_case_right);

                    if (worst_case_left.count_units() != 0) report.emplace_back("Next wave", worst_case_left.to_string(army_format), worst_case_left.to_string(compact_format));
                    if (best_case_right.count_units() != 0) report.emplace_back("Next wave", best_case_right.to_string(army_format), best_case_right.to_string(army_format));
                    left = worst_case_left;
                    right = best_case_right;
                } // run(...)
            }; // struct turtle
        } // namespace black_marsh
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
