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
 * @brief Modern C++20 interface for iterating over HIPO events
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
 *
 * // With index tracking
 * for (auto [event, idx] : file.enumerate()) {
 *     fmt::print("Event {}: {} particles\n", idx, event["REC::Particle"].rows());
 * }
 *
 * // Process subset
 * for (auto event : file.take(1000)) {
 *     // First 1000 events only
 * }
 * @endcode
 */
#pragma once

#include <bank.h>
#include <dictionary.h>
#include <event.h>
#include <reader.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
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
class ChainIterator;

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
 * - Functional-style processing (for_each, transform, filter)
 *
 * @code
 * hipo::EventFile file("data.hipo");
 *
 * // Simple iteration
 * for (auto event : file) {
 *     auto& particles = event["REC::Particle"];
 * }
 *
 * // With index
 * for (auto [event, idx] : file.enumerate()) {
 *     // idx is the event index
 * }
 *
 * // Process subset
 * for (auto event : file.take(1000)) {
 *     // First 1000 events
 * }
 *
 * // Functional style
 * file.for_each([](auto& event) {
 *     auto& particles = event["REC::Particle"];
 * });
 * @endcode
 */
class EventFile {
    friend class EventProxy;

   public:
    class Iterator;
    class EnumerateView;
    class TakeView;
    class FilterView;

    using iterator = Iterator;
    using const_iterator = Iterator;

    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Open a HIPO file for reading
     * @param filename Path to the HIPO file
     * @param tags Optional event tags to filter (empty = all events)
     * @param verbosity Reader verbosity level (0 = quiet)
     * @throws std::runtime_error if file cannot be opened
     */
    explicit EventFile(std::string_view filename,
                       const std::vector<long>& tags = {},
                       int verbosity = 0)
        : m_filename(filename) {
        m_reader = std::make_unique<reader>();
        if (!tags.empty()) m_reader->setTags(tags);
        m_reader->setVerbose(verbosity);

        ErrorCode status = m_reader->open(m_filename.c_str());
        if (status != ErrorCode::NO_ERROR) {
            throw std::runtime_error("Failed to open HIPO file: " + m_filename +
                                     " (Error: " + ErrorCode::toString(status) + ")");
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

    // =========================================================================
    // File Information
    // =========================================================================

    /// File path
    [[nodiscard]] std::string_view filename() const noexcept { return m_filename; }

    /// Total number of events in the file
    [[nodiscard]] std::size_t size() const noexcept { return m_reader->getEntries(); }

    /// Check if file is empty
    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    /// Number of records in the file
    [[nodiscard]] std::size_t record_count() const noexcept { return m_reader->getNRecords(); }

    /// Access the file's schema dictionary
    [[nodiscard]] const dictionary& schemas() const noexcept { return m_dictionary; }

    /// Get list of available bank names
    [[nodiscard]] std::vector<std::string> bank_names() {
        return m_dictionary.getSchemaList();
    }

    /// Check if a bank schema exists
    [[nodiscard]] bool has_bank(std::string_view name) const {
        return m_dictionary.hasSchema(name.data());
    }

    // Legacy aliases
    [[nodiscard]] std::size_t entries() const noexcept { return size(); }
    [[nodiscard]] std::size_t nrecords() const noexcept { return record_count(); }

    // =========================================================================
    // Random Access
    // =========================================================================

    /// Seek to a specific event index
    bool seek(long event_index) { return m_reader->gotoEvent(event_index); }

    /// Read the next event into the provided event object
    bool read(event& evt) { return m_reader->next(evt); }

    /// Get event at specific index (random access)
    bool get_event(event& evt, long index) { return m_reader->getEvent(evt, index); }

    // Legacy aliases
    bool goto_event(long idx) { return seek(idx); }
    bool read_next_event(event& evt) { return read(evt); }
    bool getEvent(event& evt, int index) { return get_event(evt, index); }

    // =========================================================================
    // Bank Factory
    // =========================================================================

    /// Create bank instance from schema name
    [[nodiscard]] bank create_bank(std::string_view name) const {
        return bank(m_dictionary.getSchema(name.data()));
    }

    /// Get cached bank template (for repeated use)
    [[nodiscard]] bank& get_bank_template(std::string_view name) {
        return m_bank_templates[std::string(name)];
    }

    // Legacy alias
    [[nodiscard]] bank getBank(const std::string& name) { return create_bank(name); }

    // =========================================================================
    // Error Handling
    // =========================================================================

    /// Get error code from last operation
    [[nodiscard]] ErrorCode error_code() const noexcept { return m_reader->getErrorCode(); }

    // Legacy alias
    [[nodiscard]] ErrorCode getErrorCode() const noexcept { return error_code(); }

    // =========================================================================
    // Configuration Access
    // =========================================================================

    /// Check if user configuration contains a key
    [[nodiscard]] bool has_config(std::string_view name) const {
        return m_reader->hasConfig(std::string(name));
    }

    /// Get user configuration value by key
    [[nodiscard]] std::optional<std::string> get_config(std::string_view name) const {
        if (m_reader->hasConfig(std::string(name))) {
            return m_reader->getConfig(std::string(name));
        }
        return std::nullopt;
    }

    /// Get all configuration keys
    [[nodiscard]] std::vector<std::string> config_keys() const {
        // Note: This would need reader support to enumerate keys
        return {};
    }

    // Legacy aliases
    [[nodiscard]] bool hasConfig(const std::string& name) const { return has_config(name); }
    [[nodiscard]] std::string getConfig(const std::string& name) const {
        return get_config(name).value_or("");
    }
    void showConfig() const { m_reader->showConfig(); }

    // =========================================================================
    // Record Information
    // =========================================================================

    /// Get record positions
    [[nodiscard]] std::span<const RecordPosition> record_positions() const {
        return m_reader->getRecordPositions();
    }

    /// Get total event count
    [[nodiscard]] int event_count() const noexcept { return m_reader->getEventCount(); }

    // Legacy aliases
    [[nodiscard]] const std::vector<RecordPosition>& getRecordPositions() const {
        return m_reader->getRecordPositions();
    }
    [[nodiscard]] int getEventCount() const noexcept { return event_count(); }

    // =========================================================================
    // Filtered Reading
    // =========================================================================

    /// Get next event containing all specified banks
    bool next_event_with_banks(event& evt, std::span<const std::string> bank_names) {
        std::vector<std::string> names(bank_names.begin(), bank_names.end());
        return m_reader->nextEvent(evt, names);
    }

    // Legacy alias
    bool nextEvent(event& evt, const std::vector<std::string>& bankNames) {
        return next_event_with_banks(evt, bankNames);
    }

    // =========================================================================
    // Debug/Info Display
    // =========================================================================

    /// Print record positions and event counts
    void show_records() const { m_reader->showRecords(); }

    /// Print file info summary
    void show_info() const { m_reader->showInfo(); }

    // Legacy aliases
    void showRecords() const { show_records(); }
    void showInfo() const { show_info(); }

    // =========================================================================
    // Advanced Access
    // =========================================================================

    /// Get the underlying reader (advanced use)
    [[nodiscard]] reader& get_reader() noexcept { return *m_reader; }
    [[nodiscard]] const reader& get_reader() const noexcept { return *m_reader; }

    // Legacy aliases
    [[nodiscard]] reader& getReader() noexcept { return *m_reader; }
    [[nodiscard]] const reader& getReader() const noexcept { return *m_reader; }

    // =========================================================================
    // Range-based Iteration
    // =========================================================================

    [[nodiscard]] Iterator begin();
    [[nodiscard]] Iterator end();

    // =========================================================================
    // Enhanced Iteration Views
    // =========================================================================

    /**
     * @brief Iterate with event index
     *
     * @code
     * for (auto [event, idx] : file.enumerate()) {
     *     fmt::print("Event {}\n", idx);
     * }
     * @endcode
     */
    [[nodiscard]] EnumerateView enumerate();

    /**
     * @brief Iterate over first N events
     *
     * @code
     * for (auto event : file.take(1000)) {
     *     // Process first 1000 events
     * }
     * @endcode
     */
    [[nodiscard]] TakeView take(std::size_t count);

    /**
     * @brief Iterate over events matching a predicate
     *
     * @code
     * for (auto event : file.filter([](auto& e) {
     *     return e["REC::Particle"].rows() > 0;
     * })) {
     *     // Only events with particles
     * }
     * @endcode
     */
    template <typename Predicate>
    [[nodiscard]] FilterView filter(Predicate&& pred);

    // =========================================================================
    // Functional-style Processing (implementations after Iterator is defined)
    // =========================================================================

    /// Apply function to each event
    template <typename Func>
    void for_each(Func&& func);

    /// Apply function to each event with index
    template <typename Func>
    void for_each_indexed(Func&& func);

    /// Count events matching a predicate
    template <typename Predicate>
    [[nodiscard]] std::size_t count_if(Predicate&& pred);

    /// Find first event matching a predicate
    template <typename Predicate>
    [[nodiscard]] std::optional<std::size_t> find_if(Predicate&& pred);

   private:
    std::string m_filename;
    std::unique_ptr<reader> m_reader;
    dictionary m_dictionary;
    mutable std::unordered_map<std::string, bank> m_bank_templates;

    void initialize_bank_cache() {
        const auto& schema_names = m_dictionary.getSchemaList();
        m_bank_templates.reserve(schema_names.size());

        for (const auto& name : schema_names) {
            schema& sch = m_dictionary.getSchema(name.c_str());
            m_bank_templates.emplace(name, bank(sch));
        }
    }

};

// Legacy alias
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
 *
 *     // Check if bank exists and has data
 *     if (auto& cal = event["REC::Calorimeter"]; !cal.empty()) {
 *         // Process calorimeter data
 *     }
 * }
 * @endcode
 *
 * @note EventProxy does not own any data - it's valid only for the current iteration.
 */
class EventProxy {
    friend class EventFile;
    friend class EventFile::Iterator;
    friend class chain;
    friend class ChainIterator;

   public:
    EventProxy() : m_event(nullptr), m_file(nullptr) {}

    EventProxy(EventProxy&&) noexcept = default;
    EventProxy& operator=(EventProxy&&) noexcept = default;

    // -------------------------------------------------------------------------
    // Bank Access - Single Bank
    // -------------------------------------------------------------------------

    /// Get bank by name (returns reference to cached bank)
    [[nodiscard]] bank& operator[](std::string_view bank_name) const {
        return get(bank_name);
    }

    /// Explicit get method (same as operator[])
    [[nodiscard]] bank& get(std::string_view bank_name) const {
        std::string name(bank_name);
        bank& b = m_file->m_bank_templates[name];
        m_event->read(b);
        return b;
    }

    /// Check if bank exists in schema
    [[nodiscard]] bool has_bank(std::string_view bank_name) const {
        return m_file->has_bank(bank_name);
    }

    /// Get bank if it exists and has rows, otherwise nullopt
    [[nodiscard]] std::optional<std::reference_wrapper<bank>>
    get_if_present(std::string_view bank_name) const {
        if (!has_bank(bank_name)) return std::nullopt;
        auto& b = get(bank_name);
        if (b.empty()) return std::nullopt;
        return b;
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
     * @return Tuple of pointers to bank objects
     */
    template <typename... Names>
    [[nodiscard]] auto banks(Names&&... names) const {
        return std::make_tuple(&get(std::forward<Names>(names))...);
    }

    /// Get underlying event object
    [[nodiscard]] event& raw_event() const noexcept { return *m_event; }

   private:
    event* m_event;
    EventFile* m_file;

    EventProxy(event* ev, EventFile* file) : m_event(ev), m_file(file) {}
};

// Legacy alias
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
    using iterator_category = std::input_iterator_tag;
    using value_type = EventProxy;
    using difference_type = std::ptrdiff_t;
    using pointer = EventProxy*;
    using reference = EventProxy;

    /// Default constructor creates end iterator
    Iterator() : m_reader(nullptr), m_file(nullptr), m_exhausted(true) {}

    Iterator& operator++() {
        if (m_reader && !m_exhausted) {
            if (!m_reader->next(m_current_event)) {
                m_exhausted = true;
            }
        }
        return *this;
    }

    void operator++(int) { ++(*this); }

    [[nodiscard]] value_type operator*() {
        return EventProxy(&m_current_event, m_file);
    }

    [[nodiscard]] bool operator==(const Iterator& other) const noexcept {
        return m_exhausted == other.m_exhausted;
    }

    [[nodiscard]] bool operator!=(const Iterator& other) const noexcept {
        return !(*this == other);
    }

   private:
    reader* m_reader;
    EventFile* m_file;
    event m_current_event;
    bool m_exhausted;

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
// EnumerateView - View for iterating with index
// =============================================================================

class EventFile::EnumerateView {
public:
    class EnumerateIterator {
    public:
        struct Value {
            EventProxy event;
            std::size_t index;
        };

        EnumerateIterator() : m_iter(), m_index(0) {}
        EnumerateIterator(Iterator iter, std::size_t idx)
            : m_iter(std::move(iter)), m_index(idx) {}

        EnumerateIterator& operator++() {
            ++m_iter;
            ++m_index;
            return *this;
        }

        [[nodiscard]] Value operator*() { return {*m_iter, m_index}; }

        [[nodiscard]] bool operator==(const EnumerateIterator& other) const {
            return m_iter == other.m_iter;
        }
        [[nodiscard]] bool operator!=(const EnumerateIterator& other) const {
            return !(*this == other);
        }

    private:
        Iterator m_iter;
        std::size_t m_index;
    };

    explicit EnumerateView(EventFile* file) : m_file(file) {}

    [[nodiscard]] EnumerateIterator begin() {
        return EnumerateIterator(m_file->begin(), 0);
    }
    [[nodiscard]] EnumerateIterator end() {
        return EnumerateIterator(m_file->end(), 0);
    }

private:
    EventFile* m_file;
};

// =============================================================================
// TakeView - View for iterating over first N events
// =============================================================================

class EventFile::TakeView {
public:
    class TakeIterator {
    public:
        TakeIterator() : m_iter(), m_remaining(0) {}
        TakeIterator(Iterator iter, std::size_t count)
            : m_iter(std::move(iter)), m_remaining(count) {}

        TakeIterator& operator++() {
            if (m_remaining > 0) {
                --m_remaining;
                ++m_iter;
            }
            return *this;
        }

        [[nodiscard]] EventProxy operator*() { return *m_iter; }

        [[nodiscard]] bool operator==(const TakeIterator& other) const {
            if (m_remaining == 0 && other.m_remaining == 0) return true;
            return m_iter == other.m_iter && m_remaining == other.m_remaining;
        }
        [[nodiscard]] bool operator!=(const TakeIterator& other) const {
            return !(*this == other);
        }

    private:
        Iterator m_iter;
        std::size_t m_remaining;
    };

    TakeView(EventFile* file, std::size_t count)
        : m_file(file), m_count(count) {}

    [[nodiscard]] TakeIterator begin() {
        return TakeIterator(m_file->begin(), m_count);
    }
    [[nodiscard]] TakeIterator end() {
        return TakeIterator(m_file->end(), 0);
    }

private:
    EventFile* m_file;
    std::size_t m_count;
};

// =============================================================================
// FilterView - View for iterating over filtered events
// =============================================================================

class EventFile::FilterView {
public:
    using Predicate = std::function<bool(EventProxy&)>;

    class FilterIterator {
    public:
        FilterIterator() : m_iter(), m_end(), m_pred(nullptr) {}
        FilterIterator(Iterator iter, Iterator end, Predicate* pred)
            : m_iter(std::move(iter)), m_end(std::move(end)), m_pred(pred) {
            advance_to_match();
        }

        FilterIterator& operator++() {
            ++m_iter;
            advance_to_match();
            return *this;
        }

        [[nodiscard]] EventProxy operator*() { return *m_iter; }

        [[nodiscard]] bool operator==(const FilterIterator& other) const {
            return m_iter == other.m_iter;
        }
        [[nodiscard]] bool operator!=(const FilterIterator& other) const {
            return !(*this == other);
        }

    private:
        Iterator m_iter;
        Iterator m_end;
        Predicate* m_pred;

        void advance_to_match() {
            while (m_iter != m_end) {
                auto proxy = *m_iter;
                if ((*m_pred)(proxy)) break;
                ++m_iter;
            }
        }
    };

    FilterView(EventFile* file, Predicate pred)
        : m_file(file), m_pred(std::move(pred)) {}

    [[nodiscard]] FilterIterator begin() {
        return FilterIterator(m_file->begin(), m_file->end(), &m_pred);
    }
    [[nodiscard]] FilterIterator end() {
        return FilterIterator(m_file->end(), m_file->end(), &m_pred);
    }

private:
    EventFile* m_file;
    Predicate m_pred;
};

// =============================================================================
// EventFile Method Implementations
// =============================================================================

inline EventFile::Iterator EventFile::begin() {
    // Reset to beginning of file for fresh iteration
    m_reader->gotoEvent(0);
    return Iterator(m_reader.get(), this);
}

inline EventFile::Iterator EventFile::end() {
    return Iterator();
}

inline EventFile::TakeView EventFile::take(std::size_t count) {
    return TakeView(this, count);
}

inline EventFile::EnumerateView EventFile::enumerate() {
    return EnumerateView(this);
}

template <typename Predicate>
inline EventFile::FilterView EventFile::filter(Predicate&& pred) {
    return FilterView(this, std::forward<Predicate>(pred));
}

// =============================================================================
// EventFile Functional Method Implementations
// =============================================================================

template <typename Func>
void EventFile::for_each(Func&& func) {
    for (auto event : *this) {
        func(event);
    }
}

template <typename Func>
void EventFile::for_each_indexed(Func&& func) {
    std::size_t idx = 0;
    for (auto event : *this) {
        func(event, idx++);
    }
}

template <typename Predicate>
std::size_t EventFile::count_if(Predicate&& pred) {
    std::size_t count = 0;
    for (auto event : *this) {
        if (pred(event)) ++count;
    }
    return count;
}

template <typename Predicate>
std::optional<std::size_t> EventFile::find_if(Predicate&& pred) {
    std::size_t idx = 0;
    for (auto event : *this) {
        if (pred(event)) return idx;
        ++idx;
    }
    return std::nullopt;
}

}  // namespace hipo
