
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED

#include "../settlers_online/balancer.hpp"

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
