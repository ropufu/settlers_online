using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c prefix_tree.hpp. */
    class Node<T, U>
        where T : struct, IComparable<T>, IEquatable<T>
        where U : IEnumerable<T>
    {
        private T value = default(T);
        private List<Node<T, U>> children = new List<Node<T, U>>();
        private Boolean isTerminal = false;
        // ~~ Cache fields to speed up search and stuff. ~~
        private U terminus = default(U);
        private Node<T, U> parent = null;
        private List<Node<T, U>> termini = new List<Node<T, U>>();
        private Int32 depth = 0;

        public Node<T, U> MakeChild(T value, List<Node<T, U>> vertices)
        {
            if (vertices.IsNull()) throw new ArgumentNullException(nameof(vertices));
            var child = new Node<T, U>()
            {
                value = value,
                parent = this,
                depth = this.depth + 1
            };
            vertices.Add(child);
            this.children.Add(child);
            return child;
        }

        public void MarkAsTerminal(U value)
        {
            this.terminus = value; // Update terminus.
            if (this.isTerminal) return; // No need to do anything else if the vertex has already been marked.

            // Mark the node as terminal.
            this.isTerminal = true;
            // Record the terminal node in the ancestors.
            var ancestor = this;
            while (!ancestor.IsNull())
            {
                ancestor.termini.Add(this);
                ancestor = ancestor.parent;
            }
        }

        public T Value => this.value;
        public Int32 Depth => this.depth;
        public Node<T, U> Parent => this.parent;

        public ICollection<Node<T, U>> Children => this.children.AsReadOnly();
        public ICollection<Node<T, U>> Termini => this.termini.AsReadOnly();

        public Node<T, U> GetChild(T value)
        {
            foreach (var child in this.children) if (child.value.Equals(value)) return child;
            return null;
        }

        public IEnumerable<U> BuildTermini() => from v in this.termini select v.terminus;

        public U FirstTerminus() => this.termini[0].terminus;

        public override String ToString()
        {
            var builder = new StringBuilder();
            if (!this.value.Equals(default(T))) builder.Append(this.value).Append(" -- ");
            foreach (var child in this.children) builder.Append(child.value);
            if (this.isTerminal) builder.Append(" [").Append(this.terminus).Append("]");
            return builder.ToString();
        }
    }

    /** Mirrors structural behavior of \c prefix_tree.hpp. */
    class PrefixTree<T, U>
        where T : struct, IComparable<T>, IEquatable<T>
        where U : IEnumerable<T>
    {
        private List<Node<T, U>> vertices = null;

        public PrefixTree()
        {
            this.vertices = new List<Node<T, U>>() { new Node<T, U>() };
        }

        public void Clear()
        {
            this.vertices.Clear(); // Clear the vertex collection.
            this.vertices.Add(new Node<T, U>()); // Add root element.
        }

        public void Add(params U[] collection)
        {
            if (collection.IsNull()) throw new ArgumentNullException(nameof(collection));
            foreach (var item in collection) this.Add(item);
        }

        public void Add(IEnumerable<U> collection)
        {
            if (collection.IsNull()) throw new ArgumentNullException(nameof(collection));
            foreach (var item in collection) this.Add(item);
        }

        public void Add(U path)
        {
            if (path.IsNull()) throw new ArgumentNullException(nameof(path));
            // Start at the root node, and "develop" the path.
            var currentVertex = this.vertices[0]; // Current position.
            foreach (var x in path)
            {
                var isNewBranch = true; // Indicates whether the path has reached the forking point.
                var nextVertex = default(Node<T, U>); // Index of the next matched node.
                foreach (var child in currentVertex.Children)
                {
                    // Try to match the current symbol in the path to the child.
                    if (child.Value.Equals(x))
                    {
                        nextVertex = child;
                        isNewBranch = false; // If matched, no need to create new branch.
                        break;
                    }
                }
                // If the symbol has not been matched to children, create a new branch.
                if (isNewBranch) nextVertex = currentVertex.MakeChild(x, this.vertices);

                // Update the position.
                currentVertex = nextVertex;
            }

            // The end of the path has been reached: we are at the terminal node.
            // Mark the node as terminal.
            currentVertex.MarkAsTerminal(path);
        }

        private Node<T, U> MatchNode(U query)
        {
            var currentVertex = this.vertices[0];
            foreach (var x in query)
            {
                currentVertex = currentVertex.GetChild(x);
                if (currentVertex.IsNull()) return null;
            }
            return currentVertex;
        } // MatchNode(...)

        public U First(U query, out Boolean isSingle)
        {
            isSingle = false;
            var match = this.MatchNode(query);
            if (match.IsNull()) return default(U);
            isSingle = (match.Termini.Count == 1);
            return match.FirstTerminus();
        }

        public IEnumerable<U> Find(U query, out Int32 count, ref U first)
        {
            count = 0;
            var match = this.MatchNode(query);
            if (match.IsNull()) return new List<U>();
            count = (match.Termini.Count);
            if (count == 1) first = match.FirstTerminus();
            return match.BuildTermini();
        }

        public IEnumerable<U> this[U query] => this.MatchNode(query)?.BuildTermini() ?? new List<U>();

        public Int32 Count(U query) => this.MatchNode(query)?.Termini.Count ?? 0;
    }
}
