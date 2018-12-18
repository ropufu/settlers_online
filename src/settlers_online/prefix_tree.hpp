
#ifndef ROPUFU_SETTLERS_ONLINE_PREFIX_TREE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_PREFIX_TREE_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <cstdlib> // std::exit
#include <initializer_list> // std::initializer_list
#include <map> // std::map
#include <optional> // std::optional, std::nullopt
#include <set> // std::set
#include <string> // std::string
#include <vector> // std::vector

namespace ropufu::settlers_online
{

    /** @brief A structure to hold a collection of strings, with fast prefix search. */
    template <typename t_letter_type, typename t_word_type, typename t_key_type>
    struct prefix_tree;

    /** @brief Node of a prefix tree.
        *  Each node has three(!) properties attached to it: "key", "link" and "terminus".
        *  "Key" is unique within the entire tree. "Link" is unique only between siblings.
        *  In other words, "link" indicates which branch to take. For example, in the tree
        *               a
        *              / \
        *             b   c
        *                / \
        *               d   f
        *  the strings "a", "b", "c", "d", "f" represent "links" of corresponding nodes.
        *  Terminus can be missing; its presence indicates that the node terminates the
        *  underlying sequence of "links". All leaf nodes must be terminal. A "terminus"
        *  is the concatenation of all "links" in the ancestor nodes, starting with the
        *  root node, and including the node in question.
        *  The right-most terminus in the example above is the string "acefg".
        */
    template <typename t_letter_type, typename t_word_type, typename t_key_type>
    struct prefix_tree_node
    {
        using type = prefix_tree_node<t_letter_type, t_word_type, t_key_type>;
        using letter_type = t_letter_type;
        using word_type = t_word_type;
        using key_type = t_key_type;

        friend struct prefix_tree<t_letter_type, t_word_type, t_key_type>;

    private:
        using index_type = std::size_t;
        using collection_type = std::vector<type>;

        // ~~ Essential fields. ~~
        letter_type m_letter = {}; // Letter associated with the stop, unique only between siblings.
        std::optional<key_type> m_key = std::nullopt; // Key to the associated element in the tree-inducing collection (if any).
        std::map<letter_type, index_type> m_children_indices = {}; // Indices of child nodes in \c prefix_tree node collection.
        // ~~ Fields to speed up search and stuff. ~~
        index_type m_self_index = 0; // Index of this node in \c prefix_tree node collection.
        index_type m_parent_index = 0; // Index of the parent node in \c prefix_tree node collection.
        std::set<key_type> m_synonym_keys = {}; // Keys to synonym elements (self excluded) in the tree-inducing collection.

        prefix_tree_node() noexcept { }

        explicit prefix_tree_node(const letter_type& letter) noexcept
            : m_letter(letter)
        {

        } // prefix_tree_node(...)

        /** Adds a non-station child with specified link value and parent to the vertex collection \p global_vertex_collection. */
        bool try_emplace_child(const letter_type& letter, collection_type& node_collection, index_type& child_index) noexcept
        {
            // Check if there is already a child with the same \c letter.
            auto search = this->m_children_indices.find(letter);
            if (search != this->m_children_indices.end()) return false;

            // Create a new child if not.
            type next_stop{ letter };
            child_index = static_cast<index_type>(node_collection.size());

            next_stop.m_self_index = child_index;
            next_stop.m_parent_index = this->m_self_index;

            this->m_children_indices.emplace(letter, child_index); // Add the child to the current vertex.

            // Warning: invalidates this pointer.
            node_collection.push_back(next_stop); // Store the new vertex in the railway repository.
            return true;
        } // try_emplace_child(...)

        /** @brief Records \p key as a synonym to all ancsetor nodes, current node excluded. */
        void propagate_synonym(const key_type& key, collection_type& node_collection) noexcept
        {
            // Record the station index in all ancestor records.
            index_type parent_index = this->m_parent_index;
            while (parent_index != 0)
            {
                type& ancestor = node_collection[parent_index];
                ancestor.m_synonym_keys.insert(key);
                parent_index = ancestor.m_parent_index;
            } // while (...)
        } // propagate_synonym(...)

        /** @brief Records \p key as an explicit synonym the current node and all its ancsetors. */
        void mark_as_synonym_to(const key_type& key, collection_type& node_collection) noexcept
        {
            if (this->m_key.has_value() && this->m_key.value() == key) return; // No self-synonyms allowed.
            this->m_synonym_keys.insert(key);
            this->propagate_synonym(key, node_collection);
        } // mark_as_synonym_to(...)

        /** @brief Marks an element at the specified index in the vertex collection \p global_vertex_collection as terminal.
         *  @warning Does not perform checks on whether \p path is actually the concatenation of links.
         */
        bool try_mark_as_word(const key_type& key, collection_type& node_collection) noexcept
        {
            bool was_word = this->m_key.has_value();
            if (was_word) return false;

            this->m_key = key;
            this->propagate_synonym(key, node_collection);
            return true;
        } // try_mark_as_word(...)

        /** Index of the child with specified link value  in \c prefix_tree node collection, or zero if none such exist. */
        index_type operator [](const letter_type& letter) const noexcept
        {
            auto search = this->m_children_indices.find(letter);
            return (search != this->m_children_indices.end()) ? (search->second) : 0;
        } // operator [](...)

    public:
        /** Letter associated with the node. */
        const letter_type& letter() const noexcept { return this->m_letter; }

        /** Key to the associated element in the tree-inducing collection (if any). */
        const std::optional<key_type>& key() const noexcept { return this->m_key; }

        bool is_word() const noexcept { return this->m_key.has_value(); }

        /** Keys to synonym elements (self excluded) in the tree-inducing collection. */
        const std::set<key_type> synonym_keys() const noexcept { return this->m_synonym_keys; }
    }; // struct prefix_tree_node

    /** @brief A structure to hold a collection of strings, with fast prefix search. */
    template <typename t_letter_type, typename t_word_type, typename t_key_type>
    struct prefix_tree
    {
        using type = prefix_tree<t_letter_type, t_word_type, t_key_type>;
        using node_type = prefix_tree_node<t_letter_type, t_word_type, t_key_type>;
        using letter_type = t_letter_type;
        using word_type = t_word_type;
        using key_type = t_key_type;

    private:
        using index_type = std::size_t;
        using collection_type = std::vector<node_type>;

        collection_type m_nodes = {}; // Collection of railway stops.

        /** @brief Develops a search query against the railway map. */
        index_type develop(const word_type& query) const noexcept
        {
            // Start at the root node, and "develop" the path.
            index_type current_index = 0;
            for (const letter_type& x : query)
            {
                const node_type& current_vertex = this->m_nodes[current_index];
                current_index = current_vertex[x];
                if (current_index == 0) return 0;
            } // for (...)
            return current_index;
        } // develop(...)

        /** @brief Develops a search query against the railway map, creating new branches as necessary. */
        index_type active_develop(const word_type& query) noexcept
        {
            // Start at the root node, and "develop" the path.
            index_type current_index = 0;
            for (const letter_type& x : query)
            {
                node_type& current_vertex = this->m_nodes[current_index];
                current_index = current_vertex[x];
                // If the symbol has not been matched to children, create a new branch.
                if (current_index == 0)
                {
                    if (!current_vertex.try_emplace_child(x, this->m_nodes, current_index)) std::exit(8); // This should never happen.
                } // if (...)
            } // for (...)
            return current_index;
        } // active_develop(...)

    public:
        prefix_tree() noexcept
        {
            this->clear();
        } // prefix_tree(...)

        void clear() noexcept
        {
            node_type root{};
            this->m_nodes.clear(); // Clear the vertex collection.
            this->m_nodes.push_back(root); // Add root element.
        } // clear(...)

        void add_synonym(const key_type& key, const word_type& synonym) noexcept
        {
            index_type current_index = this->active_develop(synonym); // Develop the path.
            if (current_index == 0) return; // Empty paths not allowed.
            // The end of the path has been reached: we are at the end of the word.
            // Mark the node as an explicit synonym.
            this->m_nodes[current_index].mark_as_synonym_to(key, this->m_nodes);
        } // add_synonym(...)

        bool try_add_single(const key_type& key, const word_type& path) noexcept
        {
            index_type current_index = this->active_develop(path); // Develop the path.
            if (current_index == 0) return false; // Empty paths not allowed.
            // The end of the path has been reached: we are at the end of the word.
            // Mark the node as word-containing.
            if (!this->m_nodes[current_index].try_mark_as_word(key, this->m_nodes)) return false; // Duplicate words not allowed.
            return true;
        } // try_add_single(...)

        template <typename t_word_collection_type>
        bool try_add_many(const key_type& key, const t_word_collection_type& paths) noexcept
        {
            bool has_succeeded = true;
            for (const word_type& path : paths) has_succeeded &= this->try_add_single(key, path);
            return has_succeeded;
        } // try_add_many(...)

        /** List all keys for elements in the tree-inducing collection starting with a given prefix. */
        std::optional<node_type> search(const word_type& query) const noexcept
        {
            index_type matched_index = this->develop(query);
            if (matched_index == 0) return std::nullopt;
            const node_type& matched = this->m_nodes[matched_index];
            return matched;
        } // search(...)

        /** List all keys for elements in the tree-inducing collection starting with a given prefix. */
        std::optional<node_type> operator [](const word_type& query) const noexcept { return this->search(query); }

        /** Checks if there are elements in the tree-inducing collection that match a given prefix exactly. */
        bool contains(const word_type& word) const noexcept
        {
            index_type matched_index = this->develop(word);
            if (matched_index == 0) return false;
            const node_type& matched = this->m_nodes[matched_index];
            return matched.is_word();
        } // match(...)

        /** Checks if there are elements in the tree-inducing collection that match any of the given prefixes exactly. */
        template <typename t_word_collection_type>
        bool contains_any(const t_word_collection_type& words) const noexcept
        {
            for (const word_type& word : words) if (this->contains(word)) return true;
            return false;
        } // match_any(...)

        /** Checks if there are elements in the tree-inducing collection that match all of the given prefixes exactly. */
        template <typename t_word_collection_type>
        bool contains_all(const t_word_collection_type& words) const noexcept
        {
            for (const word_type& word : words) if (!this->contains(word)) return false;
            return true;
        } // match_all(...)

        /** Checks if there are elements in the tree-inducing collection starting with a given prefix. */
        bool match(const word_type& query) const noexcept
        {
            index_type matched_index = this->develop(query);
            return (matched_index != 0);
        } // match(...)

        /** Checks if there are elements in the tree-inducing collection starting with any of the given prefixes. */
        template <typename t_word_collection_type>
        bool match_any(const t_word_collection_type& queries) const noexcept
        {
            for (const word_type& query : queries) if (this->match(query)) return true;
            return false;
        } // match_any(...)

        /** Checks if there are elements in the tree-inducing collection starting with all of the given prefixes. */
        template <typename t_word_collection_type>
        bool match_all(const t_word_collection_type& queries) const noexcept
        {
            for (const word_type& query : queries) if (!this->match(query)) return false;
            return true;
        } // match_all(...)
    }; // struct prefix_tree

    template <typename t_key_type>
    using char_tree_t = prefix_tree<char, std::string, t_key_type>;
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_PREFIX_TREE_HPP_INCLUDED
