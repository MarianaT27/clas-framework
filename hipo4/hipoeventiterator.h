#pragma once

#include <bank.h>
#include <dictionary.h>
#include <event.h>
#include <iostream>
#include <reader.h>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cstdlib>
#include <cstring>

// For terminal width detection
#ifdef __unix__
#include <unistd.h>
#include <sys/ioctl.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace hipo {
class bank_map {
private:
    struct column_info {
        int type_id;
        int row_offset;
        int type_size;
    };

    bank* m_bank;
    schema* m_schema;
    int m_rows;
    std::vector<std::pair<std::string, int>> m_column_indices;
    std::vector<column_info> column_info_cache;

    inline void print_value_at(int column, int row, int width) const {
        int type = column_info_cache[column].type_id;
        int offset = column_info_cache[column].row_offset + row * column_info_cache[column].type_size;

        switch(type) {
            case kByte:
                printf("%*d ", width, (int)m_bank->get<int8_t>(column, row, type, offset));
                break;
            case kShort:
                printf("%*d ", width, (int)m_bank->get<int16_t>(column, row, type, offset));
                break;
            case kInt:
                printf("%*d ", width, m_bank->get<int32_t>(column, row, type, offset));
                break;
            case kFloat:
                printf("%*.3f ", width, m_bank->get<float>(column, row, type, offset));
                break;
            case kDouble:
                printf("%*.3f ", width, m_bank->get<double>(column, row, type, offset));
                break;
            case kLong:
                printf("%*lld ", width, (long long)m_bank->get<int64_t>(column, row, type, offset));
                break;
            default:
                printf("%*s ", width, "?");
        }
    }

    std::string get_type_name(int column) const {
        int type = column_info_cache[column].type_id;
        switch(type) {
            case kByte:   return "byte";
            case kShort:  return "short";
            case kInt:    return "int";
            case kFloat:  return "float";
            case kDouble: return "double";
            case kLong:   return "long";
            default:      return "unknown";
        }
    }

    int get_column_width(int column) const {
        std::string name = m_schema->getEntryName(column);
        int min_width = name.length();
        int type = column_info_cache[column].type_id;

        switch(type) {
            case kByte:   return std::max(min_width, 4);
            case kShort:  return std::max(min_width, 6);
            case kInt:    return std::max(min_width, 8);
            case kFloat:  return std::max(min_width, 9);
            case kDouble: return std::max(min_width, 9);
            case kLong:   return std::max(min_width, 12);
            default:      return min_width;
        }
    }

    int get_terminal_width() const {
        const char* columns_env = getenv("COLUMNS");
        if (columns_env) {
            int width = atoi(columns_env);
            if (width > 0) return width;
        }

        #ifdef TIOCGWINSZ
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) return w.ws_col;
        #endif

        return 120; 
    }

    void truncate_string(char* dest, const char* src, int max_width) const {
        int len = strlen(src);
        if (len <= max_width) {
            strcpy(dest, src);
        } else if (max_width >= 3) {
            strncpy(dest, src, max_width - 3);
            dest[max_width - 3] = '.';
            dest[max_width - 2] = '.';
            dest[max_width - 1] = '.';
            dest[max_width] = '\0';
        } else {
            dest[0] = '\0';
        }
    }

    void print_horizontal_line(const std::vector<int>& widths) const {
        printf("├");
        printf("──────┼");
        for (size_t i = 0; i < widths.size(); i++) {
            for (int j = 0; j < widths[i] + 2; j++) printf("─");
            printf(i < widths.size() - 1 ? "┼" : "┤\n");
        }
    }

    void print_top_border(const std::vector<int>& widths) const {
        printf("├");
        printf("──────┬");
        for (size_t i = 0; i < widths.size(); i++) {
            for (int j = 0; j < widths[i] + 2; j++) printf("─");
            printf(i < widths.size() - 1 ? "┬" : "┤\n");
        }
    }

    void print_bottom_border(const std::vector<int>& widths) const {
        printf("└");
        printf("──────┴");
        for (size_t i = 0; i < widths.size(); i++) {
            for (int j = 0; j < widths[i] + 2; j++) printf("─");
            printf(i < widths.size() - 1 ? "┴" : "┘\n");
        }
    }

public:
    bank_map(bank& b) noexcept : m_bank(&b), m_schema(&b.getSchema()), m_rows(b.getRows()) {
        int count = m_schema->getEntries();
        m_column_indices.reserve(count);
        column_info_cache.reserve(count);
        for (int i = 0; i < m_schema->getEntries(); i++) {
            m_column_indices.emplace_back(m_schema->getEntryName(i), i);
            std::pair<int, int> parts = m_schema->getOffsetParts(i);
            column_info_cache.emplace_back(column_info{
                .type_id = m_schema->getEntryType(i),
                .row_offset = m_rows * parts.first,
                .type_size = parts.second
            });
        }
    }

    /// Get number of rows in the bank
    inline int get_rows() const noexcept { return m_rows; }

    template <typename T>
    inline T get_as(const std::string& columnName, int rowIndex) {
        for (const auto& [name, idx] : m_column_indices) {
            if (name == columnName) 
                return m_bank->get<T>(idx, rowIndex, column_info_cache[idx].type_id, column_info_cache[idx].row_offset + rowIndex * column_info_cache[idx].type_size);
        }
        printf("---> error(get_as | bank_map) : unknown type for [%s]\n", columnName.c_str());
        return 0;
    }

    /// Get bank
    inline bank& get_bank() const noexcept { return *m_bank; }

    /// Display bank contents in a formatted table
    void show() const {
        show(false, -1);
    }

    /// Display bank contents in a formatted table, limiting to max_rows if specified
    void show(int max_rows) const {
        show(false, max_rows);
    }

    /// Display bank contents in a formatted table, with option to show all rows
    void show(bool show_all_rows, int max_rows = -1) const {
        auto& row_list = m_bank->getRowList();
        int n_columns = m_schema->getEntries();

        bool loop_all_rows = show_all_rows || !m_bank->getMutableRowList().isInitialized()
                             || row_list.size() == get_rows();

        std::vector<int> widths;
        for (int col = 0; col < n_columns; col++) {
            widths.push_back(get_column_width(col));
        }

        // Get terminal width and calculate how many columns we can fit
        int term_width = get_terminal_width();
        int available_width = term_width - 10;  // Reserve space for row column and borders

        // Determine which columns to display
        std::vector<int> cols_to_show;
        int total_width = 6;  // "row" column width with borders
        bool truncated = false;

        for (int col = 0; col < n_columns; col++) {
            int col_width = widths[col] + 3;  // column width + " │ "
            if (total_width + col_width + 1 <= available_width) {
                cols_to_show.push_back(col);
                total_width += col_width;
            } else {
                truncated = true;
                break;
            }
        }

        // If we can't show any columns, show at least the first one
        if (cols_to_show.empty() && n_columns > 0) {
            cols_to_show.push_back(0);
            total_width += widths[0] + 3;
            truncated = (n_columns > 1);
        }

        // Build widths vector for only the columns we're showing
        std::vector<int> display_widths;
        for (int col : cols_to_show) {
            display_widths.push_back(widths[col]);
        }

        // Calculate total table width
        int table_width = 8;  // row column (6) + 2 for borders
        for (int w : display_widths) {
            table_width += w + 3;  // width + " │ "
        }

        // Print bank info header (single line box style)
        printf("\n");

        // Build info text
        char info_text[512];
        if (loop_all_rows) {
            if (truncated) {
                snprintf(info_text, sizeof(info_text), " Bank: %s    Rows: %d    Columns: %lu/%d shown",
                         m_schema->getName().c_str(), get_rows(),
                         cols_to_show.size(), n_columns);
            } else {
                snprintf(info_text, sizeof(info_text), " Bank: %s    Rows: %d",
                         m_schema->getName().c_str(), get_rows());
            }
        } else {
            if (truncated) {
                snprintf(info_text, sizeof(info_text), " Bank: %s    Rows: %lu (filtered)    Columns: %lu/%d shown",
                         m_schema->getName().c_str(), row_list.size(),
                         cols_to_show.size(), n_columns);
            } else {
                snprintf(info_text, sizeof(info_text), " Bank: %s    Rows: %lu (filtered)",
                         m_schema->getName().c_str(), row_list.size());
            }
        }

        // Top border (same width as table)
        printf("┌");
        for (int i = 0; i < table_width - 2; i++) printf("─");
        printf("┐\n");

        // Bank info (padded to match table width)
        printf("│%-*s│\n", table_width - 2, info_text);

        // Bottom border that connects to table top border
        print_top_border(display_widths);

        printf("│ %4s │", "row");
        for (size_t i = 0; i < cols_to_show.size(); i++) {
            int col = cols_to_show[i];
            std::string name = m_schema->getEntryName(col);
            printf(" %-*s │", display_widths[i], name.c_str());
        }
        printf("\n");

        printf("│ %4s │", "");
        for (size_t i = 0; i < cols_to_show.size(); i++) {
            int col = cols_to_show[i];
            std::string type_name = get_type_name(col);
            printf(" \033[90m%-*s\033[0m │", display_widths[i], type_name.c_str());
        }
        printf("\n");

        print_horizontal_line(display_widths);

        int rows_to_show = get_rows();
        if (max_rows > 0 && max_rows < rows_to_show) {
            rows_to_show = max_rows;
        }
        if (!loop_all_rows && (int)row_list.size() < rows_to_show) {
            rows_to_show = row_list.size();
        }

        if (loop_all_rows) {
            for (int row = 0; row < rows_to_show; row++) {
                printf("│ %4d │", row);
                for (size_t i = 0; i < cols_to_show.size(); i++) {
                    int col = cols_to_show[i];
                    printf(" ");
                    print_value_at(col, row, display_widths[i]);
                    printf("│");
                }
                printf("\n");
            }
        } else {
            int count = 0;
            for (auto const& row : row_list) {
                if (count >= rows_to_show) break;
                printf("│ %4d │", row);
                for (size_t i = 0; i < cols_to_show.size(); i++) {
                    int col = cols_to_show[i];
                    printf(" ");
                    print_value_at(col, row, display_widths[i]);
                    printf("│");
                }
                printf("\n");
                count++;
            }
        }

        print_bottom_border(display_widths);

        if (max_rows > 0 && max_rows < get_rows()) {
            printf("  ... showing %d of %d total rows\n", max_rows, get_rows());
        }
        if (truncated) {
            printf("  Note: %d columns hidden due to terminal width. Use show_columns() to view specific columns.\n",
                   n_columns - (int)cols_to_show.size());
        }
        printf("\n");
    }

};

class hipoeventfile {
    friend class iter_event;

   public:
    class iterator;
    using const_iterator = iterator;

   private:
    std::unique_ptr<hipo::reader> reader;
    dictionary dict;
    mutable std::unordered_map<std::string, bank> bank_templates;

   public:
    /**
     * @brief Construct from a HIPO file path
     * @param filename Path to the HIPO file
     * @param tags Optional list of tags to filter events
     * @param verbose Verbosity level for the reader
     * @throws std::runtime_error if the file cannot be opened
     */
    explicit hipoeventfile(const std::string& filename, const std::vector<long>& tags = {}, const int verbose = 0) {
        reader = std::make_unique<hipo::reader>();
        if (!tags.empty()) reader->setTags(tags);
        reader->setVerbose(verbose);
        ErrorCode status = reader->open(filename.c_str());
        if (status != ErrorCode::NO_ERROR) throw std::runtime_error("Failed to open HIPO file: " + filename + " (Error code: " + ErrorCode::toString(status) + ")");
        reader->readDictionary(dict);

        // Pre-create bank templates for all known schemas
        const std::vector<std::string>& schema_list = dict.getSchemaList();
        bank_templates.reserve(schema_list.size());
        for (const std::string& name : schema_list) {
            schema& sch = dict.getSchema(name.c_str());
            bank_templates.emplace(name, bank(sch));
        }
    }

    // Move semantics only - no copying
    hipoeventfile(hipoeventfile&& other) noexcept = default;
    hipoeventfile& operator=(hipoeventfile&& other) noexcept = default;
    hipoeventfile(const hipoeventfile&) = delete;
    hipoeventfile& operator=(const hipoeventfile&) = delete;

    iterator begin();
    iterator end();
};

class iter_event {
    friend class hipoeventfile;
    friend class hipoeventfile::iterator;

   private:
    event* event_ptr;
    hipoeventfile* file_ptr;
    iter_event(event* ev, hipoeventfile* file) : event_ptr(ev), file_ptr(file) {}

   public:
    iter_event() : event_ptr(nullptr), file_ptr(nullptr) {}
    iter_event(iter_event&&) noexcept = default;
    iter_event& operator=(iter_event&&) noexcept = default;

    inline bank& get_bank(const std::string& bankName) const {
        bank& bank_template = file_ptr->bank_templates[bankName];
        event_ptr->read(bank_template);
        return bank_template;
    }

    inline bank_map get_bank_map(const std::string& bankName) const {
        return bank_map(get_bank(bankName));
    }
};

class hipoeventfile::iterator {
    friend class hipoeventfile;

   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = iter_event;
    using reference = iter_event;
    using pointer = iter_event*;

   private:
    hipo::reader* reader_ptr;
    hipoeventfile* file_ptr;
    event current_event;
    bool at_end;

    iterator(hipo::reader* rdr, hipoeventfile* file) : reader_ptr(rdr), file_ptr(file), at_end(false) {
        if (!reader_ptr || !reader_ptr->next(current_event)) {
            reader_ptr = nullptr;
            file_ptr = nullptr;
            at_end = true;
        } 
    }

   public:
    iterator() : reader_ptr(nullptr), file_ptr(nullptr), at_end(true) {}

    inline iterator& operator++() {
        if (!reader_ptr || at_end) return *this;
        if (!reader_ptr->next(current_event)) at_end = true;
        return *this;
    }

    inline value_type operator*() { return iter_event(&current_event, file_ptr); }
    inline bool operator!=(const iterator& other) const { return at_end != other.at_end; }
    inline bool operator==(const iterator& other) const { return at_end == other.at_end; }
};

inline hipoeventfile::iterator hipoeventfile::begin() { return {reader.get(), this}; }
inline hipoeventfile::iterator hipoeventfile::end() { return {}; }

}  // namespace hipo