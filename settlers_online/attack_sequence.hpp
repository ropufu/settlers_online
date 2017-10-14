
#ifndef ROPUFU_SETTLERS_ONLINE_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ATTACK_SEQUENCE_HPP_INCLUDED

#include "unit_type.hpp"

#include <cstddef>
//#include <cstdint>
#include <exception>
#include <type_traits>

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Abstract class (CRTP) for determining attack sequence of an army.
         *  @remark For more information on CRTP see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
         *  @remark The class has to implement the following functions:
         *          bool peek_do_high_damage(const unit_type& unit) const noexcept
         *          std::size_t peek_count_high_damage(const unit_type& unit, std::size_t count_units) const noexcept
         *          bool peek_do_splash(const unit_type& unit) const noexcept
         *          bool did_last_splash(const unit_type& unit) const noexcept
         */
        template <typename t_derived_type>
        struct attack_sequence
        {
            typedef t_derived_type derived_type;
            typedef attack_sequence<t_derived_type> type;

        private:
            std::size_t m_round_index = 0; // Zero-based index of the current round.
            std::size_t m_phase_index = 0; // Zero-based index of the current phase within the round.
            std::size_t m_unit_index = 0; // Zero-based index of the attacking unit within the phase.
            bool m_is_destruction = false; // Indicates that destruction is in progress.

        public:
            /** Zero-based index of the current round. */
            std::size_t round_index() const noexcept { return this->m_round_index; }

            /** Zero-based index of the current phase within the round. */
            std::size_t phase_index() const noexcept { return this->m_phase_index; }

            /** Zero-based index of the attacking unit within the phase. */
            std::size_t unit_index() const noexcept { return this->m_unit_index; }

            /** Indicator of destruction phase. */
            bool is_destruction() const noexcept { return this->m_is_destruction; }

            /** Sets the destruction flag. */
            void start_destruction()
            {
                if (this->m_is_destruction) throw std::logic_error("<start_destruction> cannot be called twice");

                this->m_is_destruction = true;
                this->m_round_index = 0;
                this->m_phase_index = 0;
                this->m_unit_index = 0;
            }

            /** Advances to the next round. */
            void next_round() noexcept 
            {
                this->m_round_index++;
                this->m_phase_index = 0;
                this->m_unit_index = 0;
            }

            /** Advances to the next phase. */
            void next_phase() noexcept 
            {
                this->m_phase_index++; 
                this->m_unit_index = 0;
            }

            /** Advances to the next unit. */
            void next_unit() noexcept 
            {
                this->m_unit_index++; 
            }

            /** Increments \c unit_index by \p count. */
            void next_unit(std::size_t count) noexcept { this->m_unit_index += count; }

            /** @brief Indicates whether the current unit will do high damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_high_damage(const unit_type& unit) const noexcept
            {
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::peek_do_high_damage), 
                    decltype(&type::peek_do_high_damage)>::value;
                static_assert(!is_overwritten, "static polymorphic function <peek_do_high_damage> was not overwritten.");
                const derived_type* that = static_cast<const derived_type*>(this);
                return that->peek_do_high_damage(unit);
            }
            
            /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
             *  @param unit Type of attacking units.
             *  @param count_units Number of attacking units.
             */
            std::size_t peek_count_high_damage(const unit_type& unit, std::size_t count_units) const noexcept
            {
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::peek_count_high_damage), 
                    decltype(&type::peek_count_high_damage)>::value;
                static_assert(!is_overwritten, "static polymorphic function <peek_count_high_damage> was not overwritten.");
                const derived_type* that = static_cast<const derived_type*>(this);
                return that->peek_count_high_damage(unit, count_units);
            }

            /** @brief Indicates whether the current unit will do splash damage.
             *  @param unit Type of attacking unit.
             */
            bool peek_do_splash(const unit_type& unit) const noexcept
            {
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::peek_do_splash), 
                    decltype(&type::peek_do_splash)>::value;
                static_assert(!is_overwritten, "static polymorphic function <peek_do_splash> was not overwritten.");
                const derived_type* that = static_cast<const derived_type*>(this);
                return that->peek_do_splash(unit);
            }

            /** @brief Indicates whether the previous unit did splash damage.
             *  @param unit Type of attacking unit.
             */
            bool did_last_splash(const unit_type& unit) const noexcept
            {
                constexpr bool is_overwritten = std::is_same<
                    decltype(&derived_type::did_last_splash), 
                    decltype(&type::did_last_splash)>::value;
                static_assert(!is_overwritten, "static polymorphic function <did_last_splash> was not overwritten.");
                const derived_type* that = static_cast<const derived_type*>(this);
                return that->did_last_splash(unit);
            }
        };
    }
}

#endif // ROPUFU_SETTLERS_ONLINE_ATTACK_SEQUENCE_HPP_INCLUDED
