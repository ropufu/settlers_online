
#ifndef ROPUFU_SETTLERS_ONLINE_BINOMIAL_POOL_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BINOMIAL_POOL_HPP_INCLUDED

#include <aftermath/probability.hpp>
#include <aftermath/random.hpp>

#include "army.hpp"
#include "unit_group.hpp"
#include "unit_type.hpp"

#include <cstddef> // std::size_t
#include <map> // std::map
#include <random> // std::default_random_engine

//#include <iostream>

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Class for generating bernoulli and binomial variates.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        template <typename t_engine_type = std::default_random_engine>
        struct binomial_pool
        {
            using type = binomial_pool<t_engine_type>;
            using engine_type = t_engine_type;
            using uniform_type = typename engine_type::result_type;
            using binomial_distribution_type = aftermath::probability::dist_binomial;
            using bernoulli_sampler_type = aftermath::random::default_sampler_bernoulli_t<t_engine_type>;
            using binomial_lookup_sampler_type = aftermath::random::default_sampler_binomial_lookup_t<t_engine_type>;

        private:
            std::map<double, bernoulli_sampler_type> m_bernoulli_cache = { };
            std::map<double, binomial_lookup_sampler_type> m_binomial_lookup_cache = { };

        protected:
            binomial_pool() noexcept { }
            ~binomial_pool() noexcept { }

        public:
            /** Updates the sampler cache from a provided instance of \p army. */
            void cache(const army& army)
            {
                // Iterate through army groups.
                for (const unit_group& g : army.groups())
                {
                    std::size_t count = g.count(); // Number of units in the group.
                    if (count == 0) continue; // Skip empty groups.

                    const unit_type& t = g.type(); // Unit type in the group.
                    double accuracy = t.damage().accuracy(); // Accuracy of the units.
                    double splash_chance = t.damage().splash_chance(); // Splash chance of the units.

                    // Bernoulli samplers for damage and splash.
                    binomial_distribution_type bernoulli_for_damage(1, accuracy);
                    binomial_distribution_type bernoulli_for_splash(1, splash_chance);
                    this->m_bernoulli_cache.emplace(accuracy, bernoulli_sampler_type(bernoulli_for_damage)); // Does nothing if the sampler already exists.
                    this->m_bernoulli_cache.emplace(splash_chance, bernoulli_sampler_type(bernoulli_for_splash)); // Does nothing if the sampler already exists.

                    // Binomial sampler for damage.
                    binomial_distribution_type binomial_for_damage(count, accuracy);
                    auto existing_sampler_it = this->m_binomial_lookup_cache.find(accuracy); // See if the sampler already exists.
                    if (existing_sampler_it == this->m_binomial_lookup_cache.end()) // Sampler not found.
                    {
                        //std::cout << "Building binomial lookup " << accuracy << " from " << bernoulli_for_damage.number_of_trials() << " to " << binomial_for_damage.number_of_trials() << std::endl;
                        this->m_binomial_lookup_cache.emplace(accuracy, binomial_lookup_sampler_type(bernoulli_for_damage, binomial_for_damage)); // Does nothing if the sampler already exists.
                    }
                    else
                    {
                        binomial_lookup_sampler_type& existing_sampler = existing_sampler_it->second; // Existing sampler.
                        // Update existing sampler if the number of units in the current group is greater.
                        if (existing_sampler.number_of_trials_max() < count)
                        {
                            //std::cout << "Updating binomial lookup " << accuracy << " from " << bernoulli_for_damage.number_of_trials() << " to " << binomial_for_damage.number_of_trials() << std::endl;
                            existing_sampler = binomial_lookup_sampler_type(bernoulli_for_damage, binomial_for_damage);
                        }
                    }
                }
            }

            /** @brief Bernoulli sampler based on the provided \p probability_of_success. 
             *  @exception std::out_of_range \p probability_of_success is not in the interval [0, 1].
             *  @exception std::out_of_range Provided \p probability_of_success is not in the cache.
             */
            const bernoulli_sampler_type& bernoulli_sampler(double probability_of_success) const
            {
                if (probability_of_success < 0.0 || probability_of_success > 1.0) throw std::out_of_range("<probability_of_success> must be in the range from 0 to 1");
                return this->m_bernoulli_cache.at(probability_of_success);
            }

            /** @brief Binomial lookup sampler based on the provided \p probability_of_success. 
             *  @exception std::out_of_range \p probability_of_success is not in the interval [0, 1].
             *  @exception std::out_of_range Provided \p probability_of_success is not in the cache.
             */
            const binomial_lookup_sampler_type& binomial_lookup_sampler(double probability_of_success) const
            {
                if (probability_of_success < 0.0 || probability_of_success > 1.0) throw std::out_of_range("<probability_of_success> must be in the range from 0 to 1");
                return this->m_binomial_lookup_cache.at(probability_of_success);
            }
            
            /** The only instance of this type. */
            static type& instance()
            {
                // Since it's a static variable, if the class has already been created, it won't be created again.
                // Note: it is thread-safe in C++11.
                static type s_instance;
                // Return a reference to our instance.
                return s_instance;
            }

            // ~~ Delete copy and move constructors and assign operators ~~
            binomial_pool(const type&) = delete;    // Copy constructor.
            binomial_pool(type&&)      = delete;    // Move constructor.
            type& operator =(const type&) = delete; // Copy assign.
            type& operator =(type&&)      = delete; // Move assign.
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_BINOMIAL_POOL_HPP_INCLUDED
