using Newtonsoft.Json;
using System;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c damage.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public struct Damage : IEquatable<Damage>
    {
        [JsonProperty("low", Required = Required.Always)]
        private Int32 low;
        [JsonProperty("high", Required = Required.Always)]
        private Int32 high;
        [JsonProperty("accuracy", Required = Required.Always)]
        private Double accuracy;
        [JsonProperty("splash chance")]
        private Double splashChance;

        private void Validate<T>(T dummy)
        {
            if (this.low < 0) throw new ArgumentOutOfRangeException(nameof(this.low));
            if (this.high < 0) throw new ArgumentOutOfRangeException(nameof(this.high));
            if (this.low > this.high) throw new ArgumentOutOfRangeException();
            if (this.accuracy < 0 || this.accuracy > 1) throw new ArgumentOutOfRangeException(nameof(this.accuracy));
            if (this.splashChance < 0 || this.splashChance > 1) throw new ArgumentOutOfRangeException(nameof(this.splashChance));
        }

        public Damage(Int32 lowDamage, Int32 highDamage, Double accuracy, Double splashChance)
        {
            this.low = lowDamage;
            this.high = highDamage;
            this.accuracy = accuracy;
            this.splashChance = splashChance;

            this.Validate(true);
        }

        public Int32 Low { get => this.low; set => this.Validate(this.low = value); }
        public Int32 High { get => this.high; set => this.Validate(this.high = value); }
        public Double Accuracy { get => this.accuracy; set => this.Validate(this.accuracy = value); }
        public Double SplashChance { get => this.splashChance; set => this.Validate(this.splashChance = value); }

        public override Boolean Equals(Object obj) => (obj is Damage) && this.Equals((Damage)obj);

        public Boolean Equals(Damage other) => this == other;

        public static Boolean operator ==(Damage x, Damage y) =>
            (x.low == y.low) &&
            (x.high == y.high) &&
            (x.accuracy == y.accuracy) &&
            (x.splashChance == y.splashChance);

        public static Boolean operator !=(Damage x, Damage y) => !(x == y);

        public override Int32 GetHashCode() => 
            this.low.GetHashCode() ^
            this.high.GetHashCode() ^
            this.accuracy.GetHashCode() ^
            this.splashChance.GetHashCode();

        public override String ToString() => $"{this.low}--{this.high} ({this.accuracy}), splash: {this.splashChance}";
    }

    /** Mirrors structural behavior of \c camp.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class Camp : IEquatable<Camp>
    {
        [JsonProperty("hit points")]
        private Int32 hitPoints = 0;
        [JsonProperty("damage reduction")]
        private Double damageReduction = 0;
        [JsonProperty("names")]
        private List<String> names = new List<String>();

        private void Validate<T>(T dummy)
        {
            if (this.hitPoints < 0) throw new ArgumentOutOfRangeException(nameof(this.hitPoints));
            if (this.damageReduction < 0 || this.damageReduction > 1) throw new ArgumentOutOfRangeException(nameof(this.damageReduction));
        }

        public Camp() { }

        public Camp(Int32 hitPoints, Double damageReduction)
        {
            this.hitPoints = hitPoints;
            this.damageReduction = damageReduction;
            this.names = new List<String>();

            this.Validate(true);
        }

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

        public Int32 HitPoints { get => this.hitPoints; set => this.Validate(this.hitPoints = value); }
        public Double DamageReduction { get => this.damageReduction; set => this.Validate(this.damageReduction = value); }
        public IList<String> Names => this.names.AsReadOnly();

        // @warning Does not perform null reference checks.
        private Boolean EqualsUnchecked(Camp other) => (this.hitPoints == other.hitPoints) && (this.damageReduction == other.damageReduction);

        public override Boolean Equals(Object obj)
        {
            // Check for null and compare run-time types.
            if (Object.ReferenceEquals(obj, null) || !this.GetType().Equals(obj.GetType())) return false;
            else return this.EqualsUnchecked((Camp)obj);
        }

        public Boolean Equals(Camp other)
        {
            // Check for null.
            if (Object.ReferenceEquals(other, null)) return false;
            else return this.EqualsUnchecked(other);
        }

        public static Boolean operator ==(Camp x, Camp y)
        {
            // Check x for null.
            if (Object.ReferenceEquals(x, null)) return Object.ReferenceEquals(y, null);
            // Know that x is not null.
            return x.Equals(y);
        }

        public static Boolean operator !=(Camp x, Camp y) => !(x == y);

        public override Int32 GetHashCode() => this.hitPoints.GetHashCode() ^ this.damageReduction.GetHashCode();

        public override String ToString() => $"hp: {this.hitPoints}, red: {this.damageReduction}";
    }
}
