
#ifndef ROPUFU_SETTLERS_ONLINE_MAP_VALUE_ITERATOR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_MAP_VALUE_ITERATOR_HPP_INCLUDED

#include <utility> // std::forward

namespace ropufu::settlers_online
{
    template <typename t_pair_iterator_type>
    struct map_value_iterator;

    template <typename t_pair_iterator_type>
    static map_value_iterator<t_pair_iterator_type> make_value_iterator(t_pair_iterator_type&& pair_iterator) noexcept
    {
        return map_value_iterator<t_pair_iterator_type>(std::forward<t_pair_iterator_type>(pair_iterator));
    } // make_value_iterator(...)

    /** @brief An iterator for \c enum_array usage in range-based for loops. */
    template <typename t_pair_iterator_type>
    struct map_value_iterator
    {
        using type = map_value_iterator<t_pair_iterator_type>;
        using pair_iterator_type = t_pair_iterator_type;

    private:
        pair_iterator_type m_pair_iterator = {};

    public:
        map_value_iterator(pair_iterator_type&& pair_iterator) noexcept
            : m_pair_iterator(pair_iterator)
        {
        } // map_value_iterator(...)

        /** Inequality operator, used as termination condition. */
        bool operator !=(const type& other) const noexcept { return this->m_pair_iterator != other.m_pair_iterator; }
        /** Equality operator. */
        bool operator ==(const type& other) const noexcept { return this->m_pair_iterator == other.m_pair_iterator; }

        /** Returns the current enum key/value pair. */
        auto operator *() const noexcept -> decltype(this->m_pair_iterator->second)
        {
            return this->m_pair_iterator->second;
        } // operator *(...)

        /** If not at the end, advances the position of the iterator by one. */
        type& operator ++() noexcept
        {
            // if (this->m_position == type::past_the_last_index) return *this; // Technically unnecessary.
            ++(this->m_pair_iterator);
            return *this;
        } // operator ++(...)
    }; // struct map_value_iterator
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_MAP_VALUE_ITERATOR_HPP_INCLUDED
