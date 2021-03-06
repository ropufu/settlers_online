
#include "config.hpp"
#include "turtle.hpp"
#include "../settlers_online/combat/army.hpp"
#include "../settlers_online/combat/unit_type.hpp"
#include "../settlers_online/io/report_entry.hpp"
#include "../settlers_online/io/unit_database.hpp"
#include "../settlers_online/char_string.hpp"
#include "../settlers_online/enums.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <exception> // std::exception
#include <iostream> // std::cout, std::endl, std::cin
#include <string> // std::string, std::to_string, std::getline, std::stol, std::stod
#include <system_error> // std::error_code, std::errc

// ~~ Type shortcuts ~~
using unit_faction = ropufu::settlers_online::unit_faction;
using unit_database = ropufu::settlers_online::unit_database;
using char_string = ropufu::settlers_online::char_string;
using battle_weather = ropufu::settlers_online::battle_weather;
using damage_percentage_type = typename ropufu::settlers_online::damage_bonus_type::percentage_type;

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
    weather,
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

    if (command == "hitpoints" || command == "hit" || command == "hp") return command_name::hit_points;
    if (command == "reduction" || command == "red" || command == "r") return command_name::damage_reduction;

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
    if (command == "weather" || command == "w") return command_name::weather;
    if (command == "n") return command_name::n;
    if (command == "log") return command_name::log;
    if (command == "run" || command == "x") return command_name::run;

    return command_name::not_recognized;
} // parse_command(...)

void help(const std::string& argument) noexcept
{
    std::string dummy { };
    ropufu::aftermath::enum_array<unit_faction, void> faction_list { };
    command_name command = parse_command(argument, dummy);
    switch (command)
    {
        case command_name::units:
            std::cout << "You can either type \"units\" to display all units, or \"units <faction name>\"," << std::endl << "where faction is one of:" << std::endl;
            for (unit_faction faction : faction_list)
            {
                std::string faction_name = std::to_string(faction);
                if (faction_name.length() > 2) std::cout << '\t' << faction_name << std::endl;
            }
            break;
        default:
        std::cout << R"?(
    Command Name        | Description
===================================================================
    quit, q, exit       | Exit the program.
    help, h, ?          | Display help.
    units, u            | Lists units from a specified faction.
    left, l             | Get or set left army.
    right, r            | Get or set right army.
    weather, w          | Get or set weather conditions.
    n                   | Gets or sets the number of simulations.
    log                 | Displays one battle report.
    run, x              | Executes the simulations.
===================================================================
Following the left / right command, one could use:
    Command Name        | Description
===================================================================
... camp, c             | Get or set camp.
... skills, s           | Get skills.
===================================================================
Following the left / right camp command, one could use:
    Command Name        | Description
===================================================================
... hitpoints, hit, hp  | Get or set camp hit points.
... reduction, red, r   | Get or set camp damage reduction.
===================================================================
Commands with get or set option will take an optional argument to set
the value of corresponding parameter.

You can also run the program with arguments:
    "left army" "string army" ([/l] | [/r]) [/w "weather"]
The first two are required (don't omit the quotation marks) and will
automatically populate the left and right armies.
One of the optional arguments can be either /l or /r. When provided,
the program will automatically execute "log" for /l, or "run" for /r,
and then quit.
You can also set the weather by using the /w argument followed by the
weather description.
)?";
            break;
    }
} // help(...)

void display_units(const ropufu::settlers_online::black_marsh::turtle& t, const std::string& faction_name) noexcept
{
    unit_faction faction = unit_faction::general;
    bool do_take_all = !ropufu::aftermath::detail::try_parse_enum(faction_name, faction);
    for (const auto& pair : t.database().data())
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
        } // if (...)
    } // for (...)
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
    ropufu::settlers_online::black_marsh::turtle lucy {}; // Default config file.
    ropufu::settlers_online::black_marsh::config& config = lucy.config();

    // Ordered arguments.
    if (argc > 2)
    {
        std::string left_army_string = std::string(argv[1]);
        std::string right_army_string = std::string(argv[2]);
        lucy.parse_left(left_army_string);
        lucy.parse_right(right_army_string);
    } // if (...)

    // Unordered arguments.
    bool has_log_flag = false;
    bool has_run_flag = false;
    for (std::int32_t i = 3; i < argc; ++i)
    {
        // <argc> is at least <i + 1>.
        std::string sw = std::string(argv[i]);
        if (sw == "-l") has_log_flag = true;
        if (sw == "-r") has_run_flag = true;
        if (sw == "-w" && argc > i + 1)
        {
            std::string sw_value = std::string(argv[i + 1]);
            battle_weather w = battle_weather::none;
            if (ropufu::aftermath::detail::try_parse_enum(sw_value, w))
            {
                lucy.set_weather(w);
            } // if (...)
        } // if (...)
    } // for (...)
    
    if (has_log_flag && has_run_flag) std::cout << "Switch conflict." << std::endl;
    else
    {
        if (has_log_flag) { lucy.log(); return 0; }
        if (has_run_flag) { lucy.run(); return 0; }
    } // else (...)

    ::welcome();
    while (true)
    {
        std::string command {};
        std::string argument {};

        //unwind_errors(false);
        lucy.logger().unwind();

        std::cout << "> ";
        command = ::read_line();

        switch (::parse_command(command, argument))
        {
            case command_name::quit: return quit();
            case command_name::help:
                ::help(argument);
                break;
            case command_name::units:
                ::display_units(lucy, argument);
                break;
            case command_name::left:
                if (!argument.empty()) lucy.parse_left(argument);
                std::cout << "Left army: ";
                ::print_elements(lucy.left_sequence(), " + ");
                std::cout << std::endl;
                break;
            case command_name::right:
                if (!argument.empty()) lucy.parse_right(argument);
                std::cout << "Right army: ";
                ::print_elements(lucy.right_sequence(), " + ");
                std::cout << std::endl;
                break;
            case command_name::weather:
                if (!argument.empty())
                {
                    battle_weather w = battle_weather::none;
                    if (ropufu::aftermath::detail::try_parse_enum(argument, w)) lucy.set_weather(w);
                    else std::cout << "Weather \"" << argument << "\" not recognized." << std::endl;
                } // if (...)
                std::cout << "Weather conditions: " << std::to_string(lucy.weather()) << std::endl;
                break;
            case command_name::left_camp:
                std::cout << "Left camp: " << config.left().camp() << std::endl;
                break;
            case command_name::right_camp:
                std::cout << "Right camp: " << config.right().camp() << std::endl;
                break;
            case command_name::left_camp_reduction:
                if (!argument.empty())
                {
                    std::error_code ec {};
                    auto camp = config.left().camp();
                    damage_percentage_type damage_reduction = damage_percentage_type::from_proportion(std::stod(argument));
                    camp.set_damage_reduction(damage_reduction, ec);
                    config.left().set_camp(camp);
                } // if (...)
                std::cout << "Left camp damage recution: " << config.left().camp().damage_reduction().to_double() << std::endl;
                break;
            case command_name::right_camp_reduction:
                if (!argument.empty())
                {
                    std::error_code ec {};
                    auto camp = config.right().camp();
                    damage_percentage_type damage_reduction = damage_percentage_type::from_proportion(std::stod(argument));
                    camp.set_damage_reduction(damage_reduction, ec);
                    config.right().set_camp(camp);
                } // if (...)
                std::cout << "Right camp damage recution: " << config.right().camp().damage_reduction().to_double() << std::endl;
                break;
            case command_name::left_camp_hit_points:
                if (!argument.empty())
                {
                    auto camp = config.left().camp();
                    camp.set_hit_points(static_cast<std::size_t>(std::stol(argument)));
                    config.left().set_camp(camp);
                } // if (...)
                std::cout << "Left camp hit points: " << config.left().camp().hit_points() << std::endl;
                break;
            case command_name::right_camp_hit_points:
                if (!argument.empty())
                {
                    auto camp = config.right().camp();
                    camp.set_hit_points(static_cast<std::size_t>(std::stol(argument)));
                    config.right().set_camp(camp);
                } // if (...)
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
        } // switch (...)
    } // while (...)
    return 0;
} // main(...)
