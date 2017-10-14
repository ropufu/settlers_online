
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED

#include "../settlers_online/army.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/special_abilities.hpp"
#include "../settlers_online/unit_category.hpp"
#include "../settlers_online/unit_group.hpp"
#include "../settlers_online/unit_type.hpp"

#include "generator.hpp"

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

			static bool test_deterministic()
			{
				generator& gen = generator::instance();

                settlers_online::army left_army = gen.next_army(250);
                settlers_online::army right_army = gen.next_army(270);

				tested_type combat(left_army, right_army);
				return true;
			}

		private:
			static bool is_match(const tested_type& g,
				const settlers_online::unit_type& u)
			{
				return true;
			}
		};
	}
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
