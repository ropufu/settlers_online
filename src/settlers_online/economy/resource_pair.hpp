
#ifndef ROPUFU_SETTLERS_ONLINE_RESOURCE_PAIR_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_RESOURCE_PAIR_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <ropufu/json_traits.hpp>
#include <ropufu/on_error.hpp>

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int_fast32_t
#include <functional> // std::hash
#include <ostream>    // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc

namespace ropufu::settlers_online
{
    /** Descriptor for resource quantities. */
    struct resource_pair;

    void to_json(nlohmann::json& j, const resource_pair& x) noexcept;
    void from_json(const nlohmann::json& j, resource_pair& x);

    /** Descriptor for unit types. */
    struct resource_pair
    {
        using type = resource_pair;
        // ~~ Json names ~~
        static constexpr char jstr_name[] = "resource";
        static constexpr char jstr_amount[] = "amount";

    private:
        std::string m_name = "??";
        std::int_fast32_t m_amount = 0;

        constexpr bool validate(std::error_code& /*ec*/) const noexcept { return true; }

        void coerce() noexcept
        {
            if (this->m_name.empty()) this->m_name = "??";
        } // coerce(...)

    public:
        /** @brief Resource quantities. */
        resource_pair() noexcept { }

        /** @brief Resource quantities. */
        resource_pair(const std::string& resource_name, std::int_fast32_t amount) noexcept
            : m_name(resource_name), m_amount(amount)
        {
            this->coerce();
        } // resource_pair(...)

        resource_pair(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            // Parse json entries.
            aftermath::noexcept_json::required(j, type::jstr_name, this->m_name, ec);
            aftermath::noexcept_json::required(j, type::jstr_amount, this->m_amount, ec);
        } // resource_pair(...)

        /** Resource name. */
        const std::string& name() const noexcept { return this->m_name; }
        /** Resource name. */
        void set_name(const std::string& value) noexcept
        {
            this->m_name = value;
            this->coerce();
        } // set_name(...)

        /** Resource amount. */
        std::int_fast32_t amount() const noexcept { return this->m_amount; }
        /** Resource amount. */
        void set_amount(std::int_fast32_t value) noexcept { this->m_amount = value; }

        bool empty() const noexcept { return this->m_amount == 0; }
        
        /** Checks two types for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_name == other.m_name &&
                this->m_amount == other.m_amount;
        } // operator ==(...)

        /** Checks two types for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        /** Modifies resource amount. */
        type& operator +=(std::int_fast32_t value) noexcept
        {
            this->m_amount += value;
            return *this;
        } // operator +=(...)

        /** Modifies resource amount. */
        type& operator -=(std::int_fast32_t value) noexcept
        {
            this->m_amount -= value;
            return *this;
        } // operator +=(...)

        /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
        friend type operator +(type left, std::int_fast32_t right) noexcept { left += right; return left; }
        friend type operator -(type left, std::int_fast32_t right) noexcept { left -= right; return left; }

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct resource_pair

    // ~~ Json name definitions ~~
    constexpr char resource_pair::jstr_name[];
    constexpr char resource_pair::jstr_amount[];

    void to_json(nlohmann::json& j, const resource_pair& x) noexcept
    {
        using type = resource_pair;

        j = nlohmann::json{
            {type::jstr_name, x.name()},
            {type::jstr_amount, x.amount()}
        };
    } // to_json(...)

    void from_json(const nlohmann::json& j, resource_pair& x)
    {
        using type = resource_pair;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing JSON failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::settlers_online

namespace std
{
    template <>
    struct hash<ropufu::settlers_online::resource_pair>
    {
        using argument_type = ropufu::settlers_online::resource_pair;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<std::string> string_hash = {};
            std::hash<std::int_fast32_t> int_hash = {};

            return
                string_hash(x.name()) ^
                int_hash(x.amount());
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_SETTLERS_ONLINE_RESOURCE_PAIR_HPP_INCLUDED
