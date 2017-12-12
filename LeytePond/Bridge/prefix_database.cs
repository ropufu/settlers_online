using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    using CharTree = PrefixTree<Char, String>;

    /** Mirrors structural behavior of \c prefix_database.hpp. */
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

    /** Mirrors structural behavior of \c prefix_database.hpp. */
    public abstract class PrefixDatabase<T>
    {
        protected abstract String OverrideBuildKey(T unit);

        protected abstract String OverrideBuildPrimaryName(T unit);

        protected abstract IEnumerable<String> OverrideNames(T unit);

        protected static String RelaxToLowercase(String value) => value.ToLowerInvariant();

        protected static String RelaxSpelling(String value)
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

        private Dictionary<String, T> database = new Dictionary<String, T>();
        private HashSet<String> primaryNames = new HashSet<String>();
        private CharTree primaryNameTree = new CharTree();
        private Lookup<String, String> lowercaseLookup = new Lookup<String, String>();
        private Lookup<String, String> misspelledLookup = new Lookup<String, String>();

        public PrefixDatabase()
        {

        }

        private void UpdateLookup(String key, T unit)
        {
            var relaxedNames = new HashSet<String>();
            var misspelledNames = new HashSet<String>();
            foreach (var name in this.OverrideNames(unit))
            {
                var stage1 = UnitDatabase.RelaxToLowercase(name);
                var stage2 = UnitDatabase.RelaxSpelling(stage1);
                relaxedNames.Add(stage1);
                misspelledNames.Add(stage2);
            }
            this.lowercaseLookup.Update(key, relaxedNames);
            this.misspelledLookup.Update(key, misspelledNames);
        }

        protected virtual void OnClear() { }

        public void Clear()
        {
            this.database.Clear();
            this.primaryNames.Clear();
            this.primaryNameTree.Clear();
            this.lowercaseLookup.Clear();
            this.misspelledLookup.Clear();
            this.OnClear();
        }

        protected IDictionary<String, T> Database => this.database;

        public Int32 Count => this.database.Count;

        public IEnumerable<T> Units => this.database.Values;

        public T this[String key] => this.database[key];

        public Boolean TryFind(String query, ref T unit, Func<T, Boolean> filter = null)
        {
            if (filter.IsNull()) filter = u => true;
            if (this.database.TryGetValue(query, out unit)) if (filter(unit)) return true;

            // Primary search failed. Secondary search: all lowercase!
            var key = String.Empty;
            var lowercase = UnitDatabase.RelaxToLowercase(query);
            var misspelled = UnitDatabase.RelaxSpelling(lowercase);
            var countMatches = 0;

            // Stage 0: prefix tree search.
            var isSingle = false;
            var firstPrefixMatch = this.primaryNameTree.First(lowercase, out isSingle);
            // Only one terminus matches the prefix.
            if (isSingle) lowercase = firstPrefixMatch;

            // Stage 1: lowercase lookup.
            countMatches = this.lowercaseLookup.TryFind(lowercase, ref key, maybe => filter(this.database[maybe]));
            if (countMatches >= 1)
            {
                unit = this.database[key];
                if (countMatches == 1) return true;
                App.Warnings.Push($"Multiple units ({countMatches}) match the specified query ({query}).");
                return false;
            }

            // Stage 2: misspelled lookup.
            countMatches = this.misspelledLookup.TryFind(misspelled, ref key, maybe => filter(this.database[maybe]));
            if (countMatches >= 1)
            {
                unit = this.database[key];
                if (countMatches == 1) return true;
                App.Warnings.Push($"Multiple units ({countMatches}) match the specified query ({query}).");
                return false;
            }

            return false;
        } // TryFind(...)

        protected virtual void OnLoading(T unit, out Boolean doCancel) { doCancel = false; }

        protected virtual void OnLoaded(T unit) { }

        public Boolean Add(T unit)
        {
            var doCancel = false;
            this.OnLoading(unit, out doCancel);
            if (doCancel) return false;

            var key = this.OverrideBuildKey(unit);
            var primaryName = this.OverrideBuildPrimaryName(unit);
            if (this.database.ContainsKey(key))
            {
                App.Warnings.Push($"Unit with the same key ({key}) already loaded.");
                return false;
            }
            else if (this.primaryNames.Contains(primaryName))
            {
                App.Warnings.Push($"Unit with a similar primary name ({primaryName}) already loaded.");
                return false;
            }
            else
            {
                this.database.Add(key, unit);
                this.primaryNames.Add(primaryName);
                this.primaryNameTree.Add(primaryName);
                this.UpdateLookup(key, unit);
                this.OnLoaded(unit);
                return true;
            }
        }
    }
}
