
#ifndef ROPUFU_SETTLERS_ONLINE_BATTLE_SKILL_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_BATTLE_SKILL_HPP_INCLUDED

#include "enum_array.hpp"

#include <cstddef> // std::size_t
#include <string> // std::string, std::to_string

namespace ropufu
{
    namespace settlers_online
    {
        /** @brief Traits that some units may have that modify the course of the entire battle.
         *  @remark Used internally as an indexer for \c enum_array, so don't go too high or negative. 
         **/
        enum class battle_skill : std::size_t
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
        };

        /** Mark \c battle_skill as suitable for \c enum_array storage. */
        template <>
        struct enum_capacity<battle_skill>
        {
            /** The maximum value of \c battle_skill plus one. */
            static constexpr std::size_t value = 13;
        };

        bool try_parse(const std::string& str, battle_skill& value)
        {
            if (str == "none") { value = battle_skill::none; return true; }
            if (str == "juggernaut") { value = battle_skill::juggernaut; return true; }
            if (str == "garrison_annex") { value = battle_skill::garrison_annex; return true; }
            if (str == "lightning_slash") { value = battle_skill::lightning_slash; return true; }
            if (str == "unstoppable_charge") { value = battle_skill::unstoppable_charge; return true; }
            if (str == "weekly_maintenance") { value = battle_skill::weekly_maintenance; return true; }
            if (str == "master_planner") { value = battle_skill::master_planner; return true; }
            if (str == "battle_frenzy") { value = battle_skill::battle_frenzy; return true; }
            if (str == "rapid_fire") { value = battle_skill::rapid_fire; return true; }
            if (str == "sniper_training") { value = battle_skill::sniper_training; return true; }
            if (str == "cleave") { value = battle_skill::cleave; return true; }
            if (str == "fast_learner") { value = battle_skill::fast_learner; return true; }
            if (str == "overrun") { value = battle_skill::overrun; return true; }
            return false;
        }
    }
}

namespace std
{
    std::string to_string(ropufu::settlers_online::battle_skill value)
    {
        switch (value)
        {
        case ropufu::settlers_online::battle_skill::none: return "none";
        case ropufu::settlers_online::battle_skill::juggernaut: return "juggernaut";
        case ropufu::settlers_online::battle_skill::garrison_annex: return "garrison annex";
        case ropufu::settlers_online::battle_skill::lightning_slash: return "lightning slash";
        case ropufu::settlers_online::battle_skill::unstoppable_charge: return "unstoppable charge";
        case ropufu::settlers_online::battle_skill::weekly_maintenance: return "weekly maintenance";
        case ropufu::settlers_online::battle_skill::master_planner: return "master planner";
        case ropufu::settlers_online::battle_skill::battle_frenzy: return "battle frenzy";
        case ropufu::settlers_online::battle_skill::rapid_fire: return "rapid fire";
        case ropufu::settlers_online::battle_skill::sniper_training: return "sniper training";
        case ropufu::settlers_online::battle_skill::cleave: return "cleave";
        case ropufu::settlers_online::battle_skill::fast_learner: return "fast learner";
        case ropufu::settlers_online::battle_skill::overrun: return "overrun";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_SETTLERS_ONLINE_BATTLE_SKILL_HPP_INCLUDED
