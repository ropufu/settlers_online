
#ifndef SETTLERS_ONLINE_TEST_UNIT_GROUP_TEST_HPP_INCLUDED
#define SETTLERS_ONLINE_TEST_UNIT_GROUP_TEST_HPP_INCLUDED

#include <settlers_online/special_abilities.hpp>
#include <settlers_online/unit_category.hpp>
#include <settlers_online/unit_group.hpp>
#include <settlers_online/unit_type.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace ropufu
{
	namespace settlers_online_test
	{
		struct unit_group_test
		{
			typedef unit_group_test type;
			typedef settlers_online::unit_group tested_type;

			using cat = settlers_online::unit_category;
			using sa = settlers_online::special_abilities;

            static bool test_equality()
            {
                type tester = {};

                // ~~ Constructor ~~
                std::size_t count = 27;
                settlers_online::unit_type u = tester.generate_type("big dummy", 1985);
                settlers_online::unit_type v = tester.generate_type("big dummy", 1985);
                tested_type g1(u, count, 4);
                tested_type g2(u, count + 1, 4);
                tested_type g3(u, count, 5);
                tested_type g4(v, count, 4);

                if (g1 == g2) return false;
                if (g2 == g3) return false;
                if (g3 == g4) return false;
                if (g4 == g1) return false;

                g2.kill_top(); // Now the number of units in two groups is the same.
                g2.snapshot(); // Take a snapshot.
                if (g1 != g2) return false; // Make sure the two groups are equal.

                g2.kill_all();
                g2.reset(); // Revert to the case  when they were equal.
                if (g1 != g2) return false;

                return true;
            }

			static bool test_properties()
			{
                type tester = {};

				// ~~ Constructor ~~
				std::size_t count = 27;
				std::int_fast32_t metagroup_id = 4;
				settlers_online::unit_type u = tester.generate_type("no name, nn", 1985);
				tested_type g(u, count, metagroup_id);

				// ~~ Test Getters ~~
				if (!type::is_match(g, u, count, 0, metagroup_id)) return false;
				
				// ~~ Test Damage ~~
				std::size_t damage_full = u.hit_points();
				std::size_t damage_overflow = 2 * damage_full;
				std::size_t damage_half = damage_full / 2;
				
				g.try_kill_top(damage_half); // Now count should stay the same.
				if (!type::is_match(g, u, count, damage_half, metagroup_id)) return false;
				g.try_kill_top(damage_full); // Now count should be <count - 1>, and no damage should be present.
				if (!type::is_match(g, u, count - 1, 0, metagroup_id)) return false;
				g.try_kill_top(damage_overflow); // Now ount should be <count - 2>, and no damage should be present.
				if (!type::is_match(g, u, count - 2, 0, metagroup_id)) return false;
				g.try_kill_top(damage_half);
				g.reset(); // Back to where we started.
				if (!type::is_match(g, u, count, 0, metagroup_id)) return false;
				g.kill_top(); // Now count should be <count - 1>, and no damage should be present.
				if (!type::is_match(g, u, count - 1, 0, metagroup_id)) return false;
				g.try_kill_top(damage_half);
				g.kill(2); // Now count should be <count - 3>, and no damage should be present.
				if (!type::is_match(g, u, count - 3, 0, metagroup_id)) return false;
				g.kill_all();
				if (!type::is_match(g, u, 0, 0, metagroup_id)) return false;

				return true;
			}

		private:
			static bool is_match(const tested_type& g,
				const settlers_online::unit_type& u, std::size_t count, std::size_t damage_taken, std::int_fast32_t metagroup_id)
			{
				if (g.type() != u) return false;
				if (g.count() != count) return false;
				if (g.damage_taken() != damage_taken) return false;
				if (g.metagroup_id() != metagroup_id) return false;
				
				return true;
			}

            std::int_fast32_t m_seed = 0;

            settlers_online::unit_type generate_type(std::string name, std::size_t hit_points)
            {
                std::int_fast32_t id = (++this->m_seed);
                std::int_fast32_t initiative = (this->m_seed % 3);
                //std::size_t hit_points = 1985;
                std::size_t experience = 2017;
                std::size_t capacity = 270;
                std::size_t min_damage = 10;
                std::size_t max_damage = 20;
                double accuracy = 0.9;
                double splash_chance = 0.5;
                cat category = cat::artillery;
                flags_type<sa> abilities = { sa::attack_twice };

                settlers_online::unit_type u(
                    id, initiative,
                    name, hit_points, experience, capacity,
                    min_damage, max_damage, accuracy, splash_chance,
                    category, abilities);

                return u;
            }
		};
	}
}

#endif // SETTLERS_ONLINE_TEST_UNIT_GROUP_TEST_HPP_INCLUDED
