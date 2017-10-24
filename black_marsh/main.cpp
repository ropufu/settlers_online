
#include <aftermath/not_an_error.hpp>

#include "turtle.hpp"
#include "../settlers_online/army.hpp"
#include "../settlers_online/char_string.hpp"
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
... skills, s      | Get or set skills.
==============================================================
Following the left / right vcamp command, one could use:
    Command Name   | Description
==============================================================
... hitpoints, hp  | Get or set camp.
... reduction, r   | Get or set skills.
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
    bool do_take_all = !ropufu::settlers_online::try_parse(faction_name, faction);
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

    ropufu::settlers_online::black_marsh::turtle lucy { };
    if (argc > 2)
    {
        std::string left_army_string = std::string(argv[1]);
        std::string right_army_string = std::string(argv[2]);
        lucy.parse_left(left_army_string);
        lucy.parse_right(right_army_string);

        if (argc > 3)
        {
            std::string sw = std::string(argv[3]);
            if (sw == "/l")
            {
                lucy.run(true);
                return 0;
            }
            if (sw == "/r")
            {
                lucy.run(false);
                return 0;
            }
        }
    }

    welcome();
    while (true)
    {
        std::string command;
        std::string argument;

        ropufu::settlers_online::black_marsh::turtle::unwind_errors(false);
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
                std::cout << "Left army: " << lucy.left() << std::endl;
                break;
            case command_name::right:
                if (!argument.empty()) lucy.parse_right(argument);
                std::cout << "Right army: " << lucy.right() << std::endl;
                break;
            case command_name::left_camp:
                std::cout << "Left campt: " << lucy.left_camp() << std::endl;
                break;
            case command_name::right_camp:
                std::cout << "Right camp: " << lucy.right_camp() << std::endl;
                break;
            case command_name::left_camp_reduction:
                if (!argument.empty())
                {
                    auto camp = lucy.left_camp();
                    camp.set_damage_reduction(std::stod(argument));
                    lucy.set_left_camp(camp);
                }
                std::cout << "Left camp damage recution: " << lucy.left_camp().damage_reduction() << std::endl;
                break;
            case command_name::right_camp_reduction:
                if (!argument.empty())
                {
                    auto camp = lucy.right_camp();
                    camp.set_damage_reduction(std::stod(argument));
                    lucy.set_right_camp(camp);
                }
                std::cout << "Right camp damage recution: " << lucy.right_camp().damage_reduction() << std::endl;
                break;
            case command_name::left_camp_hit_points:
                if (!argument.empty())
                {
                    auto camp = lucy.left_camp();
                    camp.set_hit_points(static_cast<std::size_t>(std::stol(argument)));
                    lucy.set_left_camp(camp);
                }
                std::cout << "Left camp hit points: " << lucy.left_camp().hit_points() << std::endl;
                break;
            case command_name::right_camp_hit_points:
                if (!argument.empty())
                {
                    auto camp = lucy.right_camp();
                    camp.set_hit_points(static_cast<std::size_t>(std::stol(argument)));
                    lucy.set_right_camp(camp);
                }
                std::cout << "Right camp hit points: " << lucy.right_camp().hit_points() << std::endl;
                break;
            case command_name::left_skills:
                std::cout << "Left skills:" << std::endl;
                lucy.print_left_skills();
                break;
            case command_name::right_skills:
                std::cout << "Right skills:" << std::endl;
                lucy.print_right_skills();
                break;
            case command_name::n:
                if (!argument.empty()) lucy.set_simulation_count(static_cast<std::size_t>(std::stol(argument)));
                std::cout << "Number of simulations: " << lucy.simulation_count() << std::endl;
                break;
            case command_name::log:
                lucy.run(true);
                break;
            case command_name::run:
                lucy.run(false);
                break;
            default:
                std::cout << "Command \"" << command << "\" not recognized. Try \"help\" to get a list of avaiable commands, or \"quit\" to quit." << std::endl;
                break;
        }
    }
    return 0;
} // main(...)
