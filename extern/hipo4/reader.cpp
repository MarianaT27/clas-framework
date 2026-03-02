#include "reader.h"

namespace hipo {

// ============================================================================
// file_event_index
// ============================================================================

void file_event_index::add_record(int event_count, int64_t position, int record_bytes) {
    if (m_cumulative_events.empty())
        m_cumulative_events.push_back(0);

    int cumulative = m_cumulative_events.back() + event_count;
    m_cumulative_events.push_back(cumulative);
    m_positions.push_back(position);
    m_record_bytes.push_back(record_bytes);
}

void file_event_index::clear() {
    m_cumulative_events.clear();
    m_positions.clear();
    m_record_bytes.clear();
    m_current_event = -1;
    m_current_record = 0;
    m_current_record_event = -1;
}

void file_event_index::rewind() {
    m_current_event = -1;
    m_current_record = 0;
    m_current_record_event = -1;
}

bool file_event_index::can_advance() const noexcept {
    return m_current_event < max_events() - 1;
}

bool file_event_index::advance() noexcept {
    if (m_cumulative_events.empty()) return false;
    if (m_current_event + 1 < m_cumulative_events[m_current_record + 1]) {
        m_current_event++;
        m_current_record_event++;
        return true;
    }
    if (static_cast<int>(m_cumulative_events.size()) < m_current_record + 3)
        return false;
    m_current_event++;
    m_current_record++;
    m_current_record_event = 0;
    return true;
}

bool file_event_index::goto_event(int event_number) noexcept {
    if (event_number >= max_events()) return false;

    auto it = std::lower_bound(m_cumulative_events.begin(), m_cumulative_events.end(), event_number + 1);
    int record_idx = static_cast<int>(it - m_cumulative_events.begin()) - 1;

    m_current_record = record_idx;
    m_current_record_event = event_number - m_cumulative_events[record_idx];
    m_current_event = event_number;
    return true;
}

int file_event_index::max_events() const noexcept {
    if (m_cumulative_events.empty()) return 0;
    return m_cumulative_events.back();
}

int64_t file_event_index::record_position(int record_index) const noexcept {
    return m_positions[record_index];
}

int file_event_index::record_bytes(int record_index) const noexcept {
    return m_record_bytes[record_index];
}

// ============================================================================
// reader
// ============================================================================

result<file_metadata> reader::count_events(std::string_view path) {
    auto src = file_source::open(path);
    if (!src) return unexpected(src.error());
    auto& fs = *src;

    if (fs.size() < FILE_HEADER_SIZE)
        return unexpected(error_code::file_too_small);

    // Read file header
    char hdr[FILE_HEADER_SIZE];
    auto rd = fs.read_at(0, hdr, FILE_HEADER_SIZE);
    if (!rd) return unexpected(rd.error());

    auto unique_word = read_value<uint32_t>(hdr + FH_UNIQUE_WORD_OFFSET);
    if (unique_word != HIPO_FILE_UNIQUE_WORD)
        return unexpected(error_code::invalid_magic_number);

    auto magic = read_value<int32_t>(hdr + FH_MAGIC_NUMBER_OFFSET);
    bool swapped = (magic == static_cast<int32_t>(0x0001dac0));

    auto header_words   = maybe_swap32(read_value<int32_t>(hdr + FH_HEADER_LENGTH_OFFSET), swapped);
    auto index_len      = maybe_swap32(read_value<int32_t>(hdr + FH_INDEX_ARRAY_LEN_OFFSET), swapped);
    auto uh_len         = maybe_swap32(read_value<int32_t>(hdr + FH_USER_HEADER_LEN_OFFSET), swapped);
    auto record_count   = maybe_swap32(read_value<int32_t>(hdr + FH_RECORD_COUNT_OFFSET), swapped);

    int64_t pos = static_cast<int64_t>(header_words) * 4
                + index_len + uh_len + pad4(uh_len);

    int64_t total_events = 0;
    int num_records = 0;

    while (pos + 32 <= fs.size()) {
        char buf[32];
        auto r = fs.read_at(pos, buf, 32);
        if (!r || *r < 32) break;

        auto rec_len = read_value<int32_t>(buf + 0);
        auto evt_cnt = read_value<int32_t>(buf + 12);
        auto bit_w   = read_value<int32_t>(buf + 20);
        auto m       = read_value<int32_t>(buf + 28);

        bool sw = (m == static_cast<int32_t>(0x0001dac0));
        rec_len = maybe_swap32(rec_len, sw);
        evt_cnt = maybe_swap32(evt_cnt, sw);
        bit_w   = maybe_swap32(bit_w, sw);

        auto htype = static_cast<header_type>((bit_w >> 8) & 0xF);
        if (htype == header_type::hipo_trailer) break;
        if (m != static_cast<int32_t>(HEADER_MAGIC) && !sw) break;

        if (evt_cnt > 0) {
            total_events += evt_cnt;
            num_records++;
        }

        pos += static_cast<int64_t>(rec_len) * 4;
    }

    // Prefer header record_count if available and scan found fewer
    if (record_count > num_records && record_count > 0)
        num_records = record_count;

    return file_metadata{total_events, num_records};
}

result<file_metadata> reader::scan_metadata(std::string_view path, reader_options opts) {
    reader r;
    r.m_opts = std::move(opts);
    r.m_info.path = std::string(path);

    auto src = file_source::open(path);
    if (!src) return unexpected(src.error());
    r.m_file_source = std::move(*src);
    r.m_info.file_size = r.m_file_source.size();

    if (r.m_info.file_size < FILE_HEADER_SIZE)
        return unexpected(error_code::file_too_small);

    auto hdr_res = r.read_file_header();
    if (!hdr_res) return unexpected(hdr_res.error());

    if (!r.m_opts.tags.empty())
        r.m_tag_set.insert(r.m_opts.tags.begin(), r.m_opts.tags.end());

    // Fast path: extract metadata from trailer index (2 reads instead of N)
    auto trailer_result = r.scan_counts_from_trailer();
    if (trailer_result) return *trailer_result;

    // Fallback: sequential record header scan
    return r.scan_counts();
}

result<reader> reader::open(std::string_view path, reader_options opts) {
    reader r;
    r.m_opts = std::move(opts);
    r.m_info.path = std::string(path);

    auto src = file_source::open(path);
    if (!src) return unexpected(src.error());
    r.m_file_source = std::move(*src);
    r.m_info.file_size = r.m_file_source.size();

    if (r.m_info.file_size < FILE_HEADER_SIZE)
        return unexpected(error_code::file_too_small);

    auto hdr_res = r.read_file_header();
    if (!hdr_res) return unexpected(hdr_res.error());

    if (!r.m_opts.tags.empty())
        r.m_tag_set.insert(r.m_opts.tags.begin(), r.m_opts.tags.end());

    auto idx_res = r.scan_index();
    if (!idx_res) return unexpected(idx_res.error());

    auto dict_res = r.read_dictionary();
    if (!dict_res) return unexpected(dict_res.error());

    r.m_info.total_events = r.m_index.max_events();
    r.m_info.record_count = r.m_index.num_records();

    return r;
}

result<void> reader::read_file_header() {
    char buf[FILE_HEADER_SIZE];

    auto rd = m_file_source.read_at(0, buf, FILE_HEADER_SIZE);
    if (!rd) return unexpected(rd.error());

    auto unique_word = read_value<uint32_t>(buf + FH_UNIQUE_WORD_OFFSET);
    if (unique_word != HIPO_FILE_UNIQUE_WORD)
        return unexpected(error_code::invalid_magic_number);

    auto magic = read_value<int32_t>(buf + FH_MAGIC_NUMBER_OFFSET);
    m_byte_swapped = (magic == static_cast<int32_t>(0x0001dac0));

    m_header_length_words = read_value<int32_t>(buf + FH_HEADER_LENGTH_OFFSET);
    m_index_array_length  = read_value<int32_t>(buf + FH_INDEX_ARRAY_LEN_OFFSET);
    m_bit_info            = read_value<int32_t>(buf + FH_BIT_INFO_OFFSET);
    m_user_header_length  = read_value<int32_t>(buf + FH_USER_HEADER_LEN_OFFSET);
    m_info.trailer_position = read_value<int64_t>(buf + FH_TRAILER_POS_OFFSET);
    auto record_count     = read_value<int32_t>(buf + FH_RECORD_COUNT_OFFSET);

    m_header_length_words   = maybe_swap32(m_header_length_words, m_byte_swapped);
    m_index_array_length    = maybe_swap32(m_index_array_length, m_byte_swapped);
    m_bit_info              = maybe_swap32(m_bit_info, m_byte_swapped);
    m_user_header_length    = maybe_swap32(m_user_header_length, m_byte_swapped);
    m_info.trailer_position = maybe_swap64(m_info.trailer_position, m_byte_swapped);
    record_count            = maybe_swap32(record_count, m_byte_swapped);

    m_info.version = m_bit_info & BITINFO_VERSION_MASK;
    m_info.record_count = record_count;
    m_info.has_trailer_index = (m_bit_info >> BITINFO_TRAILER_WITH_INDEX_BIT) & 1;

    // First record position: header bytes + index + user header + user header padding
    int uh_padding = pad4(m_user_header_length);
    m_first_record_pos = static_cast<int64_t>(m_header_length_words) * 4
                       + m_index_array_length
                       + m_user_header_length
                       + uh_padding;

    return ok();
}

result<void> reader::scan_index() {
    // Try trailer-first scanning: read the index from the trailer record.
    // Note: v1 C++ writer sets trailer_position but does NOT set the
    // has_trailer_index bit, so we attempt this whenever trailer_position > 0.
    if (m_info.trailer_position > 0 &&
        m_info.trailer_position < m_info.file_size) {

        // Reuse m_record to avoid temporary record allocation
        int trailer_bytes = static_cast<int>(m_info.file_size - m_info.trailer_position);
        auto res = m_record.read_from(m_file_source, m_info.trailer_position,
                                       m_info.file_size, trailer_bytes);

        if (res && m_record.event_count() > 0) {
            auto ev_data = m_record.event_data(0);
            if (ev_data) {
                // Parse the index structure (group=32111, item=1)
                auto [pos, length] = [&]() -> std::pair<int,int> {
                    auto ev = *ev_data;
                    int position = 16;
                    int event_size = (ev.size() >= 8) ?
                        read_value<int32_t>(ev.data() + 4) : static_cast<int>(ev.size());
                    if (event_size > static_cast<int>(ev.size())) event_size = static_cast<int>(ev.size());
                    while (position + 8 <= event_size) {
                        auto gid = read_value<uint16_t>(ev.data() + position);
                        auto iid = read_value<uint8_t>(ev.data() + position + 2);
                        auto len = read_value<int32_t>(ev.data() + position + 4);
                        if (gid == 32111 && iid == 1)
                            return {position + 8, len};
                        position += len + 8;
                    }
                    return {-1, 0};
                }();

                if (pos > 0 && length > 0) {
                    auto ev = *ev_data;
                    const char* index_data = ev.data() + pos;
                    int rows = length / 32;

                    m_index.clear();
                    for (int i = 0; i < rows; ++i) {
                        int64_t rec_position = read_value<int64_t>(index_data + i * 8);
                        int rec_length = read_value<int32_t>(index_data + rows * 8 + i * 4);
                        int entries = read_value<int32_t>(index_data + rows * 12 + i * 4);
                        int64_t uid1 = read_value<int64_t>(index_data + rows * 16 + i * 8);

                        rec_position = maybe_swap64(rec_position, m_byte_swapped);
                        rec_length   = maybe_swap32(rec_length, m_byte_swapped);
                        entries      = maybe_swap32(entries, m_byte_swapped);
                        uid1         = maybe_swap64(uid1, m_byte_swapped);

                        if (m_tag_set.empty() || m_tag_set.contains(uid1))
                            m_index.add_record(entries, rec_position, rec_length);
                    }
                    m_index.rewind();

                    // Parse column stats bank (group=32112, item=1) if present
                    parse_trailer_stats(*ev_data);

                    return ok();
                }
            }
        }
    }

    return force_scan();
}

result<void> reader::force_scan() {
    m_index.clear();
    int64_t pos = m_first_record_pos;

    while (pos + RECORD_HEADER_SIZE <= m_info.file_size) {
        char hdr_buf[RECORD_HEADER_SIZE];
        auto rd = m_file_source.read_at(pos, hdr_buf, RECORD_HEADER_SIZE);
        if (!rd || *rd < RECORD_HEADER_SIZE) break;

        auto record_length = read_value<int32_t>(hdr_buf + 0);
        auto event_count   = read_value<int32_t>(hdr_buf + 12);
        auto magic         = read_value<int32_t>(hdr_buf + 28);
        auto bit_info_w    = read_value<int32_t>(hdr_buf + 20);

        bool swapped = (magic == static_cast<int32_t>(0x0001dac0));
        record_length = maybe_swap32(record_length, swapped);
        event_count   = maybe_swap32(event_count, swapped);
        bit_info_w    = maybe_swap32(bit_info_w, swapped);

        auto htype = static_cast<header_type>((bit_info_w >> 8) & 0xF);
        if (htype == header_type::hipo_trailer)
            break;

        if (magic != static_cast<int32_t>(HEADER_MAGIC) && !swapped)
            break;

        if (event_count > 0)
            m_index.add_record(event_count, pos, record_length * 4);

        pos += static_cast<int64_t>(record_length) * 4;
    }

    m_index.rewind();
    return ok();
}

result<file_metadata> reader::scan_counts() {
    int64_t pos = m_first_record_pos;
    int64_t total_events = 0;
    int num_records = 0;
    const bool need_tags = !m_tag_set.empty();
    const int read_size = need_tags ? 48 : 32;

    while (pos + read_size <= m_info.file_size) {
        char hdr[48];
        auto rd = m_file_source.read_at(pos, hdr, read_size);
        if (!rd || *rd < read_size) break;

        auto record_length = read_value<int32_t>(hdr + 0);
        auto event_count   = read_value<int32_t>(hdr + 12);
        auto bit_info_w    = read_value<int32_t>(hdr + 20);
        auto magic         = read_value<int32_t>(hdr + 28);

        bool swapped = (magic == static_cast<int32_t>(0x0001dac0));
        record_length = maybe_swap32(record_length, swapped);
        event_count   = maybe_swap32(event_count, swapped);
        bit_info_w    = maybe_swap32(bit_info_w, swapped);

        auto htype = static_cast<header_type>((bit_info_w >> 8) & 0xF);
        if (htype == header_type::hipo_trailer)
            break;

        if (magic != static_cast<int32_t>(HEADER_MAGIC) && !swapped)
            break;

        if (event_count > 0) {
            bool accept = true;
            if (need_tags) {
                auto uid1 = maybe_swap64(read_value<int64_t>(hdr + 40), swapped);
                accept = m_tag_set.contains(uid1);
            }
            if (accept) {
                total_events += event_count;
                num_records++;
            }
        }

        pos += static_cast<int64_t>(record_length) * 4;
    }

    return file_metadata{total_events, num_records};
}

result<file_metadata> reader::scan_counts_from_trailer() {
    if (m_info.trailer_position <= 0 ||
        m_info.trailer_position >= m_info.file_size)
        return unexpected(error_code::invalid_trailer);

    int trailer_bytes = static_cast<int>(m_info.file_size - m_info.trailer_position);
    auto res = m_record.read_from(m_file_source, m_info.trailer_position,
                                   m_info.file_size, trailer_bytes);
    if (!res || m_record.event_count() == 0)
        return unexpected(error_code::invalid_trailer);

    auto ev_data = m_record.event_data(0);
    if (!ev_data)
        return unexpected(error_code::invalid_trailer);

    // Parse the index structure (group=32111, item=1)
    auto ev = *ev_data;
    int position = EVENT_HEADER_SIZE;
    int event_size = (ev.size() >= 8) ?
        read_value<int32_t>(ev.data() + 4) : static_cast<int>(ev.size());
    if (event_size > static_cast<int>(ev.size())) event_size = static_cast<int>(ev.size());

    int index_pos = -1;
    int index_length = 0;
    while (position + 8 <= event_size) {
        auto gid = read_value<uint16_t>(ev.data() + position);
        auto iid = read_value<uint8_t>(ev.data() + position + 2);
        auto len = read_value<int32_t>(ev.data() + position + 4);
        if (gid == 32111 && iid == 1) {
            index_pos = position + 8;
            index_length = len;
            break;
        }
        position += len + 8;
    }

    if (index_pos < 0 || index_length <= 0)
        return unexpected(error_code::invalid_trailer);

    const char* index_data = ev.data() + index_pos;
    int rows = index_length / 32;

    int64_t total_events = 0;
    int num_records = 0;
    bool need_tags = !m_tag_set.empty();

    for (int i = 0; i < rows; ++i) {
        int entries = read_value<int32_t>(index_data + rows * 12 + i * 4);
        entries = maybe_swap32(entries, m_byte_swapped);

        bool accept = true;
        if (need_tags) {
            int64_t uid1 = read_value<int64_t>(index_data + rows * 16 + i * 8);
            uid1 = maybe_swap64(uid1, m_byte_swapped);
            accept = m_tag_set.contains(uid1);
        }

        if (accept && entries > 0) {
            total_events += entries;
            num_records++;
        }
    }

    return file_metadata{total_events, num_records};
}

result<void> reader::read_dictionary() {
    int64_t dict_pos = static_cast<int64_t>(m_header_length_words) * 4;

    result<void> res;
    if (m_index.num_records() > 0) {
        int dict_bytes = static_cast<int>(m_index.record_position(0) - dict_pos);
        if (dict_bytes > 0)
            res = m_record.read_from(m_file_source, dict_pos, m_info.file_size, dict_bytes);
        else
            res = m_record.read_from(m_file_source, dict_pos, m_info.file_size);
    } else {
        res = m_record.read_from(m_file_source, dict_pos, m_info.file_size);
    }
    if (!res) return res;

    // Try user header first (Java-style dictionary embedding)
    auto uh = m_record.user_header();
    if (!uh.empty()) {
        std::string_view uh_text(uh.data(), uh.size());
        if (uh_text.find('{') != std::string_view::npos) {
            auto parse_res = m_dict.parse_all(uh_text);
            if (parse_res && m_dict.size() > 0)
                return ok();
        }
    }

    // C++ style: iterate events, extract structure (120,2) from each
    int nevents = m_record.event_count();
    for (int i = 0; i < nevents; ++i) {
        auto ev = m_record.event_data(i);
        if (!ev) continue;

        auto str = m_record.find_structure(i, DICT_GROUP, DICT_ITEM);
        if (!str) continue;

        auto span = *str;
        if (span.size() <= 8) continue;

        int data_length = read_value<int32_t>(span.data() + 4) & 0x00FFFFFF;
        int header_size_bytes = (read_value<int32_t>(span.data() + 4) >> 24) & 0xFF;
        std::string_view schema_str(span.data() + 8 + header_size_bytes,
                                    data_length - header_size_bytes);

        auto null_pos = schema_str.find('\0');
        if (null_pos != std::string_view::npos)
            schema_str = schema_str.substr(0, null_pos);

        if (!schema_str.empty())
            (void)m_dict.parse(schema_str);
    }

    return ok();
}

bool reader::has_next() const noexcept {
    return m_index.can_advance();
}

bool reader::next() {
    if (!m_index.can_advance()) return false;

    int prev_record = m_index.current_record();
    m_index.advance();
    int new_record = m_index.current_record();

    if (new_record != prev_record || m_loaded_record != new_record) {
        int64_t pos = m_index.record_position(new_record);
        int rbytes = m_index.record_bytes(new_record);
        result<void> res;
        if (rbytes > 0)
            res = m_record.read_from(m_file_source, pos, m_info.file_size, rbytes);
        else
            res = m_record.read_from(m_file_source, pos, m_info.file_size);
        if (!res) return false;
        m_loaded_record = new_record;
    }
    return true;
}

void reader::rewind() {
    m_index.rewind();
    m_loaded_record = -1;
}

result<void> reader::goto_event(int event_number) {
    int prev_record = m_index.current_record();
    if (!m_index.goto_event(event_number))
        return unexpected(error_code::event_index_out_of_range);

    int new_record = m_index.current_record();
    if (new_record != prev_record || m_loaded_record != new_record) {
        int64_t pos = m_index.record_position(new_record);
        int rbytes = m_index.record_bytes(new_record);
        result<void> res;
        if (rbytes > 0)
            res = m_record.read_from(m_file_source, pos, m_info.file_size, rbytes);
        else
            res = m_record.read_from(m_file_source, pos, m_info.file_size);
        if (!res) return res;
        m_loaded_record = new_record;
    }
    return ok();
}

result<void> reader::skip(int n) {
    if (n <= 0) return ok();
    int target = m_index.current_event() + n;
    return goto_event(target);
}

void reader::read_bank(bank_view_set& banks) {
    int event_in_record = m_index.current_record_event();
    (void)banks.update_from_record(m_record, event_in_record);
}

result<void> reader::load_record(record& rec, int record_index) {
    if (record_index < 0 || record_index >= m_index.num_records())
        return unexpected(error_code::record_invalid_index);

    int64_t pos = m_index.record_position(record_index);
    int rbytes = m_index.record_bytes(record_index);
    if (rbytes > 0)
        return rec.read_from(m_file_source, pos, m_info.file_size, rbytes);
    else
        return rec.read_from(m_file_source, pos, m_info.file_size);
}

void reader::parse_trailer_stats(std::span<const char> trailer_event) {
    // Scan trailer event for stats structure (group=32112, item=1)
    const char* buf = trailer_event.data();
    int total = static_cast<int>(trailer_event.size());
    int pos = EVENT_HEADER_SIZE;

    while (pos + BANK_STRUCTURE_SIZE <= total) {
        auto gid = read_value<uint16_t>(buf + pos);
        auto iid = read_value<uint8_t>(buf + pos + 2);
        auto size_word = read_value<int32_t>(buf + pos + 4);
        int data_length = size_word & 0x00FFFFFF;
        int header_size = (size_word >> 24) & 0xFF;

        if (gid == STATS_GROUP && iid == STATS_ITEM) {
            int actual_data = data_length - header_size;
            const char* data = buf + pos + BANK_STRUCTURE_SIZE + header_size;

            // Schema: record/I(off=0), group/I(off=4), item/I(off=8),
            //         col/I(off=12), min/D(off=16), max/D(off=24)
            // row_length = 32
            int row_len = 32;
            int nrows = actual_data / row_len;
            if (nrows <= 0) return;

            // Parse column-major data
            m_column_stats.clear();
            for (int r = 0; r < nrows; ++r) {
                int rec_idx = read_value<int32_t>(data + nrows * 0  + r * 4);
                int grp     = read_value<int32_t>(data + nrows * 4  + r * 4);
                int itm     = read_value<int32_t>(data + nrows * 8  + r * 4);
                int cidx    = read_value<int32_t>(data + nrows * 12 + r * 4);
                double mn   = read_value<double> (data + nrows * 16 + r * 8);
                double mx   = read_value<double> (data + nrows * 24 + r * 8);

                // Grow vector if needed
                while (static_cast<int>(m_column_stats.size()) <= rec_idx)
                    m_column_stats.emplace_back();

                m_column_stats[rec_idx].columns[record_column_stats::pack_key(grp, itm, cidx)] =
                    {mn, mx};
            }
            return;
        }

        pos += data_length + BANK_STRUCTURE_SIZE;
    }
}

} // namespace hipo
