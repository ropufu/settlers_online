using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.LeytePond.Bridge
{
    struct Optional<T>
    {
        private Boolean hasValue;
        private T value;

        public Boolean HasValue => this.hasValue;

        public T Value
        {
            get
            {
                if (!this.hasValue) throw new InvalidOperationException();
                return this.value;
            }
        }

        public static implicit operator Optional<T>(T value)
        {
            if (Object.ReferenceEquals(value, null)) return new Optional<T>();
            return new Optional<T>() { hasValue = true, value = value };
        }
    }

    /** Mirrors structural behavior of \c prefix_tree.hpp. */
    class PrefixTreeNode<TLetter, TWord, TKey>
        where TLetter : struct, IComparable<TLetter>, IEquatable<TLetter>
        where TWord : IEnumerable<TLetter>
        where TKey : IEquatable<TKey>
        //where TKey : struct
    {
        private TLetter letter = default(TLetter); // Letter associated with the stop, unique only between siblings.
        private Optional<TKey> key = default(Optional<TKey>); // Key to the associated element in the tree-inducing collection (if any).
        private Dictionary<TLetter, Int32> childrenIndices = new Dictionary<TLetter, Int32>(); // Indices of child nodes in \c prefix_tree node collection.
        // ~~ Cache fields to speed up search and stuff. ~~
        private Int32 selfIndex = 0; // Index of this node in \c prefix_tree node collection.
        private Int32 parentIndex = 0; // Index of the parent node in \c prefix_tree node collection.
        private HashSet<TKey> synonymKeys = new HashSet<TKey>(); // Keys to synonym elements (self excluded) in the tree-inducing collection.

        private PrefixTreeNode(TLetter letter)
        {
            this.letter = letter;
        }

        public static PrefixTreeNode<TLetter, TWord, TKey> MakeRoot() => new PrefixTreeNode<TLetter, TWord, TKey>(default(TLetter));

        /** Adds a non-station child with specified link value and parent to the vertex collection \p global_vertex_collection. */
        public Boolean TryEmplaceChild(TLetter letter, List<PrefixTreeNode<TLetter, TWord, TKey>> nodeCollection, ref Int32 childIndex)
        {
            // Check if there is already a child with the same \c letter.
            if (this.childrenIndices.ContainsKey(letter)) return false;

            // Create a new child if not.
            var nextStop = new PrefixTreeNode<TLetter, TWord, TKey>(letter);
            childIndex = nodeCollection.Count;

            nextStop.selfIndex = childIndex;
            nextStop.parentIndex = this.selfIndex;

            this.childrenIndices.Add(letter, childIndex); // Add the child to the current vertex.

            // Warning: invalidates this pointer.
            nodeCollection.Add(nextStop); // Store the new vertex in the railway repository.
            return true;
        } // try_emplace_child(...)

        /** @brief Records \p key as a synonym to all ancsetor nodes, current node excluded. */
        private void PropagateSynonym(TKey key, List<PrefixTreeNode<TLetter, TWord, TKey>> nodeCollection)
        {
            // Record the station index in all ancestor records.
            var parentIndex = this.parentIndex;
            while (parentIndex != 0)
            {
                var ancestor = nodeCollection[parentIndex];
                ancestor.synonymKeys.Add(key);
                parentIndex = ancestor.parentIndex;
            } // while (...)
        } // propagate_synonym(...)

        /** @brief Records \p key as an explicit synonym the current node and all its ancsetors. */
        public void MarkAsSynonymTo(TKey key, List<PrefixTreeNode<TLetter, TWord, TKey>> nodeCollection)
        {
            if (this.key.HasValue && this.key.Value.Equals(key)) return; // No self-synonyms allowed.
            this.synonymKeys.Add(key);
            this.PropagateSynonym(key, nodeCollection);
        } // mark_as_synonym_to(...)

        /** @brief Marks an element at the specified index in the vertex collection \p global_vertex_collection as terminal.
         *  @warning Does not perform checks on whether \p path is actually the concatenation of links.
         */
        public Boolean TryMarkAsWord(TKey key, List<PrefixTreeNode<TLetter, TWord, TKey>> nodeCollection)
        {
            var wasWord = this.key.HasValue;
            if (wasWord) return false;

            this.key = key;
            this.PropagateSynonym(key, nodeCollection);
            return true;
        } // try_mark_as_word(...)

        /** Index of the child with specified link value  in \c prefix_tree node collection, or zero if none such exist. */
        public Int32 this[TLetter letter]
        {
            get
            {
                var key = default(Int32);
                if (this.childrenIndices.TryGetValue(letter, out key)) return key;
                return 0;
            }
        }

        /** Letter associated with the node. */
        public TLetter Letter => this.letter;
        /** Key to the associated element in the tree-inducing collection (if any). */
        public Optional<TKey> Key => this.key;
        public Boolean IsWord => this.key.HasValue;
        /** Keys to synonym elements (self excluded) in the tree-inducing collection. */
        public HashSet<TKey> SynonymKeys => this.synonymKeys;
    }

    /** Mirrors structural behavior of \c prefix_tree.hpp. */
    class PrefixTree<TLetter, TWord, TKey>
        where TLetter : struct, IComparable<TLetter>, IEquatable<TLetter>
        where TWord : IEnumerable<TLetter>
        where TKey : IEquatable<TKey>
        //where TKey : struct
    {
        private List<PrefixTreeNode<TLetter, TWord, TKey>> nodes = new List<PrefixTreeNode<TLetter, TWord, TKey>>(); // Collection of railway stops.

        /** @brief Develops a search query against the railway map. */
        private Int32 Develop(TWord query)
        {
            var currentIndex = 0;
            foreach (TLetter x in query)
            {
                var currentVertex = this.nodes[currentIndex];
                currentIndex = currentVertex[x];
                if (currentIndex == 0) return 0;
            } // for (...)
            return currentIndex;
        } // develop(...)

        /** @brief Develops a search query against the railway map, creating new branches as necessary. */
        private Int32 ActiveDevelop(TWord query)
        {
            var currentIndex = 0;
            foreach (TLetter x in query)
            {
                var currentVertex = this.nodes[currentIndex];
                currentIndex = currentVertex[x];
                if (currentIndex == 0)
                {
                    if (!currentVertex.TryEmplaceChild(x, this.nodes, ref currentIndex)) throw new ShouldNotHappenException(); // This should never happen.
                } // if (...)
            } // for (...)
            return currentIndex;
        } // active_develop(...)

        public PrefixTree()
        {
            this.Clear();
        }

        public void Clear()
        {
            var root = PrefixTreeNode<TLetter, TWord, TKey>.MakeRoot();
            this.nodes.Clear(); // Clear the vertex collection.
            this.nodes.Add(root); // Add root element.
        }

        public void AddSynonym(TKey key, TWord synonym)
        {
            var currentIndex = this.ActiveDevelop(synonym); // Develop the path.
            if (currentIndex == 0) return; // Empty paths not allowed.
            // The end of the path has been reached: we are at the end of the word.
            // Mark the node as an explicit synonym.
            this.nodes[currentIndex].MarkAsSynonymTo(key, this.nodes);
        } // try_add_single(...)

        public Boolean TryAddSingle(TKey key, TWord path)
        {
            var currentIndex = this.ActiveDevelop(path); // Develop the path.
            if (currentIndex == 0) return false; // Empty paths not allowed.
            // The end of the path has been reached: we are at the end of the word.
            // Mark the node as word-containing.
            if (!this.nodes[currentIndex].TryMarkAsWord(key, this.nodes)) return false; // Duplicate words not allowed.
            return true;
        } // try_add_single(...)

        public Boolean TryAddMany(TKey key, IEnumerable<TWord> paths)
        {
            var hasSucceeded = true;
            foreach (TWord path in paths) hasSucceeded &= this.TryAddSingle(key, path);
            return hasSucceeded;
        } // try_add_many(...)

        /** List all keys for elements in the tree-inducing collection starting with a given prefix. */
        public PrefixTreeNode<TLetter, TWord, TKey> Search(TWord query)
        {
            var matchedIndex = this.Develop(query);
            if (matchedIndex == 0) return null;
            var matched = this.nodes[matchedIndex];
            return matched;
        } // search(...)

        /** List all keys for elements in the tree-inducing collection starting with a given prefix. */
        public PrefixTreeNode<TLetter, TWord, TKey> this [TWord query] => this.Search(query);

        /** Checks if there are elements in the tree-inducing collection that match a given prefix exactly. */
        public Boolean Contains(TWord word)
        {
            var matchedIndex = this.Develop(word);
            if (matchedIndex == 0) return false;
            var matched = this.nodes[matchedIndex];
            return matched.IsWord;
        } // match(...)

        /** Checks if there are elements in the tree-inducing collection that match any of the given prefixes exactly. */
        public Boolean ContainsAny(IEnumerable<TWord> words)
        {
            foreach (TWord word in words) if (this.Contains(word)) return true;
            return false;
        } // match_any(...)

        /** Checks if there are elements in the tree-inducing collection that match all of the given prefixes exactly. */
        public Boolean ContainsAll(IEnumerable<TWord> words)
        {
            foreach (TWord word in words) if (!this.Contains(word)) return false;
            return true;
        } // match_all(...)

        /** Checks if there are elements in the tree-inducing collection starting with a given prefix. */
        public Boolean Match(TWord query)
        {
            var matchedIndex = this.Develop(query);
            return (matchedIndex != 0);
        } // match(...)

        /** Checks if there are elements in the tree-inducing collection starting with any of the given prefixes. */
        public Boolean MatchAny(IEnumerable<TWord> queries)
        {
            foreach (TWord query in queries) if (this.Match(query)) return true;
            return false;
        } // match_any(...)

        /** Checks if there are elements in the tree-inducing collection starting with all of the given prefixes. */
        public Boolean MatchAll(IEnumerable<TWord> queries)
        {
            foreach (TWord query in queries) if (!this.Match(query)) return false;
            return true;
        } // match_all(...)
    }
}
