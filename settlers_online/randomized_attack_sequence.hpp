
#ifndef ROPUFU_SETTLERS_ONLINE_RANDOMIZED_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_RANDOMIZED_ATTACK_SEQUENCE_HPP_INCLUDED

#include "attack_sequence.hpp"
#include "binomial_pool.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int_fast32_t
#include <random> // std::default_random_engine, std::seed_seq

namespace ropufu
{
    namespace settlers_online
    {
        /** Attack sequence where each attack always results in the same low or high value.  */
        template <typename t_engine_type = std::default_random_engine>
        struct randomized_attack_sequence : public attack_sequence<randomized_attack_sequence<t_engine_type>>
        {
            using type = randomized_attack_sequence<t_engine_type>;
            using base_type = attack_sequence<randomized_attack_sequence<t_engine_type>>;
            using engine_type = t_engine_type;
            using pool_type = binomial_pool<engine_type>;

        private:
            pool_type& m_pool = pool_type::instance();
            engine_type m_engine;
            bool m_did_last_splash = false;

        public:
            randomized_attack_sequence()
                : m_engine()
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::seed_seq ss = { 875, 393, 19, static_cast<std::int_fast32_t>(now.time_since_epoch().count()) };
                this->m_engine.seed(ss);
            }

            /** @brief Indicates whether the current unit will do high damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_high_damage(const unit_type& unit) noexcept
            {
                double x = unit.accuracy();
                if (x == 0) return false;
                if (x == 1) return true;
                return this->m_pool.bernoulli_sampler(x)(this->m_engine) == 1;
            }
            
            /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
             *  @param unit Type of attacking units.
             *  @param count_units Number of attacking units.
             */
            std::size_t peek_count_high_damage(const unit_type& unit, std::size_t count_units) noexcept
            {
                double x = unit.accuracy();
                if (x == 0) return false;
                if (x == 1) return true;
                return this->m_pool.binomial_lookup_sampler(x)(count_units, this->m_engine);
            }

            /** @brief Indicates whether the current unit will do splash damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_splash(const unit_type& unit) noexcept
            {
                double x = unit.splash_chance();
                if (x == 0) return false;
                if (x == 1) return true;
                this->m_did_last_splash = (this->m_pool.bernoulli_sampler(x)(this->m_engine) == 1);
                return this->m_did_last_splash;
            }

            /** @brief Indicates whether the previous unit did splash damage.
             *  @param unit Type of attacking unit.
             */
            bool did_last_splash(const unit_type& unit) noexcept
            {
                double x = unit.splash_chance();
                if (x == 0) return false;
                if (x == 1) return true;
                return this->m_did_last_splash;
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_RANDOMIZED_ATTACK_SEQUENCE_HPP_INCLUDED
