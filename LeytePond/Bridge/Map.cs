using Newtonsoft.Json;
using System;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    [JsonObject(MemberSerialization.OptIn)]
    class Map
    {
        [JsonProperty("units")]
        private List<UnitType> units = new List<UnitType>();
        [JsonProperty("camps")]
        private List<Camp> camps = new List<Camp>();
        [JsonProperty("adventures")]
        private List<Adventure> adventures = new List<Adventure>();

        public ICollection<UnitType> Units { get => this.units.AsReadOnly(); }
        public ICollection<Camp> Camps { get => this.camps.AsReadOnly(); }
        public ICollection<Adventure> Adventures { get => this.adventures.AsReadOnly(); }
    }

    /** Mirrors structural behavior of \c camp.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class Adventure
    {
        [JsonProperty("name")]
        private String name = String.Empty;
        [JsonProperty("maps")]
        private List<String> maps = new List<String>();
        [JsonProperty("units")]
        private List<String> unitNames = new List<String>();

        private List<String> keys = new List<String>();
        private List<UnitType> units = new List<UnitType>();

        public Adventure(String name)
        {
            this.name = name;
            this.maps = new List<String>();
            this.unitNames = new List<String>();
        }

        public void Trim()
        {
            this.name = this.name.DeepTrim();
            if (this.maps.IsNull()) this.maps = new List<String>();
            for (var i = 0; i < this.maps.Count; i++) this.maps[i] = this.maps[i].DeepTrim();
        }

        public void MapAt(Int32 index, UnitType unit)
        {
            if (index < 0 || index > this.unitNames.Count) throw new ArgumentOutOfRangeException(nameof(index));
            if (index > this.units.Count) throw new NotSupportedException("Previous indices have to be mapped first.");
            if (unit.IsNull()) throw new ArgumentNullException(nameof(unit));

            if (index == this.units.Count) this.units.Add(unit);
            else if (this.units[index] != unit) App.Warnings.Push($"Unit {this.units[index].FirstName} from adventure {this.name} has already been mapped.");
        }

        public Boolean Has(UnitType unit) => this.units.Contains(unit);

        public String Name { get => this.name; set => this.name = value; }
        public IList<String> Maps => this.maps;
        public IList<String> UnitNames => this.unitNames;

        public String MapsString => String.Join(", ", this.maps);

        public List<String> Keys => this.keys;
        public IList<UnitType> Units => this.units;
    }
}
