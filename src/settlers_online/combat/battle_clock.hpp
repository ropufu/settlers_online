
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_CLOCK_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_CLOCK_HPP_INCLUDED

#include <cstddef> // std::size_t

namespace ropufu::settlers_online
{
    struct battle_clock
    {
    private:
        std::size_t m_round_index = 0; // Zero-based index of the current round.
        std::size_t m_phase_index = 0; // Zero-based index of the current phase within the round.
        std::size_t m_unit_index = 0;  // Zero-based index of the attacking unit within the phase.
        bool m_is_destruction = false; // Indicates that destruction is in progress.

    public:
        battle_clock() noexcept { }

        std::size_t round_index() const noexcept { return this->m_round_index; }
        std::size_t phase_index() const noexcept { return this->m_phase_index; }
        std::size_t unit_index() const noexcept { return this->m_unit_index; }
        bool is_destruction() const noexcept { return this->m_is_destruction; }

        /** Advances to the next round. */
        void next_round() noexcept 
        {
            ++this->m_round_index;
            this->m_phase_index = 0;
            this->m_unit_index = 0;
        } // next_round(...)

        /** Advances to the next phase. */
        void next_phase() noexcept 
        {
            ++this->m_phase_index;
            this->m_unit_index = 0;
        } // next_phase(...)

        /** Advances to the next unit. */
        void next_unit() noexcept 
        {
            ++this->m_unit_index;
        } // next_unit(...)

        /** Increments \c unit_index by \p count. */
        void next_unit(std::size_t count) noexcept
        {
            this->m_unit_index += count;
        } // next_unit(...)

        void start_destruction() noexcept
        {
            this->m_is_destruction = true;
            this->m_round_index = 0;
            this->m_phase_index = 0;
            this->m_unit_index = 0;
        } // start_destruction(...)
    }; // struct battle_clock
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_CLOCK_HPP_INCLUDED
