
#ifndef ROPUFU_SETTLERS_ONLINE_NAME_DATABASE_HPP_INCLUDED
#define ROPUFU_SETTLERS_ONLINE_NAME_DATABASE_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include "char_string.hpp"
#include "logger.hpp"
#include "prefix_tree.hpp"

#include <cstddef>  // std::size_t, std::nullptr_t
#include <map>      // std::map
#include <optional> // std::optional, std::nullopt
#include <set>      // std::set
#include <stdexcept>    // std::out_of_range
#include <string>       // std::string
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::is_same
#include <vector>       // std::vector

namespace ropufu::settlers_online
{
    /** @brief Class for accessing items by name. */
    template <typename t_derived_type, typename t_value_type, typename t_key_type>
    struct name_database
    {
        using type = name_database<t_derived_type, t_value_type, t_key_type>;
        using derived_type = t_derived_type;
        using value_type = t_value_type;
        using key_type = t_key_type;
        using name_type = std::string;

    private:
        using tree_type = char_tree_t<key_type>;
        using node_type = typename tree_type::node_type;

        value_type m_invalid = {}; // Used to indicates invalid return result.
        std::map<key_type, value_type> m_database = {};
        tree_type m_name_tree = {}; // Fast prefix search by relaxed names.

        /** Relaxed lookup stage 1. */
        static name_type relax_to_lowercase(const name_type& query) noexcept
        {
            name_type relaxed = query;
            char_string::to_lower(relaxed);
            return relaxed;
        } // relax_to_lowercase(...)

        /** Relaxed lookup stage 2. Assuming stage 1 has already been applied. */
        static name_type relax_spelling(const name_type& query) noexcept
        {
            name_type relaxed = query;
            // Plural: man -> men.
            char_string::replace(relaxed, "men", "man");
            // Plural: ...es or ...s.
            if (char_string::ends_with(relaxed, "es")) relaxed = relaxed.substr(0, relaxed.length() - 2);
            else if (char_string::ends_with(relaxed, "s")) relaxed = relaxed.substr(0, relaxed.length() - 1);

            // Collapse all repeated letters for "reasonably" long words.
            if (relaxed.length() > 4)
            {
                char previous = relaxed[0];
                for (std::size_t i = 1; i < relaxed.size(); ++i)
                {
                    if (relaxed[i] == previous) relaxed.erase(i, 1);
                    else previous = relaxed[i];
                } // for (...)
            } // if (...)
            
            return relaxed;
        } // relax_spelling(...)

        static std::set<name_type> relax(const std::set<name_type>& names) noexcept
        {
            std::set<name_type> result = names;
            for (const name_type& name : names)
            {
                name_type stage1 = type::relax_to_lowercase(name);
                name_type stage2 = type::relax_spelling(stage1);
                result.insert(stage1);
                result.insert(stage2);
            } // for (...)
            return result;
        } // relax(...)

        std::set<name_type> relax(const value_type& unit, std::set<name_type>& strict_names) const noexcept
        {
            std::set<name_type> result = this->build_names(unit);
            
            bool do_cancel = false;
            this->on_relaxing(unit, do_cancel);
            if (do_cancel) return result;

            result = type::relax(result);
            this->on_relaxed(unit, result, strict_names);
            for (const name_type& name : strict_names) result.insert(name);

            return result;
        } // relax(...)

        const value_type& as_single(const node_type& search_result, std::nullptr_t /*filter*/, bool& is_valid) const noexcept
        {
            is_valid = true;
            if (search_result.is_word()) // Perfect match.
            {
                return this->m_database.at(search_result.key().value());
            } // if (...)
            if (search_result.synonym_keys().size() == 1) // Extrapolated perfect match.
            {
                return this->m_database.at(*(search_result.synonym_keys().begin()));
            } // if (...)

            is_valid = false;
            return this->m_invalid;
        } // as_single(...)

        template <typename t_predicate_type>
        const value_type& as_single(const node_type& search_result, const t_predicate_type& filter, bool& is_valid) const noexcept
        {
            is_valid = true;
            if (search_result.is_word()) // Perfect match.
            {
                const value_type& maybe = this->m_database.at(search_result.key().value());
                if (filter(maybe)) return maybe;
            } // if (...)
            std::size_t count_synonyms = 0;
            const value_type* last_synonym_ptr = nullptr;
            for (const key_type& synonym_key : search_result.synonym_keys())
            {
                const value_type& maybe = this->m_database.at(synonym_key);
                if (filter(maybe))
                {
                    ++count_synonyms;
                    last_synonym_ptr = &maybe;
                } // if (...)
            } // for (...)
            if (count_synonyms == 1) return *last_synonym_ptr;

            is_valid = false;
            return this->m_invalid;
        } // as_single(...)

    protected:
        /** @brief Retrieves the database key. */
        key_type on_build_key(const value_type& item) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::on_build_key), 
                decltype(&type::on_build_key)>::value;
            static_assert(is_overwritten, "on_build_key(..) -> key_type has not been overloaded.");

            const derived_type* that = static_cast<const derived_type*>(this);
            return that->on_build_key(item);
        } // on_build_key(...)

        /** @brief Retrieves the names of the database entry. */
        std::set<name_type> on_build_names(const value_type& item) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::on_build_names), 
                decltype(&type::on_build_names)>::value;
            static_assert(is_overwritten, "on_build_names(..) -> std::set<name_type> has not been overloaded.");

            const derived_type* that = static_cast<const derived_type*>(this);
            return that->on_build_names(item);
        } // on_relaxing(...)

        /** @brief Happens before \c relax(...) is called. */
        void on_relaxing(const value_type& item, bool& do_cancel) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::on_relaxing), 
                decltype(&type::on_relaxing)>::value;
            static_assert(is_overwritten, "on_relaxing(..) -> void has not been overloaded.");

            const derived_type* that = static_cast<const derived_type*>(this);
            that->on_relaxing(item, do_cancel);
        } // on_relaxing(...)

        /** @brief Happens after \c relax(...) has been called. */
        void on_relaxed(const value_type& item, const std::set<name_type>& relaxed_names, std::set<name_type>& strict_names) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::on_relaxed), 
                decltype(&type::on_relaxed)>::value;
            static_assert(is_overwritten, "on_relaxed(..) -> void has not been overloaded.");

            const derived_type* that = static_cast<const derived_type*>(this);
            that->on_relaxed(item, relaxed_names, strict_names);
        } // on_relaxed(...)

        /** @brief Happens when \c clear() is called. */
        void on_clear() noexcept
        {
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::on_clear), 
                decltype(&type::on_clear)>::value;
            static_assert(is_overwritten, "on_clear(..) -> void has not been overloaded.");

            derived_type* that = static_cast<derived_type*>(this);
            that->on_clear();
        } // on_clear(...)

        /** @brief Happens when an item is about to be loaded. */
        template <typename t_logger_type>
        void on_loading(const value_type& item, bool& do_cancel, t_logger_type& logger) const noexcept
        {
            do_cancel = false;
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::template on_loading<t_logger_type>), 
                decltype(&type::template on_loading<t_logger_type>)>::value;
            static_assert(is_overwritten, "on_loading(..) -> void has not been overloaded.");

            const derived_type* that = static_cast<const derived_type*>(this);
            that->on_loading(item, do_cancel, logger);
        } // on_loading(...)

        /** @brief Happens when an item has been loaded. */
        void on_loaded(const value_type& item) noexcept
        {
            constexpr bool is_overwritten = !std::is_same<
                decltype(&derived_type::on_loaded), 
                decltype(&type::on_loaded)>::value;
            static_assert(is_overwritten, "on_loaded(..) -> void has not been overloaded.");

            derived_type* that = static_cast<derived_type*>(this);
            that->on_loaded(item);
        } // on_loaded(...)

    public:
        name_database() noexcept { }

        /** Clears the contents of the database. */
        void clear() noexcept
        {
            this->m_database.clear();
            this->m_name_tree.clear();
            this->on_clear();
        } // clear(...)

        bool is_valid(const value_type& item) const noexcept { return &item != &this->m_invalid; }

        key_type build_key(const value_type& item) const noexcept { return this->on_build_key(item); }

        std::set<name_type> build_names(const value_type& item) const noexcept { return this->on_build_names(item); }

        const std::map<key_type, value_type>& data() const noexcept { return this->m_database; }

        /** @brief Access elements by key.
         *  @exception std::out_of_range 
         *  @param ec Set to std::errc::bad_address if specified \p key is not in the database.
         */
        const value_type& at(const key_type& key, std::error_code& ec) const noexcept
        {
            auto search = this->m_database.find(key);
            if (search != this->m_database.end()) return search->second;

            aftermath::detail::on_error(ec, std::errc::bad_address, "<key> is not present in the database.");
            return this->m_invalid;
        } // at(...)

        /** @brief Tries to find an item matching the \p query in the database.
         *  @param filter Allows filtering search results. Use \c nullptr if you do not want fitering.
         */
        template <typename t_predicate_type>
        const value_type& find(const name_type& query, const t_predicate_type& filter) const noexcept 
        {
            detail::no_logger logger {};
            return this->find(query, filter, logger);
        } // find(...)

        template <typename t_predicate_type, typename t_logger_type>
        const value_type& find(const name_type& query, const t_predicate_type& filter, t_logger_type& logger) const noexcept 
        {
            name_type lowercase = type::relax_to_lowercase(query);
            name_type misspelled = type::relax_spelling(lowercase);
            bool is_valid = false;

            // Stage 0: prefix tree search.
            std::optional<node_type> search_a = this->m_name_tree.search(query);
            if (search_a.has_value())
            {
                const value_type& item = this->as_single(search_a.value(), filter, is_valid);
                if (is_valid) return item;
            } // if (...)

            // Stage 1: lowercase lookup.
            std::optional<node_type> search_b = this->m_name_tree.search(lowercase);
            if (search_b.has_value())
            {
                const value_type& item = this->as_single(search_b.value(), filter, is_valid);
                if (is_valid) return item;
                logger.write(std::string("Multiple units match the specified query: ") + lowercase + std::string("."));
                return this->m_invalid;
            } // if (...)

            // Stage 2: misspelled lookup.
            std::optional<node_type> search_c = this->m_name_tree.search(misspelled);
            if (search_c.has_value())
            {
                const value_type& item = this->as_single(search_c.value(), filter, is_valid);
                if (is_valid) return item;
                logger.write(std::string("Multiple units match the specified query: ") + misspelled + std::string("."));
                return this->m_invalid;
            } // if (...)

            return this->m_invalid;
        } // find(...)

        template <typename t_logger_type>
        bool add(const value_type& item, t_logger_type& logger) noexcept
        {
            bool do_cancel = false;
            this->on_loading(item, do_cancel, logger);
            if (do_cancel) return false;

            key_type key = this->build_key(item);
            std::set<name_type> names = this->build_names(item); // Names for logging purposes.
            std::set<name_type> strict_names {};
            std::set<name_type> tree_names = this->relax(item, strict_names);
            std::set<name_type> tree_synonyms = type::relax(strict_names);

            if (this->m_database.count(key) != 0)
            {
                logger.write(std::string("Unit with id (") + std::to_string(key) + std::string(") already exists."));
                return false;
            } // if (...)
            else if (this->m_name_tree.contains_any(tree_names))
            {
                logger.write(std::string("Unit with a similar name (") + char_string::join(names, ", ") + std::string(") already exists."));
                return false;
            } // if (...)
            else
            {
                this->m_database.emplace(key, item);
                this->m_name_tree.try_add_many(key, tree_names);
                for (const name_type& explicit_synonym : tree_synonyms) this->m_name_tree.add_synonym(key, explicit_synonym);
                this->on_loaded(item);
                return true;
            } // if (...)
        } // add(...)
    }; // struct name_database
} // namespace ropufu::settlers_online

#endif // ROPUFU_SETTLERS_ONLINE_NAME_DATABASE_HPP_INCLUDED
