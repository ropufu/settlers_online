
#ifndef ROPUFU_SETTLERS_ONLINE_ATTACK_SEQUENCE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_ATTACK_SEQUENCE_HPP_INCLUDED

#include "army.hpp"
#include "battle_clock.hpp"
#include "unit_group.hpp"

#include <cstddef>      // std::size_t
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::is_same_v, std::is_base_of_v

namespace ropufu::settlers_online
{
    /** @brief Base class for determining attack sequence of a unit group. */
    template <typename t_derived_type>
    struct attack_sequence
    {
        using type = attack_sequence<t_derived_type>;
        using derived_type = t_derived_type;

        attack_sequence() noexcept { }

        attack_sequence(const unit_group& /*g*/, std::size_t /*group_index*/, std::error_code& /*ec*/) noexcept { }
        
        /** @brief Indicates whether the current unit will do high damage. */
        bool peek_do_high_damage(const battle_clock& clock) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::peek_do_high_damage),
                decltype(&type::peek_do_high_damage)>;
            static_assert(is_overwritten, "peek_do_high_damage(..) -> bool has not been overloaded.");

            derived_type* that = static_cast<derived_type*>(this);
            return that->peek_do_high_damage(clock);
        } // peek_do_high_damage(...)
        
        /** @brief Counts the number of units in the range, starting with the current unit, that will do high damage.
         *  @param count_units Number of attacking units.
         */
        std::size_t peek_count_high_damage(std::size_t count_units, const battle_clock& clock) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::peek_count_high_damage), 
                decltype(&type::peek_count_high_damage)>;
            static_assert(is_overwritten, "peek_count_high_damage(..) -> std::size_t has not been overloaded.");

            derived_type* that = static_cast<derived_type*>(this);
            return that->peek_count_high_damage(count_units, clock);
        } // peek_count_high_damage(...)

        /** @brief Indicates whether the current unit will do splash damage. */
        bool peek_do_splash(const battle_clock& clock) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::peek_do_splash), 
                decltype(&type::peek_do_splash)>;
            static_assert(is_overwritten, "peek_do_splash(..) -> bool has not been overloaded.");

            derived_type* that = static_cast<derived_type*>(this);
            return that->peek_do_splash(clock);
        } // peek_do_splash(...)

        /** @brief Indicates whether the previous unit did splash damage. */
        bool did_last_splash(const battle_clock& clock) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::did_last_splash), 
                decltype(&type::did_last_splash)>;
            static_assert(is_overwritten, "did_last_splash(..) -> bool has not been overloaded.");

            derived_type* that = static_cast<derived_type*>(this);
            return that->did_last_splash(clock);
        } // did_last_splash(...)
    }; // struct attack_sequence

    /** @brief Stores battle parameters determining how an army is to attack its opponent. */
    template <typename t_group_sequence_type>
    struct army_sequence
    {
        using type = army_sequence<t_group_sequence_type>;
        using sequence_type = t_group_sequence_type;

    private:
        sequence_type m_invalid = {};
        std::vector<sequence_type> m_sequences = {}; // Attack sequences for each unit group.

        static constexpr void traits_check()
        {
            static_assert(std::is_base_of_v<attack_sequence<sequence_type>, sequence_type>, "t_group_sequence_type has to be derived from attack_sequence<t_group_sequence_type>.");
        } // traits_check(...)

    public:
        army_sequence() noexcept { type::traits_check(); }

        /** Prepares two conditioned armies for combat. */
        army_sequence(const army& a) noexcept
        {
            type::traits_check();
            std::error_code ec {};
            std::size_t m = a.count_groups();

            this->m_sequences.reserve(m);
            for (std::size_t i = 0; i < m; ++i)
            {
                const unit_group& g = a[i];
                this->m_sequences.emplace_back(g, i, ec);
            } // for (...)

            this->m_sequences.shrink_to_fit();
        } // army_sequence(...)

        const std::vector<sequence_type>& sequences() const noexcept { return this->m_sequences; }

        /** @warning \p group_index bounds are not checked. */
        const sequence_type& operator [](std::size_t group_index) const { return this->m_sequences[group_index]; }

        /** @warning \p group_index bounds are not checked. */
        sequence_type& operator [](std::size_t group_index) { return this->m_sequences[group_index]; }

        /** @warning \p group_index bounds are not checked. */
        const sequence_type& at(std::size_t group_index, std::error_code& ec) const noexcept
        {
            if (group_index < this->m_sequences.size()) return this->operator [](group_index);
            ec = std::make_error_code(std::errc::value_too_large);
            return this->m_invalid;
        } // at(...)

        /** @warning \p group_index bounds are not checked. */
        sequence_type& at(std::size_t group_index, std::error_code& ec) noexcept
        {
            if (group_index < this->m_sequences.size()) return this->operator [](group_index);
            ec = std::make_error_code(std::errc::value_too_large);
            return this->m_invalid;
        } // at(...)
    }; // struct army_mechanics
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_ATTACK_SEQUENCE_HPP_INCLUDED
