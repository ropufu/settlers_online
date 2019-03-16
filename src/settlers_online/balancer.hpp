
#ifndef ROPUFU_SETTLERS_ONLINE_BALANCER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BALANCER_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include <cstddef> // std::size_t
#include <numeric> // std::lcm
#include <string>  // std::string
#include <system_error> // std::error_code
#include <vector> // std::vector

namespace ropufu::settlers_online
{
    template <typename t_mask_type>
    struct balancer_token
    {
        using type = balancer_token<t_mask_type>;
        using mask_type = t_mask_type;

        mask_type mask;
    }; // struct balancer_token

    /** Balances production and consumption of intermediate resource to maximize output of the final resource. */
    struct balancer
    {
        using type = balancer;
        using mask_type = std::size_t;
        using token_type = balancer_token<mask_type>;
        /** 12 hours expressed in seconds. */
        static constexpr std::size_t default_time_interval = 60 * 60 * 12;

    private:
        std::error_code m_error_code = {};
        std::vector<std::size_t> m_intermediate_production_frequencies = {}; // Number of intermediate resource production cycles per /c m_common_time.
        std::vector<std::size_t> m_intermediate_consumption_frequencies = {}; // Number of intermediate resource consumption cycles per /c m_common_time.
        std::size_t m_common_time = 0; // Time interval in which both consumption and production cycles fit perfectly (whole number of times).
        std::size_t m_intermediate_produced = 1;
        std::size_t m_intermediate_consumed = 1;
        std::size_t m_final_produced = 1;

        static mask_type two_pow(std::size_t power)
        {
            mask_type result = 1;
            for (std::size_t i = 0; i < power; ++i) result <<= 1;
            return result;
        } // two_pow(...)

    public:
        template <typename t_action_if_set, typename t_action_if_unset>
        static void for_bit_mask(mask_type mask, std::size_t width, t_action_if_set&& on_set, t_action_if_unset&& on_unset) noexcept
        {
            for (std::size_t position = 0; position < width; ++position)
            {
                if ((mask & 1) == 1) on_set(position);
                else on_unset(position);
                mask >>= 1;
            } // while (...)
        } // for_bit_mask(...)

        static std::vector<std::size_t> which_bits_set(mask_type mask) noexcept
        {
            std::vector<std::size_t> result {};
            std::size_t position = 0;
            while (mask != 0)
            {
                if ((mask & 1) == 1) result.push_back(position);
                mask >>= 1;
                ++position;
            } // while (...)
            return result;
        } // which_bits_set(...)

        balancer() noexcept { }

        balancer(const std::vector<std::size_t>& intermediate_times) noexcept
            : balancer(intermediate_times, intermediate_times, this->m_error_code)
        {
        } // balancer(...)

        balancer(const std::vector<std::size_t>& intermediate_production_times, const std::vector<std::size_t>& intermediate_consumption_times, std::error_code& ec) noexcept
            : m_intermediate_production_frequencies(intermediate_production_times), m_intermediate_consumption_frequencies(intermediate_consumption_times)
        {
            if (intermediate_production_times.size() != intermediate_consumption_times.size())
            {
                /** @todo Replace with aftermath \c on_error call. */
                aftermath::detail::on_error(ec, std::errc::invalid_argument, "Number of slots should be the same for production and consumption buildings.");
                this->m_error_code = ec;
                return;
            } // if (...)

            this->m_common_time = 1;
            for (std::size_t x : intermediate_production_times) this->m_common_time = std::lcm(this->m_common_time, x);
            for (std::size_t x : intermediate_consumption_times) this->m_common_time = std::lcm(this->m_common_time, x);

            for (std::size_t& x : this->m_intermediate_production_frequencies) x = this->m_common_time / x;
            for (std::size_t& x : this->m_intermediate_consumption_frequencies) x = this->m_common_time / x;
        } // balancer(...)

        /** Indicates if the \c balancer is well-formed. */
        bool good() const noexcept { return this->m_error_code.value() == 0; }

        /** Number of intermediate resource production cycles per /c common_time(). */
        const std::vector<std::size_t>& intermediate_production_frequencies() const noexcept { return this->m_intermediate_production_frequencies; }

        /** Number of intermediate resource consumption cycles per /c common_time(). */
        const std::vector<std::size_t>& intermediate_consumption_frequencies() const noexcept { return this->m_intermediate_consumption_frequencies; }

        /** Time interval in which both consumption and production cycles fit perfectly (whole number of times). */
        std::size_t common_time() const noexcept { return this->m_common_time; }

        /** Amount of intermediate resource produced per cycle. */
        std::size_t intermediate_produced() const noexcept { return this->m_intermediate_produced; }
        /** Amount of intermediate resource consumed per cycle. */
        std::size_t intermediate_consumed() const noexcept { return this->m_intermediate_consumed; }
        /** Amount of final resource produced per cycle. */
        std::size_t final_produced() const noexcept { return this->m_final_produced; }

        /** Sets the amount of resource production / consumption per corresponding cycle. */
        void set_production_ratios(std::size_t intermediate_produced, std::size_t intermediate_consumed, std::size_t final_produced) noexcept
        {
            this->m_intermediate_produced = intermediate_produced;
            this->m_intermediate_consumed = intermediate_consumed;
            this->m_final_produced = final_produced;
        } // set_production_ratios(...)

        /** Calculates the best mask  */
        token_type run() const noexcept
        {
            if (this->m_error_code.value() != 0) return {}; // Panic, do nothing!

            std::size_t n = this->m_intermediate_production_frequencies.size();
            std::size_t largest_negative_balance = 0;
            mask_type best_mask = 0;

            for (mask_type mask = 0; mask < type::two_pow(n); ++mask)
            {
                std::size_t positive_balance = 0;
                std::size_t negative_balance = 0;
                type::for_bit_mask(mask, n,
                    [&] (std::size_t position) { positive_balance += this->m_intermediate_produced * this->m_intermediate_production_frequencies[position]; },
                    [&] (std::size_t position) { negative_balance += this->m_intermediate_consumed * this->m_intermediate_consumption_frequencies[position]; }
                );
                if (positive_balance >= negative_balance && negative_balance >= largest_negative_balance)
                {
                    largest_negative_balance = negative_balance;
                    best_mask = mask;
                } // if (...)
            } // for (...)

            return { best_mask };
        } // run(...)

        /** Calculates the production rate of the final resource over a specified time interval. */
        double final_production_rate(token_type token, std::size_t time_interval_in_seconds = type::default_time_interval) const noexcept
        {
            if (this->m_error_code.value() != 0) return 0; // Panic, do nothing!

            std::size_t n = this->m_intermediate_production_frequencies.size();
            std::size_t final_balance = 0;

            // ~~ Consumption of intermediate resource is production of final resource ~~
            type::for_bit_mask(token.mask, n,
                [&] (std::size_t position) { },
                [&] (std::size_t position) { final_balance += this->m_final_produced * this->m_intermediate_consumption_frequencies[position]; }
            );

            return (final_balance * time_interval_in_seconds) / static_cast<double>(this->m_common_time);
        } // final_production_rate(...)
    }; // struct balancer
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BALANCER_HPP_INCLUDED
