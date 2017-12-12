using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    using CharTree = PrefixTree<Char, String>;
    
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class UnitDatabase : PrefixDatabase<UnitType>
    {
        private static readonly UnitDatabase instance = new UnitDatabase();

        public static UnitDatabase Instance => UnitDatabase.instance;

        public static String BuildKey(UnitType unit)
        {
            var key = unit.FirstName;
            foreach (var name in unit.Names) if (name.Length < key.Length) key = name;
            return key;
        }

        public static String BuildPrimaryName(UnitType unit) => unit.FirstName.ToLowerInvariant();

        public static IEnumerable<String> Names(UnitType unit) => unit.Names;

        protected override String OverrideBuildKey(UnitType unit) => UnitDatabase.BuildKey(unit);

        protected override String OverrideBuildPrimaryName(UnitType unit) => UnitDatabase.BuildPrimaryName(unit);

        protected override IEnumerable<String> OverrideNames(UnitType unit) => UnitDatabase.Names(unit);

        public static List<String> BuildAdventureKeys(Adventure a)
        {
            var keys = new List<String>(a.Maps.Count + 1);
            keys.Add(UnitDatabase.RelaxArticles(a.Name));
            foreach (var name in a.Maps) keys.Add(UnitDatabase.RelaxArticles(name));
            return keys;
        }

        private static String RelaxArticles(String value)
        {
            var articles = new String[] { "the", "a", "an" };
            value = value.DeepTrim().ToLowerInvariant();
            foreach (var a in articles)
            {
                var b = a + " ";
                var c = " " + b;
                while (value.StartsWith(b)) value = value.Substring(b.Length);
                value = value.Replace(c, String.Empty);
            }
            return UnitDatabase.RelaxSpelling(value);
        }

        private HashSet<Int32> ids = new HashSet<Int32>();
        private List<Camp> campDatabase = new List<Camp>();
        private List<Adventure> adventureDatabase = new List<Adventure>();

        public UnitDatabase()
        {

        }

        protected override void OnClear()
        {
            this.ids.Clear();
            this.campDatabase.Clear();
            this.adventureDatabase.Clear();
        }

        public IEnumerable<UnitType> Generals => from pair in this.Database where pair.Value.Is(UnitFaction.General) select pair.Value;

        public IEnumerable<Adventure> Adventures => this.adventureDatabase;
        
        public Boolean TryFindAdventure(String query, ref List<UnitType> units)
        {
            if (units.IsNull()) throw new ArgumentNullException(nameof(units));
            // if (units.IsNull()) units = new List<UnitType>();
            if (String.IsNullOrWhiteSpace(query)) return false;

            var adventureNames = query.Split(new Char[] { '+' }, StringSplitOptions.RemoveEmptyEntries);
            var keys = new HashSet<String>();
            foreach (var name in adventureNames) keys.Add(UnitDatabase.RelaxArticles(name));

            var isGood = false;
            foreach (var key in keys)
            {
                foreach (var a in this.adventureDatabase)
                    if (a.Keys.Contains(key))
                    {
                        isGood = true;
                        units.AddRange(a.Units);
                        break;
                    }
            }

            return isGood;
        }

        protected override void OnLoading(UnitType unit, out Boolean doCancel)
        {
            doCancel = false;
            if (this.ids.Contains(unit.Id))
            {
                App.Warnings.Push($"Unit with the same id ({unit.Id}) already loaded.");
                doCancel = true;
            }
        }

        protected override void OnLoaded(UnitType unit)
        {
            this.ids.Add(unit.Id);
        }

        private Int32 LoadUnits(Map map)
        {
            var count = 0;
            foreach (var unit in map.Units)
            {
                unit.Trim();
                if (base.Add(unit)) ++count;
            }
            return count;
        }

        private void LoadCamps(Map map)
        {
            foreach (var c in map.Camps)
            {
                c.Trim();
                var key = UnitDatabase.RelaxToLowercase(c.Names[0]);

                var isGood = true;
                foreach (var existing in this.campDatabase)
                    if (UnitDatabase.RelaxToLowercase(existing.Names[0]) == key)
                    {
                        isGood = false;
                        App.Warnings.Push($"Camp with the same key ({key}) already loaded.");
                        break;
                    }

                if (isGood) this.campDatabase.Add(c);
            }
        }

        private void LoadAdventures(Map map)
        {
            foreach (var a in map.Adventures)
            {
                a.Trim();
                var keys = UnitDatabase.BuildAdventureKeys(a);
                a.Keys.AddRange(keys);

                var isGood = true;
                foreach (var key in keys)
                {
                    foreach (var existing in this.adventureDatabase)
                        if (existing.Keys.Contains(key))
                        {
                            isGood = false;
                            App.Warnings.Push($"Adventure with the same key ({key}) already loaded.");
                            break;
                        }
                }
                if (isGood) this.adventureDatabase.Add(a);
            }
        }

        private void VerifyAdventures()
        {
            foreach (var a in this.adventureDatabase)
            {
                for (var i = 0; i < a.UnitNames.Count; ++i)
                {
                    var u = default(UnitType);
                    var unitName = a.UnitNames[i];
                    if (!this.TryFind(unitName, ref u)) App.Warnings.Push($"Unit {unitName} from adventure {a.Name} not found.");
                    else a.MapAt(i, u);
                }
            }
        }

        public Int32 LoadFromFolder(String folderPath)
        {
            var count = 0;
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
                    count += this.LoadUnits(map);
                    this.LoadCamps(map);
                    this.LoadAdventures(map);
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
            this.VerifyAdventures();
            return count;
        }
    }
}
