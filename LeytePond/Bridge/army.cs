using System;
using System.Linq;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c army.hpp. */
    public class Army
    {
        private List<UnitGroup> groups = new List<UnitGroup>();
        private Camp camp = new Camp();
        private Double frenzyBonus = 0;
        private EnumArray<BattleSkill, Int32> skills = new EnumArray<BattleSkill, Int32>();
        private List<BattleTrait> traits = new List<BattleTrait>();
        
        private void Validate<T>(T dummy)
        {
            if (this.frenzyBonus < 0) throw new ArgumentOutOfRangeException(nameof(this.frenzyBonus));
            if (Object.ReferenceEquals(this.camp, null)) throw new ArgumentOutOfRangeException(nameof(this.camp));
            if (Object.ReferenceEquals(this.groups, null)) throw new ArgumentOutOfRangeException(nameof(this.groups));
        }

        public Army() { }

        public Army(IList<UnitGroup> groups)
        {
            if (Object.ReferenceEquals(groups, null)) throw new ArgumentNullException(nameof(groups));

            var types = new HashSet<UnitType>();
            this.groups = new List<UnitGroup>(groups.Count);
            foreach (var g in groups)
            {
                if (!types.Add(g.Unit))
                {
                    this.groups.Clear();
                    return;
                }
                this.groups.Add(g);
            }
            this.groups.Sort();
        }
        
        private Boolean Has(Func<UnitType, Boolean> filter)
        {
            foreach (var g in this.groups) if (filter(g.Unit)) return true;
            return false;
        }

        public Boolean Has(UnitFaction faction) => this.Has(u => u.Faction == faction);
        public Boolean Has(UnitCategory category) => this.Has(u => u.Category == category);
        public Boolean Has(BattleTrait trait) => this.traits.Contains(trait);

        public Camp Camp { get => this.camp; set => this.Validate(this.camp = value); }
        public Double FrenzyBonus { get => this.frenzyBonus; set => this.Validate(this.frenzyBonus = value); }
        public EnumArray<BattleSkill, Int32> Skills { get => this.skills; }
        public List<BattleTrait> Traits { get => this.traits; }

        public List<UnitGroup> Groups { get => this.groups; }
        public IEnumerable<UnitType> Types { get => from g in this.groups select g.Unit; }
        public IEnumerable<Int32> CountsByType { get => from g in this.groups select g.Count; }

        public Boolean IsEmpty => this.groups.Count == 0;
        public Int32 CountGroups { get => this.groups.Count; }
        public Int32 CountUnits
        {
            get
            {
                var count = 0;
                foreach (var g in this.groups) count += g.Count;
                return count;
            }
        }

        public String ToString(Func<UnitType, String> format)
        {
            if (this.groups.Count == 0) return "empty";
            var groupStrings = new List<String>(this.groups.Count);
            foreach (var g in this.groups) groupStrings.Add($"{g.Count}{format(g.Unit)}");
            return String.Join(" ", groupStrings);
        }

        public String ToCompactString() => this.ToString(u => UnitDatabase.BuildKey(u));

        public override String ToString() => this.ToString(u => " " + u.FirstName);

        public override Int32 GetHashCode()
        {
            return this.groups.GetHashCode();
        }
    }
}
