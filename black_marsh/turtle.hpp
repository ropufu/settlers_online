
#ifndef ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED

#include <aftermath/not_an_error.hpp>
#include <aftermath/probability.hpp>

#include "config.hpp"

#include "../settlers_online/unit_database.hpp"
#include "../settlers_online/army_parser.hpp"
#include "../settlers_online/army.hpp"
#include "../settlers_online/binomial_pool.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/combat_result.hpp"
#include "../settlers_online/conditioned_army.hpp"
#include "../settlers_online/randomized_attack_sequence.hpp"
#include "../settlers_online/trivial_attack_sequence.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <exception> // std::exception
#include <iostream> // std::cout, std::endl
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

                template <typename t_action_type>
                static double elapsed_seconds(t_action_type action)
                {
                    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                    action();
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                
                    double elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1'000.0;
                    return elapsed_seconds;
                } // elapsed_seconds

                static void unwind_errors(bool do_skip_log)
                {
                    aftermath::quiet_error& err = aftermath::quiet_error::instance();
                
                    if (!err.good()) std::cout << "~~ Oh no! Errors encoutered: ~~" << std::endl;
                    else if (!err.empty()) std::cout << "~~ Something to keep in mind: ~~" << std::endl;
                    while (!err.empty())
                    {
                        aftermath::quiet_error_descriptor desc = err.pop();
                        if (do_skip_log && desc.severity() == aftermath::severity_level::not_at_all) continue; // Skip log messages.
                
                        std::cout << '\t' <<
                            " " << std::to_string(desc.severity()) <<
                            " " << std::to_string(desc.error_code()) <<
                            " on line " << desc.caller_line_number() <<
                            " of <" << desc.caller_function_name() << ">: " << desc.description() << std::endl;
                    }
                }

            private:
                ropufu::settlers_online::army m_left = { };
                ropufu::settlers_online::army m_right = { };
                std::size_t m_count_combat_sims = 10'000;
                std::size_t m_count_destruct_sims_per_combat = 50;
                
                static bool try_parse_army(const std::string& str, army& value)
                {
                    const unit_database& db = unit_database::instance();

                    army_parser parser = str;
                    if (!parser.good())
                    {
                        std::cout << "Parsing army \"" << str << "\" failed." << std::endl;
                        return false;
                    }
                    if (!parser.try_build_fast(db, value)) 
                    {
                        std::cout << "Reconstructing army \"" << str << "\" from database failed." << std::endl;
                        return false;
                    }
                    return true;
                }

            public:
                turtle() noexcept
                {
                    this->reload();
                }

                void reload() noexcept
                {
                    config& c = config::instance();
                    unit_database& db = unit_database::instance();

                    c.read();
                    std::string maps_path = c["maps folder"];
                    std::size_t n1 = c["simulations"];
                    std::size_t n2 = c["destructions per sim"];
                    this->m_count_combat_sims = n1;
                    this->m_count_destruct_sims_per_combat = n2;

                    // ~~ Build unit database ~~
                    std::cout << "Loaded " << db.load_from_folder(maps_path) << " units." << std::endl;
                    this->m_left = { };
                    this->m_right = { };
                    type::unwind_errors(false);
                }

                const army& left() const noexcept { return this->m_left; }
                const army& right() const noexcept { return this->m_right; }

                std::size_t simulation_count() const noexcept { return this->m_count_combat_sims; }
                void set_simulation_count(std::size_t value) noexcept
                {
                    this->m_count_combat_sims = value;
                    config::instance()["simulations"] = value;
                    config::instance().write();
                }

                void parse_left(const std::string& army_string) noexcept
                {
                    type::try_parse_army(army_string, this->m_left);
                    type::unwind_errors(false);
                }

                void parse_right(const std::string& army_string) noexcept
                {
                    type::try_parse_army(army_string, this->m_right);
                    type::unwind_errors(false);
                }

                void run(bool is_log = false)
                {
                    unwind_errors(false);
                    if (this->m_left.count_groups() == 0 || this->m_right.count_groups() == 0)
                    {
                        std::cout << "Armies (left and right) have to be set prior to execution." << std::endl;
                        return;
                    }
                
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
                    sequencer_type left_seq = { };
                    sequencer_type right_seq = { };
                
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
                        ropufu::settlers_online::combat_result result = combat.execute(left_seq, right_seq);

                        std::size_t combat_rounds = result.number_of_rounds();
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
                        std::cout << "Losses in " << this->m_left[k].type().names().front() << ":" << std::endl;
                        std::cout << left_losses[k] << std::endl;
                    }
                
                    unwind_errors(false);
                } // run(...)
            }; // struct turtle
        } // namespace black_marsh
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_BLACK_MARSH_TURTLE_HPP_INCLUDED
