
#include <aftermath/not_an_error.hpp>

#include "army_test.hpp"
#include "combat_mechanics_test.hpp"
#include "unit_group_test.hpp"
#include "unit_type_test.hpp"

#include <chrono> // std::chrono::steady_clock, std::chrono::duration_cast
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <iostream> // std::cout, std::endl

using army_test = ropufu::settlers_online_test::army_test;
using combat_mechanics_test = ropufu::settlers_online_test::combat_mechanics_test;
using unit_type_test = ropufu::settlers_online_test::unit_type_test;
using unit_group_test = ropufu::settlers_online_test::unit_group_test;

using quiet_error = ropufu::aftermath::quiet_error;

template <typename t_test_type>
bool run_test(t_test_type test, const std::string& name)
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

std::int32_t main()
{
    quiet_error& err = quiet_error::instance();

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
        std::cout << '\t' <<
            " level " << static_cast<std::size_t>(desc.severity()) <<
            " error # " << static_cast<std::size_t>(desc.error_code()) <<
            " on line " << desc.caller_line_number() <<
            " of <" << desc.caller_function_name() << ">:\t" << desc.description() << std::endl;
    }

    return 0;
}
