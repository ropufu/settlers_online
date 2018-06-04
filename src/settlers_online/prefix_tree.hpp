
#ifndef ROPUFU_SETTLERS_ONLINE_PREFIX_TREE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_PREFIX_TREE_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <initializer_list> // std::initializer_list
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu
{
    namespace settlers_online
    {
        namespace detail
        {
            /** @brief Node of a prefix tree.
             *  Each node has two(!) properties attached to it: "value" and "terminus".
             *  Value represents one element in the underlying sequences.
             *  Terminus can be missing; its presence indicates that the node terminates the underlying
             *  sequence of values. It is worth pointing out that a "terminus" is the concatenation of
             *  all values in the ancestor nodes and the node in question.
             */
            template <typename t_value_type, typename t_terminus_type>
            struct node_indexer
            {
                using type = node_indexer<t_value_type, t_terminus_type>;
                using value_type = t_value_type;
                using terminus_type = t_terminus_type;

            private:
                value_type m_value = { }; // Value associated with the node.
                std::vector<std::size_t> m_children_indices = { }; // Indeces of the child nodes in the tree collection.
                bool m_is_terminal = false; // Indicates if this node has a terminus.
                // ~~ Cache fields to speed up search and stuff. ~~
                terminus_type m_terminus = { }; // Value of the terminus (default otherwise).
                std::size_t m_self_index = 0; // Global index of this node.
                std::size_t m_parent_index = 0; // Global index of the parent node.
                std::vector<std::size_t> m_termini_indices = { }; // Global indices of the descendant (self included) terminal nodes.
                std::size_t m_depth = 0; // Number of ancestors (self excluded).

            public:
                node_indexer() noexcept { }

                /** Adds a child with specified value and parent to the vertex collection \p vertices. */
                static std::size_t emplace_child(const value_type& value, std::size_t parent_index, std::vector<type>& vertices) noexcept
                {
                    type child { };
                    std::size_t child_index = vertices.size();

                    child.m_value = value;
                    child.m_self_index = child_index;
                    child.m_parent_index = vertices[parent_index].m_self_index;
                    child.m_depth = vertices[parent_index].m_depth + 1;

                    vertices.push_back(child); // Store the new vertex in the tree repository.
                    vertices[parent_index].m_children_indices.push_back(child_index); // Add the child to the current vertex.

                    return child_index;
                }

                /** Marks an element at the specified index in the vertex collection \p vertices as terminal. */
                static void mark_as_terminal(std::vector<type>& vertices, std::size_t index, const terminus_type& path) noexcept
                {
                    vertices[index].m_terminus = path; // Update terminus.
                    if (vertices[index].m_is_terminal) return; // No need to do anything else if the vertex has already been marked.

                    // Mark the node as terminal.
                    vertices[index].m_is_terminal = true;
                    vertices[index].m_termini_indices.push_back(index);
                    // Record the terminal node in the ancestors.
                    std::size_t ancestor_index = index;
                    while (ancestor_index != 0)
                    {
                        ancestor_index = vertices[ancestor_index].m_parent_index;
                        vertices[ancestor_index].m_termini_indices.push_back(index);
                    }
                }

                /** Value associated with the node. */
                const value_type& value() const noexcept { return this->m_value; }

                /** Number of ancestors (self excluded). */
                std::size_t depth() const noexcept { return this->m_depth; }
                
                /** Global index of this node. */
                std::size_t self_index() const noexcept { return this->m_self_index; }

                /** Global index of the parent node. */
                std::size_t parent_index() const noexcept { return this->m_parent_index; }

                /** Global indices of child nodes. */
                const std::vector<std::size_t> children_indices() const noexcept { return this->m_children_indices; }
                ///** Global indices of child nodes. */
                //std::vector<std::size_t> children_indices() noexcept { return this->m_children_indices; }

                /** Global indices of termini at this node or its descendants. */
                const std::vector<std::size_t> termini_indices() const noexcept { return this->m_termini_indices; }
                ///** Global indices of termini at this node or its descendants. */
                //std::vector<std::size_t> termini_indices() noexcept { return this->m_termini_indices; }

                /** Global index of the child with specified value, or zero if none such exist. */
                std::size_t get_child_index(const value_type& value, const std::vector<type>& vertices) const noexcept;

                /** List of termini at this node or its descendants. */
                std::vector<terminus_type> build_termini(const std::vector<type>& vertices) const noexcept;

                /** The first terminus at this node or its descendants. */
                terminus_type first_terminus(const std::vector<type>& vertices) const noexcept { return vertices[this->m_termini_indices.front()].m_terminus; }
            }; // struct node_indexer

            template <typename t_value_type, typename t_terminus_type>
            std::size_t node_indexer<t_value_type, t_terminus_type>::get_child_index(const value_type& value, const std::vector<node_indexer<t_value_type, t_terminus_type>>& vertices) const noexcept
            {
                using type = node_indexer<t_value_type, t_terminus_type>;

                for (std::size_t child_index : this->m_children_indices)
                {
                    const type& maybe = vertices[child_index];
                    if (maybe.m_value == value) return child_index;
                }
                return 0;
            } // node_indexer::get_child_index(...)

            template <typename t_value_type, typename t_terminus_type>
            std::vector<t_terminus_type> node_indexer<t_value_type, t_terminus_type>::build_termini(const std::vector<node_indexer<t_value_type, t_terminus_type>>& vertices) const noexcept
            {
                using terminus_type = t_terminus_type;

                std::vector<terminus_type> result { };
                result.reserve(this->m_termini_indices.size());
                for (std::size_t terminus_index : this->m_termini_indices) result.push_back(vertices[terminus_index].m_terminus);
                return result;
            } // node_indexer::build_termini(...)
        } // namespace detail

        /** @brief A structure to hold a collection of strings, with fast prefix search. */
        template <typename t_value_type, typename t_terminus_type>
        struct prefix_tree
        {
            using type = prefix_tree<t_value_type, t_terminus_type>;
            using node_type = detail::node_indexer<t_value_type, t_terminus_type>;
            using value_type = t_value_type;
            using terminus_type = t_terminus_type;

        private:
            std::vector<node_type> m_vertices = std::vector<node_type>(1);

            std::size_t match_node_index(const terminus_type& query) const noexcept
            {
                std::size_t current_index = 0;
                for (const value_type& x : query)
                {
                    const node_type& current_vertex = this->m_vertices[current_index];
                    current_index = current_vertex.get_child_index(x, this->m_vertices);
                    if (current_index == 0) return 0;
                }
                return current_index;
            } // match_node(...)

        public:
            prefix_tree() noexcept { }

            void clear() noexcept
            {
                this->m_vertices.clear(); // Clear the vertex collection.
                this->m_vertices.emplace_back(); // Add root element.
            } // clear(...)

            void add(std::initializer_list<terminus_type> collection) noexcept
            {
                for (const t_terminus_type& item : collection) this->add(item);
            } // add(...)

            void add(const std::vector<terminus_type>& collection) noexcept
            {
                for (const t_terminus_type& item : collection) this->add(item);
            } // add(...)

            void add(const terminus_type& path) noexcept
            {
                // Start at the root node, and "develop" the path.
                std::size_t current_index = 0; // Current position.
                for (const value_type& x : path)
                {
                    bool is_new_branch = true; // Indicates whether the path has reached the forking point.
                    std::size_t next_index = 0; // Index of the next matched node.
                    for (std::size_t child_index : this->m_vertices[current_index].children_indices())
                    {
                        // Try to match the current symbol in the path to the child.
                        if (this->m_vertices[child_index].value() == x)
                        {
                            next_index = child_index;
                            is_new_branch = false; // If matched, no need to create new branch.
                            break;
                        }
                    }
                    // If the symbol has not been matched to children, create a new branch.
                    if (is_new_branch) next_index = node_type::emplace_child(x, current_index, this->m_vertices);

                    // Update the position.
                    current_index = next_index;
                }

                // The end of the path has been reached: we are at the terminal node.
                // Mark the node as terminal.
                node_type::mark_as_terminal(this->m_vertices, current_index, path);
            } // add(...)

            /** First terminus starting with a given prefix. */
            terminus_type first(const terminus_type& query, bool& is_single) const noexcept
            {
                is_single = false;
                std::size_t matched_index = this->match_node_index(query);
                if (matched_index == 0) return { };
                is_single = (this->m_vertices[matched_index].termini_indices().size() == 1);
                return this->m_vertices[matched_index].first_terminus(this->m_vertices);
            } // operator [](...)

            /** List all termini starting with a given prefix. */
            std::vector<terminus_type> operator [](const terminus_type& query) const noexcept
            {
                std::size_t matched_index = this->match_node_index(query);
                if (matched_index == 0) return { };
                return this->m_vertices[matched_index].build_termini(this->m_vertices);
            } // operator [](...)

            /** Count termini starting with a given prefix. */
            std::size_t count(const terminus_type& query) const noexcept
            {
                std::size_t matched_index = this->match_node_index(query);
                if (matched_index == 0) return 0;
                return this->m_vertices[matched_index].termini_indices().size();
            } // count(...)
        }; // struct prefix_tree

        using char_tree = prefix_tree<char, std::string>;
    } // namespace settlers_online
} // namespace ropufu

#endif // ROPUFU_SETTLERS_ONLINE_PREFIX_TREE_HPP_INCLUDED
