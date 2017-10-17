
#include <aftermath/not_an_error.hpp>

#include "../settlers_online/unit_database.hpp"
#include "../settlers_online/army_parser.hpp"

#include "army_test.hpp"
#include "combat_mechanics_test.hpp"
#include "unit_group_test.hpp"
#include "unit_type_test.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <exception>
#include <iostream>
#include <string>

using army_test = ropufu::settlers_online_test::army_test;
using combat_mechanics_test = ropufu::settlers_online_test::combat_mechanics_test;
using unit_type_test = ropufu::settlers_online_test::unit_type_test;
using unit_group_test = ropufu::settlers_online_test::unit_group_test;

using unit_database = ropufu::settlers_online::unit_database;
using quiet_error = ropufu::aftermath::quiet_error;

template <typename t_test>
bool run_test(t_test test, std::string name)
{
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    bool result = test();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    double elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1'000.0;

    std::cout
        << (result ? "test passed: " : "test failed: ")
        << name << ". Elapsed time: "
        << elapsed_seconds << "s." << std::endl;

    return result;
}

std::int32_t main(std::int32_t argc, char* argv[], char* envp[])
{
    quiet_error& err = quiet_error::instance();
    unit_database& db = unit_database::instance();

    std::cout << "Loaded " << db.load_from_folder("./../maps/") << " units." << std::endl;

    std::string army_string = (argc > 1 ? std::string(argv[1]) : "1r 32 s 12 Bowman");
    ropufu::settlers_online::army_parser parser = army_string;
    std::cout << "Parsing army \"" << army_string << "\"...";
    if (!parser.good()) std::cout << " failed." << std::endl;
    else
    {
        std::cout << " succeed: " << parser.size() << " groups acknowledged." << std::endl;
        ropufu::settlers_online::army a;
        if (!parser.try_build_fast(db, a)) std::cout << "Unit not recognized." << std::endl;
        else std::cout << "Army of " << a.groups().size() << " created." << std::endl;
    }

    //run_test([]() { return false; });
    // ~~ Unit type tests ~~
    run_test(unit_type_test::test_equality, "<unit_type> equality");
    run_test(unit_type_test::test_properties, "<unit_type> properties");
    // ~~ Unit group tests ~~
    run_test(unit_group_test::test_equality, "<unit_group> equality");
    run_test(unit_group_test::test_properties, "<unit_group> properties");
    // ~~ Army tests ~~
    run_test(army_test::test_equality, "<army> equality");
    //run_test(army_test::test_properties, "<army> properties");
    // ~~ Combat tests ~~
    run_test(combat_mechanics_test::test_deterministic, "<combat_mechanics> deterministic");

    
    if (!err.good()) std::cout << "~~ Oh no! Errors encoutered: ~~" << std::endl;
    else if (!err.empty()) std::cout << "~~ Something to keep in mind: ~~" << std::endl;
    while (!err.empty())
    {
        ropufu::aftermath::quiet_error_descriptor desc = err.pop();
        if (desc.severity() == ropufu::aftermath::severity_level::not_at_all) continue; // Skip information messages.
        std::cout << '\t' <<
            " level " << static_cast<std::size_t>(desc.severity()) <<
            " error # " << static_cast<std::size_t>(desc.error_code()) <<
            " on line " << desc.caller_line_number() <<
            " of <" << desc.caller_function_name() << ">:\t" << desc.description() << std::endl;
    }

    return 0;
}
