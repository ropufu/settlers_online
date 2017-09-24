
#ifndef ROPUFU_FLAGS_TYPE_HPP_INCLUDED
#define ROPUFU_FLAGS_TYPE_HPP_INCLUDED

#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace ropufu
{
	template <typename t_enum_type>
	struct flags_type
	{
		typedef flags_type<t_enum_type> type;
		typedef t_enum_type enum_type;
		typedef typename std::underlying_type<t_enum_type>::type underlying_type;

	private:
		underlying_type m_value = underlying_type();

	public:
		flags_type() = default;

		flags_type(enum_type flag)
			: m_value(static_cast<underlying_type>(flag))
		{
		}

		flags_type(underlying_type flags)
			: m_value(flags)
		{
			// TODO: check range.
		}

		flags_type(std::initializer_list<enum_type> flags_list)
		{
			this->m_value = 0;
			for (auto s : flags_list) this->m_value |= static_cast<underlying_type>(s);
		}

		bool has(enum_type flag) const
		{
			return (static_cast<underlying_type>(flag) & (this->m_value)) != 0;
		}

		type& operator |=(const type& other)
		{
			this->m_value |= other.m_value;
			return *this;
		}

		type& operator &=(const type& other)
		{
			this->m_value &= other.m_value;
			return *this;
		}

		type& operator ^=(const type& other)
		{
			this->m_value ^= other.m_value;
			return *this;
		}

		type& operator |=(enum_type flag)
		{
			this->m_value |= static_cast<underlying_type>(flag);
			return *this;
		}

		type& operator &=(enum_type flag)
		{
			this->m_value &= static_cast<underlying_type>(flag);
			return *this;
		}

		type& operator ^=(enum_type flag)
		{
			this->m_value ^= static_cast<underlying_type>(flag);
			return *this;
		}

		explicit operator /*typename*/ enum_type() const
		{
			return this->m_value;
		}

		underlying_type value() const
		{
			return this->m_value;
		}

		bool operator ==(const type& other) const
		{
			return this->m_value == other.m_value;
		}

		bool operator !=(const type& other) const
		{
			return !(this->operator ==(other));
		}
	};
}

#endif // ROPUFU_FLAGS_TYPE_HPP_INCLUDED
