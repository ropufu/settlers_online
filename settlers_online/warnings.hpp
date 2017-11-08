
#ifndef ROPUFU_SETTLERS_ONLINE_WARNINGS_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_WARNINGS_HPP_INCLUDED

#include <string> // std::string
#include <deque> // std::deque

namespace ropufu
{
    namespace settlers_online
    {
        /** Handles warning messages. */
        struct warnings : public std::deque<std::string>
        {
            using type = warnings;
        }; // struct warnings
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_WARNINGS_HPP_INCLUDED
