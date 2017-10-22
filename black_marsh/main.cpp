
#include <aftermath/not_an_error.hpp>

#include "turtle.hpp"
#include "../settlers_online/char_string.hpp"
#include "../settlers_online/unit_faction.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <iostream> // std::cout, std::endl, std::cin
#include <string> // std::string, std::to_string, std::getline, std::stoi

// ~~ Singleton types ~~
using unit_faction = ropufu::settlers_online::unit_faction;
using unit_database = ropufu::settlers_online::unit_database;
using char_string = ropufu::settlers_online::char_string;
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

enum struct command_name
{
    not_recognized,
    quit, // Exit the program.
    help, // Display help.
    units, // Lists all units.
    left, // Left army.
    right, // Right army.
    n, // Number of sumulations.
    log, // Run simulation in log mode.
    run // Run simulation in regular mode.
};

command_name parse_command(const std::string& command, std::string& argument)
{
    std::string key = char_string::deep_trim_copy(command);
    argument = "";

    std::size_t index_of_space = key.find(" ");
    if (index_of_space != std::string::npos) 
    {
        argument = key.substr(index_of_space + 1);
        key = key.substr(0, index_of_space);
    }

    if (key == "quit" || key == "q" || key == "exit") return command_name::quit;
    if (key == "help" || key == "h" || key == "?") return command_name::help;
    if (key == "units" || key == "unit" || key == "u") return command_name::units;
    if (key == "left" || key == "l") return command_name::left;
    if (key == "right" || key == "r") return command_name::right;
    if (key == "n") return command_name::n;
    if (key == "log") return command_name::log;
    if (key == "run") return command_name::run;

    return command_name::not_recognized;
}

void help(const std::string& argument)
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
}

void display_units(const std::string& faction_name)
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
}

std::int32_t quit()
{
    std::cout << "Buh-bye ^^//~~" << std::endl;
    return 0;
}

std::string read_line()
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}

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
                if (!(lucy.left().empty() || lucy.left().has(unit_faction::general))) std::cout << "Warning! Left army does not have any generals. Type \"units general\" for suggestions." << std::endl;
                break;
            case command_name::right:
                if (!argument.empty()) lucy.parse_right(argument);
                std::cout << "Right army: " << lucy.right() << std::endl;
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
}
