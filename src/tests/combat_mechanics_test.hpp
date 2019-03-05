
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED

#include "../settlers_online/combat/army.hpp"
#include "../settlers_online/combat/combat_result.hpp"
#include "../settlers_online/combat/battle.hpp"
#include "../settlers_online/combat/trivial_attack_sequence.hpp"
#include "../settlers_online/combat/randomized_attack_sequence.hpp"
#include "../settlers_online/combat/unit_group.hpp"
#include "../settlers_online/combat/unit_type.hpp"
#include "../settlers_online/logger.hpp"

#include "generator.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <random>
#include <string>
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online_test
{
    struct combat_mechanics_test
    {
        using type = combat_mechanics_test;
        using engine_type = std::mt19937;
        using sequencer_type_a = settlers_online::trivial_attack_sequence<true>;
        using sequencer_type_b = settlers_online::trivial_attack_sequence<false>;
        using sequencer_type_c = settlers_online::randomized_attack_sequence<engine_type>;
        using tested_type_1 = settlers_online::battle<sequencer_type_a, sequencer_type_b>;
        using tested_type_2 = settlers_online::battle<sequencer_type_c, sequencer_type_c>;
        
        using bw = settlers_online::battle_weather;
        using weather_array = ropufu::aftermath::enum_array<bw, void>;

        static bool test_deterministic()
        {
            generator gen {};
            std::error_code ec {};
            settlers_online::detail::no_logger logger {};

            settlers_online::army left_army = gen.next_army(250);
            settlers_online::army right_army = gen.next_army(270);

            weather_array www {};
            for (bw weather : www)
            {
                tested_type_1 combat(left_army, right_army, weather, ec);
                
                // std::size_t battle_rounds = 
                    combat.execute(logger, ec);
                std::size_t destruction_rounds = 0;
                for (std::size_t i = 0; i < 100; ++i) destruction_rounds += combat.peek_destruction(ec);
                destruction_rounds /= 100;

                if (static_cast<bool>(ec)) return false;
            } // for (...)

            return true;
        } // test_deterministic(...)

        static bool test_randomized()
        {
            generator gen {};
            std::error_code ec {};
            settlers_online::detail::no_logger logger {};

            settlers_online::army left_army = gen.next_army(250);
            settlers_online::army right_army = gen.next_army(270);

            weather_array www {};
            for (bw weather : www)
            {
                tested_type_2 combat(left_army, right_army, weather, ec);
                
                // std::size_t battle_rounds = 
                    combat.execute(logger, ec);
                std::size_t destruction_rounds = 0;
                for (std::size_t i = 0; i < 100; ++i) destruction_rounds += combat.peek_destruction(ec);
                destruction_rounds /= 100;

                if (static_cast<bool>(ec)) return false;
            } // for (...)

            return true;
        } // test_randomized(...)
    }; // struct combat_mechanics_test
} // namespace ropufu::settlers_online_test

#endif // ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
