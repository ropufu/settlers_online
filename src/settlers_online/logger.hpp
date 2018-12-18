
#ifndef ROPUFU_SETTLERS_ONLINE_LOGGER_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_LOGGER_HPP_INCLUDED

#include <cstddef> // std::nullptr_t
#include <string> // std::string
#include <deque> // std::deque
#include <iostream> // std::cout, std::endl
#include <sstream> // std::ostringstream

namespace ropufu
{
    namespace settlers_online
    {
        namespace detail
        {
            /** Handles messages. */
            struct logger
            {
                using type = logger;
                static constexpr bool is_enabled = true;

            private:
                std::deque<std::string> m_entries = {};
                std::ostringstream m_message_stream = std::ostringstream();

            public:
                void clear() noexcept
                {
                    this->m_entries.clear();
                    this->m_message_stream.clear();
                    this->m_message_stream.str("");
                }

                /** Adds an entry to the log. */
                void write(const std::string& value) noexcept { this->m_entries.push_back(value); }
                /** Adds an entry to the log. */
                void write(std::string&& value) noexcept { this->m_entries.push_back(value); }

                /** Flushes the message stream to logger. */
                type& operator <<(std::nullptr_t /**terminator*/) noexcept
                {
                    this->m_entries.push_back(this->m_message_stream.str());

                    this->m_message_stream.clear();
                    this->m_message_stream.str("");
                    return *this;
                } // operator <<(...)

                /** Appends the provided value to the next message. */
                template <typename t_value_type>
                type& operator <<(const t_value_type& value) noexcept { this->m_message_stream << value; return *this; }
                /** Appends the provided value to the next message. */
                template <typename t_value_type>
                type& operator <<(t_value_type&& value) noexcept { this->m_message_stream << value; return *this; }

                void unwind() noexcept 
                {
                    this->unwind([] (const std::string& w) { std::cout << '\t' << w << std::endl; });
                } // unwind(...)

                template <typename t_format_type>
                void unwind(const t_format_type& format) noexcept
                {
                    if (this->m_entries.empty()) return;
                    while (!this->m_entries.empty()) 
                    {
                        format(this->m_entries.front());
                        this->m_entries.pop_front();
                    }
                } // unwind(...)
            }; // struct logger

            /** Mimics the structure of \c logger, but does nothing. */
            struct no_logger
            {
                using type = no_logger;
                static constexpr bool is_enabled = false;

                constexpr void clear() const noexcept { }

                /** Adds an entry to the log. */
                constexpr void write(const std::string&) const noexcept { }
                /** Adds an entry to the log. */
                constexpr void write(std::string&&) const noexcept { }
                
                /** Flushes the message stream to logger. */
                constexpr const type& operator <<(std::nullptr_t) const noexcept { return *this; }
                /** Appends the provided value to the next message. */
                template <typename t_value_type>
                constexpr const type& operator <<(const t_value_type&) const noexcept { return *this; }
                /** Appends the provided value to the next message. */
                template <typename t_value_type>
                constexpr const type& operator <<(t_value_type&&) const noexcept { return *this; }

                constexpr void unwind() const noexcept { }
                template <typename t_format_type>
                constexpr void unwind(const t_format_type&) const noexcept { }
            }; // struct no_logger
        } // namespace detail
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_LOGGER_HPP_INCLUDED
