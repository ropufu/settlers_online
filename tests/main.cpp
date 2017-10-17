
#include "army_test.hpp"
#include "combat_mechanics_test.hpp"
#include "unit_group_test.hpp"
#include "unit_type_test.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>

#include <experimental/filesystem>
#include <nlohmann/json.hpp>

using army_test = ropufu::settlers_online_test::army_test;
using combat_mechanics_test = ropufu::settlers_online_test::combat_mechanics_test;
using unit_type_test = ropufu::settlers_online_test::unit_type_test;
using unit_group_test = ropufu::settlers_online_test::unit_group_test;

namespace fs = std::experimental::filesystem;

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

/** Load units from .json files. */
void load(const std::string& folder_path = "./../maps/") noexcept
{
    for (const fs::directory_entry& p : fs::directory_iterator(folder_path))
    {
        std::ifstream i(p.path()); // Try to open the file for reading.
        if (!i.good()) continue; // Stop on failure.

        try
        {
            nlohmann::json map;// = nlohmann::json::parse(i);
			i >> map;
			if (map.count("units") != 0)
			{
				for (const nlohmann::json& unit : map["units"])
				{
					ropufu::settlers_online::unit_type u = unit;
					//std::string unit_names = unit.at("names");
					std::cout << u.names().front() << std::endl;
				}
			}
        }
        catch (const std::exception& e)
        {
            std::cout << "Failed while reading " << p.path() << ": " << e.what() << std::endl;
            continue;
        }
        catch (...)
        {
            std::cout << "Oh no!.." << std::endl;
            continue;
        }
    }
}

std::int32_t main()
{
    load();
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

    return 0;
}
