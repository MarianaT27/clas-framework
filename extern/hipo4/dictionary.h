#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "result.h"
#include "schema.h"

namespace hipo {

/**
 * @brief A collection of schemas that defines the structure of a HIPO file.
 *
 * A dictionary maps schema names to schema objects and supports lookup by
 * name, by positional index, and by group number. It is populated either
 * by adding individual schema objects or by parsing schema definition
 * strings (single or batch).
 *
 * When reading a HIPO file, the reader extracts the dictionary record from
 * the file header and uses parse_all() to reconstruct the full set of
 * schemas. When writing, the writer serializes the dictionary back into
 * the file header.
 *
 * Name lookups use a sorted (name, index) table for O(log N) binary search.
 *
 * Example:
 * @code
 *   dictionary dict;
 *   dict.parse("event::particle/100/1{pid/S,px/F,py/F,pz/F}").value_or_throw();
 *   auto& s = dict.find("event::particle").value().get();
 * @endcode
 *
 * @see schema for individual bank structure definitions
 * @see reader for how dictionaries are loaded from files
 * @see writer for how dictionaries are written to files
 */
class dictionary {
public:
    /** @brief Default-construct an empty dictionary. */
    dictionary() = default;

    /**
     * @brief Find a schema by name.
     * @param name The schema name to look up (e.g. "event::particle").
     * @return A const reference to the schema (wrapped in reference_wrapper),
     *         or error_code::schema_not_found if no schema with that name exists.
     */
    result<std::reference_wrapper<const schema>> find(std::string_view name) const noexcept;

    /**
     * @brief Check whether a schema with the given name exists in this dictionary.
     * @param name The schema name to check.
     * @return True if the dictionary contains a schema with this name.
     */
    bool contains(std::string_view name) const noexcept;

    /**
     * @brief Add a schema to the dictionary.
     *
     * Inserts the schema and updates the internal sorted lookup table.
     * If a schema with the same name already exists, the behavior depends
     * on the implementation (typically replaces the existing entry).
     *
     * @param s The schema to add (moved into the dictionary).
     */
    void add(schema s);

    /**
     * @brief Parse a single schema definition string and add it to the dictionary.
     *
     * The string should be in the format "name/group/item{col/type,...}".
     *
     * @param schema_string The schema definition to parse.
     * @return ok() on success, or an error_code if parsing fails.
     */
    result<void> parse(std::string_view schema_string);

    /**
     * @brief Parse multiple schema definitions from a text block.
     *
     * Each line in the text is treated as a separate schema definition
     * string. Blank lines and lines that fail to parse are handled
     * according to the implementation.
     *
     * @param dict_text Multi-line text containing one schema definition per line.
     * @return ok() on success, or the first error_code encountered.
     */
    result<void> parse_all(std::string_view dict_text);

    /**
     * @brief Merge schemas from another dictionary into this one.
     *
     * Schemas already present in this dictionary (by name) are skipped.
     * If a schema with the same name exists but has a different layout
     * (different columns or types), returns error_code::schema_inconsistent.
     *
     * @param other The dictionary to merge from.
     * @return ok() on success, or error_code::schema_inconsistent on conflict.
     */
    result<void> merge(const dictionary& other);

    /**
     * @brief Return a list of all schema names in the dictionary.
     * @return Vector of schema name strings.
     */
    std::vector<std::string> schema_names() const;

    /** @brief Return the number of schemas in the dictionary. */
    int size() const noexcept { return static_cast<int>(m_schemas.size()); }

    /** @brief Return an iterator to the first schema. */
    auto begin() const noexcept { return m_schemas.begin(); }

    /** @brief Return an iterator past the last schema. */
    auto end()   const noexcept { return m_schemas.end(); }

    /**
     * @brief Access a schema by positional index.
     * @param index Zero-based index into the schema list.
     * @return Const reference to the schema at the given position.
     * @note No bounds checking. Passing an out-of-range index is undefined behavior.
     */
    const schema& operator[](int index) const noexcept { return m_schemas[index]; }

    /**
     * @brief Find all schemas that share a given group number.
     * @param group The group number to search for.
     * @return Vector of const references to matching schemas (may be empty).
     */
    std::vector<std::reference_wrapper<const schema>> find_by_group(int group) const;

    /**
     * @brief Validate all schemas and check for duplicate (group, item) pairs.
     *
     * Calls schema::validate() on each schema, then checks that no two
     * schemas share the same (group, item) identifier.
     *
     * @return ok() on success, or the first error_code encountered.
     */
    result<void> validate() const;

    /**
     * @brief Print all schemas to stdout for debugging.
     *
     * Outputs each schema's name, group, item, and column definitions
     * in a human-readable format.
     */
    void show() const;

private:
    /** @brief Ordered list of all schemas in this dictionary. */
    std::vector<schema>                          m_schemas;
    /** @brief Sorted (name, index) pairs for O(log N) name lookup. */
    std::vector<std::pair<std::string, int>>     m_lookup; // sorted name -> index
};

} // namespace hipo
