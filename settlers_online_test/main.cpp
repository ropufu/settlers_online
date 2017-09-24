
#include "army_test.hpp"
#include "unit_group_test.hpp"
#include "unit_type_test.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>

using unit_type_test = ropufu::settlers_online_test::unit_type_test;
using unit_group_test = ropufu::settlers_online_test::unit_group_test;
using army_test = ropufu::settlers_online_test::army_test;

template <typename t_test>
bool run_test(t_test test, std::string name)
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	bool result = test();

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	std::cout
		<< (result ? "test passed: " : "test failed: ")
		<< name << ". Elapsed time: "
		<< elapsed_seconds.count() << "s." << std::endl;

	return result;
}

std::int32_t main()
{
	std::cout << "Hello " << (8 * sizeof(std::uint_fast32_t)) << "-bit fast world!" << std::endl;
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

	return 0;
}
