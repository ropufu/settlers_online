
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_UNIT_TYPE_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_UNIT_TYPE_TEST_HPP_INCLUDED

#include "../settlers_online/enums.hpp"
#include "../settlers_online/combat/unit_type.hpp"

#include "generator.hpp"

#include <cstdint>
#include <functional>
#include <ostream>
#include <string>
#include <system_error> // std::error_code, std::errc

namespace ropufu
{
    namespace settlers_online_test
    {
        struct unit_type_test
        {
            using type = unit_type_test;
            using tested_type = settlers_online::unit_type;

            static bool test_properties()
            {
                return true;
            }

            static bool test_equality()
            {
                generator gen {};
                std::error_code ec {};

                std::string name = "no name, nn";
                std::size_t hit_points = 1985;
                tested_type u1 = gen.next_type(name, hit_points);
                tested_type u2 = u1;
                u2.set_names({ "baka", "dummy" });
                if (u1 != u2) return false;

                std::size_t low_damage = u1.damage({}).low();
                std::size_t high_damage = u1.damage({}).high();
                double accuracy = u1.damage({}).accuracy();
                double splash_chance = u1.damage({}).splash_chance();
                splash_chance = (splash_chance == 0) ? 0.5 : (splash_chance / 2);

                u2.set_damage(settlers_online::damage(low_damage, high_damage, accuracy, splash_chance, ec), ec);
                if (u1 == u2) return false;

                return !static_cast<bool>(ec);
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_UNIT_TYPE_TEST_HPP_INCLUDED
