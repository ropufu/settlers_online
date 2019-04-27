
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_ISLAND_MAP_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_ISLAND_MAP_TEST_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include "../settlers_online/char_string.hpp"
#include "../settlers_online/economy/building.hpp"
#include "../settlers_online/economy/dimension.hpp"
#include "../settlers_online/economy/blueprint.hpp"
#include "../settlers_online/economy/island_map.hpp"

#include <cmath>   // std::round
#include <cstddef> // std::size_t
#include <chrono>  // std::chrono::seconds, std::literals::chrono_literals
#include <system_error> // std::error_code
#include <vector> // std::vector

namespace ropufu::settlers_online_test
{
    struct island_map_test
    {
        using type = island_map_test;
        using building = settlers_online::building;
        using vertex_index = settlers_online::vertex_index;
        using tested_type = settlers_online::island_map;

    private:
        /**
         *   o-o-o-o-o-o-o-o-o-o-o
         *   | | | | | | | | | | |
         *   o-o-o-o-o-o-o-o-o-o-o
         *   | | | |###| | | | | |
         *   o-o-o-o###o-o-o-o-o-o
         *   | | | |###| | |###| |
         *   o-o-o-o-#-o-o-o###o-o
         *   | | | |#|#| | |###| |
         *   o-o-o-o-o-o-o-o-o-o-o
         *   | | | | | | | | | | |
         *   o-o-o-o-o-o-o-o-o-o-o
         */
        static tested_type map_1() noexcept
        {
            std::error_code ec {};

            settlers_online::dimension d2 {{2, 2}};
            settlers_online::dimension d3 {{3, 2}};
            settlers_online::footprint l3 {d3};
            for (std::size_t i = 0; i < l3.cells().height(); ++i)
            {
                for (std::size_t j = 0; j < l3.cells().width(); ++j)
                {
                    bool is_blocked = true;
                    if (i == 0 || i == l3.cells().height() - 1) is_blocked = false;
                    if (j == 0 || j == l3.cells().width() - 1) is_blocked = false;
                    if (i == 4 && (j == 1 || j == 3)) is_blocked = false; // Make sideways entrance edges walkable.
                    if (i == 5 && j == 2) is_blocked = false; // Make verticale entrance edge walkable.
                    l3.cells()(i, j) = is_blocked;
                } // for (...)
            } // for (...)
            d3.set_entrance({2, 1}, ec);

            building two {"Two", d2, ec};
            building three {"Improved Storehouse", d3, l3, ec};

            // Corner positions.
            vertex_index two_good {2, 7};
            vertex_index three_good {1, 3};
            // Offset by anchors.
            two_good.offset(two.dimensions().anchor());
            three_good.offset(three.dimensions().anchor());

            tested_type island {5, 10};
            island.build(three, three_good, ec);
            island.build(two, two_good, ec);
            if (ec.value() != 0) return {};

            return island;
        } // map_1(...)

    public:
        static bool test_island_map_1() noexcept
        {
            std::error_code ec {};

            settlers_online::dimension d1 {{1, 1}};
            settlers_online::dimension d2 {{2, 2}};
            settlers_online::dimension d3 {{3, 2}};

            building empty {};
            building one {"One", d1, ec};
            building two {"Two", d2, ec};
            building three {"Pi", d3, ec};

            if (ec.value() != 0) return false;

            // Corner positions.
            vertex_index three_bad {3, 1};
            vertex_index three_good {1, 3};
            vertex_index two_bad {0, 9};
            // Offset by anchors.
            three_bad.offset(three.dimensions().anchor());
            three_good.offset(three.dimensions().anchor());
            two_bad.offset(two.dimensions().anchor());

            tested_type island {5, 10};
            if (island.can_be_built(three, three_bad)) return false; // Row overflow.
            if (!island.can_be_built(three, three_good)) return false; // Just fine.
            if (island.can_be_built(two, two_bad)) return false; // Column overflow.

            island.build(three, three_bad, ec); // Should not be allowed.
            if (ec.value() == 0) return false;
            ec.clear();

            island.build(three, three_good, ec); // Okay.
            if (ec.value() != 0) return false;

            island.build(two, three_good, ec); // Should not be allowed: it's allready occupied by "three".
            if (ec.value() == 0) return false;
            ec.clear();

            return true;
        } // test_island_map_1(...)

        /**
         *   o-o-o-o-o-o-o-o-o-o-o
         *   | | | | | | | | | | |
         *   o-o-o-o-o-o-o-o-o-o-o
         *   | | | |###| | | | | |
         *   o-o-o-o###o-o-o-o-o-o
         *   | | | |###| | |###| |
         *   o-o-o-x-*-x-o-o###x-o
         *   | | | |#|#| | |###| |
         *   o-o-o-x-x-x-x-x-x-x-o
         *   | | | | | | | | | | |
         *   o-o-o-o-o-o-o-o-o-o-o
         */
        static bool test_island_map_1_pathfinder() noexcept
        {
            std::error_code ec {};
            tested_type island = type::map_1(); // Map of two buildings, with a single one-way-only path between the two.
            // std::cout << island.layout() << std::endl;
            std::vector<vertex_index> reference_path = {
                {3, 4}, {3, 5}, {3, 6}, {3, 7}, {4, 7}, {4, 8}
            };

            for (const auto& from : island.buildings())
            {
                for (const auto& to : island.buildings())
                {
                    if (&from == &to) continue;
                    std::vector<vertex_index> path = island.path(from, to, ec);
                    if (ec.value() == 0) // This is the walkable path.
                    {
                        if (path != reference_path) return false;
                    } // if (...)
                    ec.clear(); // This was the impossible path (entrance to the storehouse is not otself walkable).
                } // for (...)
            } // for (...)

            if (ec.value() != 0) return false;

            return true;
        } // test_island_map_pathfinder(...)
    }; // struct island_map_test
} // namespace ropufu::settlers_online_test

#endif // ROPUFU_SETTLERS_ONLINE_TEST_ISLAND_MAP_TEST_HPP_INCLUDED
