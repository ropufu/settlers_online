
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED

#include "../settlers_online/army.hpp"
#include "../settlers_online/combat_result.hpp"
#include "../settlers_online/combat_mechanics.hpp"
#include "../settlers_online/trivial_attack_sequence.hpp"
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
            typedef settlers_online::trivial_attack_sequence<true> sequencer_type;

            static bool test_deterministic()
            {
                generator& gen = generator::instance();

                settlers_online::army left_army = gen.next_army(250);
                settlers_online::army right_army = gen.next_army(270);

				sequencer_type left_seq = { };
				sequencer_type right_seq = { };

				tested_type combat(left_army, right_army);
				settlers_online::combat_result result = combat.execute(left_seq, right_seq);
				std::size_t destruction_rounds = 0;
				for (std::size_t i = 0; i < 100; i++) destruction_rounds += combat.destruct(left_seq, right_seq);
				destruction_rounds /= 100;
                return (destruction_rounds + result.number_of_rounds()) > 2;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_TEST_COMBAT_MECHANICS_TEST_HPP_INCLUDED
