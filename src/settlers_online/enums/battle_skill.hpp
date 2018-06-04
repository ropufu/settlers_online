
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_SKILL_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_SKILL_HPP_INCLUDED

#include <ropufu/enum_array.hpp>

#include <cstddef>     // std::size_t
#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::settlers_online
{
    /** @brief Traits that some units may have that modify the course of the entire battle.
     *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
     **/
    enum struct battle_skill : std::size_t
    {
        none = 0,
        juggernaut = 1,         // Increases the general's (faction: general) attack damage by 20/40/60. These attacks have a 33/66/100% chance of dealing splash damage.
        garrison_annex = 2,     // Increases the unit capacity (faction: general) by 5/10/15.
        lightning_slash = 3,    // The general (faction: general) attacks twice per round. That second attack's initiative is \c last_strike.
        unstoppable_charge = 4, // Increases the maximum attack damage of your swift units (faction: cavalry) by 1/2/3 and their attacks have a 33/66/100% chance of dealing splash damage.
        weekly_maintenance = 5, // Increases the attack damage of your heavy units (faction: artillery) by 10/20/30.
        master_planner = 6,     // Adds 10% to this army's accuracy.
        battle_frenzy = 7,      // Increases the attack damage of this army by 10/20/30% for every combat round past the first.
        rapid_fire = 8,         // Increases the maximum attack damage of your Bowmen by 5/10/15.
        sniper_training = 9,    // Increases your Longbowmen's and regular Marksmen's minimum attack damage by 45/85/130% and the maximum by 5/10/15%.
        cleave = 10,            // Increases the attack damage of Elite Soldiers by 4/8/12 and their attacks have a 33/66/100% chance of dealing splash damage.
        fast_learner = 11,      // Increases the XP gained from enemy units defeated by this army by 10/20/30%.
        overrun = 12            // Decreases the HP of enemy bosses by 8/16/25%.
    }; // struct battle_skill
} // namespace ropufu::settlers_online

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_skill x) noexcept
    {
        using argument_type = ropufu::settlers_online::battle_skill;
        switch (x)
        {
            case argument_type::none: return "none";
            case argument_type::juggernaut: return "juggernaut";
            case argument_type::garrison_annex: return "garrison annex";
            case argument_type::lightning_slash: return "lightning slash";
            case argument_type::unstoppable_charge: return "unstoppable charge";
            case argument_type::weekly_maintenance: return "weekly maintenance";
            case argument_type::master_planner: return "master planner";
            case argument_type::battle_frenzy: return "battle frenzy";
            case argument_type::rapid_fire: return "rapid fire";
            case argument_type::sniper_training: return "sniper training";
            case argument_type::cleave: return "cleave";
            case argument_type::fast_learner: return "fast learner";
            case argument_type::overrun: return "overrun";
            default: return "unknown <battle_skill> " + std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c battle_skill as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::settlers_online::battle_skill>
    {
        using underlying_type = std::underlying_type_t<ropufu::settlers_online::battle_skill>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 13;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::settlers_online::battle_skill>
    {
        using enum_type = ropufu::settlers_online::battle_skill;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "none") { to = enum_type::none; return true; }
            if (from == "juggernaut") { to = enum_type::juggernaut; return true; }
            if (from == "garrison annex") { to = enum_type::garrison_annex; return true; }
            if (from == "lightning slash") { to = enum_type::lightning_slash; return true; }
            if (from == "unstoppable charge") { to = enum_type::unstoppable_charge; return true; }
            if (from == "weekly maintenance") { to = enum_type::weekly_maintenance; return true; }
            if (from == "master planner") { to = enum_type::master_planner; return true; }
            if (from == "battle frenzy") { to = enum_type::battle_frenzy; return true; }
            if (from == "rapid fire") { to = enum_type::rapid_fire; return true; }
            if (from == "sniper training") { to = enum_type::sniper_training; return true; }
            if (from == "cleave") { to = enum_type::cleave; return true; }
            if (from == "fast learner") { to = enum_type::fast_learner; return true; }
            if (from == "overrun") { to = enum_type::overrun; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_SKILL_HPP_INCLUDED
