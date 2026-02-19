#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <string_view>
#include <vector>

#include "bank.h"
#include "constants.h"

namespace hipo {

/**
 * @brief Builds a serialised event buffer from banks and raw structures.
 *
 * An event is laid out as:
 * @code
 * [EVNT:4][total_size:4][tag:4][reserved:4]   -- 16-byte event header
 * [group:2][item:1][type:1][data_size:4][data...]  -- per-structure
 * @endcode
 *
 * Banks are added as type-11 structures. Dictionary entries use type-6
 * (string) structures with group=120, item=2.
 *
 * @note NOT thread-safe. Each thread should use its own event_builder instance.
 *
 * @see writer::add_event, bank
 */
class event_builder {
public:
    /**
     * @brief Construct an event_builder with a default 64 KiB buffer reservation.
     */
    event_builder() {
        m_buffer.reserve(64 * 1024);
        reset();
    }

    /**
     * @brief Construct an event_builder with a custom initial buffer capacity.
     * @param initial_capacity Number of bytes to reserve for the internal buffer.
     */
    explicit event_builder(int initial_capacity) {
        m_buffer.reserve(initial_capacity);
        reset();
    }

    /**
     * @brief Reset the builder to an empty event.
     *
     * Clears all previously added structures and writes a fresh 16-byte
     * event header (EVNT magic, zero size, zero tag).
     */
    void reset() {
        m_buffer.resize(EVENT_HEADER_SIZE);
        m_buffer[0] = 'E'; m_buffer[1] = 'V';
        m_buffer[2] = 'N'; m_buffer[3] = 'T';
        write_value<uint32_t>(m_buffer.data() + 4, EVENT_HEADER_SIZE);
        write_value<uint32_t>(m_buffer.data() + 8, 0);
        write_value<uint32_t>(m_buffer.data() + 12, 0);
    }

    /**
     * @brief Add a bank to the event as a type-11 structure.
     *
     * The bank's schema group and item are used as the structure header
     * fields. Banks with zero data size are silently ignored.
     *
     * @param b The bank to serialise into this event.
     */
    void add(const bank& b) {
        if (b.data_size() == 0) return;
        add_structure(b.get_schema().group(), b.get_schema().item(), 11,
                      b.data());
    }

    /**
     * @brief Add a raw structure with explicit group, item, type, and data.
     *
     * @param group Structure group identifier (16-bit).
     * @param item  Structure item identifier (8-bit).
     * @param type  Structure type code (8-bit, e.g. 11 for banks, 6 for strings).
     * @param data  Raw data payload for the structure.
     */
    void add_structure(int group, int item, int type, std::span<const char> data) {
        int old_size = size();
        int struct_size = BANK_STRUCTURE_SIZE + static_cast<int>(data.size());
        m_buffer.resize(old_size + struct_size);
        char* ptr = m_buffer.data() + old_size;
        write_value<uint16_t>(ptr + 0, static_cast<uint16_t>(group));
        write_value<uint8_t>(ptr + 2, static_cast<uint8_t>(item));
        write_value<uint8_t>(ptr + 3, static_cast<uint8_t>(type));
        write_value<uint32_t>(ptr + 4, static_cast<uint32_t>(data.size()));
        std::memcpy(ptr + BANK_STRUCTURE_SIZE, data.data(), data.size());
        // Update total event size
        write_value<uint32_t>(m_buffer.data() + 4, static_cast<uint32_t>(old_size + struct_size));
    }

    /**
     * @brief Add a string as a type-6 structure.
     *
     * Convenience method for dictionary schema entries.
     *
     * @param group Structure group identifier (typically 120 for dictionary entries).
     * @param item  Structure item identifier (typically 2 for dictionary entries).
     * @param str   String content to store.
     */
    void add_string(int group, int item, std::string_view str) {
        add_structure(group, item, 6, {str.data(), str.size()});
    }

    /**
     * @brief Set the event tag in the event header.
     * @param tag Tag value to write into the header.
     */
    void set_tag(int tag) noexcept {
        write_value<uint32_t>(m_buffer.data() + 8, static_cast<uint32_t>(tag));
    }

    /**
     * @brief Return a read-only span over the complete serialised event.
     * @return A span covering the event header and all appended structures.
     */
    std::span<const char> data() const noexcept {
        return {m_buffer.data(), static_cast<size_t>(size())};
    }

    /**
     * @brief Return the total size of the serialised event in bytes.
     * @return Size including the 16-byte header and all structures.
     */
    int size() const noexcept {
        return static_cast<int>(read_value<uint32_t>(m_buffer.data() + 4));
    }

private:
    std::vector<char> m_buffer;
};

} // namespace hipo
