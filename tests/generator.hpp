
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_GENERATOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_GENERATOR_HPP_INCLUDED

#include "../settlers_online/army.hpp"
#include "../settlers_online/special_abilities.hpp"
#include "../settlers_online/unit_category.hpp"
#include "../settlers_online/unit_group.hpp"
#include "../settlers_online/unit_type.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ropufu
{
	namespace settlers_online_test
	{
        /** @brief Class for generating units, groups, and armies.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
		struct generator
		{
			using type = generator;
			using cat = settlers_online::unit_category;
			using sa = settlers_online::special_abilities;

		private:
			std::int_fast32_t m_seed = 0;

        protected:
            generator() noexcept { }
            ~generator() noexcept { }

        public:
            /** The only instance of this type. */
            static type& instance()
            {
                // Since it's a static variable, if the class has already been created, it won't be created again.
                // Note: it is thread-safe in C++11.
                static type s_instance;
                // Return a reference to our instance.
                return s_instance;
            }

            // ~~ Delete copy and move constructors and assign operators ~~
            generator(const type&) = delete; // Copy constructor.
            generator(type&&)      = delete; // Move constructor.
            type& operator =(const type&) = delete; // Copy assign.
			type& operator =(type&&)      = delete; // Move assign.
			
            settlers_online::unit_type next_type(const std::string& name, std::size_t hit_points)
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

			settlers_online::unit_group next_group(std::size_t count, const std::string& name, std::int_fast32_t metagroup_id)
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

			settlers_online::army next_army(std::size_t capacity)
			{
				std::size_t count_groups = 4 + ((++this->m_seed)  % 4);
				std::size_t group_size = capacity / count_groups;

				std::vector<settlers_online::unit_group> groups(0);
				groups.reserve(count_groups);
				char name = 'a';
				for (std::size_t i = 0; i < count_groups; i++) 
				{
					if (capacity == 0) break;
					std::size_t count = i + group_size;
					if (i == count_groups - 1 || count > capacity) count = capacity;
					capacity -= count;

					groups.push_back(this->next_group(count, std::string({ name }), (i % 3)));
					name = static_cast<char>(static_cast<std::size_t>(name) + 1);
				}
				groups.shrink_to_fit();
				return settlers_online::army(groups);
			}
		};
	}
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_GENERATOR_HPP_INCLUDED
