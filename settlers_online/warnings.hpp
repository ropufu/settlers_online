
#ifndef ROPUFU_SETTLERS_ONLINE_WARNINGS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_WARNINGS_HPP_INCLUDED

#include <string> // std::string
#include <deque> // std::deque
#include <iostream> // std::cout, std::endl

namespace ropufu
{
    namespace settlers_online
    {
        /** Handles warning messages. */
        struct warnings : public std::deque<std::string>
        {
            using type = warnings;

            void unwind() noexcept 
            {
                this->unwind([] (const std::string& w) { std::cout << '\t' << w << std::endl; });
            } // unwind(...)

            template <typename t_format_type>
            void unwind(const t_format_type& format) noexcept
            {
                if (this->empty()) return;
                while (!this->empty()) 
                {
                    format(this->front());
                    this->pop_front();
                }
            } // unwind(...)
        }; // struct warnings
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_WARNINGS_HPP_INCLUDED
