
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED

#include "../settlers_online/char_string.hpp"
#include "../settlers_online/economy/balancer.hpp"

#include <cmath>   // std::round
#include <cstddef> // std::size_t
#include <chrono>  // std::chrono::seconds, std::literals::chrono_literals
#include <system_error> // std::error_code
#include <vector> // std::vector

namespace ropufu::settlers_online_test
{
    struct balancer_test
    {
        using type = unit_type_test;
        using tested_type = settlers_online::balancer_t;

    public:
        static bool test_time_string() noexcept
        {
            std::chrono::seconds t1 = 4min + 32s;
            std::chrono::milliseconds t2 = t1;
            std::chrono::milliseconds t3 = t1 + 100ms;
            
            std::string four_three_two_a = settlers_online::char_string::from_duration(t1);
            std::string four_three_two_b = settlers_online::char_string::from_duration(t2);
            std::string four_three_two_point_one = settlers_online::char_string::from_duration(t3);
            if ((four_three_two_a != "4min 32s") ||
                (four_three_two_b != "4min 32s") ||
                (four_three_two_point_one != "4min 32.1s")) return false;

            std::error_code ec {};

            std::chrono::seconds t1_roundtrip = settlers_online::char_string::template to_duration<std::chrono::seconds>(four_three_two_a, ec);
            std::chrono::milliseconds t2_roundtrip = settlers_online::char_string::template to_duration<std::chrono::milliseconds>(four_three_two_b, ec);
            std::chrono::milliseconds t3_roundtrip = settlers_online::char_string::template to_duration<std::chrono::milliseconds>(four_three_two_point_one, ec);
            if (ec.value() != 0) return false;
            
            std::chrono::seconds t_bad = settlers_online::char_string::template to_duration<std::chrono::seconds>("", ec);;
            if (ec.value() == 0) return false;
            ec.clear();

            if (t1 != t1_roundtrip) return false;
            if (t2 != t2_roundtrip) return false;
            if (t3 != t3_roundtrip) return false;
            return true;
        }

        static bool test_farms_1() noexcept
        {
            using namespace std::literals::chrono_literals;
            std::error_code ec {};
            
            // Production buildings.
            std::vector<std::chrono::seconds> times {
                4min + 28s,
                4min + 36s, 
                4min + 24s,
                4min + 48s,
                4min + 40s,
                4min + 32s};

            tested_type b {times, times, ec};

            // Number of slots should be the same for production and consumption buildings.
            if (ec.value() != 0) return false;

            auto arrangement = b.run();
            if (arrangement.mask != 44) return false;
            
            double balance_per_12h = b.final_production_rate(arrangement, 12h);
            if (std::round(balance_per_12h) != 472) return false;

            return !static_cast<bool>(ec);
        }
    }; // struct balancer_test
} // namespace ropufu::settlers_online_test

#endif // ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED
