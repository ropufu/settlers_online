
#ifndef ROPUFU_SETTLERS_ONLINE_BALANCER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BALANCER_HPP_INCLUDED

#include <ropufu/on_error.hpp>

#include <cstddef> // std::size_t
#include <chrono>  // std::chrono::seconds
#include <limits>  // std::numeric_limits::is_integer
#include <numeric> // std::lcm
#include <string>  // std::string
#include <system_error> // std::error_code
#include <type_traits>  // std::is_same_v
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

    /** @brief Balances production and consumption of intermediate resource to maximize output of the final resource. */
    template <typename t_duration_type = std::chrono::seconds>
    struct balancer;

    /** @brief Balances production and consumption of intermediate resource to maximize output of the final resource.
     *  @remark This is the default implementation of \c balancer<...> struct, namely \c balancer<>.
     */
    using balancer_t = balancer<>;

    /** @brief Balances production and consumption of intermediate resource to maximize output of the final resource. */
    template <typename t_duration_type>
    struct balancer
    {
        using type = balancer<t_duration_type>;
        using duration_type = t_duration_type;
        using mask_type = std::size_t;
        using token_type = balancer_token<mask_type>;

    private:
        std::error_code m_error_code = {};
        duration_type m_common_time = {}; // Time interval in which both consumption and production cycles fit perfectly (whole number of times).
        std::vector<std::size_t> m_intermediate_production_frequencies = {}; // Number of intermediate resource production cycles per /c m_common_time.
        std::vector<std::size_t> m_intermediate_consumption_frequencies = {}; // Number of intermediate resource consumption cycles per /c m_common_time.
        std::size_t m_intermediate_produced = 1;
        std::size_t m_intermediate_consumed = 1;
        std::size_t m_final_produced = 1;

        static constexpr void traits_check()
        {
            using rep_type = typename duration_type::rep;
            using period_type = typename duration_type::period;
            static_assert(std::is_same_v<duration_type, std::chrono::duration<rep_type, period_type>>, "t_duration_type has to be std::chrono::duration.");
            static_assert(std::numeric_limits<rep_type>::is_integer, "Tick count has to be an integer type.");
        } // traits_check(...)

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

        balancer() noexcept { type::traits_check(); }

        /** @todo Template on an iterable collection with \c size() method, rather than have std::vector. */
        balancer(const std::vector<duration_type>& intermediate_times) noexcept
            : balancer(intermediate_times, intermediate_times, this->m_error_code)
        {
        } // balancer(...)

        /** @todo Template on an iterable collection with \c size() method, rather than have std::vector. */
        balancer(
            const std::vector<duration_type>& intermediate_production_times,
            const std::vector<duration_type>& intermediate_consumption_times,
            std::error_code& ec) noexcept
        {
            type::traits_check();
            if (intermediate_production_times.size() != intermediate_consumption_times.size())
            {
                aftermath::detail::on_error(ec, std::errc::invalid_argument, "Number of slots should be the same for production and consumption buildings.");
                this->m_error_code = ec;
                return;
            } // if (...)

            for (const duration_type& x : intermediate_production_times)
                if (x.count() <= 0)
                {
                    aftermath::detail::on_error(ec, std::errc::invalid_argument, "Production times should be positive.");
                    this->m_error_code = ec;
                    return;
                } // if (...)
            for (const duration_type& x : intermediate_consumption_times)
                if (x.count() <= 0)
                {
                    aftermath::detail::on_error(ec, std::errc::invalid_argument, "Consumption times should be positive.");
                    this->m_error_code = ec;
                    return;
                } // if (...)

            this->m_intermediate_production_frequencies.reserve(intermediate_production_times.size());
            this->m_intermediate_consumption_frequencies.reserve(intermediate_consumption_times.size());

            std::size_t common_time = 1;
            for (const duration_type& x : intermediate_production_times) common_time = std::lcm(common_time, x.count());
            for (const duration_type& x : intermediate_consumption_times) common_time = std::lcm(common_time, x.count());
            this->m_common_time = duration_type { static_cast<typename duration_type::rep>(common_time) };

            for (const duration_type& x : intermediate_production_times) this->m_intermediate_production_frequencies.push_back(common_time / x.count());
            for (const duration_type& x : intermediate_consumption_times) this->m_intermediate_consumption_frequencies.push_back(common_time / x.count());
        } // balancer(...)

        /** Indicates if the \c balancer is well-formed. */
        bool good() const noexcept { return this->m_error_code.value() == 0; }

        /** Number of intermediate resource production cycles per /c common_time(). */
        const std::vector<std::size_t>& intermediate_production_frequencies() const noexcept { return this->m_intermediate_production_frequencies; }

        /** Number of intermediate resource consumption cycles per /c common_time(). */
        const std::vector<std::size_t>& intermediate_consumption_frequencies() const noexcept { return this->m_intermediate_consumption_frequencies; }

        /** Time interval in which both consumption and production cycles fit perfectly (whole number of times). */
        const duration_type& common_time() const noexcept { return this->m_common_time; }

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
        template <typename t_rep_type, typename t_period_type>
        double final_production_rate(token_type token, std::chrono::duration<t_rep_type, t_period_type> time_interval) const noexcept
        {
            if (this->m_error_code.value() != 0) return 0; // Panic, do nothing!

            std::size_t n = this->m_intermediate_production_frequencies.size();
            std::size_t final_balance = 0;

            // ~~ Consumption of intermediate resource is production of final resource ~~
            type::for_bit_mask(token.mask, n,
                [&] (std::size_t /*position*/) { },
                [&] (std::size_t position) { final_balance += this->m_final_produced * this->m_intermediate_consumption_frequencies[position]; }
            );

            using duration_type_a = std::chrono::duration<double, typename duration_type::period>;
            using duration_type_b = std::chrono::duration<double, t_period_type>;
            using duration_type_c = std::common_type_t<duration_type_a, duration_type_b>;

            double common_time = std::chrono::duration_cast<duration_type_c>(this->m_common_time).count();
            double requested_time = std::chrono::duration_cast<duration_type_c>(time_interval).count();
            return (final_balance * requested_time) / common_time;
        } // final_production_rate(...)
    }; // struct balancer
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BALANCER_HPP_INCLUDED
