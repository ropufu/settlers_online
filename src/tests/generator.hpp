
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_GENERATOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_GENERATOR_HPP_INCLUDED

#include "../settlers_online/enums.hpp"
#include "../settlers_online/combat/army.hpp"
#include "../settlers_online/combat/damage.hpp"
#include "../settlers_online/combat/unit_group.hpp"
#include "../settlers_online/combat/unit_type.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online_test
{
    /** @brief Class for generating units, groups, and armies. */
    struct generator
    {
        using type = generator;
        using fac = settlers_online::unit_faction;
        using cat = settlers_online::unit_category;
        using bat = settlers_online::battle_trait;
        using bah = settlers_online::battle_phase;

    private:
        static std::size_t s_seed;

    public:
        generator() noexcept { }
        
        settlers_online::unit_type next_type(const std::string& name, std::size_t hit_points)
        {
            std::size_t id = (++type::s_seed);
            bah initiative = static_cast<bah>(type::s_seed % 3);
            std::size_t experience = 2017;
            std::size_t capacity = 270;
            std::size_t min_damage = 10;
            std::size_t max_damage = 20;
            double accuracy = 0.9;
            double splash_chance = 0.5;

            std::error_code ec {};

            settlers_online::unit_type u(id, initiative, hit_points, settlers_online::damage(min_damage, max_damage, accuracy, splash_chance, ec), ec);
            u.set_names({ name });
            u.set_faction(fac::non_player_adventure);
            u.set_category(cat::artillery);
            u.set_experience(experience);
            u.set_capacity(capacity);
            u.set_trait(bat::dazzle, type::s_seed % 4 == 0);
            u.set_trait(bat::intercept, type::s_seed % 5 == 0);

            return u;
        } // next_type(...)

        settlers_online::unit_group next_group(std::size_t count, const std::string& name, std::int_fast32_t metagroup_id)
        {
            settlers_online::unit_type u = this->next_type(name, 1985);
            return settlers_online::unit_group(u, count, metagroup_id);
        } // next_group(...)

        settlers_online::army next_army(std::size_t capacity)
        {
            std::size_t count_groups = 4 + ((++type::s_seed)  % 4);
            std::size_t group_size = capacity / count_groups;

            std::vector<settlers_online::unit_group> groups(0);
            groups.reserve(count_groups);
            char name = 'a';
            for (std::size_t i = 0; i < count_groups; ++i) 
            {
                if (capacity == 0) break;
                std::size_t count = i + group_size;
                if (i == count_groups - 1 || count > capacity) count = capacity;
                capacity -= count;

                groups.push_back(this->next_group(count, std::string({ name }), (i % 3)));
                name = static_cast<char>(static_cast<std::size_t>(name) + 1);
            }
            groups.shrink_to_fit();

            std::error_code ec {};
            return settlers_online::army(groups, settlers_online::camp(250), ec);
        } // next_army(...)
    }; // struct generator

    std::size_t generator::s_seed = 0;
} // ropufu::settlers_online_test

#endif // ROPUFU_SETTLERS_ONLINE_TEST_GENERATOR_HPP_INCLUDED
