using Newtonsoft.Json;
using System;
using System.Linq;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c unit_type.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class UnitType : IComparable<UnitType>, IEquatable<UnitType>
    {
        [JsonProperty("id", Required = Required.Always)]
        private Int32 id = 0;
        [JsonProperty("names", Required = Required.Always)]
        private List<String> names = new List<String>();
        [JsonProperty("codenames")]
        private List<String> codenames = new List<String>();
        [JsonProperty("faction"), JsonConverter(typeof(JsonCppEnumConverter<UnitFaction>))]
        private UnitFaction faction = UnitFaction.NonPlayerAdventure;
        [JsonProperty("category"), JsonConverter(typeof(JsonCppEnumConverter<UnitCategory>))]
        private UnitCategory category = UnitCategory.Unknown;
        [JsonProperty("capacity")]
        private Int32 capacity = 0;
        [JsonProperty("hit points", Required = Required.Always)]
        private Int32 hitPoints = 0;
        [JsonProperty("damage", Required = Required.Always)]
        private Damage damage = new Damage();
        [JsonProperty("experience when killed")]
        private Int32 experience = 0;
        [JsonProperty("phases", Required = Required.Always), JsonConverter(typeof(JsonCppFlagsConverter<BattlePhase>))]
        private List<BattlePhase> attackPhases = new List<BattlePhase>();
        [JsonProperty("special abilities"), JsonConverter(typeof(JsonCppFlagsConverter<SpecialAbility>))]
        private List<SpecialAbility> abilities = new List<SpecialAbility>();
        [JsonProperty("traits"), JsonConverter(typeof(JsonCppFlagsConverter<BattleTrait>))]
        private List<BattleTrait> traits = new List<BattleTrait>();

        public Int32 CompareTo(UnitType other) => this.id - other.id;

        public UnitType() { }

        public void Trim()
        {
            var trimmedNames = new List<String>();
            foreach (var name in this.names)
            {
                var trimmed = name.DeepTrim();
                if (!String.IsNullOrEmpty(trimmed)) trimmedNames.Add(trimmed);
            }
            if (trimmedNames.Count == 0) trimmedNames.Add(String.Empty);
            this.names = trimmedNames;
        }

        public Int32 Id { get => this.id; set => this.id = value; }
        public List<String> Names { get => this.names; }
        public List<String> Codenames { get => this.codenames; }
        public Int32 Experience { get => this.experience; set => this.experience = value; }
        public Int32 Capacity { get => this.capacity; set => this.capacity = value; }
        public Int32 HitPoints { get => this.hitPoints; set => this.hitPoints = value; }
        public Damage Damage { get => this.damage; set => this.damage = value; }
        public List<BattlePhase> AttackPhases { get => this.attackPhases; }
        public List<SpecialAbility> Abilities { get => this.abilities; }
        public List<BattleTrait> Traits { get => this.traits; }
        public UnitFaction Faction { get => this.faction; set => this.faction = value; }
        public UnitCategory Category { get => this.category; set => this.category = value; }
        
        public String FirstName => this.names.Count == 0 ? "??" : this.names[0];

        public Boolean Is(UnitFaction faction) => this.faction == faction;

        public override String ToString() => this.FirstName;

        public override Int32 GetHashCode() => this.id;

        public String NamesString => String.Join(", ", this.names);

        public Boolean HasAbilities => this.abilities.Count > 0;
        public Boolean HasTraits => this.traits.Count > 0;

        public String AttackPhasesString => String.Join(", ", from x in this.attackPhases select x.ToReadable());
        public String AbilitiesString => String.Join(", ", from x in this.abilities select x.ToReadable());
        public String TraitsString => String.Join(", ", from x in this.traits select x.ToReadable());

        // @warning Does not perform null reference checks.
        private Boolean EqualsUnchecked(UnitType other) => (this.id == other.id);

        public override Boolean Equals(Object obj)
        {
            // Check for null and compare run-time types.
            if (Object.ReferenceEquals(obj, null) || !this.GetType().Equals(obj.GetType())) return false;
            else return this.EqualsUnchecked((UnitType)obj);
        }

        public Boolean Equals(UnitType other)
        {
            // Check for null.
            if (Object.ReferenceEquals(other, null)) return false;
            else return this.EqualsUnchecked(other);
        }

        public static Boolean operator ==(UnitType x, UnitType y)
        {
            // Check x for null.
            if (Object.ReferenceEquals(x, null)) return Object.ReferenceEquals(y, null);
            // Know that x is not null.
            return x.Equals(y);
        }

        public static Boolean operator !=(UnitType x, UnitType y) => !(x == y);
    }
}
