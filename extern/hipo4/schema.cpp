#include "schema.h"

#include <algorithm>
#include <charconv>
#include <string_view>

#include <fmt/format.h>

namespace hipo {

namespace {

std::string_view trim(std::string_view sv) noexcept {
    auto start = sv.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string_view::npos) return {};
    auto end = sv.find_last_not_of(" \t\n\r\f\v");
    return sv.substr(start, end - start + 1);
}

result<column_type> type_from_char(std::string_view t) noexcept {
    if (t == "B") return column_type::byte_t;
    if (t == "S") return column_type::short_t;
    if (t == "I") return column_type::int_t;
    if (t == "F") return column_type::float_t;
    if (t == "D") return column_type::double_t;
    if (t == "L") return column_type::long_t;
    return unexpected(error_code::schema_parse_failed);
}

// Split string_view by delimiter, call func for each token
template <typename F>
void split(std::string_view sv, char delim, F&& func) {
    while (!sv.empty()) {
        auto pos = sv.find(delim);
        auto token = sv.substr(0, pos);
        func(token);
        if (pos == std::string_view::npos) break;
        sv.remove_prefix(pos + 1);
    }
}

} // anonymous namespace

result<schema> schema::parse(std::string_view name, int group, int item,
                             std::string_view columns) {
    std::vector<schema_entry> entries;
    std::vector<std::pair<std::string, int>> lookup;
    int offset = 0;
    int index = 0;
    bool had_error = false;

    split(columns, ',', [&](std::string_view token) {
        if (had_error) return;
        auto slash = token.find('/');
        if (slash == std::string_view::npos) {
            had_error = true;
            return;
        }
        auto col_name = trim(token.substr(0, slash));
        auto col_type_str = trim(token.substr(slash + 1));

        auto ct = type_from_char(col_type_str);
        if (!ct) {
            had_error = true;
            return;
        }

        int ts = type_size(*ct);
        entries.push_back(schema_entry{
            .name      = std::string(col_name),
            .type      = *ct,
            .type_size = ts,
            .offset    = offset,
        });
        lookup.emplace_back(std::string(col_name), index);
        offset += ts;
        ++index;
    });

    if (had_error || entries.empty())
        return unexpected(error_code::schema_parse_failed);

    std::sort(lookup.begin(), lookup.end(),
              [](auto const& a, auto const& b) { return a.first < b.first; });

    return schema(std::string(name), group, item,
                  std::move(entries), std::move(lookup));
}

result<void> schema::validate() const noexcept {
    if (m_entries.empty())
        return unexpected(error_code::schema_empty);

    // Check for duplicate column names (m_lookup is sorted by name)
    for (size_t i = 1; i < m_lookup.size(); ++i) {
        if (m_lookup[i].first == m_lookup[i - 1].first)
            return unexpected(error_code::schema_duplicate_column);
    }

    // Check type_size matches and no overlapping offsets
    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto const& e = m_entries[i];
        if (e.type_size != type_size(e.type))
            return unexpected(error_code::schema_offset_overlap);

        if (i + 1 < m_entries.size()) {
            int end = e.offset + e.type_size;
            if (end > m_entries[i + 1].offset)
                return unexpected(error_code::schema_offset_overlap);
        }
    }

    return ok();
}

result<int> schema::column_index(std::string_view col_name) const noexcept {
    auto it = std::lower_bound(m_lookup.begin(), m_lookup.end(), col_name,
        [](auto const& pair, std::string_view sv) { return pair.first < sv; });
    if (it != m_lookup.end() && it->first == col_name)
        return it->second;
    return unexpected(error_code::schema_column_not_found);
}

result<column_handle> schema::get_handle(std::string_view col_name) const noexcept {
    auto idx = column_index(col_name);
    if (!idx) return unexpected(idx.error());
    return column_handle(*idx);
}

bool schema::contains(std::string_view col_name) const noexcept {
    return column_index(col_name).has_value();
}

std::string schema::to_schema_string() const {
    std::string columns;
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        if (i > 0) columns += ',';
        char type_char = [](column_type t) -> char {
            switch (t) {
            case column_type::byte_t:   return 'B';
            case column_type::short_t:  return 'S';
            case column_type::int_t:    return 'I';
            case column_type::float_t:  return 'F';
            case column_type::double_t: return 'D';
            case column_type::long_t:   return 'L';
            }
            return '?';
        }(m_entries[i].type);
        fmt::format_to(std::back_inserter(columns), "{}/{}", m_entries[i].name, type_char);
    }
    return fmt::format("{{{}/{}/{}}}{{{}}}", m_name, m_group, m_item, columns);
}

std::string schema::to_json_string() const {
    std::string result;
    result += fmt::format("{{ \"name\": \"{}\", \"group\": {}, \"item\": {}, \"info\": \" \",",
                          m_name, m_group, m_item);
    result += "\"entries\": [ ";
    auto type_str = [](column_type t) -> const char* {
        switch (t) {
        case column_type::byte_t:   return "B";
        case column_type::short_t:  return "S";
        case column_type::int_t:    return "I";
        case column_type::float_t:  return "F";
        case column_type::double_t: return "D";
        case column_type::long_t:   return "L";
        }
        return "?";
    };
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        if (i > 0) result += ',';
        result += fmt::format("{{\"name\":\"{}\", \"type\":\"{}\", \"info\":\" \"}}",
                              m_entries[i].name, type_str(m_entries[i].type));
    }
    result += "] }";
    return result;
}

} // namespace hipo
