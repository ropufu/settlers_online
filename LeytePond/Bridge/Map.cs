using Newtonsoft.Json;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    [JsonObject(MemberSerialization.OptIn)]
    class Map
    {
        [JsonProperty("units")]
        private List<UnitType> units = new List<UnitType>();

        public List<UnitType> Units { get => this.units; }
    }
}
