using Newtonsoft.Json;
using System;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c damage.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public struct Damage
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
    }

    /** Mirrors structural behavior of \c camp.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public struct Camp
    {
        [JsonProperty("hit points")]
        private Int32 hitPoints;
        [JsonProperty("damage reduction")]
        private Double damageReduction;

        private void Validate<T>(T dummy)
        {
            if (this.hitPoints < 0) throw new ArgumentOutOfRangeException(nameof(this.hitPoints));
            if (this.damageReduction < 0 || this.damageReduction > 1) throw new ArgumentOutOfRangeException(nameof(this.damageReduction));
        }

        public Camp(Int32 hitPoints, Double damageReduction)
        {
            this.hitPoints = hitPoints;
            this.damageReduction = damageReduction;

            this.Validate(true);
        }

        public Int32 HitPoints { get => this.hitPoints; set => this.Validate(this.hitPoints = value); }
        public Double DamageReduction { get => this.damageReduction; set => this.Validate(this.damageReduction = value); }
    }
}
