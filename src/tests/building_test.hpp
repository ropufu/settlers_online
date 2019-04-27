
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_BUILDING_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_BUILDING_TEST_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include "../settlers_online/char_string.hpp"
#include "../settlers_online/economy/building.hpp"
#include "../settlers_online/economy/dimension.hpp"
#include "../settlers_online/economy/blueprint.hpp"

#include <cmath>   // std::round
#include <cstddef> // std::size_t
#include <chrono>  // std::chrono::seconds, std::literals::chrono_literals
#include <system_error> // std::error_code
#include <vector> // std::vector

namespace ropufu::settlers_online_test
{
    struct building_test
    {
        using type = building_test;
        using tested_type = settlers_online::building;

    public:
        static bool test_simple_layout() noexcept
        {
            std::error_code ec {};

            settlers_online::dimension d1 {{1, 1}};
            settlers_online::dimension d2 {{2, 2}};
            settlers_online::dimension d3 {{3, 2}};

            tested_type empty {};
            tested_type one {"One", d1, ec};
            tested_type two {"Two", d2, ec};
            tested_type three {"Pi", d3, ec};

            if (ec.value() != 0) return false;

            return true;
        } // test_simple_layout(...)

        static bool test_dimension_json() noexcept
        {
            std::error_code ec {};

            // o---o---o
            // | # |   |
            // o---o-#-o
            // |   |   |
            // o---o---o
            // |   |   |
            // o---#---o
            settlers_online::dimension d1 {{3, 2}};
            settlers_online::dimension d2 {{2, 3}, {0, 0}, {1, 1}, ec};
            settlers_online::dimension d3 {{2, 3}, {0, 0}, {1, 1}, ec};
            d3.set_anchor({2, 1}, ec);
            if (ec.value() != 0) return false;

            nlohmann::json j1 = d1;
            nlohmann::json j2 = d2;
            nlohmann::json j3 = d3;

            settlers_online::dimension h1 = j1;
            settlers_online::dimension h2 = j2;
            settlers_online::dimension h3 = j3;
            
            if (d1 != h1) return false;
            if (d2 != h2) return false;
            if (d3 != h3) return false;

            return true;
        } // test_building_json(...)

        static bool test_building_json() noexcept
        {
            std::error_code ec {};

            // o---o---o
            // | # |   |
            // o---o-#-o
            // |   |   |
            // o---o---o
            // |   |   |
            // o---#---o
            settlers_online::dimension d {{3, 2}};
            settlers_online::footprint b {d};
            b.cells()(1, 1) = true; // Top left face.
            b.cells()(2, 3) = true; // Second from the top, right horizontal edge.
            b.cells()(6, 2) = true; // Bottom row, middle vertex.

            tested_type one {"Roundtrip", d, b, ec};
            if (ec.value() != 0) return false;

            nlohmann::json j = one;
            tested_type two = j;
            nlohmann::json k = two;
            if (j != k) return false;

            return true;
        } // test_building_json(...)

        static bool test_bad_layout() noexcept
        {
            std::error_code ec {};

            settlers_online::dimension d1 {{2, 4}};
            settlers_online::dimension d2 {{2, 2}};
            settlers_online::dimension d3 {{3, 2}};

            settlers_online::footprint b1 {4, 2};
            settlers_online::footprint b2 {3, 2};
            settlers_online::footprint b3 {3, 1};

            tested_type one {"One", d1, b1, ec};
            if (ec.value() == 0) return false;
            ec.clear();

            tested_type two {"Two", d2, b2, ec};
            if (ec.value() == 0) return false;
            ec.clear();

            tested_type three {"Three", d3, b3, ec};
            if (ec.value() == 0) return false;
            ec.clear();

            return true;
        } // test_simple_layout(...)
    }; // struct building_test
} // namespace ropufu::settlers_online_test

#endif // ROPUFU_SETTLERS_ONLINE_TEST_BUILDING_TEST_HPP_INCLUDED
