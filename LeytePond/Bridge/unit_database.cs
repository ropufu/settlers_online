using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
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
        private Lookup<String, String> lowercaseLookup = new Lookup<String, String>();
        private Lookup<String, String> misspelledLookup = new Lookup<String, String>();

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
            this.lowercaseLookup.Clear();
            this.misspelledLookup.Clear();
        }

        public IEnumerable<UnitType> Units { get => this.database.Values; }

        public UnitType this[String key] { get => this.database[key]; }

        public Boolean TryFind(String query, ref UnitType unit, Func<UnitType, Boolean> filter = null)
        {
            if (Object.ReferenceEquals(filter, null)) filter = u => true;
            if (this.database.TryGetValue(query, out unit)) if (filter(unit)) return true;

            var key = String.Empty;
            var lowercase = UnitDatabase.RelaxToLowercase(query);
            var misspelled = UnitDatabase.RelaxSpelling(lowercase);
            var countMatches = 0;

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
                    foreach (var u in map.Units)
                    {
                        u.Trim();
                        var key = UnitDatabase.BuildKey(u);
                        if (this.database.ContainsKey(key))
                        {
                            App.Warnings.Push($"Unit with the same name ({key}) already loaded.");
                        }
                        else if (this.ids.Contains(u.Id))
                        {
                            App.Warnings.Push($"Unit with the same id ({u.Id}) already loaded.");
                        }
                        else
                        {
                            this.database.Add(key, u);
                            this.ids.Add(u.Id);
                            this.UpdateLookup(key, u);
                            count++;
                        }
                    }
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
            return count;
        }
    }
}
