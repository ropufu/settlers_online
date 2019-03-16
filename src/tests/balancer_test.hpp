
#ifndef ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED

#include "../settlers_online/balancer.hpp"

#include <cstddef> // std::size_t
#include <system_error> // std::error_code
#include <vector> // std::vector

namespace ropufu::settlers_online_test
{
    struct balancer_test
    {
        using type = unit_type_test;
        using tested_type = settlers_online::balancer;

    private:
        static std::size_t t(std::size_t hours, std::size_t minutes, std::size_t seconds) noexcept
        {
            return 60 * (60 * hours + minutes) + seconds;
        } // t(...)

        static std::size_t t(std::size_t minutes, std::size_t seconds) noexcept
        {
            return 60 * minutes + seconds;
        } // t(...)

    public:
        static bool test_farms_1() noexcept
        {
            std::error_code ec {};
            
            // Production buildings.
            std::vector<std::size_t> production_times { t(4, 28), t(4, 36), t(4, 24), t(4, 48), t(4, 40), t(4, 32) };
            // Consumption buildings.
            std::vector<std::size_t> consumption_times { t(4, 28), t(4, 36), t(4, 24), t(4, 48), t(4, 40), t(4, 32) };

            tested_type b {production_times, consumption_times, ec};

            // Number of slots should be the same for production and consumption buildings.
            if (ec.value() != 0) return false;

            auto arrangement = b.run();
            if (arrangement.mask != 44) return false;

            return !static_cast<bool>(ec);
        }
    }; // struct balancer_test
} // namespace ropufu::settlers_online_test

#endif // ROPUFU_SETTLERS_ONLINE_TEST_BALANCER_TEST_HPP_INCLUDED
