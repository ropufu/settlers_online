using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c adventure.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class Adventure
    {
        [JsonProperty("name")]
        private String name = String.Empty;
        [JsonProperty("description")]
        private String description = String.Empty;
        [JsonProperty("is player")]
        private Boolean isSuggestedAsPlayer = false;
        [JsonProperty("maps")]
        private List<String> maps = new List<String>();
        [JsonProperty("units")]
        private List<String> unitNames = new List<String>();

        private List<String> keys = null;
        private List<UnitType> units = null;

        public Adventure(String name)
        {
            this.name = name;
            this.maps = new List<String>();
            this.unitNames = new List<String>();
        }

        public void BuildKeys()
        {
            this.keys = new List<String>(this.maps.Count + 1)
            {
                this.name.RelaxCase().RelaxArticles()
            };
            foreach (var name in this.maps) keys.Add(name.RelaxCase().RelaxArticles());
        }

        public void Trim()
        {
            this.name = this.name.DeepTrim();
            if (this.maps.IsNull()) this.maps = new List<String>();

            for (var i = 0; i < this.maps.Count; ++i) this.maps[i] = this.maps[i].DeepTrim();
        }

        public void LinkUnitAt(Int32 index, UnitType unit)
        {
            if (index < 0 || index > this.unitNames.Count) throw new ArgumentOutOfRangeException(nameof(index));
            if (this.units.IsNull())
            {
                if (index != 0) throw new NotSupportedException("Previous indices have to be mapped first.");
                this.units = new List<UnitType>();
            }
            if (index > this.units.Count) throw new NotSupportedException("Previous indices have to be mapped first.");
            if (unit.IsNull()) throw new ArgumentNullException(nameof(unit));

            if (index == this.units.Count) this.units.Add(unit);
            else if (this.units[index] != unit) App.Warnings.Push($"Unit {this.units[index].FirstName} from adventure {this.name} has already been mapped.");
        }

        public Boolean Has(UnitType unit) => this.units.Contains(unit);

        public String Name { get => this.name; set => this.name = value; }
        public Boolean IsSuggestedAsPlayer { get => this.isSuggestedAsPlayer; set => this.isSuggestedAsPlayer = value; }
        public IList<String> Maps => this.maps;
        public IList<String> UnitNames => this.unitNames;

        public String MapsString => String.Join(", ", this.maps);

        public String Description => String.IsNullOrWhiteSpace(this.description) ? this.MapsString : this.description;

        public List<String> Keys => this.keys;
        public IList<UnitType> Units => this.units;
    }
}
