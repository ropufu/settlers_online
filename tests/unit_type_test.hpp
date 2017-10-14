
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_UNIT_TYPE_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_UNIT_TYPE_TEST_HPP_INCLUDED

#include "../settlers_online/special_abilities.hpp"
#include "../settlers_online/unit_category.hpp"
#include "../settlers_online/unit_type.hpp"

#include "generator.hpp"

#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace ropufu
{
	namespace settlers_online_test
	{
		struct unit_type_test
		{
			using type = unit_type_test;
			using tested_type = settlers_online::unit_type;

			using cat = settlers_online::unit_category;
			using sa = settlers_online::special_abilities;

			static bool test_properties()
			{
				// ~~ Constructor ~~
				std::int_fast32_t id = 137;
				std::int_fast32_t initiative = 3;
				std::string name = "no name, nn";
				std::size_t hit_points = 1985;
				std::size_t experience = 2017;
                std::size_t capacity = 270;
				std::size_t min_damage = 10;
				std::size_t max_damage = 20;
				double accuracy = 0.9;
				double splash_chance = 0.5;
                bool do_attack_weakest_target = false;
                bool is_not_weak = false;
				cat category = cat::artillery;
				flags_type<sa> abilities = { sa::attack_twice };

				tested_type u(
					id, initiative,
					name, hit_points, experience, capacity,
					min_damage, max_damage, accuracy, splash_chance,
					category, abilities);

				// ~~ Test Getters ~~
				if (!type::is_match(u,
					id, initiative,
					name, hit_points, experience, capacity,
					min_damage, max_damage, accuracy, splash_chance,
                    do_attack_weakest_target, is_not_weak,
					category, abilities)) return false;

				// ~~ Update ~~
				name = "new name, m";
				experience = 4;
                capacity = 220;
				min_damage = 65535;
				max_damage = 65537;
				accuracy = 0.8;
				splash_chance = 0.95;
				abilities = { sa::ignore_tower_bonus, sa::none };
                do_attack_weakest_target = false;
                is_not_weak = true;

				u.set_name(name);
				u.set_experience(experience);
                u.set_capacity(capacity);
				u.set_damage(min_damage, max_damage, accuracy, splash_chance);
				u.set_abilities(abilities);
                u.set_do_attack_weakest_target(do_attack_weakest_target);
                u.set_is_not_weak(is_not_weak);

				// ~~ Test Setters ~~
				if (!type::is_match(u,
					id, initiative,
					name, hit_points, experience, capacity,
					min_damage, max_damage, accuracy, splash_chance,
                    do_attack_weakest_target, is_not_weak,
					category, abilities)) return false;

				return true;
			}

			static bool test_equality()
			{
				// ~~ Constructor ~~
				std::int_fast32_t id = 137;
				std::int_fast32_t initiative = 3;
				std::string name = "no name, nn";
				std::size_t hit_points = 1985;
				std::size_t experience = 2017;
                std::size_t capacity = 270;
				std::size_t min_damage = 10;
				std::size_t max_damage = 20;
				double accuracy = 0.9;
				double splash_chance = 0.5;
				cat category = cat::artillery;
				flags_type<sa> abilities = { sa::attack_twice };

				tested_type u1(
					id, initiative, 
					name, hit_points, experience, capacity,
					min_damage, max_damage, accuracy, splash_chance, 
					category, abilities);
				tested_type u2(
					id, initiative,
					name, hit_points, experience, capacity,
					min_damage, max_damage, splash_chance, accuracy, // Intentionally swapped accuracy and splash_chance.
					category, abilities);

				if (u1 == u2) return false;
				u2.set_damage(min_damage, max_damage, accuracy, splash_chance); // Revert to original setting.
				if (u1 != u2) return false;

				return true;
			}

		private:
			static bool is_match(const tested_type& u,
				std::int_fast32_t id, std::int_fast32_t initiative,
				std::string name, std::size_t hit_points, std::size_t experience, std::size_t capacity,
				std::size_t min_damage, std::size_t max_damage, double accuracy, double splash_chance,
                bool do_attack_weakest_target, bool is_not_weak,
				cat category, flags_type<sa> abilities)
			{
				if (u.id() != id)                 return false;
				if (u.initiative() != initiative) return false;
				if (u.name() != name)             return false;
				if (u.hit_points() != hit_points) return false;
				if (u.experience() != experience) return false;
                if (u.capacity() != capacity)     return false;
				if (u.min_damage() != min_damage) return false;
				if (u.max_damage() != max_damage) return false;
				if (u.accuracy() != accuracy)     return false;
				if (u.splash_chance() != splash_chance) return false;

                if (u.do_attack_weakest_target() != do_attack_weakest_target) return false;
                if (u.is_not_weak() != is_not_weak) return false;

				if (u.category() != category)   return false;
				if (u.abilities() != abilities) return false;

				return true;
			}
		};
	}
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_UNIT_TYPE_TEST_HPP_INCLUDED
