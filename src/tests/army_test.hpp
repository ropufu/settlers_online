
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_ARMY_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_ARMY_TEST_HPP_INCLUDED

#include "../settlers_online/combat/army.hpp"
#include "../settlers_online/combat/unit_group.hpp"
#include "../settlers_online/combat/unit_type.hpp"

#include "generator.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

namespace ropufu
{
    namespace settlers_online_test
    {
        struct army_test
        {
            using type = army_test;
            using tested_type = settlers_online::army;

            static bool test_equality()
            {
                generator gen {};

                settlers_online::unit_group g1 = gen.next_group(12, "a", 1);
                settlers_online::unit_group g2 = gen.next_group(100, "b", 2);
                settlers_online::unit_group g3 = gen.next_group(8, "c", 1);
                settlers_online::unit_group g3x = gen.next_group(8, "c", 2);

                // ~~ Constructor ~~
                std::vector<settlers_online::unit_group> groups_a = { g1, g2, g3 };
                std::vector<settlers_online::unit_group> groups_b = { g1, g3, g2 };
                std::vector<settlers_online::unit_group> groups_x = { g3x, g2, g1 }; // Different metagroup mask.

                settlers_online::camp defence;

                std::error_code ec {};
                tested_type a1(groups_a, defence, ec);
                tested_type a2(groups_b, defence, ec);
                tested_type a3({ g3, g1, g2 }, defence, ec);
                tested_type ax(groups_x, defence, ec);

                if (ec) return false;

                if (a1 != a2) return false; // Permutations should not affect armies.
                if (a2 != a3) return false; // Permutations should not affect armies.
                if (a3 != a1) return false; // Permutations should not affect armies.
                if (a1 == ax) return false; // Different metagroup mask.

                // // Check for original  orderingmatching.
                // for (std::size_t i = 0; i < groups_b.size(); ++i) if (groups_b[i] != a2[a2.order_original()[i]]) return false;

                // // Make sure it is different for different armies.
                // bool is_matched = true;
                // for (std::size_t i = 0; i < groups_a.size(); ++i) if (groups_a[i] != a2[a2.order_original()[i]]) is_matched = false;
                // if (is_matched) return false;
                // is_matched = true;
                // for (std::size_t i = 0; i < groups_b.size(); ++i) if (groups_b[i] != a1[a1.order_original()[i]]) is_matched = false;
                // if (is_matched) return false;

                return true;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_ARMY_TEST_HPP_INCLUDED
