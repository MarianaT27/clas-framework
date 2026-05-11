#pragma once

#include "constants.h"
#include "datastream.h"
#include "dictionary.h"
#include "event.h"
#include "hipoexceptions.h"
#include "record.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace hipo {

///
/// @brief Per-file shared resource for the chain I/O pipeline.
///
/// Opens a HIPO file exactly once and exposes its header, dictionary, and
/// per-record position/event-count index to many worker threads at once.
/// Replaces the pattern in chain::process_impl ([chain.h:1077-1084]) where
/// each worker thread re-opens its own hipo::reader.
///
/// Thread-safety: all accessors are safe to call from any thread. loadRecord()
/// serializes I/O on an internal mutex — for higher throughput, place a
/// RecordPrefetcher in front (the prefetcher serializes through its own
/// dedicated I/O thread and lets workers consume from a cache).
///
class FileHandle {
public:
    FileHandle() = default;
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    ~FileHandle() = default;

    /// Open the file/URL. The right datastream backend is chosen by
    /// scheme. `tags` is the legacy event-tag filter (empty = read everything).
    /// Throws hipo::file_error on failure.
    void open(std::string_view url, const std::vector<long>& tags = {});

    /// True if the file was successfully opened.
    bool is_open() const noexcept { return m_stream != nullptr; }

    /// URL/path the handle was opened with.
    const std::string& url() const noexcept { return m_url; }

    /// Parsed bank-schema dictionary (populated at open time).
    hipo::dictionary&       dictionary()       noexcept { return m_dictionary; }
    const hipo::dictionary& dictionary() const noexcept { return m_dictionary; }

    /// Total records excluding the trailer/index record.
    int num_records() const noexcept {
        return static_cast<int>(m_record_positions.size());
    }

    /// Total events across all records.
    long num_events() const noexcept { return m_total_events; }

    /// Byte offset in the file where record `irec` starts.
    long record_position(int irec) const { return m_record_positions.at(irec); }

    /// Number of events in record `irec`.
    int record_event_count(int irec) const { return m_record_event_counts.at(irec); }

    /// Total file size in bytes (0 if unknown).
    long file_size() const noexcept { return m_file_size; }

    /// Header bit-info / version metadata.
    const fileHeader_t& header() const noexcept { return m_header; }

    ///
    /// @brief Thread-safe: read record `irec` into `out`.
    ///
    /// Serializes through an internal mutex. Returns false if irec is out of
    /// range; throws hipo::record_error on I/O failure.
    ///
    bool loadRecord(hipo::record& out, int irec) {
        if (irec < 0 || irec >= num_records()) return false;
        std::lock_guard<std::mutex> lock(m_io_mutex);
        out.readRecord(*m_stream, m_record_positions[irec], 0);
        return true;
    }

    ///
    /// @brief Allocate + read record `irec` into a new heap object.
    ///
    /// Used by the RecordPrefetcher / RecordCache to share records across
    /// worker threads via shared_ptr.
    ///
    std::shared_ptr<hipo::record> loadRecordShared(int irec) {
        auto r = std::make_shared<hipo::record>();
        if (!loadRecord(*r, irec)) return nullptr;
        return r;
    }

    /// Direct access for advanced users (e.g., the RecordPrefetcher I/O
    /// thread, which owns access during its run). Not thread-safe.
    hipo::datastream& stream() noexcept { return *m_stream; }
    std::mutex&       io_mutex() noexcept { return m_io_mutex; }

private:
    std::string                 m_url;
    std::unique_ptr<datastream> m_stream;
    std::mutex                  m_io_mutex;

    fileHeader_t                m_header{};
    long                        m_file_size = 0;

    hipo::dictionary            m_dictionary;

    // Record-level index built from the trailer.
    std::vector<long>           m_record_positions;
    std::vector<int>            m_record_event_counts;
    long                        m_total_events = 0;
};

inline void FileHandle::open(std::string_view url,
                             const std::vector<long>& tags) {
    m_url = std::string(url);
    m_stream = make_datastream(url);
    if (!m_stream) {
        throw file_error("hipo::FileHandle: empty URL");
    }
    m_stream->open(m_url.c_str());
    m_file_size = m_stream->size();

    // -------------------------------------------------------------
    // Read the 80-byte file header.
    // -------------------------------------------------------------
    std::vector<char> hbuf(80);
    m_stream->position(0);
    m_stream->read(hbuf.data(), 80);

    m_header.uniqueid         = *reinterpret_cast<int*>(&hbuf[FH_UNIQUE_WORD_OFFSET]);
    m_header.filenumber       = *reinterpret_cast<int*>(&hbuf[FH_FILE_NUMBER_OFFSET]);
    m_header.headerLength     = *reinterpret_cast<int*>(&hbuf[FH_HEADER_LENGTH_OFFSET]);
    m_header.recordCount      = *reinterpret_cast<int*>(&hbuf[FH_RECORD_COUNT_OFFSET]);
    m_header.indexArrayLength = *reinterpret_cast<int*>(&hbuf[FH_INDEX_ARRAY_LEN_OFFSET]);
    int word_8                = *reinterpret_cast<int*>(&hbuf[FH_BIT_INFO_OFFSET]);
    m_header.userHeaderLength = *reinterpret_cast<int*>(&hbuf[FH_USER_HEADER_LEN_OFFSET]);
    m_header.magicNumber      = *reinterpret_cast<int*>(&hbuf[FH_MAGIC_NUMBER_OFFSET]);
    m_header.userRegister     = *reinterpret_cast<long*>(&hbuf[FH_USER_REGISTER_OFFSET]);
    m_header.trailerPosition  = *reinterpret_cast<long*>(&hbuf[FH_TRAILER_POS_OFFSET]);

    if (m_header.magicNumber == HEADER_MAGIC_BE) {
        m_header.uniqueid         = __builtin_bswap32(m_header.uniqueid);
        m_header.filenumber       = __builtin_bswap32(m_header.filenumber);
        m_header.headerLength     = __builtin_bswap32(m_header.headerLength);
        m_header.recordCount      = __builtin_bswap32(m_header.recordCount);
        m_header.userHeaderLength = __builtin_bswap32(m_header.userHeaderLength);
        m_header.indexArrayLength = __builtin_bswap32(m_header.indexArrayLength);
        word_8                    = __builtin_bswap32(word_8);
        m_header.userRegister     = __builtin_bswap64(m_header.userRegister);
        m_header.trailerPosition  = __builtin_bswap64(m_header.trailerPosition);
    }
    m_header.version             = word_8 & BITINFO_VERSION_MASK;
    m_header.bitInfo             = (word_8 >> BITINFO_VERSION_BITS) & STRUCT_SIZE_MASK;
    m_header.firstRecordPosition =
        4 * m_header.headerLength + m_header.userHeaderLength;

    // -------------------------------------------------------------
    // Read the dictionary record (first record after the file header).
    // -------------------------------------------------------------
    {
        hipo::record dictRecord;
        dictRecord.readRecord(*m_stream, m_header.headerLength * 4L, 0);
        int nevents = dictRecord.getEventCount();
        hipo::event tmp;
        hipo::structure schemaStructure;
        for (int i = 0; i < nevents; i++) {
            dictRecord.readHipoEvent(tmp, i);
            tmp.getStructure(schemaStructure, DICT_GROUP, DICT_ITEM);
            m_dictionary.parse(schemaStructure.getStringAt(0).c_str());
        }
    }

    // -------------------------------------------------------------
    // Read the trailer/index record (position is in file header).
    // -------------------------------------------------------------
    if (m_header.trailerPosition <= 0) {
        // File was not properly closed; no random-access index available.
        return;
    }
    hipo::record trailer;
    trailer.readRecord(*m_stream, m_header.trailerPosition, 0);
    if (trailer.getEventCount() == 0) return;

    hipo::event idxEvent;
    trailer.readHipoEvent(idxEvent, 0);
    hipo::structure base;
    idxEvent.getStructure(base, FILE_INDEX_GROUP, FILE_INDEX_ITEM);

    int rows = base.getSize() / 32;
    m_record_positions.reserve(rows);
    m_record_event_counts.reserve(rows);
    for (int i = 0; i < rows; i++) {
        long position = base.getLongAt(i * 8);
        int  entries  = base.getIntAt (rows * 12 + i * 4);
        long uid1     = base.getLongAt(rows * 16 + i * 8);
        if (tags.empty()) {
            m_record_positions.push_back(position);
            m_record_event_counts.push_back(entries);
            m_total_events += entries;
        } else {
            bool accept = false;
            for (long t : tags) if (t == uid1) accept = true;
            if (accept) {
                m_record_positions.push_back(position);
                m_record_event_counts.push_back(entries);
                m_total_events += entries;
            }
        }
    }
}

} // namespace hipo
