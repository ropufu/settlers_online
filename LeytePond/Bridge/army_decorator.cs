using System;
using Newtonsoft.Json;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c army_decorator.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class ArmyDecorator
    {
        [JsonProperty("camp"), JsonConverter(typeof(JsonCampConverter))]
        private Camp camp = new Camp();
        [JsonProperty("skills"), JsonConverter(typeof(JsonSkillMapConverter))]
        private Dictionary<String, EnumArray<BattleSkill, Int32>> skills = new Dictionary<String, EnumArray<BattleSkill, Int32>>();

        public Camp Camp { get => this.camp; set => this.camp = value; }
        public Dictionary<String, EnumArray<BattleSkill, Int32>> Skills { get => this.skills; }

        public void Decorate(Army a, Warnings warnings)
        {
            a.Camp = this.camp; // Overwrite camp.
            a.Skills.Initialize(); // Reset skills.

            var prefix = System.Environment.NewLine + "\t  |---- ";
            foreach (var pair in this.skills)
            {
                var isMatch = false;
                foreach (var g in a.Groups) foreach (var name in g.Unit.Names) if (name == pair.Key) isMatch = true;
                if (!isMatch) continue;

                var skillStrings = new List<String>();
                foreach (var skillPair in pair.Value)
                {
                    if (skillPair.Value == 0) continue;
                    skillStrings.Add($"{skillPair.Key.ToString().ToCpp()} ({skillPair.Value})");
                    a.Skills[skillPair.Key] = skillPair.Value;
                }
                if (skillStrings.Count > 0) warnings.Push($"Applying skills: {String.Join(prefix, skillStrings)}.");
            }
        }
    }
}
