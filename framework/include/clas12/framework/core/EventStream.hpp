#pragma once

/**
 * @file EventStream.hpp
 * @brief Coroutine-based event stream wrapper for elegant event filtering
 *
 * This file provides EventStream, a wrapper around HIPO event iteration
 * that integrates with C++20 coroutines for elegant event filtering.
 *
 * @see Requirements 5.2, 5.4
 */

#include <functional>
#include <optional>
#include <vector>

#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Generator.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

/**
 * @brief Coroutine-based event stream for elegant event processing
 *
 * EventStream wraps HIPO event iteration and provides a coroutine-friendly
 * interface for event filtering. It supports both traditional iteration
 * and coroutine-based processing.
 *
 * Example usage with coroutines:
 * @code
 * // Define a coroutine that filters events
 * Generator<Event&> filter_electrons(EventStream& stream) {
 *     while (auto event = stream.next()) {
 *         auto& electrons = event->get().particles(clas12::PIDs::ELECTRON);
 *         if (!electrons.empty()) {
 *             co_yield event->get();
 *         }
 *     }
 * }
 *
 * // Use the filtered stream
 * EventStream stream(hipo_file);
 * for (Event& event : filter_electrons(stream)) {
 *     // Process only events with electrons
 * }
 * @endcode
 *
 * Example usage with filter():
 * @code
 * EventStream stream(hipo_file);
 * stream.filter([](Event& e) {
 *     return !e.particles(clas12::PIDs::ELECTRON).empty();
 * });
 *
 * while (auto event = stream.next()) {
 *     // Only events passing the filter
 * }
 * @endcode
 *
 * @see Requirement 5.2
 */
class EventStream {
public:
    /// Filter predicate type
    using FilterPredicate = std::function<bool(Event&)>;

    /**
     * @brief Construct an EventStream from a HIPO EventFile
     *
     * @param file Reference to the HIPO EventFile to iterate over
     */
    explicit EventStream(hipo::EventFile& file)
        : m_file(&file)
        , m_iterator(file.begin())
        , m_end(file.end())
        , m_exhausted(false) {}

    /**
     * @brief Default constructor creates an empty stream
     */
    EventStream() : m_file(nullptr), m_exhausted(true) {}

    // Move-only semantics
    EventStream(EventStream&&) noexcept = default;
    EventStream& operator=(EventStream&&) noexcept = default;
    EventStream(const EventStream&) = delete;
    EventStream& operator=(const EventStream&) = delete;

    /**
     * @brief Get the next event from the stream
     *
     * Returns the next event that passes all registered filters.
     * If no more events are available, returns std::nullopt.
     *
     * @return std::optional<std::reference_wrapper<Event>> The next event, or nullopt if exhausted
     */
    auto next() -> std::optional<std::reference_wrapper<Event>> {
        while (!m_exhausted && m_iterator != m_end) {
            // Get the current HIPO event proxy
            auto hipo_proxy = *m_iterator;
            
            // Create Framework Event from the HIPO proxy
            m_current_event.emplace(hipo_proxy);
            
            // Advance iterator for next call
            ++m_iterator;
            if (m_iterator == m_end) {
                m_exhausted = true;
            }
            
            // Apply filters
            if (passes_filters(*m_current_event)) {
                return std::ref(*m_current_event);
            }
        }
        
        return std::nullopt;
    }

    /**
     * @brief Add a filter predicate to the stream
     *
     * Events that don't pass the filter will be skipped during iteration.
     * Multiple filters can be added and are applied in order (AND logic).
     *
     * @param predicate Function returning true for events to keep
     * @return EventStream& Reference to this for chaining
     */
    auto filter(FilterPredicate predicate) -> EventStream& {
        m_filters.push_back(std::move(predicate));
        return *this;
    }

    /**
     * @brief Clear all filters
     * @return EventStream& Reference to this for chaining
     */
    auto clear_filters() -> EventStream& {
        m_filters.clear();
        return *this;
    }

    /**
     * @brief Check if the stream is exhausted
     * @return bool True if no more events are available
     */
    [[nodiscard]] auto exhausted() const noexcept -> bool {
        return m_exhausted;
    }

    /**
     * @brief Reset the stream to the beginning
     *
     * Resets iteration to the start of the file. Filters are preserved.
     */
    auto reset() -> void {
        if (m_file) {
            m_iterator = m_file->begin();
            m_end = m_file->end();
            m_exhausted = false;
            m_current_event.reset();
        }
    }

    /**
     * @brief Get the number of registered filters
     * @return std::size_t Number of filters
     */
    [[nodiscard]] auto filter_count() const noexcept -> std::size_t {
        return m_filters.size();
    }

    /**
     * @brief Check if the stream has any filters
     * @return bool True if filters are registered
     */
    [[nodiscard]] auto has_filters() const noexcept -> bool {
        return !m_filters.empty();
    }

private:
    hipo::EventFile* m_file;
    hipo::EventFile::Iterator m_iterator;
    hipo::EventFile::Iterator m_end;
    bool m_exhausted;
    std::vector<FilterPredicate> m_filters;
    std::optional<Event> m_current_event;

    /**
     * @brief Check if an event passes all registered filters
     * @param event The event to check
     * @return bool True if event passes all filters
     */
    auto passes_filters(Event& event) -> bool {
        for (const auto& filter : m_filters) {
            if (!filter(event)) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief Create a generator that yields events from a stream
 *
 * This is a convenience function for creating a coroutine-based
 * event generator from an EventStream.
 *
 * @param stream The event stream to iterate over
 * @return Generator<Event&> A generator yielding events
 *
 * Example:
 * @code
 * EventStream stream(file);
 * for (Event& event : events(stream)) {
 *     // Process each event
 * }
 * @endcode
 */
inline auto events(EventStream& stream) -> Generator<Event&> {
    while (auto event = stream.next()) {
        co_yield event->get();
    }
}

/**
 * @brief Create a filtered event generator
 *
 * Creates a generator that yields only events passing the given predicate.
 *
 * @param stream The event stream to iterate over
 * @param predicate Filter function returning true for events to keep
 * @return Generator<Event&> A generator yielding filtered events
 *
 * Example:
 * @code
 * EventStream stream(file);
 * auto has_electrons = [](Event& e) {
 *     return !e.particles(clas12::PIDs::ELECTRON).empty();
 * };
 *
 * for (Event& event : filtered_events(stream, has_electrons)) {
 *     // Only events with electrons
 * }
 * @endcode
 */
inline auto filtered_events(EventStream& stream, 
                            std::function<bool(Event&)> predicate) -> Generator<Event&> {
    while (auto event = stream.next()) {
        if (predicate(event->get())) {
            co_yield event->get();
        }
    }
}

/**
 * @brief Create a generator that yields events with a specific particle type
 *
 * Convenience function for filtering events by particle presence.
 *
 * @param stream The event stream to iterate over
 * @param pid The particle ID to require
 * @return Generator<Event&> A generator yielding events with the particle type
 *
 * Example:
 * @code
 * EventStream stream(file);
 * for (Event& event : events_with_particle(stream, clas12::PIDs::ELECTRON)) {
 *     // Only events with electrons
 * }
 * @endcode
 */
inline auto events_with_particle(EventStream& stream, 
                                  clas12::PIDs pid) -> Generator<Event&> {
    while (auto event = stream.next()) {
        if (!event->get().particles(pid).empty()) {
            co_yield event->get();
        }
    }
}

/**
 * @brief Create a generator that yields events with minimum particle count
 *
 * @param stream The event stream to iterate over
 * @param pid The particle ID to count
 * @param min_count Minimum number of particles required
 * @return Generator<Event&> A generator yielding events meeting the criteria
 */
inline auto events_with_min_particles(EventStream& stream,
                                       clas12::PIDs pid,
                                       std::size_t min_count) -> Generator<Event&> {
    while (auto event = stream.next()) {
        if (event->get().particles(pid).size() >= min_count) {
            co_yield event->get();
        }
    }
}

}  // namespace clas12::framework
