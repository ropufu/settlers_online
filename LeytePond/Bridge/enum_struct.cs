using System;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    static class CppEnumParser
    {
        public static String ToCpp(this String csharpValue)
        {
            var builder = new StringBuilder();
            var wasLowercase = false;
            foreach (var c in csharpValue)
            {
                var isUppercase = Char.IsUpper(c);
                if (wasLowercase && isUppercase) builder.Append(' ');
                wasLowercase = !isUppercase;
                builder.Append(Char.ToLowerInvariant(c));
            }
            return builder.ToString().Trim();
        }

        public static String ToCsharp(this String cppValue)
        {
            var builder = new StringBuilder();
            var wasSpace = true;
            foreach (var c in cppValue)
            {
                var isSpace = Char.IsWhiteSpace(c);
                if (!isSpace) builder.Append(wasSpace ? Char.ToUpperInvariant(c) : c);
                wasSpace = isSpace;
            }
            return builder.ToString().Trim();
        }

        public static String ToReadable(this String camelString)
        {
            var builder = new StringBuilder();
            var wasLowercase = false;
            foreach (var c in camelString)
            {
                var isUppercase = Char.IsUpper(c);
                if (wasLowercase && isUppercase) builder.Append(' ');
                wasLowercase = !isUppercase;
                builder.Append(c);
            }
            return builder.ToString().Trim();
        }

        public static String ToReadable<TEnum>(this TEnum e) where TEnum : struct, IConvertible => e.ToString().ToReadable();
        public static TEnum CppParse<TEnum>(this String cppString) where TEnum : struct, IConvertible => (TEnum)Enum.Parse(typeof(TEnum), cppString.ToCsharp());
        public static void CppParse<TEnum>(this String cppString, out TEnum e) where TEnum : struct, IConvertible => e = cppString.CppParse<TEnum>();
    }

    /** Mirrors \c special_ability.hpp. */
    public enum SpecialAbility
    {
        None = 0,
        AttackWeakestTarget = 1, // When attacking enemies this unit will sort them by \c hit_points instead of \c id.
        NotWeak = 2,             // This unit will be not be affected by attacker's \c do_attack_weakest_target, if any.
        TowerBonus = 3,          // This unit gets damage reduction in towers.
        IgnoreTowerBonus = 4,    // When attacking, inflicted damage will not be affected by defenders' possible \c tower_bonus.
        RapidFire = 5,           // Indicates that this unis is affected by friendly skill \c battle_skill::rapid_fire.
        SniperTraining = 6,      // Indicates that this unis is affected by friendly skill \c battle_skill::sniper_training.
        Cleave = 7,              // Indicates that this unis is affected by friendly skill \c battle_skill::cleave.
        Overrun = 8              // Indicates that this unis is affected by enemy skill \c battle_skill::overrun.
    }

    /** Mirrors \c battle_trait.hpp. */
    public enum BattleTrait
    {
        None = 0,
        Dazzle = 1,             // Enemy accuracy is reduced to 0%.
        Intercept = 2,          // Enemy units deal 5% less damage and their ability \c do_attack_weakest_target is ignored.
        ExplosiveAmmunition = 3 // Ranged units get \c do_attack_weakest_target and 100\% \c splash_chance.
    }

    /** Mirrors \c battle_phase.hpp. */
    public enum BattlePhase
    {
        FirstStrike = 0,
        Normal = 1,
        LastStrike = 2
    }

    /** Mirrors \c battle_skill.hpp. */
    public enum BattleSkill
    {
        None = 0,
        Juggernaut = 1,        // Increases the general's (faction: general) attack damage by 20/40/60. These attacks have a 33/66/100% chance of dealing splash damage.
        GarrisonAnnex = 2,     // Increases the unit capacity (faction: general) by 5/10/15.
        LightningSlash = 3,    // The general (faction: general) attacks twice per round. That second attack's initiative is \c last_strike.
        UnstoppableCharge = 4, // Increases the maximum attack damage of your swift units (faction: cavalry) by 1/2/3 and their attacks have a 33/66/100% chance of dealing splash damage.
        WeeklyMaintenance = 5, // Increases the attack damage of your heavy units (faction: artillery) by 10/20/30.
        MasterPlanner = 6,     // Adds 10% to this army's accuracy.
        BattleFrenzy = 7,      // Increases the attack damage of this army by 10/20/30% for every combat round past the first.
        RapidFire = 8,         // Increases the maximum attack damage of your Bowmen by 5/10/15.
        SniperTraining = 9,    // Increases your Longbowmen's and regular Marksmen's minimum attack damage by 45/85/130% and the maximum by 5/10/15%.
        Cleave = 10,           // Increases the attack damage of Elite Soldiers by 4/8/12 and their attacks have a 33/66/100% chance of dealing splash damage.
        FastLearner = 11,      // Increases the XP gained from enemy units defeated by this army by 10/20/30%.
        Overrun = 12           // Decreases the HP of enemy bosses by 8/16/25%.
    }

    /** Mirrors \c unit_faction.hpp. */
    public enum UnitFaction
    {
        NonPlayerAdventure = 0,  // All adventure enemy units.
        NonPlayerExpedition = 1, // All expedition enemy units.
        General = 2,    // Generals / champions.
        Expedition = 3, // Combat academy (expedition) units.
        Common = 4,     // Common barracks units.
        Elite = 5       // Elite barracks units.
    }

    /** Mirrors \c unit_category.hpp. */
    public enum UnitCategory
    {
        Unknown = 0,
        Melee = 1,
        Ranged = 2,
        Cavalry = 3,
        Artillery = 4,
        Elite = 5
    }
}
