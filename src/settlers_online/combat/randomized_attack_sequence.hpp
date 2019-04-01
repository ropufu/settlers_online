
#ifndef ROPUFU_SETTLERS_ONLINE_RANDOMIZED_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_RANDOMIZED_ATTACK_SEQUENCE_HPP_INCLUDED

#include <ropufu/probability.hpp>
#include <ropufu/random.hpp>

#include "attack_sequence.hpp"
#include "battle_clock.hpp"
#include "damage.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <chrono>  // std::chrono::high_resolution_clock
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <random>  // std::mt19937, std::seed_seq
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    /** Attack sequence where each attack always results in the same low or high value.  */
    template <typename t_engine_type = std::mt19937,
        typename t_bernoulli_sampler_type = aftermath::random::bernoulli_sampler<t_engine_type, double>,
        typename t_binomial_lookup_type = aftermath::random::binomial_lookup<t_engine_type, std::size_t, double>>
    struct randomized_attack_sequence : public attack_sequence<randomized_attack_sequence<t_engine_type, t_bernoulli_sampler_type, t_binomial_lookup_type>>
    {
        using type = randomized_attack_sequence<t_engine_type, t_bernoulli_sampler_type, t_binomial_lookup_type>;
        using base_type = attack_sequence<type>;
        using engine_type = t_engine_type;
        using bernoulli_sampler_type = t_bernoulli_sampler_type;
        using binomial_lookup_type = t_binomial_lookup_type;

        using bernoulli_distribution_type = typename bernoulli_sampler_type::distribution_type;
        using binomial_distribution_type = typename binomial_lookup_type::distribution_type;

    private:
        engine_type m_engine; // Uniform PRNG.
        bernoulli_sampler_type m_splash_sampler = {};
        bernoulli_sampler_type m_accuracy_sampler = {};
        binomial_lookup_type m_accuracy_lookup = {};
        bool m_did_last_splash = false;

    public:
        randomized_attack_sequence() noexcept
            : m_engine()
        {
        } // randomized_attack_sequence(...)

        randomized_attack_sequence(const unit_group& g, std::size_t group_index, std::error_code& ec) noexcept
            : m_engine(),
            m_splash_sampler(bernoulli_distribution_type(g.unit().base_damage().splash_chance(), ec)),
            m_accuracy_sampler(bernoulli_distribution_type(g.unit().base_damage().accuracy(), ec)),
            m_accuracy_lookup(
                binomial_distribution_type(1, g.unit().base_damage().accuracy(), ec),
                binomial_distribution_type(g.alive_as_attacker() ? g.count_as_attacker() : 1, g.unit().base_damage().accuracy(), ec), ec)
        {
            std::int32_t seed_offset = static_cast<std::int32_t>(group_index);
            auto now = std::chrono::high_resolution_clock::now();
            std::seed_seq ss { seed_offset + 875, 393, 19, static_cast<std::int32_t>(now.time_since_epoch().count()) };
            this->m_engine.seed(ss);
        } // randomized_attack_sequence(...)

        /** @brief Indicates whether the current unit will do high damage. */
        bool peek_do_high_damage(const battle_clock& /*clock*/) noexcept
        {
            return this->m_accuracy_sampler(this->m_engine);
        } // peek_do_high_damage(...)
        
        /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
         *  @param count_units Number of attacking units.
         */
        std::size_t peek_count_high_damage(std::size_t count_units, const battle_clock& /*clock*/) noexcept
        {
            return this->m_accuracy_lookup(count_units, this->m_engine);
        } // peek_count_high_damage(...)

        /** @brief Indicates whether the current unit will do splash damage. */
        bool peek_do_splash(const battle_clock& /*clock*/) noexcept
        {
            return this->m_splash_sampler(this->m_engine);
        } // peek_do_splash(...)

        /** @brief Indicates whether the previous unit did splash damage. */
        bool did_last_splash(const battle_clock& /*clock*/) noexcept
        {
            return this->m_splash_sampler(this->m_engine);
        } // did_last_splash(...)
    }; // struct randomized_attack_sequence
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_RANDOMIZED_ATTACK_SEQUENCE_HPP_INCLUDED