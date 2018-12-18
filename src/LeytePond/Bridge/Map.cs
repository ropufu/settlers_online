using Newtonsoft.Json;
using System;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    // @todo Mirror in c++.
    [JsonObject(MemberSerialization.OptIn)]
    public class Map
    {
        [JsonProperty("units")]
        private List<UnitType> units = new List<UnitType>();
        [JsonProperty("camps")]
        private List<Camp> camps = new List<Camp>();
        [JsonProperty("adventures")]
        private List<Adventure> adventures = new List<Adventure>();

        private UnitDatabase unitDatabase = new UnitDatabase();
        private CampDatabase campDatabase = new CampDatabase();
        private AdventureDatabase adventureDatabase = new AdventureDatabase();

        //public ICollection<UnitType> Units { get => this.units.AsReadOnly(); }
        //public ICollection<Camp> Camps { get => this.camps.AsReadOnly(); }
        //public ICollection<Adventure> Adventures { get => this.adventures.AsReadOnly(); }

        public UnitDatabase Units => this.unitDatabase;
        public CampDatabase Camps => this.campDatabase;
        public AdventureDatabase Adventures => this.adventureDatabase;

        private static Int32 Load<T, TKey>(ICollection<T> units, NameDatabase<T, TKey> database)
            where TKey : IEquatable<TKey>
        {
            var count = 0;
            foreach (var unit in units) if (database.Add(unit)) ++count;
            return count;
        }

        private void LinkAdventureUnits()
        {
            foreach (var a in this.adventureDatabase.All)
            {
                for (var i = 0; i < a.UnitNames.Count; ++i)
                {
                    var unitName = a.UnitNames[i];
                    var u = this.unitDatabase.Find(unitName, null);
                    //if (!this.unitDatabase.TryFind(unitName, ref u, null))
                    if (u == default(UnitType)) App.Warnings.Push($"Unit {unitName} from adventure {a.Name} not found.");
                    else a.LinkUnitAt(i, u);
                }
            }
        }

        public static Map LoadFromFolder(String folderPath)
        {
            var result = new Map();

            var countUnits = 0;
            var countAdventures = 0;
            var countCamps = 0;
            var files = new String[] { };
            try
            {
                files = System.IO.Directory.GetFiles(folderPath);
            }
            catch (System.IO.IOException) { return result; }
            catch (UnauthorizedAccessException) { return result; }

            foreach (var p in files)
            {
                try
                {
                    var json = System.IO.File.ReadAllText(p);
                    var map = JsonConvert.DeserializeObject<Map>(json);
                    countUnits += Map.Load(map.units, result.unitDatabase);
                    countAdventures += Map.Load(map.adventures, result.adventureDatabase);
                    countCamps += Map.Load(map.camps, result.campDatabase);
                }
                catch (JsonReaderException)
                {
                    App.Warnings.Push($"Error while parsing file ({p}).");
                }
                catch (ArgumentException)
                {
                    App.Warnings.Push($"Error while parsing <UnitType> in file ({p}).");
                }
                catch (System.IO.IOException)
                {
                    App.Warnings.Push($"Error reading file ({p}).");
                }
                catch (System.Security.SecurityException)
                {
                    App.Warnings.Push($"Security error reading file ({p}).");
                }
                catch (UnauthorizedAccessException)
                {
                    App.Warnings.Push($"Authorization error reading file ({p}).");
                }
            }
            result.LinkAdventureUnits();
            return result;
        }
    }
}
