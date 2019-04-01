
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_UNIT_GROUP_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_UNIT_GROUP_TEST_HPP_INCLUDED

#include "../settlers_online/combat/unit_group.hpp"
#include "../settlers_online/combat/unit_type.hpp"

#include "generator.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>
#include <system_error> // std::error_code, std::errc

namespace ropufu
{
    namespace settlers_online_test
    {
        struct unit_group_test
        {
            using type = unit_group_test;
            using tested_type = settlers_online::unit_group;

            static bool test_equality()
            {
                generator gen {};
                std::error_code ec {};

                // ~~ Constructor ~~
                std::size_t count = 27;
                settlers_online::unit_type u = gen.next_type("big dummy", 1985);
                settlers_online::unit_type v = gen.next_type("big dummy", 1985);
                tested_type g1(u, count, 4);
                tested_type g2(u, count + 1, 4);
                tested_type g3(u, count, 5);
                tested_type g4(v, count, 4);

                if (g1 == g2) return false;
                if (g2 == g3) return false;
                if (g3 == g4) return false;
                if (g4 == g1) return false;

                g2.kill_top_defender(); // Now the number of units in two groups is the same.
                g2.snapshot(); // Take a snapshot.
                if (g1 != g2) return false; // Make sure the two groups are equal.

                g2.kill_all_defender();
                g2.reset_count_defender(g1.count_as_defender()); // Revert to the case when they were equal.
                if (g1 != g2) return false;

                return !static_cast<bool>(ec);
            }

            static bool test_properties()
            {
                generator gen {};
                std::error_code ec {};

                // ~~ Constructor ~~
                std::size_t count = 27;
                std::int_fast32_t metagroup_id = 4;
                settlers_online::unit_type u = gen.next_type("no name, nn", 1985);
                tested_type g(u, count, metagroup_id);
                std::size_t total_hit_popints = g.total_hit_points_defender();

                // ~~ Test Getters ~~
                if (!type::is_match(g, u, count, total_hit_popints, metagroup_id)) return false;
                
                // ~~ Test Damage ~~
                std::size_t damage_full = u.hit_points();
                std::size_t damage_overflow = 2 * damage_full;
                std::size_t damage_half = damage_full / 2;
                

                g.damage_no_splash(damage_half); // Count should still be <count>.
                if (!type::is_match(g, u, count, total_hit_popints - damage_half, metagroup_id)) return false;

                g.damage_no_splash(damage_full); // Now count should be <count - 1>, and no damage should be present.
                if (!type::is_match(g, u, count - 1, total_hit_popints - damage_full, metagroup_id)) return false;

                g.damage_no_splash(damage_overflow); // Now ount should be <count - 2>, and no damage should be present.
                if (!type::is_match(g, u, count - 2, total_hit_popints - 2 * damage_full, metagroup_id)) return false;

                g.damage_no_splash(damage_half);
                g.reset_count_defender(count); // Back to where we started.
                if (!type::is_match(g, u, count, total_hit_popints, metagroup_id)) return false;

                g.kill_top_defender(); // Now count should be <count - 1>, and no damage should be present.
                if (!type::is_match(g, u, count - 1, total_hit_popints - damage_full, metagroup_id)) return false;

                g.damage_no_splash(damage_half);
                g.kill_top_defender(); // Now count should be <count - 2>, and no damage should be present.
                if (!type::is_match(g, u, count - 2, total_hit_popints - 2 * damage_full, metagroup_id)) return false;

                g.kill_all_defender();
                if (!type::is_match(g, u, 0, 0, metagroup_id)) return false;

                return !static_cast<bool>(ec);
            }

        private:
            static bool is_match(const tested_type& g,
                const settlers_online::unit_type& u, std::size_t count, std::size_t total_hit_points, std::int_fast32_t metagroup_id)
            {
                if (g.unit() != u) return false;
                if (g.count_as_defender() != count) return false;
                if (g.total_hit_points_defender() != total_hit_points) return false;
                if (g.metagroup_id() != metagroup_id) return false;
                
                return true;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_UNIT_GROUP_TEST_HPP_INCLUDED
