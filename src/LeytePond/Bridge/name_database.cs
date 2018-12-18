using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c prefix_database.hpp. */
    public abstract class NameDatabase<T, TKey>
        where TKey : IEquatable<TKey>
        //where TKey : struct
    {
        protected abstract TKey OnBuildKey(T unit);
        protected abstract HashSet<String> OnBuildNames(T unit);
        protected virtual void OnRelaxing(T unit, ref Boolean doCancel) { }
        protected virtual void OnRelaxed(T unit, HashSet<String> relaxedNames, ref HashSet<String> strictNames) { }
        protected virtual void OnClear() { }
        protected virtual void OnLoading(T unit, ref Boolean doCancel) { }
        protected virtual void OnLoaded(T unit) { }

        //private T invalid = default(T); // Used to indicates invalid return result.
        private Dictionary<TKey, T> database = new Dictionary<TKey, T>();
        private PrefixTree<Char, String, TKey> nameTree = new PrefixTree<Char, String, TKey>(); // Fast prefix search by relaxed names.

        private List<String> suggestions = new List<String>();

        /** Relaxed lookup stage 1. */
        private static String RelaxToLowercase(String query) => query.ToLowerInvariant();

        /** Relaxed lookup stage 2. Assuming stage 1 has already been applied. */
        private static String RelaxSpelling(String query)
        {
            // Plural: man -> men.
            var relaxed = query.Replace("men", "man");
            // Plural: ...es or ...s.
            if (relaxed.EndsWith("es")) relaxed = relaxed.Substring(0, relaxed.Length - 2);
            else if (relaxed.EndsWith("s")) relaxed = relaxed.Substring(0, relaxed.Length - 1);

            // Collapse all repeated letters for "reasonably" long words.
            if (relaxed.Length > 4)
            {
                var previous = relaxed[0];
                for (var i = 1; i < relaxed.Length; ++i)
                {
                    if (relaxed[i] == previous) relaxed = relaxed.Remove(i, 1);
                    else previous = relaxed[i];
                } // for (...)
            } // if (...)

            return relaxed;
        } // relax_spelling(...)

        private static HashSet<String> Relax(HashSet<String> names)
        {
            var result = new HashSet<String>(names);
            foreach (var name in names)
            {
                var stage1 = NameDatabase<T, TKey>.RelaxToLowercase(name);
                var stage2 = NameDatabase<T, TKey>.RelaxSpelling(stage1);
                result.Add(stage1);
                result.Add(stage2);
            } // for (...)
            return result;
        }

        private HashSet<String> Relax(T unit, ref HashSet<String> strictNames)
        {
            var result = this.BuildNames(unit);

            var doCancel = false;
            this.OnRelaxing(unit, ref doCancel);
            if (doCancel) return result;

            result = NameDatabase<T, TKey>.Relax(result);
            this.OnRelaxed(unit, result, ref strictNames);
            foreach (var name in strictNames) result.Add(name);

            return result;
        } // relax(...)

        T AsSingle(PrefixTreeNode<Char, String, TKey> searchResult, ref Boolean isValid)
        {
            isValid = true;
            if (searchResult.IsWord) // Perfect match.
            {
                return this.database[searchResult.Key.Value];
            } // if (...)
            if (searchResult.SynonymKeys.Count == 1) // Extrapolated perfect match.
            {
                return this.database[searchResult.SynonymKeys.First()];
            } // if (...)

            isValid = false;
            return default(T);
        } // as_single(...)

        T AsSingle(PrefixTreeNode<Char, String, TKey> searchResult, Func<T, Boolean> filter, ref Boolean isValid)
        {
            if (Object.ReferenceEquals(filter, null)) return this.AsSingle(searchResult, ref isValid);

            isValid = true;
            if (searchResult.IsWord) // Perfect match.
            {
                var maybe = this.database[searchResult.Key.Value];
                if (filter(maybe)) return maybe;
            } // if (...)
            var countSynonyms = 0;
            T lastSynonym = default(T);
            foreach (TKey synonymKey in searchResult.SynonymKeys)
            {
                var maybe = this.database[synonymKey];
                if (filter(maybe))
                {
                    ++countSynonyms;
                    lastSynonym = maybe;
                } // if (...)
            } // for (...)
            if (countSynonyms == 1) return lastSynonym;

            isValid = false;
            return default(T);
        } // as_single(...)

    public NameDatabase()
        {
        }

        /** Clears the contents of the database. */
        public void Clear()
        {
            this.database.Clear();
            this.nameTree.Clear();
            this.OnClear();
        } // clear(...)

        public TKey BuildKey(T unit) => this.OnBuildKey(unit);
        public HashSet<String> BuildNames(T unit) => this.OnBuildNames(unit);
        public Dictionary<TKey, T> Data => this.database;

        public Int32 Count => this.database.Count;

        public IEnumerable<T> All => from pair in this.Data select pair.Value;

        public IEnumerable<String> Suggestions => this.suggestions;

        /** @brief Access elements by key.
         *  @exception std::out_of_range 
         *  @param ec Set to std::errc::bad_address if specified \p key is not in the database.
         */
        public T this[TKey key] => this.database[key];

        public T Find(String query, Func<T, Boolean> filter)
        {
            var lowercase = NameDatabase<T, TKey>.RelaxToLowercase(query);
            var misspelled = NameDatabase<T, TKey>.RelaxSpelling(lowercase);
            bool isValid = false;

            // Stage 0: prefix tree search.
            this.suggestions.Clear();
            var searchA = this.nameTree.Search(query);
            if (!Object.ReferenceEquals(searchA, null))
            {
                var unit = this.AsSingle(searchA, filter, ref isValid);
                if (isValid) return unit;
                else
                {
                    foreach (var synonymKey in searchA.SynonymKeys)
                    {
                        var maybe = this.database[synonymKey];
                        var names = this.BuildNames(maybe);
                        var name = names.Count == 0 ? "??" : names.First();
                        if (filter == null || filter(maybe)) this.suggestions.Add(name);
                    }
                }
            } // if (...)

            // Stage 1: lowercase lookup.
            var searchB = this.nameTree.Search(lowercase);
            if (!Object.ReferenceEquals(searchB, null))
            {
                var unit = this.AsSingle(searchB, filter, ref isValid);
                if (isValid)
                {
                    this.suggestions.Clear();
                    return unit;
                }
                App.Warnings.Push($"Multiple units match the specified query: {lowercase}.");
                return default(T);
            } // if (...)

            // Stage 2: misspelled lookup.
            var searchC = this.nameTree.Search(misspelled);
            if (!Object.ReferenceEquals(searchC, null))
            {
                var unit = this.AsSingle(searchC, filter, ref isValid);
                if (isValid)
                {
                    this.suggestions.Clear();
                    return unit;
                }
                App.Warnings.Push($"Multiple units match the specified query: {misspelled}.");
                return default(T);
            } // if (...)

            return default(T);
        } // try_find(...)

        public Boolean Add(T unit)
        {
            var doCancel = false;
            this.OnLoading(unit, ref doCancel);
            if (doCancel) return false;

            var key = this.BuildKey(unit);
            var names = this.BuildNames(unit); // Names for logging purposes.
            var strictNames = new HashSet<String>();
            var treeNames = this.Relax(unit, ref strictNames);
            var treeSynonyms = NameDatabase<T, TKey>.Relax(strictNames);

            if (this.database.ContainsKey(key))
            {
                App.Warnings.Push($"Unit with id ({key}) already exists.");
                return false;
            } // if (...)
            else if (this.nameTree.ContainsAny(treeNames))
            {
                App.Warnings.Push($"Unit with a similar name ({String.Join(", ", names)}) already exists.");
                return false;
            } // if (...)
            else
            {
                this.database.Add(key, unit);
                this.nameTree.TryAddMany(key, treeNames);
                foreach (var explicitSynonym in treeSynonyms) this.nameTree.AddSynonym(key, explicitSynonym);
                this.OnLoaded(unit);
                return true;
            } // if (...)
        } // add(...)
    }
}
