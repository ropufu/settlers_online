using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    using CharTree = PrefixTree<Char, String>;

    /** Mirrors structural behavior of \c unit_database.hpp. */
    class Lookup<TKey, TSkeleton>
    {
        private Dictionary<TKey, HashSet<TSkeleton>> skeleton = new Dictionary<TKey, HashSet<TSkeleton>>();
        private Dictionary<TSkeleton, HashSet<TKey>> inverseSkeleton = new Dictionary<TSkeleton, HashSet<TKey>>();

        public void Clear()
        {
            this.skeleton.Clear();
            this.inverseSkeleton.Clear();
        }

        public void Update(TKey key, HashSet<TSkeleton> values)
        {
            this.skeleton.Add(key, values);

            foreach (TSkeleton weak in values)
            {
                var matches = default(HashSet<TKey>);
                if (this.inverseSkeleton.TryGetValue(weak, out matches)) matches.Add(key);
                else
                {
                    matches = new HashSet<TKey>{ key };
                    this.inverseSkeleton.Add(weak, matches);
                }
            }
        }

        public Int32 TryFind(TSkeleton query, ref TKey key, Func<TKey, Boolean> filter)
        {
            var matches = default(HashSet<TKey>);
            if (this.inverseSkeleton.TryGetValue(query, out matches))
            {
                var count = 0;
                foreach (var maybe in matches)
                {
                    if (filter(maybe))
                    {
                        key = maybe;
                        count++;
                    }
                }
                return count;
            }
            return 0;
        }
    }
    
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class UnitDatabase
    {
        private static readonly UnitDatabase instance = new UnitDatabase();

        public static UnitDatabase Instance => UnitDatabase.instance;

        public static String BuildKey(UnitType unit)
        {
            var key = unit.FirstName;
            foreach (var name in unit.Names) if (name.Length < key.Length) key = name;
            return key;
        }

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

        public static String BuildPrimaryName(UnitType unit) => unit.FirstName.ToLowerInvariant();

        private static String RelaxToLowercase(String value) => value.ToLowerInvariant();
        private static String RelaxSpelling(String value)
        {
            var relaxed = value.Replace("men", "man");
            if (relaxed.EndsWith("es")) relaxed = relaxed.Substring(0, relaxed.Length - 2);
            else if (relaxed.EndsWith("s")) relaxed = relaxed.Substring(0, relaxed.Length - 1);

            if (relaxed.Length > 4)
            {
                var builder = new StringBuilder();
                var previous = relaxed[0];
                builder.Append(previous);
                foreach (var c in relaxed) if (c != previous) { builder.Append(c); previous = c; }
                relaxed = builder.ToString();
            }

            return relaxed;
        }

        private Dictionary<String, UnitType> database = new Dictionary<String, UnitType>();
        private HashSet<Int32> ids = new HashSet<Int32>();
        private HashSet<String> primaryNames = new HashSet<String>();
        private Lookup<String, String> lowercaseLookup = new Lookup<String, String>();
        private Lookup<String, String> misspelledLookup = new Lookup<String, String>();
        private CharTree primaryNameTree = new CharTree();
        // @todo Move the following to another database class(es), and rewrite database for generic (prefix) search.
        private List<Camp> campDatabase = new List<Camp>();
        private List<Adventure> adventureDatabase = new List<Adventure>();


        public UnitDatabase()
        {

        }

        private void UpdateLookup(String key, UnitType unit)
        {
            var relaxedNames = new HashSet<String>();
            var misspelledNames = new HashSet<String>();
            foreach (var name in unit.Names)
            {
                var stage1 = UnitDatabase.RelaxToLowercase(name);
                var stage2 = UnitDatabase.RelaxSpelling(stage1);
                relaxedNames.Add(stage1);
                misspelledNames.Add(stage2);
            }
            this.lowercaseLookup.Update(key, relaxedNames);
            this.misspelledLookup.Update(key, misspelledNames);
        }

        public void Clear()
        {
            this.database.Clear();
            this.ids.Clear();
            this.primaryNames.Clear();
            this.lowercaseLookup.Clear();
            this.misspelledLookup.Clear();
            this.primaryNameTree.Clear();

            this.campDatabase.Clear();
            this.adventureDatabase.Clear();
        }

        public IEnumerable<UnitType> Generals => from pair in this.database where pair.Value.Is(UnitFaction.General) select pair.Value;

        public Int32 Count => this.database.Count;

        public IEnumerable<UnitType> Units => this.database.Values;

        public IEnumerable<Adventure> Adventures => this.adventureDatabase;

        public UnitType this[String key] => this.database[key];

        public Boolean TryFind(String query, ref UnitType unit, Func<UnitType, Boolean> filter = null)
        {
            if (filter.IsNull()) filter = u => true;
            if (this.database.TryGetValue(query, out unit)) if (filter(unit)) return true;

            var key = String.Empty;
            var lowercase = UnitDatabase.RelaxToLowercase(query);
            var misspelled = UnitDatabase.RelaxSpelling(lowercase);
            var countMatches = 0;

            var isSingle = false;
            var firstPrefixMatch = this.primaryNameTree.First(lowercase, out isSingle);
            if (isSingle) lowercase = firstPrefixMatch;

            countMatches = this.lowercaseLookup.TryFind(lowercase, ref key, maybe => filter(this.database[maybe]));
            if (countMatches >= 1)
            {
                unit = this.database[key];
                if (countMatches == 1) return true;
                App.Warnings.Push($"Multiple units ({countMatches}) match the specified query ({query}).");
                return false;
            }

            countMatches = this.misspelledLookup.TryFind(misspelled, ref key, maybe => filter(this.database[maybe]));
            if (countMatches >= 1)
            {
                unit = this.database[key];
                if (countMatches == 1) return true;
                App.Warnings.Push($"Multiple units ({countMatches}) match the specified query ({query}).");
                return false;
            }

            return false;
        }
        
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

        private Int32 LoadUnits(Map map)
        {
            var count = 0;
            foreach (var u in map.Units)
            {
                u.Trim();
                var key = UnitDatabase.BuildKey(u);
                var primaryName = UnitDatabase.BuildPrimaryName(u);
                if (this.database.ContainsKey(key))
                {
                    App.Warnings.Push($"Unit with the same key ({key}) already loaded.");
                }
                else if (this.ids.Contains(u.Id))
                {
                    App.Warnings.Push($"Unit with the same id ({u.Id}) already loaded.");
                }
                else if (this.primaryNames.Contains(primaryName))
                {
                    App.Warnings.Push($"Unit with a similar primary name ({u.FirstName}) already loaded.");
                }
                else
                {
                    this.database.Add(key, u);
                    this.ids.Add(u.Id);
                    this.primaryNames.Add(primaryName);
                    this.UpdateLookup(key, u);
                    this.primaryNameTree.Add(primaryName);
                    count++;
                }
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
            //this.primaryNameTree.Reduce();
            this.VerifyAdventures();
            return count;
        }
    }
}
