#pragma once

#include <bank.h>
#include <dictionary.h>
#include <event.h>
#include <reader.h>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>



namespace hipo {

// Lightweight value wrapper with convenient type conversions
class bank_value {
private:
    std::variant<int8_t, int16_t, int32_t, int64_t, float, double> m_value;

public:
    // Constructors
    bank_value() : m_value(0) {}

    template<typename T>
    bank_value(T value) : m_value(value) {}

    // Implicit conversion operators for convenient usage
    operator int() const {
        return std::visit([](auto&& arg) -> int {
            return static_cast<int>(arg);
        }, m_value);
    }

    operator float() const {
        return std::visit([](auto&& arg) -> float {
            return static_cast<float>(arg);
        }, m_value);
    }

    operator double() const {
        return std::visit([](auto&& arg) -> double {
            return static_cast<double>(arg);
        }, m_value);
    }

    operator int64_t() const {
        return std::visit([](auto&& arg) -> int64_t {
            return static_cast<int64_t>(arg);
        }, m_value);
    }

    // Explicit access to specific types
    template<typename T>
    T as() const {
        return std::visit([](auto&& arg) -> T {
            return static_cast<T>(arg);
        }, m_value);
    }

    // Direct access to variant for std::visit compatibility
    const auto& variant() const { return m_value; }
    auto& variant() { return m_value; }
};

// Ultra-fast row accessor
class bank_row_view {
private:
    bank* m_bank;
    int m_row;

public:
    __attribute__((always_inline))
    bank_row_view(bank* b, int row) noexcept 
        : m_bank(b), m_row(row) {}

    template<typename T>
    __attribute__((always_inline))
    inline T get_as(int column) const noexcept {
        return m_bank->get<T>(column, m_row);
    }

    template<typename T>
    __attribute__((always_inline))
    inline T get_as(const char* column_name) const noexcept {
        int column = m_bank->getSchema().getEntryOrder(column_name);
        return m_bank->get<T>(column, m_row);
    }

    inline bank_value get(int column) const noexcept;
    inline bank_value get(const char* column_name) const noexcept;
};

// Highly optimized bank map
class bank_map {
private:
    bank* m_bank;
    schema* m_schema;
    int m_rows;
    
    std::unordered_map<std::string, int> m_column_indices;
    std::vector<int> m_column_types;
    bool m_cache_ready{false};

    void build_cache() {
        if (m_cache_ready) return;
        
        int n_entries = m_schema->getEntries();
        m_column_indices.reserve(n_entries);
        m_column_types.reserve(n_entries);
        
        for (int i = 0; i < n_entries; i++) {
            m_column_indices[m_schema->getEntryName(i)] = i;
            m_column_types.push_back(m_schema->getEntryType(i));
        }
        
        m_cache_ready = true;
    }

    // Internal methods for show() functions
    __attribute__((always_inline))
    inline int get_column_index_internal(const char* name) const noexcept {
        auto it = m_column_indices.find(name);
        return (it != m_column_indices.end()) ? it->second : -1;
    }

    __attribute__((always_inline))
    inline bank_value get_typed_internal(int column, int row) const noexcept {
        int offset = m_schema->getOffset(column, row, m_rows);
        
        switch(m_column_types[column]) {
            case kByte:   return static_cast<int8_t>(m_bank->getByteAt(offset));
            case kShort:  return static_cast<int16_t>(m_bank->getShortAt(offset));
            case kInt:    return m_bank->getIntAt(offset);
            case kFloat:  return m_bank->getFloatAt(offset);
            case kDouble: return m_bank->getDoubleAt(offset);
            case kLong:   return m_bank->getLongAt(offset);
            default:      return 0;
        }
    }

    std::string get_type_name(int column) const {
        switch(m_column_types[column]) {
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

        switch(m_column_types[column]) {
            case kByte:   return std::max(min_width, 6);
            case kShort:  return std::max(min_width, 8);
            case kInt:    return std::max(min_width, 10);
            case kFloat:  return std::max(min_width, 12);
            case kDouble: return std::max(min_width, 12);
            case kLong:   return std::max(min_width, 16);
            default:      return min_width;
        }
    }

    int get_terminal_width() const {
        // Try to get terminal width from environment or use default
        const char* columns_env = getenv("COLUMNS");
        if (columns_env) {
            int width = atoi(columns_env);
            if (width > 0) return width;
        }

        // Try using ioctl
        #ifdef TIOCGWINSZ
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
            return w.ws_col;
        }
        #endif

        return 120; // Default fallback
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

    void print_value_formatted(const bank_value& value, int width) const {
        std::visit([width](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t>) {
                printf("%*d ", width, static_cast<int>(arg));
            } else if constexpr (std::is_same_v<T, int64_t>) {
                printf("%*lld ", width, arg);
            } else if constexpr (std::is_same_v<T, float>) {
                printf("%*.5f ", width, arg);
            } else if constexpr (std::is_same_v<T, double>) {
                printf("%*.5f ", width, arg);
            }
        }, value.variant());
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
    bank_map(bank& b) : m_bank(&b), m_schema(&b.getSchema()), m_rows(b.getRows()) {
        build_cache();
    }

    void refresh() {
        m_rows = m_bank->getRows();
    }

    // FASTEST: Direct typed access with integer column index
    template<typename T>
    __attribute__((always_inline))
    inline T get_as(int column, int row) const noexcept {
        return m_bank->get<T>(column, row);
    }

    // FAST: Direct typed access with name (uses cached index)
    template<typename T>
    __attribute__((always_inline))
    inline T get_as(const char* column_name, int row) const noexcept {
        int column = get_column_index_internal(column_name);
        return m_bank->get<T>(column, row);
    }

    // Variant access
    __attribute__((always_inline))
    inline bank_value get(int column, int row) const noexcept {
        return get_typed_internal(column, row);
    }

    __attribute__((always_inline))
    inline bank_value get(const char* column_name, int row) const noexcept {
        int column = get_column_index_internal(column_name);
        return get_typed_internal(column, row);
    }

    // Row view access
    __attribute__((always_inline))
    inline bank_row_view operator[](int row) noexcept {
        return bank_row_view(m_bank, row);
    }

    // Get cached column index
    __attribute__((always_inline))
    inline int get_column_index(const char* column_name) const noexcept {
        return get_column_index_internal(column_name);
    }

    // Get cached column type
    __attribute__((always_inline))
    inline int get_column_type(int column) const noexcept {
        return m_column_types[column];
    }

    // Column-wise access
    template<typename T>
    std::vector<T> get_column_as(const char* column_name) const {
        int column = get_column_index_internal(column_name);
        if (column < 0) return {};
        
        std::vector<T> result;
        result.reserve(m_rows);
        
        for (int row = 0; row < m_rows; row++) {
            result.push_back(m_bank->get<T>(column, row));
        }
        return result;
    }

    template<typename T>
    std::vector<T> get_column_as(int column) const {
        std::vector<T> result;
        result.reserve(m_rows);
        
        for (int row = 0; row < m_rows; row++) {
            result.push_back(m_bank->get<T>(column, row));
        }
        return result;
    }

    std::vector<bank_value> get_column(const char* column_name) const {
        int column = get_column_index_internal(column_name);
        if (column < 0) return {};
        
        std::vector<bank_value> result;
        result.reserve(m_rows);
        
        for (int row = 0; row < m_rows; row++) {
            result.push_back(get_typed_internal(column, row));
        }
        return result;
    }

    std::vector<bank_value> get_column(int column) const {
        std::vector<bank_value> result;
        result.reserve(m_rows);
        
        for (int row = 0; row < m_rows; row++) {
            result.push_back(get_typed_internal(column, row));
        }
        return result;
    }

    

    // Getters
    __attribute__((always_inline))
    inline int get_rows() const noexcept { return m_rows; }
    
    __attribute__((always_inline))
    inline int get_columns() const noexcept { return m_schema->getEntries(); }
    
    __attribute__((always_inline))
    inline bank* get_bank() noexcept { return m_bank; }
    
    __attribute__((always_inline))
    inline const bank* get_bank() const noexcept { return m_bank; }

    void show() const {
        show(false, -1);
    }

    void show(int max_rows) const {
        show(false, max_rows);
    }

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
                    auto value = get_typed_internal(col, row);
                    print_value_formatted(value, display_widths[i]);
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
                    auto value = get_typed_internal(col, row);
                    print_value_formatted(value, display_widths[i]);
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

    void show_columns(const std::vector<std::string>& column_names, int max_rows = -1) const {
        std::vector<int> col_indices;
        std::vector<int> widths;
        for (const auto& col_name : column_names) {
            int col = get_column_index_internal(col_name.c_str());
            if (col < 0) {
                printf("Warning: Column '%s' not found\n", col_name.c_str());
                continue;
            }
            col_indices.push_back(col);
            widths.push_back(get_column_width(col));
        }

        if (col_indices.empty()) {
            printf("No valid columns to display\n");
            return;
        }

        int rows_to_show = (max_rows > 0 && max_rows < get_rows()) ? max_rows : get_rows();

        // Get terminal width and limit columns if needed
        int term_width = get_terminal_width();
        int available_width = term_width - 10;

        // Determine which columns to actually show
        std::vector<int> cols_to_show;
        std::vector<int> display_widths;
        int total_width = 6;
        bool truncated = false;

        for (size_t i = 0; i < col_indices.size(); i++) {
            int col_width = widths[i] + 3;
            if (total_width + col_width + 1 <= available_width) {
                cols_to_show.push_back(col_indices[i]);
                display_widths.push_back(widths[i]);
                total_width += col_width;
            } else {
                truncated = true;
                break;
            }
        }

        // Show at least one column
        if (cols_to_show.empty() && !col_indices.empty()) {
            cols_to_show.push_back(col_indices[0]);
            display_widths.push_back(widths[0]);
            total_width += widths[0] + 3;
            truncated = (col_indices.size() > 1);
        }

        // Calculate total table width
        int table_width = 8;  // row column (6) + 2 for borders
        for (int w : display_widths) {
            table_width += w + 3;  // width + " │ "
        }

        // Print bank info header (single line box style)
        printf("\n");

        char info_text[512];
        if (truncated) {
            snprintf(info_text, sizeof(info_text), " Bank: %s  |  Rows: %d  |  Columns: %lu/%lu selected shown",
                     m_schema->getName().c_str(), get_rows(),
                     cols_to_show.size(), col_indices.size());
        } else {
            snprintf(info_text, sizeof(info_text), " Bank: %s  |  Rows: %d  |  Columns: %lu selected",
                     m_schema->getName().c_str(), get_rows(),
                     col_indices.size());
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
            std::string name = m_schema->getEntryName(cols_to_show[i]);
            printf(" %-*s │", display_widths[i], name.c_str());
        }
        printf("\n");

        printf("│ %4s │", "");
        for (size_t i = 0; i < cols_to_show.size(); i++) {
            std::string type_name = get_type_name(cols_to_show[i]);
            printf(" \033[90m%-*s\033[0m │", display_widths[i], type_name.c_str());
        }
        printf("\n");

        print_horizontal_line(display_widths);

        for (int row = 0; row < rows_to_show; row++) {
            printf("│ %4d │", row);
            for (size_t i = 0; i < cols_to_show.size(); i++) {
                printf(" ");
                auto value = get_typed_internal(cols_to_show[i], row);
                print_value_formatted(value, display_widths[i]);
                printf("│");
            }
            printf("\n");
        }

        print_bottom_border(display_widths);

        if (max_rows > 0 && max_rows < get_rows()) {
            printf("  ... showing %d of %d total rows\n", max_rows, get_rows());
        }
        if (truncated) {
            printf("  Note: %lu columns hidden due to terminal width.\n",
                   col_indices.size() - cols_to_show.size());
        }
        printf("\n");
    }

    void show_rows(int start_row, int end_row) const {
        int n_columns = m_schema->getEntries();

        if (start_row < 0) start_row = 0;
        if (end_row >= get_rows()) end_row = get_rows() - 1;

        std::vector<int> widths;
        for (int col = 0; col < n_columns; col++) {
            widths.push_back(get_column_width(col));
        }

        // Get terminal width and determine which columns to show
        int term_width = get_terminal_width();
        int available_width = term_width - 10;

        std::vector<int> cols_to_show;
        int total_width = 6;
        bool truncated = false;

        for (int col = 0; col < n_columns; col++) {
            int col_width = widths[col] + 3;
            if (total_width + col_width + 1 <= available_width) {
                cols_to_show.push_back(col);
                total_width += col_width;
            } else {
                truncated = true;
                break;
            }
        }

        if (cols_to_show.empty() && n_columns > 0) {
            cols_to_show.push_back(0);
            total_width += widths[0] + 3;
            truncated = (n_columns > 1);
        }

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

        char info_text[512];
        if (truncated) {
            snprintf(info_text, sizeof(info_text), " Bank: %s  |  Rows: %d-%d  |  Columns: %lu/%d shown",
                     m_schema->getName().c_str(), start_row, end_row,
                     cols_to_show.size(), n_columns);
        } else {
            snprintf(info_text, sizeof(info_text), " Bank: %s  |  Rows: %d-%d",
                     m_schema->getName().c_str(), start_row, end_row);
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

        for (int row = start_row; row <= end_row; row++) {
            printf("│ %4d │", row);
            for (size_t i = 0; i < cols_to_show.size(); i++) {
                int col = cols_to_show[i];
                printf(" ");
                auto value = get_typed_internal(col, row);
                print_value_formatted(value, display_widths[i]);
                printf("│");
            }
            printf("\n");
        }

        print_bottom_border(display_widths);

        if (truncated) {
            printf("  Note: %d columns hidden due to terminal width. Use show_columns() to view specific columns.\n",
                   n_columns - (int)cols_to_show.size());
        }
        printf("\n");
    }
};

// Implement bank_row_view methods now that bank_map is defined
inline bank_value bank_row_view::get(int column) const noexcept {
    int offset = m_bank->getSchema().getOffset(column, m_row, m_bank->getRows());
    int type = m_bank->getSchema().getEntryType(column);
    
    switch(type) {
        case kByte:   return static_cast<int8_t>(m_bank->getByteAt(offset));
        case kShort:  return static_cast<int16_t>(m_bank->getShortAt(offset));
        case kInt:    return m_bank->getIntAt(offset);
        case kFloat:  return m_bank->getFloatAt(offset);
        case kDouble: return m_bank->getDoubleAt(offset);
        case kLong:   return m_bank->getLongAt(offset);
        default:      return 0;
    }
}

inline bank_value bank_row_view::get(const char* column_name) const noexcept {
    int column = m_bank->getSchema().getEntryOrder(column_name);
    return get(column);
}

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
    explicit hipoeventfile(const std::string& filename) {
        reader = std::make_unique<hipo::reader>();
        ErrorCode status = reader->open(filename.c_str());
        if (status != ErrorCode::NO_ERROR) {
            throw std::runtime_error("Failed to open HIPO file: " + filename + 
                                   " (Error code: " + ErrorCode::toString(status) + ")");
        }
        reader->readDictionary(dict);

        // Pre-create bank templates for all known schemas
        const auto& schema_list = dict.getSchemaList();
        bank_templates.reserve(schema_list.size());
        for (const std::string& name : schema_list) {
            schema& sch = dict.getSchema(name.c_str());
            bank_templates.emplace(name, bank(sch));
        }
    }

    hipoeventfile(const std::string& filename, const std::vector<long>& tags) {
        reader = std::make_unique<hipo::reader>();
        reader->setTags(tags);
        ErrorCode status = reader->open(filename.c_str());
        if (status != ErrorCode::NO_ERROR) {
            throw std::runtime_error("Failed to open HIPO file: " + filename + 
                                   " (Error code: " + ErrorCode::toString(status) + ")");
        }
        reader->readDictionary(dict);

        // Pre-create bank templates for all known schemas
        const auto& schema_list = dict.getSchemaList();
        bank_templates.reserve(schema_list.size());
        for (const std::string& name : schema_list) {
            schema& sch = dict.getSchema(name.c_str());
            bank_templates.emplace(name, bank(sch));
        }
    }

    hipoeventfile(hipoeventfile&& other) noexcept = default;
    hipoeventfile& operator=(hipoeventfile&& other) noexcept = default;
    hipoeventfile(const hipoeventfile&) = delete;
    hipoeventfile& operator=(const hipoeventfile&) = delete;

    void show_schemas() {
        printf("\n");
        printf("╔════════════════════════════════════════════════════════════════════════════════╗\n");
        printf("║                           HIPO BANK SCHEMAS                                    ║\n");
        printf("╚════════════════════════════════════════════════════════════════════════════════╝\n");
        
        for (auto& pair : bank_templates) {
            const std::string& name = pair.first;
            bank& bnk = pair.second;
            auto& schema = bnk.getSchema();
            
            printf("\n");
            printf("┌───────────────────────────────────────────────┐\n");
            printf("│ Bank: %-39s │\n", name.c_str());
            printf("├──────┬─────────────────────────────┬──────────┤\n");
            printf("│ %-4s │ %-27s │ %-8s │\n", 
                   "ID", "Name", "Type");
            printf("├──────┼─────────────────────────────┼──────────┤\n");
            
            int n_entries = schema.getEntries();
            for (int i = 0; i < n_entries; i++) {
                std::string entry_name = schema.getEntryName(i);
                int type = schema.getEntryType(i);
                
                std::string type_name;
                switch(type) {
                    case kByte:   type_name = "byte";   break;
                    case kShort:  type_name = "short";  break;
                    case kInt:    type_name = "int";    break;
                    case kFloat:  type_name = "float";  break;
                    case kDouble: type_name = "double"; break;
                    case kLong:   type_name = "long";   break;
                    default:      type_name = "unknown"; break;
                }
                
                printf("│ %4d │ %-27s │ %-8s │\n", 
                       i, entry_name.c_str(), type_name.c_str());
            }
            
            printf("└──────┴─────────────────────────────┴──────────┘\n");
        }
        
        printf("\n");
    }

    int get_entries() const {
        return reader->getEntries();
    }

    iterator begin();
    iterator end();
};

class iter_event {
    friend class hipoeventfile;
    friend class hipoeventfile::iterator;

   private:
    std::shared_ptr<event> event_ptr;
    hipoeventfile* file_ptr;
    mutable std::unordered_map<std::string, bank> event_banks;

    iter_event(std::shared_ptr<event> ev, hipoeventfile* file)
        : event_ptr(ev), file_ptr(file) {}

   public:
    iter_event() : event_ptr(nullptr), file_ptr(nullptr) {}
    iter_event(const iter_event&) = default;
    iter_event& operator=(const iter_event&) = default;
    iter_event(iter_event&&) noexcept = default;
    iter_event& operator=(iter_event&&) noexcept = default;

    // Declare but don't define yet (needs complete hipoeventfile type)
    inline bank& get_bank(std::string_view bankName) const;
    inline bank_map get_bank_map(std::string_view bankName) const;

    bool is_valid() const {
        return event_ptr != nullptr && file_ptr != nullptr;
    }
};
class hipoeventfile::iterator {
    friend class hipoeventfile;

   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = iter_event;
    using reference = iter_event&;
    using pointer = iter_event*;

   private:
    hipo::reader* reader_ptr;
    hipoeventfile* file_ptr;
    std::shared_ptr<event> current_event;
    iter_event current_wrap;
    bool at_end;

    iterator(hipo::reader* rdr, hipoeventfile* file) 
        : reader_ptr(rdr), file_ptr(file), at_end(false) {
        
        current_event = std::make_shared<event>();
        
        if (!reader_ptr || !reader_ptr->next(*current_event)) {
            reader_ptr = nullptr;
            file_ptr = nullptr;
            at_end = true;
            current_event.reset();
        } else {
            current_wrap = iter_event(current_event, file_ptr);
        }
    }

   public:
    iterator() : reader_ptr(nullptr), file_ptr(nullptr), at_end(true) {}

    inline iterator& operator++() {
        if (!reader_ptr || at_end) return *this;
        
        if (!reader_ptr->next(*current_event)) {
            reader_ptr = nullptr;
            file_ptr = nullptr;
            at_end = true;
            current_event.reset();
        } else {
            current_wrap.event_ptr = current_event;
            current_wrap.file_ptr = file_ptr;
        }
        return *this;
    }

    inline reference operator*() { return current_wrap; }
    inline pointer operator->() { return &current_wrap; }
    
    inline bool operator!=(const iterator& other) const { 
        return at_end != other.at_end; 
    }
    
    inline bool operator==(const iterator& other) const { 
        return at_end == other.at_end; 
    }
};

// ============================================================================
// METHOD IMPLEMENTATIONS (now that all classes are fully defined)
// ============================================================================

inline hipoeventfile::iterator hipoeventfile::begin() {
    reader->rewind();
    return {reader.get(), this};
}

inline hipoeventfile::iterator hipoeventfile::end() {
    return {};
}

// Now we can implement get_bank since hipoeventfile is complete
inline bank& iter_event::get_bank(std::string_view bankName) const {
    if (!event_ptr || !file_ptr) {
        throw std::runtime_error("Invalid HipoEvent (no event data)");
    }

    std::string bankNameStr(bankName);

    // Check if template exists
    auto template_it = file_ptr->bank_templates.find(bankNameStr);
    if (template_it == file_ptr->bank_templates.end()) {
        throw std::runtime_error("Schema not found for bank: " + bankNameStr);
    }

    // Check if we already have a bank instance for this event
    auto bank_it = event_banks.find(bankNameStr);
    if (bank_it == event_banks.end()) {
        // Create a new bank instance from the template
        auto result = event_banks.emplace(bankNameStr, template_it->second);
        bank_it = result.first;
    }

    // Read the bank data from the event into our per-event bank instance
    event_ptr->read(bank_it->second);
    return bank_it->second;
}

inline bank_map iter_event::get_bank_map(std::string_view bankName) const {
    return bank_map(get_bank(bankName));
}





}  // namespace hipo