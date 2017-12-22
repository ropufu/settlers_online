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

        private static Int32 Load<T>(ICollection<T> units, PrefixDatabase<T> database)
        {
            var count = 0;
            foreach (var unit in units) if (database.Add(unit)) ++count;
            return count;
        }

        private static void LinkAdventureUnits()
        {
            var unitDatabase = UnitDatabase.Instance;
            foreach (var a in AdventureDatabase.Instance.Adventures)
            {
                for (var i = 0; i < a.UnitNames.Count; ++i)
                {
                    var u = default(UnitType);
                    var unitName = a.UnitNames[i];
                    if (!unitDatabase.TryFind(unitName, ref u)) App.Warnings.Push($"Unit {unitName} from adventure {a.Name} not found.");
                    else a.LinkUnitAt(i, u);
                }
            }
        }

        public static Int32 LoadFromFolder(String folderPath)
        {
            var unitDatabase = UnitDatabase.Instance;
            var adventureDatabase = AdventureDatabase.Instance;
            var campDatabase = CampDatabase.Instance;

            var countUnits = 0;
            var countAdventures = 0;
            var countCamps = 0;
            var files = new String[] { };
            try
            {
                files = System.IO.Directory.GetFiles(folderPath);
            }
            catch (System.IO.IOException) { return 0; }
            catch (UnauthorizedAccessException) { return 0; }

            foreach (var p in files)
            {
                try
                {
                    var json = System.IO.File.ReadAllText(p);
                    var map = JsonConvert.DeserializeObject<Map>(json);
                    countUnits += Map.Load(map.units, unitDatabase);
                    countAdventures += Map.Load(map.adventures, adventureDatabase);
                    countCamps += Map.Load(map.camps, campDatabase);
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
            Map.LinkAdventureUnits();
            return countUnits;
        }
    }
}
