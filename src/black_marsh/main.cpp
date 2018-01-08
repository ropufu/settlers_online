
#include <aftermath/not_an_error.hpp>

#include "config.hpp"
#include "turtle.hpp"
#include "../settlers_online/army.hpp"
#include "../settlers_online/char_string.hpp"
#include "../settlers_online/report_entry.hpp"
#include "../settlers_online/unit_faction.hpp"
#include "../settlers_online/unit_type.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <iostream> // std::cout, std::endl, std::cin
#include <string> // std::string, std::to_string, std::getline, std::stol, std::stod

// ~~ Singleton types ~~
using unit_faction = ropufu::settlers_online::unit_faction;
using unit_database = ropufu::settlers_online::unit_database;
using char_string = ropufu::settlers_online::char_string;
using quiet_error = ropufu::aftermath::quiet_error;

template <typename t_collection_type>
void print_elements(const t_collection_type& container, const std::string& delimiter = ", ")
{
    if (container.empty()) std::cout << "empty";
    bool is_first = true;
    for (const auto& item : container)
    {
        if (!is_first) std::cout << delimiter;
        std::cout << item;
        is_first = false;
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
} // welcome(...)

enum struct command_name
{
    not_recognized,
    quit, // Exit the program.
    help, // Display help.
    units, // Lists all units.
    left, // Left army.
    right, // Right army.
    skills, // Generic term for army sub-clause.
    camp, // Generic term for army sub-clause.
    hit_points, // Generic term for camp sub-clause.
    damage_reduction, // Generic term for camp sub-clause.
    left_camp,
    right_camp,
    left_camp_hit_points,
    right_camp_hit_points,
    left_camp_reduction,
    right_camp_reduction,
    left_skills,
    right_skills,
    n, // Number of sumulations.
    log, // Run simulation in log mode.
    run // Run simulation in regular mode.
}; // struct command_name

void unwind_errors(bool do_skip_log, bool is_quiet = false) noexcept
{
    ropufu::aftermath::quiet_error& err = ropufu::aftermath::quiet_error::instance();

    if (!is_quiet)
    {
        if (!err.good()) std::cout << "~~ Oh no! Errors encoutered: ~~" << std::endl;
        else if (!err.empty()) std::cout << "~~ Something to keep in mind: ~~" << std::endl;
    }
    while (!err.empty())
    {
        ropufu::aftermath::quiet_error_descriptor desc = err.pop();
        if (is_quiet) continue;
        if (do_skip_log && desc.severity() == ropufu::aftermath::severity_level::not_at_all) continue; // Skip log messages.

        std::cout << '\t' <<
            " " << std::to_string(desc.severity()) <<
            " " << std::to_string(desc.error_code()) <<
            " on line " << desc.caller_line_number() <<
            " of <" << desc.caller_function_name() << ">: " << desc.description() << std::endl;
    }
} // unwind_errors(...)

void split_in_two(const std::string& expression, std::string& command, std::string& argument) noexcept
{
    command = char_string::deep_trim_copy(expression);
    argument = "";

    std::size_t index_of_space = command.find(" ");
    if (index_of_space != std::string::npos) 
    {
        argument = command.substr(index_of_space + 1);
        command = command.substr(0, index_of_space);
    }
} // split_in_two(...)

command_name parse_camp_clause(const std::string& expression, std::string& argument) noexcept
{
    std::string command = "";
    split_in_two(expression, command, argument);

    if (command == "hitpoints" || command == "hp") return command_name::hit_points;
    if (command == "reduction" || command == "r") return command_name::damage_reduction;

    return command_name::not_recognized;
} // parse_camp_clause(...)

command_name parse_army_clause(const std::string& expression, std::string& argument) noexcept
{
    std::string command = "";
    split_in_two(expression, command, argument);

    if (command == "camp" || command == "cmp" || command == "c") return command_name::camp;
    if (command == "skills" || command == "skill" || command == "s") return command_name::skills;

    return command_name::not_recognized;
} // parse_army_clause(...)

command_name parse_command(const std::string& expression, std::string& argument) noexcept
{
    std::string command = "";
    split_in_two(expression, command, argument);

    if (command == "quit" || command == "q" || command == "exit") return command_name::quit;
    if (command == "help" || command == "h" || command == "?") return command_name::help;
    if (command == "units" || command == "unit" || command == "u") return command_name::units;
    if (command == "left" || command == "l")
    {
        std::string subclause = "";
        switch (parse_army_clause(argument, subclause))
        {
            case command_name::camp:
                switch (parse_camp_clause(subclause, argument))
                {
                    case command_name::hit_points: return command_name::left_camp_hit_points;
                    case command_name::damage_reduction: return command_name::left_camp_reduction;
                    default: return command_name::left_camp;
                }
            case command_name::skills:
                argument = subclause;
                return command_name::left_skills;
            default: return command_name::left;
        }
    }
    if (command == "right" || command == "r")
    {
        std::string subclause = "";
        switch (parse_army_clause(argument, subclause))
        {
            case command_name::camp:
                switch (parse_camp_clause(subclause, argument))
                {
                    case command_name::hit_points: return command_name::right_camp_hit_points;
                    case command_name::damage_reduction: return command_name::right_camp_reduction;
                    default: return command_name::right_camp;
                }
            case command_name::skills:
                argument = subclause;
                return command_name::right_skills;
            default: return command_name::right;
        }
    }
    if (command == "n") return command_name::n;
    if (command == "log") return command_name::log;
    if (command == "run") return command_name::run;

    return command_name::not_recognized;
} // parse_command(...)

void help(const std::string& argument) noexcept
{
    std::string dummy { };
    command_name command = parse_command(argument, dummy);
    switch (command)
    {
        case command_name::units:
            std::cout << "You can either type \"units\" to display all units, or \"units <faction name>\"," << std::endl << "where faction is one of:" << std::endl;
            for (std::size_t i = 0; i < ropufu::settlers_online::enum_capacity<unit_faction>::value; i++)
            {
                std::string faction_name = std::to_string(static_cast<unit_faction>(i));
                if (faction_name.length() > 2) std::cout << '\t' << faction_name << std::endl;
            }
            break;
        default:
        std::cout << R"?(
    Command Name   | Description
==============================================================
    quit, q, exit  | Exit the program.
    help, h, ?     | Display help.
    units, u       | Lists units from a specified faction.
    left, l        | Get or set left army.
    right, r       | Get or set right army.
    n              | Gets or sets the number of simulations.
    log            | Displays one battle report.
    run            | Executes the simulations.
==============================================================
Following the left / right command, one could use:
    Command Name   | Description
==============================================================
... camp, c        | Get or set camp.
... skills, s      | Get skills.
==============================================================
Following the left / right camp command, one could use:
    Command Name   | Description
==============================================================
... hitpoints, hp  | Get or set camp hit points.
... reduction, r   | Get or set camp damage reduction.
==============================================================
Commands with get or set option will take an optional argument to set the
value of corresponding parameter.

You can also run the program with arguments: "left army" "string army" [/s]
The first two are required (don't omit the quotation marks) and will
automatically populate the left and right armies.
The third is optional and may be one of: /l /r. When provided, the program
will automatically execute "log" for /l, or "run" for 'r', and then quit. 
)?";
            break;
    }
} // help(...)

void display_units(const std::string& faction_name) noexcept
{
    unit_faction faction = unit_faction::general;
    bool do_take_all = !ropufu::settlers_online::detail::try_parse_str(faction_name, faction);
    for (const auto& pair : unit_database::instance().data())
    {
        const ropufu::settlers_online::unit_type& u = pair.second;
        if (do_take_all || u.faction() == faction)
        {
            bool is_first = true;
            std::cout << '\t';
            for (const std::string& name : u.names())
            {
                if (!is_first) std::cout << ", ";
                std::cout << name;
                is_first = false;
            }
            std::cout << std::endl;
        }
    }
} // display_units(...)

std::int32_t quit() noexcept
{
    std::cout << "Buh-bye ^^//~~" << std::endl;
    return 0;
} // quit(...)

std::string read_line() noexcept
{
    std::string line;
    std::getline(std::cin, line);
    return char_string::deep_trim_copy(line);
} // read_line(...)

std::int32_t main(std::int32_t argc, char* argv[]/*, char* envp[]*/)
{
    if (!ropufu::settlers_online::black_marsh::turtle::is_config_valid())
    {
        std::cout << "Failed to read config file." << std::endl;
        return 1;
    }

    ropufu::settlers_online::black_marsh::config& config = ropufu::settlers_online::black_marsh::config::instance();
    ropufu::settlers_online::black_marsh::turtle lucy { };
    if (argc > 2)
    {
        std::string left_army_string = std::string(argv[1]);
        std::string right_army_string = std::string(argv[2]);
        lucy.parse_left(left_army_string);
        lucy.parse_right(right_army_string);

        if (argc > 3)
        {
            unwind_errors(false);
            std::string sw = std::string(argv[3]);
            if (sw == "-l") { lucy.log(); return 0; }
            if (sw == "-r") { lucy.run(); return 0; }
        }
    }

    welcome();
    while (true)
    {
        std::string command;
        std::string argument;

        unwind_errors(false);
        lucy.logger().unwind();

        std::cout << "> ";
        command = read_line();

        switch (parse_command(command, argument))
        {
            case command_name::quit: return quit();
            case command_name::help:
                help(argument);
                break;
            case command_name::units:
                display_units(argument);
                break;
            case command_name::left:
                if (!argument.empty()) lucy.parse_left(argument);
                std::cout << "Left army: ";
                print_elements(lucy.left_sequence(), " + ");
                std::cout << std::endl;
                break;
            case command_name::right:
                if (!argument.empty()) lucy.parse_right(argument);
                std::cout << "Right army: ";
                print_elements(lucy.right_sequence(), " + ");
                std::cout << std::endl;
                break;
            case command_name::left_camp:
                std::cout << "Left campt: " << config.left().camp() << std::endl;
                break;
            case command_name::right_camp:
                std::cout << "Right camp: " << config.right().camp() << std::endl;
                break;
            case command_name::left_camp_reduction:
                if (!argument.empty())
                {
                    auto camp = config.left().camp();
                    camp.set_damage_reduction(std::stod(argument));
                    config.left().set_camp(camp);
                }
                std::cout << "Left camp damage recution: " << config.left().camp().damage_reduction() << std::endl;
                break;
            case command_name::right_camp_reduction:
                if (!argument.empty())
                {
                    auto camp = config.right().camp();
                    camp.set_damage_reduction(std::stod(argument));
                    config.right().set_camp(camp);
                }
                std::cout << "Right camp damage recution: " << config.right().camp().damage_reduction() << std::endl;
                break;
            case command_name::left_camp_hit_points:
                if (!argument.empty())
                {
                    auto camp = config.left().camp();
                    camp.set_hit_points(static_cast<std::size_t>(std::stol(argument)));
                    config.left().set_camp(camp);
                }
                std::cout << "Left camp hit points: " << config.left().camp().hit_points() << std::endl;
                break;
            case command_name::right_camp_hit_points:
                if (!argument.empty())
                {
                    auto camp = config.right().camp();
                    camp.set_hit_points(static_cast<std::size_t>(std::stol(argument)));
                    config.right().set_camp(camp);
                }
                std::cout << "Right camp hit points: " << config.right().camp().hit_points() << std::endl;
                break;
            case command_name::left_skills:
                std::cout << config.left() << std::endl;
                break;
            case command_name::right_skills:
                std::cout << config.right() << std::endl;
                break;
            case command_name::n:
                if (!argument.empty()) config.set_simulation_count(static_cast<std::size_t>(std::stol(argument)));
                std::cout << "Number of simulations: " << config.simulation_count() << std::endl;
                break;
            case command_name::log:
                for (const auto& entry : lucy.log()) std::cout << entry << std::endl;
                break;
            case command_name::run:
                for (const auto& entry : lucy.run()) std::cout << entry << std::endl;
                break;
            default:
                std::cout << "Command \"" << command << "\" not recognized. Try \"help\" to get a list of avaiable commands, or \"quit\" to quit." << std::endl;
                break;
        }
    }
    return 0;
} // main(...)