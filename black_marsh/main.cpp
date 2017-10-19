
#include <aftermath/not_an_error.hpp>
#include <aftermath/probability.hpp>

#include "../settlers_online/unit_database.hpp"
#include "../settlers_online/army_parser.hpp"
#include "../settlers_online/army.hpp"
#include "../settlers_online/binomial_pool.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/combat_result.hpp"
#include "../settlers_online/randomized_attack_sequence.hpp"
#include "../settlers_online/trivial_attack_sequence.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <exception> // std::exception
#include <iostream> // std::cout, std::endl
#include <string> // std::string, std::to_string
#include <vector> // std::vector

// ~~ Singleton types ~~
using unit_database = ropufu::settlers_online::unit_database;
using quiet_error = ropufu::aftermath::quiet_error;

using sequencer_type = ropufu::settlers_online::randomized_attack_sequence<>;
using sequencer_type_low = ropufu::settlers_online::trivial_attack_sequence<false>;
using sequencer_type_high = ropufu::settlers_online::trivial_attack_sequence<true>;

using empirical_measure = ropufu::aftermath::probability::empirical_measure<std::size_t, std::size_t, double>;

template <typename t_action_type>
void benchmark(t_action_type action, const std::string& name)
{
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    action();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    double elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1'000.0;
    std::cout << "Elapsed time: " << elapsed_seconds << "s." << std::endl;
}

bool try_parse_army(const std::string& str, ropufu::settlers_online::army& value)
{
    const unit_database& db = unit_database::instance();

    ropufu::settlers_online::army_parser parser = str;
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

void unwind_errors(bool do_skip_log)
{
    quiet_error& err = quiet_error::instance();

    if (!err.good()) std::cout << "~~ Oh no! Errors encoutered: ~~" << std::endl;
    else if (!err.empty()) std::cout << "~~ Something to keep in mind: ~~" << std::endl;
    while (!err.empty())
    {
        ropufu::aftermath::quiet_error_descriptor desc = err.pop();
        if (do_skip_log && desc.severity() == ropufu::aftermath::severity_level::not_at_all) continue; // Skip log messages.

        std::cout << '\t' <<
            " " << std::to_string(desc.severity()) <<
            " " << std::to_string(desc.error_code()) <<
            " on line " << desc.caller_line_number() <<
            " of <" << desc.caller_function_name() << ">:" << desc.description() << std::endl;
    }
}

void welcome() noexcept
{
    std::cout << R"?(
 ~~~ Lucy Turtle Simulator ~~~

///////|////////////////////////|
///|//////"--,-------;\\\|\\|\\\\
//|///////    ______   \\\|\\\\|\
////////|    |      |   |\\\\\\\\
////// //   | o = 0 |   \\\ \\\\\
//||   ||   \       /    ||   |||
//||.".||||||||||||||||||||" .|||
///////////|////|||//|\\\\\\\\\\|

If you need help at any time, please type "help" without quotation marks.
If for some reason you want to quit, type "exit" or "quit" or "q".

)?";
}

std::int32_t main(std::int32_t argc, char* argv[]/*, char* envp[]*/)
{
    if (argc < 3)
    {
        std::cout << "Usage: black_marsh \"A1\" \"A2\"" << std::endl
            << '\t' << "where A1 and A2 are string representations of two armies.";

        std::cout << "For example: black_marsh \"100r 1((cxv))\" \"20 Fox 1 Giant\"." << std::endl;
        return 0;
    }
    welcome();
    std::string left_army_string = std::string(argv[1]);
    std::string right_army_string = std::string(argv[2]);

    unit_database& db = unit_database::instance();

    // ~~ Build unit database ~~
    std::cout << "Loaded " << db.load_from_folder("./../maps/") << " units." << std::endl;
    unwind_errors(false);

    // ~~ Parse armies ~~
    ropufu::settlers_online::army left = { };
    ropufu::settlers_online::army right = { };
    if (!try_parse_army(left_army_string, left) || !try_parse_army(right_army_string, right))
    {
        unwind_errors(false);
        return 0;
    }
    unwind_errors(false);

    // ~~ Combat phase ~~
    std::cout << "~~ Simulations ~~" << std::endl;
    ropufu::settlers_online::combat_mechanics combat(left, right);
    ropufu::settlers_online::combat_mechanics snapshot = combat;

    sequencer_type::pool_type::instance().cache(combat.left());
    sequencer_type::pool_type::instance().cache(combat.right());

    // ~~ Choose sequencers
    sequencer_type left_seq = { };
    sequencer_type right_seq = { };
    std::size_t count_combat_sims = 1000;
    std::size_t count_destruct_sims_per_combat = 10;

    std::cout << "~~ Combat phase ~~" << std::endl;

    double combat_rounds = 0;
    std::vector<std::size_t> x;
    std::vector<std::size_t> y;
    std::vector<empirical_measure> left_losses(left.count_groups());
    
    for (std::size_t i = 0; i < count_combat_sims; ++i)
    {
        ropufu::settlers_online::combat_result result = combat.execute(left_seq, right_seq);
        combat.calculate_losses(x, y);
        for (std::size_t k = 0; k < x.size(); k++) left_losses[k].observe(x[k]);

        double destruction_rounds = 0;
        for (std::size_t j = 0; j < count_destruct_sims_per_combat; ++j) destruction_rounds += combat.destruct(left_seq, right_seq);
        destruction_rounds /= count_destruct_sims_per_combat;

        combat_rounds += result.number_of_rounds() + destruction_rounds;
        combat = snapshot;
    }
    combat_rounds /= count_combat_sims;
    
    std::cout << "Rounds = " << combat_rounds << std::endl;
    for (std::size_t k = 0; k < left.count_groups(); k++)
    {
        std::cout << "Losses in " << left[k].type().names().front() << ":" << std::endl;
        std::cout << left_losses[k] << std::endl;
    }

    unwind_errors(false);
    return 0;
}
