#include "dictionary.h"

#include <algorithm>
#include <charconv>
#include <fmt/format.h>

namespace hipo {

namespace {

std::string_view trim(std::string_view sv) noexcept {
    auto start = sv.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string_view::npos) return {};
    auto end = sv.find_last_not_of(" \t\n\r\f\v");
    return sv.substr(start, end - start + 1);
}

// Extract content between nth occurrence of {start} and {end}
std::string_view extract_braced(std::string_view sv, int occurrence) noexcept {
    int count = 0;
    for (size_t i = 0; i < sv.size(); ++i) {
        if (sv[i] == '{') {
            if (count == occurrence) {
                auto end = sv.find('}', i + 1);
                if (end == std::string_view::npos) return {};
                return sv.substr(i + 1, end - i - 1);
            }
            ++count;
        }
    }
    return {};
}

} // anonymous namespace

result<std::reference_wrapper<const schema>> dictionary::find(std::string_view name) const noexcept {
    auto it = std::lower_bound(m_lookup.begin(), m_lookup.end(), name,
        [](auto const& pair, std::string_view sv) { return pair.first < sv; });
    if (it != m_lookup.end() && it->first == name)
        return std::cref(m_schemas[it->second]);
    return unexpected(error_code::schema_not_found);
}

bool dictionary::contains(std::string_view name) const noexcept {
    return find(name).has_value();
}

void dictionary::add(schema s) {
    std::string name_str(s.name());
    m_schemas.push_back(std::move(s));
    int idx = static_cast<int>(m_schemas.size()) - 1;

    auto it = std::lower_bound(m_lookup.begin(), m_lookup.end(), name_str,
        [](auto const& pair, std::string const& sv) { return pair.first < sv; });
    m_lookup.emplace(it, std::move(name_str), idx);
}

result<void> dictionary::parse(std::string_view schema_string) {
    // Format: {name/group/item}{col1/T,col2/T,...}
    auto head = extract_braced(schema_string, 0);
    auto body = extract_braced(schema_string, 1);
    if (head.empty() || body.empty())
        return unexpected(error_code::dictionary_parse_failed);

    // Parse head: name/group/item
    auto slash1 = head.find('/');
    if (slash1 == std::string_view::npos)
        return unexpected(error_code::dictionary_parse_failed);
    auto slash2 = head.find('/', slash1 + 1);
    if (slash2 == std::string_view::npos)
        return unexpected(error_code::dictionary_parse_failed);

    auto name = trim(head.substr(0, slash1));
    auto group_str = trim(head.substr(slash1 + 1, slash2 - slash1 - 1));
    auto item_str = trim(head.substr(slash2 + 1));

    int group = 0, item_id = 0;
    auto [p1, ec1] = std::from_chars(group_str.data(), group_str.data() + group_str.size(), group);
    auto [p2, ec2] = std::from_chars(item_str.data(), item_str.data() + item_str.size(), item_id);
    if (ec1 != std::errc{} || ec2 != std::errc{})
        return unexpected(error_code::dictionary_parse_failed);

    auto sc = schema::parse(name, group, item_id, body);
    if (!sc)
        return unexpected(sc.error());

    add(std::move(*sc));
    return ok();
}

result<void> dictionary::parse_all(std::string_view dict_text) {
    // Dictionary text contains multiple schema strings, each in {head}{body} format
    // separated by newlines
    size_t pos = 0;
    while (pos < dict_text.size()) {
        // Skip whitespace / newlines
        pos = dict_text.find('{', pos);
        if (pos == std::string_view::npos) break;

        // Find matching closing } for head
        auto head_end = dict_text.find('}', pos);
        if (head_end == std::string_view::npos) break;

        // Find opening { for body
        auto body_start = dict_text.find('{', head_end + 1);
        if (body_start == std::string_view::npos) break;

        // Find closing } for body
        auto body_end = dict_text.find('}', body_start);
        if (body_end == std::string_view::npos) break;

        auto schema_str = dict_text.substr(pos, body_end - pos + 1);
        auto res = parse(schema_str);
        if (!res)
            return res;

        pos = body_end + 1;
    }
    return ok();
}

std::vector<std::string> dictionary::schema_names() const {
    std::vector<std::string> names;
    names.reserve(m_schemas.size());
    for (auto const& s : m_schemas)
        names.emplace_back(s.name());
    return names;
}

std::vector<std::reference_wrapper<const schema>> dictionary::find_by_group(int group) const {
    std::vector<std::reference_wrapper<const schema>> results;
    for (auto const& s : m_schemas) {
        if (s.group() == group)
            results.push_back(std::cref(s));
    }
    return results;
}

void dictionary::show() const {
    for (auto const& s : m_schemas) {
        fmt::print("{:>24s} : {:5d} {:5d} {:5d}\n",
                   s.name(), s.group(), s.item(), s.entries());
    }
}

result<void> dictionary::validate() const {
    for (auto const& s : m_schemas) {
        auto res = s.validate();
        if (!res) return res;
    }
    // Check for duplicate (group, item) pairs
    for (size_t i = 0; i < m_schemas.size(); ++i) {
        for (size_t j = i + 1; j < m_schemas.size(); ++j) {
            if (m_schemas[i].group() == m_schemas[j].group() &&
                m_schemas[i].item()  == m_schemas[j].item())
                return unexpected(error_code::schema_inconsistent);
        }
    }
    return ok();
}

result<void> dictionary::merge(const dictionary& other) {
    for (auto const& s : other) {
        auto existing = find(s.name());
        if (existing) {
            auto const& e = existing->get();
            if (e.group() != s.group() || e.item() != s.item())
                return unexpected(error_code::schema_inconsistent);
            if (e.entries() != s.entries())
                return unexpected(error_code::schema_inconsistent);
            for (int c = 0; c < e.entries(); ++c) {
                if (e.entry(c).name != s.entry(c).name ||
                    e.entry(c).type != s.entry(c).type)
                    return unexpected(error_code::schema_inconsistent);
            }
            // Identical schema already present -- skip
        } else {
            add(s);
        }
    }
    return ok();
}

} // namespace hipo
