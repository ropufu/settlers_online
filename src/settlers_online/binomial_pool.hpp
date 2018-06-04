
#ifndef ROPUFU_SETTLERS_ONLINE_BINOMIAL_POOL_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BINOMIAL_POOL_HPP_INCLUDED

#include <ropufu/probability.hpp>
#include <ropufu/random.hpp>

#include "army.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <map> // std::map
#include <random> // std::mt19937

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Class for generating bernoulli and binomial variates.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        template <typename t_engine_type = std::mt19937>
        struct binomial_pool
        {
            using type = binomial_pool<t_engine_type>;
            using engine_type = t_engine_type;
            using uniform_type = typename engine_type::result_type;
            using binomial_distribution_type = aftermath::probability::binomial_distribution<std::size_t, double>;
            using bernoulli_sampler_type = aftermath::random::sampler_bernoulli<t_engine_type, std::size_t, double>;
            using binomial_lookup_sampler_type = aftermath::random::sampler_binomial_lookup<t_engine_type, std::size_t, double>;

        private:
            bernoulli_sampler_type m_invalid_bernoulli = { };
            binomial_lookup_sampler_type m_invalid_binomial = { };

            std::map<double, bernoulli_sampler_type> m_bernoulli_cache = { };
            std::map<double, binomial_lookup_sampler_type> m_binomial_lookup_cache = { };

        protected:
            binomial_pool() noexcept { }
            ~binomial_pool() noexcept { }

        public:
            /** Updates the sampler cache from a provided instance of \p army. */
            void cache(const army& army) noexcept
            {
                // Iterate through army groups.
                for (const unit_group& g : army.groups())
                {
                    std::size_t count = g.count(); // Number of units in the group.
                    if (count == 0) continue; // Skip empty groups.

                    const unit_type& t = g.unit(); // Unit type in the group.
                    double accuracy = t.damage().accuracy(); // Accuracy of the units.
                    double splash_chance = t.damage().splash_chance(); // Splash chance of the units.

                    // Bernoulli samplers for damage and splash.
                    std::size_t min_count = 1;
                    binomial_distribution_type bernoulli_for_damage(1, accuracy);
                    binomial_distribution_type bernoulli_for_splash(1, splash_chance);
                    this->m_bernoulli_cache.emplace(accuracy, bernoulli_sampler_type(bernoulli_for_damage)); // Does nothing if the sampler already exists.
                    this->m_bernoulli_cache.emplace(splash_chance, bernoulli_sampler_type(bernoulli_for_splash)); // Does nothing if the sampler already exists.

                    // Binomial sampler for damage.
                    std::size_t max_count = count;
                    binomial_distribution_type binomial_for_damage(count, accuracy);
                    auto existing_sampler_search = this->m_binomial_lookup_cache.find(accuracy); // See if the sampler already exists.
                    if (existing_sampler_search == this->m_binomial_lookup_cache.end()) // Sampler not found.
                    {
                        //std::cout << "Building binomial lookup " << accuracy << " from " << bernoulli_for_damage.number_of_trials() << " to " << binomial_for_damage.number_of_trials() << std::endl;
                        this->m_binomial_lookup_cache.emplace(accuracy, binomial_lookup_sampler_type(min_count, max_count, accuracy)); // Does nothing if the sampler already exists.
                    }
                    else
                    {
                        binomial_lookup_sampler_type& existing_sampler = existing_sampler_search->second; // Existing sampler.
                        // Update existing sampler if the number of units in the current group is greater.
                        if (existing_sampler.number_of_trials_max() < count)
                        {
                            //std::cout << "Updating binomial lookup " << accuracy << " from " << bernoulli_for_damage.number_of_trials() << " to " << binomial_for_damage.number_of_trials() << std::endl;
                            existing_sampler = binomial_lookup_sampler_type(min_count, max_count, accuracy);
                        }
                    }
                }
            } // cache(...)

            /** @brief Bernoulli sampler based on the provided \p probability_of_success. 
             *  @exception std::logic_error \p probability_of_success is not in the interval [0, 1].
             *  @exception std::out_of_range \p probability_of_success is not in the cache.
             */
            const bernoulli_sampler_type& bernoulli_sampler(double probability_of_success) const
            {
                if (probability_of_success < 0 || probability_of_success > 1) throw std::logic_error("<probability_of_success> must lie within the interval [0, 1].");

                auto search = this->m_bernoulli_cache.find(probability_of_success);
                if (search != this->m_bernoulli_cache.end()) return search->second;

                throw std::logic_error("<probability_of_success> is not present in the cache.");
                return this->m_invalid_bernoulli;
            } // bernoulli_sampler(...)

            /** @brief Binomial lookup sampler based on the provided \p probability_of_success.
             *  @exception std::logic_error \p probability_of_success is not in the interval [0, 1].
             *  @exception std::out_of_range \p probability_of_success is not in the cache.
             */
            const binomial_lookup_sampler_type& binomial_lookup_sampler(double probability_of_success) const
            {
                if (probability_of_success < 0 || probability_of_success > 1) throw std::logic_error("<probability_of_success> must lie within the interval [0, 1].");

                auto search = this->m_binomial_lookup_cache.find(probability_of_success);
                if (search != this->m_binomial_lookup_cache.end()) return search->second;

                throw std::out_of_range("<probability_of_success> is not present in the cache.");
                return this->m_invalid_binomial;
            } // binomial_lookup_sampler(...)
            
            /** The only instance of this type. */
            static type& instance() noexcept
            {
                // Since it's a static variable, if the class has already been created, it won't be created again.
                // Note: it is thread-safe in C++11.
                static type s_instance;
                // Return a reference to our instance.
                return s_instance;
            } // instance(...)

            // ~~ Delete copy and move constructors and assign operators ~~
            binomial_pool(const type&) = delete;    // Copy constructor.
            binomial_pool(type&&)      = delete;    // Move constructor.
            type& operator =(const type&) = delete; // Copy assign.
            type& operator =(type&&)      = delete; // Move assign.
        }; // struct binomial_pool
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_BINOMIAL_POOL_HPP_INCLUDED
