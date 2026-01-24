/*
 * File:   dictionary.h
 * Author: gavalian
 *
 * Created on April 27, 2017, 10:01 AM
 * Modernized for C++20
 */

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "hipoexceptions.h"

namespace hipo {

struct schemaEntry_t {
    std::string name;
    std::string type;
    int typeId{};
    int typeSize{};
    int offset{};
};

enum Type {
    kByte = 1,
    kShort = 2,
    kInt = 3,
    kFloat = 4,
    kDouble = 5,
    kLong = 8
};

class schema {
   private:
    std::map<std::string, int> schemaEntriesMap;
    std::vector<schemaEntry_t> schemaEntries;

    // Fast lookup cache using unordered_map for O(1) access
    mutable std::unordered_map<std::string, int> schemaEntriesCache;

    int groupid{};
    int itemid{};
    int rowLength{};
    mutable int warningCount{10};

    std::string schemaName;

    int getTypeSize(int id);
    int getTypeByString(std::string& typeName);

   public:
    schema() {
        groupid = 0;
        itemid = 0;
        rowLength = 0;
    }

    schema(std::string_view name, int __group, int __item) {
        schemaName = name;
        groupid = __group;
        itemid = __item;
    }

    // Legacy constructor for const char*
    schema(const char* name, int __group, int __item)
        : schema(std::string_view(name), __group, __item) {}

    schema(const schema& s) {
        schemaName = s.schemaName;
        schemaEntries = s.schemaEntries;
        schemaEntriesMap = s.schemaEntriesMap;
        schemaEntriesCache = s.schemaEntriesCache;
        groupid = s.groupid;
        itemid = s.itemid;
    }

    virtual ~schema() = default;

    void parse(const std::string& schString);

    [[nodiscard]] std::string getName() const { return schemaName; }

    [[nodiscard]] int getGroup() const noexcept { return groupid; }

    [[nodiscard]] int getItem() const noexcept { return itemid; }

    [[nodiscard]] int getSizeForRows(int rows);

    [[nodiscard]] int getRowLength() const noexcept {
        const auto nentries = schemaEntries.size() - 1;
        const auto& sch = schemaEntries[nentries];
        return sch.offset + sch.typeSize;
    }

    /// Get column index by name. Returns -1 if not found (prints warning).
    [[nodiscard]] int getEntryOrder(std::string_view name) const noexcept;

    /// Get column index by name. Returns std::nullopt if not found (no warning).
    [[nodiscard]] std::optional<int> getEntryOrderOpt(std::string_view name) const noexcept;

    // Legacy overload
    [[nodiscard]] int getEntryOrder(const char* name) const noexcept {
        return getEntryOrder(std::string_view(name));
    }

    [[nodiscard]] bool exists(std::string_view name) const noexcept {
        return schemaEntriesMap.count(std::string(name)) > 0;
    }

    // Legacy overload
    [[nodiscard]] bool exists(const char* name) const noexcept {
        return exists(std::string_view(name));
    }

    [[nodiscard]] int getOffset(int item, int order, int rows) const noexcept {
        const auto& sch = schemaEntries[item];
        return rows * sch.offset + order * sch.typeSize;
    }

    [[nodiscard]] int getOffset(std::string_view name, int order, int rows) const {
        auto idx = getEntryOrderOpt(name);
        if (!idx) {
            throw ColumnNotFound(schemaName, name);
        }
        return getOffset(*idx, order, rows);
    }

    // Legacy overload
    [[nodiscard]] int getOffset(const char* name, int order, int rows) const {
        return getOffset(std::string_view(name), order, rows);
    }

    /// Get offset parts separately for caching (avoids repeated multiplication)
    [[nodiscard]] std::pair<int, int> getOffsetParts(int item) const noexcept {
        const auto& sch = schemaEntries[item];
        return {sch.offset, sch.typeSize};
    }

    [[nodiscard]] int getEntryType(int item) const noexcept {
        return schemaEntries[item].typeId;
    }

    [[nodiscard]] int getEntryType(std::string_view name) const noexcept {
        auto item = getEntryOrderOpt(name);
        return item ? schemaEntries[*item].typeId : -1;
    }

    // Legacy overload
    [[nodiscard]] int getEntryType(const char* name) const noexcept {
        return getEntryType(std::string_view(name));
    }

    [[nodiscard]] std::string getEntryName(int item) const noexcept {
        return schemaEntries[item].name;
    }

    [[nodiscard]] int getEntries() const noexcept {
        return static_cast<int>(schemaEntries.size());
    }

    /// Modern alias for getEntries()
    [[nodiscard]] int size() const noexcept { return getEntries(); }

    void show();

    [[nodiscard]] std::string getSchemaString();
    [[nodiscard]] std::string getSchemaStringJson();

    schema& operator=(const schema& D) {
        if (this != &D) {
            schemaName = D.schemaName;
            groupid = D.groupid;
            itemid = D.itemid;
            schemaEntries = D.schemaEntries;
            schemaEntriesMap = D.schemaEntriesMap;
            schemaEntriesCache = D.schemaEntriesCache;
        }
        return *this;
    }
};

inline int schema::getEntryOrder(std::string_view name) const noexcept {
    // Try fast cache first (O(1) lookup)
    std::string key(name);
    if (auto it = schemaEntriesCache.find(key); it != schemaEntriesCache.end()) {
        return it->second;
    }

    // Fall back to ordered map
    if (auto it = schemaEntriesMap.find(key); it != schemaEntriesMap.end()) {
        // Cache for next lookup
        schemaEntriesCache[key] = it->second;
        return it->second;
    }

    if (warningCount > 0) {
        warningCount--;
        std::cout << "Warning, hipo::schema getEntryOrder: item '" << name
                  << "' not found in bank '" << schemaName
                  << "' - data for this item is not valid" << std::endl;
    }
    return -1;
}

inline std::optional<int> schema::getEntryOrderOpt(std::string_view name) const noexcept {
    // Try fast cache first (O(1) lookup)
    std::string key(name);
    if (auto it = schemaEntriesCache.find(key); it != schemaEntriesCache.end()) {
        return it->second;
    }

    // Fall back to ordered map
    if (auto it = schemaEntriesMap.find(key); it != schemaEntriesMap.end()) {
        // Cache for next lookup
        schemaEntriesCache[key] = it->second;
        return it->second;
    }

    return std::nullopt;
}

class dictionary {
   private:
    std::map<std::string, schema> factory;

   public:
    dictionary() = default;
    virtual ~dictionary() = default;

    [[nodiscard]] std::vector<std::string> getSchemaList();

    void addSchema(schema sc) { factory[sc.getName()] = std::move(sc); }

    [[nodiscard]] bool hasSchema(std::string_view name) const noexcept {
        return factory.count(std::string(name)) != 0;
    }

    // Legacy overload
    [[nodiscard]] bool hasSchema(const char* name) const noexcept {
        return hasSchema(std::string_view(name));
    }

    /// Get schema by name. Throws SchemaNotFound if not found.
    [[nodiscard]] schema& getSchema(std::string_view name) {
        std::string key(name);
        if (auto it = factory.find(key); it != factory.end()) {
            return it->second;
        }
        throw SchemaNotFound(name);
    }

    /// Get schema by name (const version). Throws SchemaNotFound if not found.
    [[nodiscard]] const schema& getSchema(std::string_view name) const {
        std::string key(name);
        if (auto it = factory.find(key); it != factory.end()) {
            return it->second;
        }
        throw SchemaNotFound(name);
    }

    // Legacy overload
    [[nodiscard]] schema& getSchema(const char* name) {
        return getSchema(std::string_view(name));
    }

    /// Get schema by name. Returns std::nullopt if not found (no exception).
    [[nodiscard]] std::optional<std::reference_wrapper<schema>>
    getSchemaOpt(std::string_view name) noexcept {
        std::string key(name);
        if (auto it = factory.find(key); it != factory.end()) {
            return std::ref(it->second);
        }
        return std::nullopt;
    }

    /// Get schema by name (const version). Returns std::nullopt if not found.
    [[nodiscard]] std::optional<std::reference_wrapper<const schema>>
    getSchemaOpt(std::string_view name) const noexcept {
        std::string key(name);
        if (auto it = factory.find(key); it != factory.end()) {
            return std::cref(it->second);
        }
        return std::nullopt;
    }

    bool parse(const char* schemaString);
    void show();
};

}  // namespace hipo

#endif /* DICTIONARY_H */
