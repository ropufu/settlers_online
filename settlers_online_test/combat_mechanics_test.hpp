
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED

#include "../settlers_online/army.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/special_abilities.hpp"
#include "../settlers_online/unit_category.hpp"
#include "../settlers_online/unit_group.hpp"
#include "../settlers_online/unit_type.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace ropufu
{
	namespace settlers_online_test
	{
		struct combat_mechanics_test
		{
			typedef combat_mechanics_test type;
			typedef settlers_online::combat_mechanics tested_type;

			using cat = settlers_online::unit_category;
			using sa = settlers_online::special_abilities;

			static bool test_equality()
			{
				type tester = {};

                settlers_online::unit_group g1 = tester.generate_group(12, "a", 1);
                settlers_online::unit_group g2 = tester.generate_group(100, "b", 2);
                settlers_online::unit_group g3 = tester.generate_group(8, "c", 1);
                settlers_online::unit_group g3x = tester.generate_group(8, "c", 2);

				// ~~ Constructor ~~
				std::vector<settlers_online::unit_group> groups_a = { g1, g2, g3 };
                std::vector<settlers_online::unit_group> groups_b = { g1, g3, g2 };
                std::vector<settlers_online::unit_group> groups_x = { g3x, g2, g1 }; // Different metagroup mask.

				tested_type a1(groups_a);
                tested_type a2(groups_b);
                tested_type a3({ g3, g1, g2 });
                tested_type ax(groups_x);

                if (a1 != a2) return false; // Permutations should not affect armies.
                if (a2 != a3) return false; // Permutations should not affect armies.
                if (a3 != a1) return false; // Permutations should not affect armies.
                if (a1 == ax) return false; // Different metagroup mask.

                // Check for original ordering matching.
                for (std::size_t i = 0; i < groups_b.size(); i++) if (groups_b[i] != a2.in_original(i)) return false;

                // Make sure it is different for different armies.
                bool is_matched = true;
                for (std::size_t i = 0; i < groups_a.size(); i++) if (groups_a[i] != a2.in_original(i)) is_matched = false;
                if (is_matched) return false;
                is_matched = true;
                for (std::size_t i = 0; i < groups_b.size(); i++) if (groups_b[i] != a1.in_original(i)) is_matched = false;
                if (is_matched) return false;

				return true;
			}

		private:
			static bool is_match(const tested_type& g,
				const settlers_online::unit_type& u)
			{
				return true;
			}

			std::int_fast32_t m_seed = 0;

			settlers_online::unit_group generate_group(std::size_t count, std::string name, std::int_fast32_t metagroup_id)
			{
				std::int_fast32_t id = (++this->m_seed);
				std::int_fast32_t initiative = (this->m_seed  % 3);
				std::size_t hit_points = 1985;
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
				return settlers_online::unit_group(u, count);
			}
		};
	}
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
