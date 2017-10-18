
#include <aftermath/not_an_error.hpp>

#include "../settlers_online/unit_database.hpp"
#include "../settlers_online/army_parser.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <iostream> // std::cout, std::endl
#include <string> // std::string, std::to_string

using unit_database = ropufu::settlers_online::unit_database;
using quiet_error = ropufu::aftermath::quiet_error;

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
            " of <" << desc.caller_function_name() << ">:\t" << desc.description() << std::endl;
    }
}

void welcome() noexcept
{
    std::cout << R"?(
 ~~~ Lucy Turtle Simulator ~~~

///////|////////////////////////|
///|//////------------\\\|\\|\\\\
//|///////    ______   \\\|\\\\|\
////////|    |      |   |\\\\\\\\
////// //   | o = 0 |   \\\ \\\\\
//||   ||   |       |    ||   |||
//||...||||||||||||||||||||. .|||
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
    return 0;
}
