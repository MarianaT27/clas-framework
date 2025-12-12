//******************************************************************************
//*       ██╗  ██╗██╗██████╗  ██████╗     ██╗  ██╗    ██████╗                  *
//*       ██║  ██║██║██╔══██╗██╔═══██╗    ██║  ██║   ██╔═████╗                 *
//*       ███████║██║██████╔╝██║   ██║    ███████║   ██║██╔██║                 *
//*       ██╔══██║██║██╔═══╝ ██║   ██║    ╚════██║   ████╔╝██║                 *
//*       ██║  ██║██║██║     ╚██████╔╝         ██║██╗╚██████╔╝                 *
//*       ╚═╝  ╚═╝╚═╝╚═╝      ╚═════╝          ╚═╝╚═╝ ╚═════╝                  *
//************************ Jefferson National Lab (2017) ***********************
/**
 * @file hipoeventiterator.h
 * @brief Modern C++ interface for iterating over HIPO events
 *
 * This header provides a range-based iteration interface for HIPO files,
 * enabling idiomatic C++ patterns like:
 *
 * @code
 * hipo::EventFile file("data.hipo");
 * for (auto event : file) {
 *     auto& particles = event["REC::Particle"];
 *     // Process particles...
 * }
 * @endcode
 */
#pragma once

#include <bank.h>
#include <dictionary.h>
#include <event.h>
#include <reader.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// Platform-specific terminal width detection
#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace hipo {

// =============================================================================
// Forward Declarations
// =============================================================================

class EventFile;
class EventProxy;


// =============================================================================
// BankView - Lightweight view over a HIPO bank with cached column metadata
// =============================================================================

/**
 * @brief Provides efficient, cached access to HIPO bank data
 *
 * BankView wraps a hipo::bank with pre-computed column metadata for fast access.
 * It supports pretty-printing and type-safe value retrieval.
 *
 * @note This class does not own the underlying bank - it's a non-owning view.
 */
class BankView {
   public:
    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    BankView() noexcept : m_bank(nullptr), m_schema(nullptr), m_row_count(0) {}

    explicit BankView(bank& b) noexcept
        : m_bank(&b), m_schema(&b.getSchema()), m_row_count(b.getRows()) {
        cache_column_metadata();
    }

    // -------------------------------------------------------------------------
    // Core Accessors
    // -------------------------------------------------------------------------

    /// Refresh view with updated bank data (e.g., after reading new event)
    BankView& refresh(bank& b) noexcept {
        m_bank = &b;
        m_row_count = b.getRows();
        return *this;
    }

    /// Number of rows in the bank
    [[nodiscard]] int rows() const noexcept { return m_row_count; }

    /// Check if bank has any data
    [[nodiscard]] bool empty() const noexcept { return m_row_count == 0; }

    /// Check if bank is valid (non-null)
    [[nodiscard]] explicit operator bool() const noexcept { return m_bank != nullptr; }

    /// Access underlying bank (for compatibility with existing code)
    [[nodiscard]] bank& data() noexcept { return *m_bank; }
    [[nodiscard]] const bank& data() const noexcept { return *m_bank; }

    // Legacy method names for backward compatibility
    [[nodiscard]] int get_rows() const noexcept { return m_row_count; }
    [[nodiscard]] bank& get_bank() noexcept { return *m_bank; }
    [[nodiscard]] const bank& get_bank() const noexcept { return *m_bank; }

    template <typename T>
    [[nodiscard]] T get_as(const std::string& column, int row) const { return get<T>(column, row); }

    // -------------------------------------------------------------------------
    // Type-safe Value Access
    // -------------------------------------------------------------------------

    /// Get value by column name and row index
    template <typename T>
    [[nodiscard]] T get(const std::string& column, int row) const {
        for (const auto& [name, idx] : m_columns) {
            if (name == column) return m_bank->get<T>(idx, row);
        }
        std::fprintf(stderr, "[BankView] Error: unknown column '%s'\n", column.c_str());
        return T{};
    }

    /// Get value by column index and row index (faster, no lookup)
    template <typename T>
    [[nodiscard]] T get(int column, int row) const {
        return m_bank->get<T>(column, row);
    }

    // -------------------------------------------------------------------------
    // Display / Debugging
    // -------------------------------------------------------------------------

    /// Print bank contents to stdout
    void print(int max_rows = -1) const { print_table(false, max_rows); }

    /// Print all rows (ignoring any row filters)
    void print_all(int max_rows = -1) const { print_table(true, max_rows); }

   private:
    // -------------------------------------------------------------------------
    // Internal Types
    // -------------------------------------------------------------------------

    struct ColumnMeta {
        int type_id;
        int row_offset;
        int type_size;
    };

    // -------------------------------------------------------------------------
    // Member Data
    // -------------------------------------------------------------------------

    bank* m_bank;
    schema* m_schema;
    int m_row_count;
    std::vector<std::pair<std::string, int>> m_columns;
    std::vector<ColumnMeta> m_column_cache;

    // -------------------------------------------------------------------------
    // Internal Methods
    // -------------------------------------------------------------------------

    void cache_column_metadata() {
        const int count = m_schema->getEntries();
        m_columns.reserve(count);
        m_column_cache.reserve(count);

        for (int i = 0; i < count; ++i) {
            m_columns.emplace_back(m_schema->getEntryName(i), i);
            auto [offset_mult, size] = m_schema->getOffsetParts(i);
            m_column_cache.push_back({
                .type_id = m_schema->getEntryType(i),
                .row_offset = m_row_count * offset_mult,
                .type_size = size});
        }
    }

    [[nodiscard]] static int detect_terminal_width() {
        if (const char* env = std::getenv("COLUMNS")) {
            if (int w = std::atoi(env); w > 0) return w;
        }
#ifdef TIOCGWINSZ
        struct winsize ws {};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
            return ws.ws_col;
        }
#endif
        return 120;  // Default fallback
    }

    [[nodiscard]] std::string type_name(int column) const {
        switch (m_column_cache[column].type_id) {
            case kByte:   return "byte";
            case kShort:  return "short";
            case kInt:    return "int";
            case kFloat:  return "float";
            case kDouble: return "double";
            case kLong:   return "long";
            default:      return "?";
        }
    }

    [[nodiscard]] int column_width(int column) const {
        const auto name_len = static_cast<int>(m_schema->getEntryName(column).length());
        switch (m_column_cache[column].type_id) {
            case kByte:   return std::max(name_len, 4);
            case kShort:  return std::max(name_len, 6);
            case kInt:    return std::max(name_len, 8);
            case kFloat:  return std::max(name_len, 9);
            case kDouble: return std::max(name_len, 9);
            case kLong:   return std::max(name_len, 12);
            default:      return name_len;
        }
    }

    void print_value(int column, int row, int width) const {
        switch (m_column_cache[column].type_id) {
            case kByte:   std::printf("%*d ", width, m_bank->get<int8_t>(column, row)); break;
            case kShort:  std::printf("%*d ", width, m_bank->get<int16_t>(column, row)); break;
            case kInt:    std::printf("%*d ", width, m_bank->get<int32_t>(column, row)); break;
            case kFloat:  std::printf("%*.3f ", width, m_bank->get<float>(column, row)); break;
            case kDouble: std::printf("%*.3f ", width, m_bank->get<double>(column, row)); break;
            case kLong:   std::printf("%*lld ", width, static_cast<long long>(m_bank->get<int64_t>(column, row))); break;
            default:      std::printf("%*s ", width, "?");
        }
    }


    void print_border(const std::vector<int>& widths, const char* left, const char* mid,
                      const char* right, const char* fill) const {
        std::printf("%s", left);
        for (int k = 0; k < 6; ++k) std::printf("%s", fill);
        std::printf("%s", mid);
        for (size_t i = 0; i < widths.size(); ++i) {
            for (int j = 0; j < widths[i] + 2; ++j) std::printf("%s", fill);
            std::printf("%s", i < widths.size() - 1 ? mid : right);
        }
        std::printf("\n");
    }

    void print_table(bool show_all, int max_rows) const {
        if (!m_bank) return;

        const auto& row_list = m_bank->getRowList();
        const int n_cols = m_schema->getEntries();
        const bool use_all = show_all || !m_bank->getMutableRowList().isInitialized() ||
                             static_cast<int>(row_list.size()) == m_row_count;

        // Calculate column widths
        std::vector<int> widths;
        widths.reserve(n_cols);
        for (int c = 0; c < n_cols; ++c) widths.push_back(column_width(c));

        // Determine which columns fit in terminal
        const int term_width = detect_terminal_width() - 10;
        std::vector<int> visible_cols;
        int total_width = 6;
        bool truncated = false;

        for (int c = 0; c < n_cols; ++c) {
            if (total_width + widths[c] + 4 <= term_width) {
                visible_cols.push_back(c);
                total_width += widths[c] + 3;
            } else {
                truncated = true;
                break;
            }
        }

        if (visible_cols.empty() && n_cols > 0) {
            visible_cols.push_back(0);
            total_width += widths[0] + 3;
            truncated = (n_cols > 1);
        }

        std::vector<int> disp_widths;
        for (int c : visible_cols) disp_widths.push_back(widths[c]);

        const int table_width = 8 + [&] {
            int sum = 0;
            for (int w : disp_widths) sum += w + 3;
            return sum;
        }();

        // Print header
        std::printf("\n┌");
        for (int i = 0; i < table_width - 2; ++i) std::printf("─");
        std::printf("┐\n");

        const char* bank_name = m_schema->getName().c_str();
        const int display_rows = use_all ? m_row_count : static_cast<int>(row_list.size());
        if (truncated) {
            std::printf("│ Bank: %-20s  Rows: %-6d  Cols: %zu/%d │\n",
                        bank_name, display_rows, visible_cols.size(), n_cols);
        } else {
            std::printf("│ Bank: %-20s  Rows: %-6d %*s│\n",
                        bank_name, display_rows, table_width - 42, "");
        }

        print_border(disp_widths, "├", "┬", "┤", "─");

        // Column names
        std::printf("│ %4s │", "row");
        for (size_t i = 0; i < visible_cols.size(); ++i) {
            std::printf(" %-*s │", disp_widths[i], m_schema->getEntryName(visible_cols[i]).c_str());
        }
        std::printf("\n");

        // Column types (dimmed)
        std::printf("│ %4s │", "");
        for (size_t i = 0; i < visible_cols.size(); ++i) {
            std::printf(" \033[90m%-*s\033[0m │", disp_widths[i], type_name(visible_cols[i]).c_str());
        }
        std::printf("\n");

        print_border(disp_widths, "├", "┼", "┤", "─");

        // Data rows
        int rows_to_print = (max_rows > 0) ? std::min(max_rows, display_rows) : display_rows;

        if (use_all) {
            for (int r = 0; r < rows_to_print; ++r) {
                std::printf("│ %4d │", r);
                for (size_t i = 0; i < visible_cols.size(); ++i) {
                    std::printf(" ");
                    print_value(visible_cols[i], r, disp_widths[i]);
                    std::printf("│");
                }
                std::printf("\n");
            }
        } else {
            int count = 0;
            for (int r : row_list) {
                if (count >= rows_to_print) break;
                std::printf("│ %4d │", r);
                for (size_t i = 0; i < visible_cols.size(); ++i) {
                    std::printf(" ");
                    print_value(visible_cols[i], r, disp_widths[i]);
                    std::printf("│");
                }
                std::printf("\n");
                ++count;
            }
        }

        print_border(disp_widths, "└", "┴", "┘", "─");

        if (max_rows > 0 && max_rows < display_rows) {
            std::printf("  ... showing %d of %d rows\n", max_rows, display_rows);
        }
        if (truncated) {
            std::printf("  Note: %d columns hidden (terminal width)\n", n_cols - static_cast<int>(visible_cols.size()));
        }
        std::printf("\n");
    }
};

// Legacy alias for backward compatibility
using bank_map = BankView;


// =============================================================================
// EventFile - RAII wrapper for reading HIPO files with range-based iteration
// =============================================================================

/**
 * @brief RAII container for HIPO file access with modern C++ iteration support
 *
 * EventFile provides:
 * - Automatic resource management (RAII)
 * - Range-based for loop support
 * - Pre-cached bank schemas for efficient access
 * - Move-only semantics (non-copyable)
 *
 * @code
 * hipo::EventFile file("data.hipo");
 * std::cout << "Events: " << file.size() << "\n";
 *
 * for (auto event : file) {
 *     auto& particles = event["REC::Particle"];
 *     if (particles.empty()) continue;
 *     // Process...
 * }
 * @endcode
 */
class EventFile {
    friend class EventProxy;

   public:
    class Iterator;
    using iterator = Iterator;
    using const_iterator = Iterator;

    // -------------------------------------------------------------------------
    // Construction / Destruction
    // -------------------------------------------------------------------------

    /**
     * @brief Open a HIPO file for reading
     * @param filename Path to the HIPO file
     * @param tags Optional event tags to filter (empty = all events)
     * @param verbosity Reader verbosity level (0 = quiet)
     * @throws std::runtime_error if file cannot be opened
     */
    explicit EventFile(const std::string& filename, const std::vector<long>& tags = {}, int verbosity = 0) {
        m_reader = std::make_unique<reader>();
        if (!tags.empty()) m_reader->setTags(tags);
        m_reader->setVerbose(verbosity);

        ErrorCode status = m_reader->open(filename.c_str());
        if (status != ErrorCode::NO_ERROR) {
            throw std::runtime_error("Failed to open HIPO file: " + filename + " (Error: " + ErrorCode::toString(status) + ")");
        }

        m_reader->readDictionary(m_dictionary);
        initialize_bank_cache();
    }

    ~EventFile() = default;

    // Move-only semantics
    EventFile(EventFile&&) noexcept = default;
    EventFile& operator=(EventFile&&) noexcept = default;
    EventFile(const EventFile&) = delete;
    EventFile& operator=(const EventFile&) = delete;

    // -------------------------------------------------------------------------
    // File Information
    // -------------------------------------------------------------------------

    /// Total number of events in the file
    [[nodiscard]] std::size_t size() const { return m_reader->getEntries(); }

    /// Number of records in the file
    [[nodiscard]] std::size_t record_count() const { return m_reader->getNRecords(); }

    /// Access the file's schema dictionary
    [[nodiscard]] const dictionary& schemas() const { return m_dictionary; }

    // Legacy method names for backward compatibility
    [[nodiscard]] uintmax_t entries() const { return m_reader->getEntries(); }
    [[nodiscard]] uintmax_t nrecords() const { return m_reader->getNRecords(); }
    bool goto_event(long idx) { return seek(idx); }
    bool read_next_event(event& evt) { return read(evt); }

    // -------------------------------------------------------------------------
    // Random Access
    // -------------------------------------------------------------------------

    /// Seek to a specific event index
    bool seek(long event_index) { return m_reader->gotoEvent(event_index); }

    /// Read the next event into the provided event object
    bool read(event& evt) { return m_reader->next(evt); }

    // -------------------------------------------------------------------------
    // Range-based Iteration
    // -------------------------------------------------------------------------

    [[nodiscard]] Iterator begin();
    [[nodiscard]] Iterator end();

   private:
    std::unique_ptr<reader> m_reader;
    dictionary m_dictionary;
    mutable std::unordered_map<std::string, bank> m_bank_templates;
    mutable std::unordered_map<std::string, BankView> m_bank_views;

    void initialize_bank_cache() {
        const auto& schema_names = m_dictionary.getSchemaList();
        m_bank_templates.reserve(schema_names.size());
        m_bank_views.reserve(schema_names.size());

        for (const auto& name : schema_names) {
            schema& sch = m_dictionary.getSchema(name.c_str());
            m_bank_templates.emplace(name, bank(sch));
            m_bank_views.emplace(name, BankView(m_bank_templates[name]));
        }
    }
};

// Legacy alias for backward compatibility
using hipoeventfile = EventFile;


// =============================================================================
// EventProxy - Lightweight proxy for accessing banks within a single event
// =============================================================================

/**
 * @brief Non-owning proxy providing access to banks within a single event
 *
 * EventProxy is returned by dereferencing an EventFile::Iterator. It provides
 * convenient access to banks via operator[] or the banks() method for
 * structured bindings.
 *
 * @code
 * for (auto event : file) {
 *     // Single bank access
 *     auto& particles = event["REC::Particle"];
 *
 *     // Multiple banks with structured bindings
 *     auto [part, calo, track] = event.banks("REC::Particle", "REC::Calorimeter", "REC::Track");
 * }
 * @endcode
 *
 * @note EventProxy does not own any data - it's valid only for the current iteration.
 */
class EventProxy {
    friend class EventFile;
    friend class EventFile::Iterator;
    friend class chain;

   public:
    EventProxy() : m_event(nullptr), m_file(nullptr) {}

    EventProxy(EventProxy&&) noexcept = default;
    EventProxy& operator=(EventProxy&&) noexcept = default;

    // -------------------------------------------------------------------------
    // Bank Access - Single Bank
    // -------------------------------------------------------------------------

    /// Get bank by name (returns reference to cached BankView)
    [[nodiscard]] BankView& operator[](const std::string& bank_name) const {
        return get(bank_name);
    }

    /// Get bank by name (C-string overload)
    [[nodiscard]] BankView& operator[](const char* bank_name) const {
        return get(bank_name);
    }

    /// Explicit get method (same as operator[])
    [[nodiscard]] BankView& get(const std::string& bank_name) const {
        bank& b = m_file->m_bank_templates[bank_name];
        m_event->read(b);
        return m_file->m_bank_views[bank_name].refresh(b);
    }

    /// Legacy method names for backward compatibility
    [[nodiscard]] BankView& get_bank_map(const std::string& bank_name) const {
        return get(bank_name);
    }

    [[nodiscard]] bank& get_bank(const std::string& bank_name) const {
        return raw_bank(bank_name);
    }

    // -------------------------------------------------------------------------
    // Bank Access - Multiple Banks (Structured Bindings)
    // -------------------------------------------------------------------------

    /**
     * @brief Get multiple banks at once for use with structured bindings
     *
     * @code
     * auto [particles, calo, track] = event.banks(
     *     "REC::Particle", "REC::Calorimeter", "REC::Track"
     * );
     * @endcode
     *
     * @return Tuple of pointers to BankView objects
     */
    template <typename... Names>
    [[nodiscard]] auto banks(Names&&... names) const {
        return std::make_tuple(&get(std::forward<Names>(names))...);
    }

    // -------------------------------------------------------------------------
    // Raw Access (for advanced use)
    // -------------------------------------------------------------------------

    /// Get underlying hipo::bank directly (bypasses BankView)
    [[nodiscard]] bank& raw_bank(const std::string& bank_name) const {
        bank& b = m_file->m_bank_templates[bank_name];
        m_event->read(b);
        return b;
    }

   private:
    event* m_event;
    EventFile* m_file;

    EventProxy(event* ev, EventFile* file) : m_event(ev), m_file(file) {}
};

// Legacy alias for backward compatibility
using iter_event = EventProxy;


// =============================================================================
// EventFile::Iterator - Input iterator for range-based event processing
// =============================================================================

/**
 * @brief Input iterator for traversing events in an EventFile
 *
 * Satisfies the InputIterator concept, enabling range-based for loops
 * and standard algorithm compatibility.
 */
class EventFile::Iterator {
    friend class EventFile;

   public:
    // Iterator traits
    using iterator_category = std::input_iterator_tag;
    using value_type = EventProxy;
    using difference_type = std::ptrdiff_t;
    using pointer = EventProxy*;
    using reference = EventProxy;

    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    /// Default constructor creates end iterator
    Iterator() : m_reader(nullptr), m_file(nullptr), m_exhausted(true) {}

    // -------------------------------------------------------------------------
    // Iterator Operations
    // -------------------------------------------------------------------------

    /// Advance to next event
    Iterator& operator++() {
        if (m_reader && !m_exhausted) {
            if (!m_reader->next(m_current_event)) {
                m_exhausted = true;
            }
        }
        return *this;
    }

    /// Post-increment (returns void for input iterators)
    void operator++(int) { ++(*this); }

    /// Dereference - returns EventProxy for current event
    [[nodiscard]] value_type operator*() {
        return EventProxy(&m_current_event, m_file);
    }

    /// Equality comparison (for detecting end of iteration)
    [[nodiscard]] bool operator==(const Iterator& other) const {
        return m_exhausted == other.m_exhausted;
    }

    [[nodiscard]] bool operator!=(const Iterator& other) const {
        return !(*this == other);
    }

   private:
    reader* m_reader;
    EventFile* m_file;
    event m_current_event;
    bool m_exhausted;

    /// Private constructor for begin iterator
    Iterator(reader* rdr, EventFile* file)
        : m_reader(rdr), m_file(file), m_exhausted(false) {
        if (!m_reader || !m_reader->next(m_current_event)) {
            m_reader = nullptr;
            m_file = nullptr;
            m_exhausted = true;
        }
    }
};

// =============================================================================
// EventFile Iterator Factory Methods
// =============================================================================

inline EventFile::Iterator EventFile::begin() {
    return Iterator(m_reader.get(), this);
}

inline EventFile::Iterator EventFile::end() {
    return Iterator();
}

}  // namespace hipo
