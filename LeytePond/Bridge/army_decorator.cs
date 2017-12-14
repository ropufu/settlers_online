using System;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.ComponentModel;
using System.Collections;
using System.Collections.Specialized;

namespace Ropufu.LeytePond.Bridge
{
    public class SkillMap : IEnumerable<KeyValuePair<String, EnumArray<BattleSkill, Int32>>>, INotifyCollectionChanged
    {
        private Dictionary<String, EnumArray<BattleSkill, Int32>> map = new Dictionary<String, EnumArray<BattleSkill, Int32>>();

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        public SkillMap()
        {

        }

        public Int32 Count => this.map.Count;

        public Boolean Contains(String key) => this.map.ContainsKey(key);

        public Boolean TryGetValue(String key, out EnumArray<BattleSkill, Int32> skills) => this.map.TryGetValue(key, out skills);

        public void Add(String key, EnumArray<BattleSkill, Int32> skills)
        {
            this.map.Add(key, skills);
            this.CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
            skills.CollectionChanged += (s, e) => this.CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }

        public EnumArray<BattleSkill, Int32> this[String key]
        {
            get
            {
                var skills = default(EnumArray<BattleSkill, Int32>);
                if (this.TryGetValue(key, out skills)) return skills;
                // Create new entry.
                skills = new EnumArray<BattleSkill, Int32>();
                this.Add(key, skills);
                return skills;
            }
        }

        public IEnumerator<KeyValuePair<String, EnumArray<BattleSkill, Int32>>> GetEnumerator() => this.map.GetEnumerator();

        IEnumerator IEnumerable.GetEnumerator() => this.GetEnumerator();
    }

    /** Mirrors structural behavior of \c army_decorator.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class ArmyDecorator : INotifyPropertyChanged
    {
        private Boolean isListening = false;

        [JsonProperty("camp"), JsonConverter(typeof(JsonCampConverter))]
        private Camp camp = new Camp();
        [JsonProperty("skills"), JsonConverter(typeof(JsonSkillMapConverter))]
        private SkillMap skills = new SkillMap();

        public event PropertyChangedEventHandler PropertyChanged;

        public void Listen()
        {
            if (this.isListening) throw new NotSupportedException();
            this.isListening = true;
            this.skills.CollectionChanged += (s, e) => this.PropertyChanged?.Invoke(s, new PropertyChangedEventArgs(nameof(this.Skills)));
        }

        public Camp Camp
        {
            get => this.camp;
            set
            {
                this.camp = value;
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Camp)));
            }
        }

        public SkillMap Skills => this.skills;

        public void Decorate(Army a, Warnings warnings)
        {
            a.Camp = this.camp; // Overwrite camp.
            a.Skills.Clear(); // Reset skills.

            var prefix = System.Environment.NewLine + "\t  |---- ";
            foreach (var pair in this.skills)
            {
                var isMatch = false;
                foreach (var g in a) if (g.Count > 0) foreach (var name in g.Unit.Names) if (name == pair.Key) isMatch = true;
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
