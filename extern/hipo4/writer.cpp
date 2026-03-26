#include "writer.h"

#include <cstring>
#include <optional>

namespace hipo {

writer::~writer() {
    if (m_is_open)
        (void)close();
}

void writer::set_dictionary(const dictionary& dict) {
    // Copy all schemas from the provided dictionary
    for (auto const& s : dict)
        m_dict.add(s);
}

result<void> writer::open(std::string_view path, writer_options opts) {
    if (m_is_open)
        return unexpected(error_code::writer_already_open);

    m_opts = opts;

    // Build dictionary record: each schema becomes an event with structure(120, 2, type=6)
    record_builder_options rb_opts;
    rb_opts.max_events = m_dict.size() + 16;
    rb_opts.max_record_bytes = 1024 * 1024;
    rb_opts.compression = opts.compression;
    record_builder dict_builder(rb_opts);

    event_builder ev;
    for (auto const& sch : m_dict) {
        auto schema_json = sch.to_json_string();
        auto schema_str  = sch.to_schema_string();
        ev.reset();
        ev.add_string(DICT_GROUP, 1, schema_json);  // JSON schema (item=1) — required by Java reader
        ev.add_string(DICT_GROUP, DICT_ITEM, schema_str);  // text schema (item=2)
        (void)dict_builder.add_event(ev.data());
    }

    auto build_res = dict_builder.build();
    if (!build_res)
        return unexpected(build_res.error());

    int dict_record_size = dict_builder.record_size();

    // Open file
    auto sink = file_sink::open(path);
    if (!sink) return unexpected(sink.error());
    m_sink = std::move(*sink);

    // Write 56-byte file header
    char header[FILE_HEADER_SIZE];
    std::memset(header, 0, FILE_HEADER_SIZE);

    write_value<uint32_t>(header + FH_UNIQUE_WORD_OFFSET,     HIPO_FILE_UNIQUE_WORD);
    write_value<int32_t> (header + FH_FILE_NUMBER_OFFSET,     1);
    write_value<int32_t> (header + FH_HEADER_LENGTH_OFFSET,   FILE_HEADER_WORDS);
    write_value<int32_t> (header + FH_RECORD_COUNT_OFFSET,    0);  // patched at close
    write_value<int32_t> (header + FH_INDEX_ARRAY_LEN_OFFSET, 0);
    write_value<int32_t> (header + FH_BIT_INFO_OFFSET, 6);  // version 6, matching Java HIPO writer
    write_value<int32_t> (header + FH_USER_HEADER_LEN_OFFSET, dict_record_size);
    write_value<int32_t> (header + FH_MAGIC_NUMBER_OFFSET,    static_cast<int32_t>(HEADER_MAGIC));
    write_value<int64_t> (header + FH_USER_REGISTER_OFFSET,   0);
    write_value<int64_t> (header + FH_TRAILER_POS_OFFSET,     0);  // patched at close
    write_value<int32_t> (header + FH_USER_INT1_OFFSET,       0);
    write_value<int32_t> (header + FH_USER_INT2_OFFSET,       0);

    auto wr = m_sink.write(header, FILE_HEADER_SIZE);
    if (!wr) return unexpected(wr.error());

    // Write dictionary record
    auto dict_data = dict_builder.record_data();
    wr = m_sink.write(dict_data.data(), static_cast<int>(dict_data.size()));
    if (!wr) return unexpected(wr.error());

    // Initialize data record builder
    record_builder_options data_opts;
    data_opts.max_events = opts.max_events;
    data_opts.max_record_bytes = opts.max_record_bytes;
    data_opts.compression = opts.compression;
    m_builder = record_builder(data_opts);

    // Initialize stats tracker if enabled
    if (m_opts.collect_statistics) {
        m_stats_tracker.set_dictionary(m_dict);
        m_record_stats.clear();
    }

    m_is_open = true;
    return ok();
}

result<void> writer::add_event(const event_builder& ev) {
    return add_event(ev.data());
}

result<void> writer::add_event(std::span<const char> event_data) {
    if (!m_is_open)
        return unexpected(error_code::writer_not_open);

    if (!m_builder.add_event(event_data)) {
        // Record full -- flush it, then add to new record
        auto res = write_record();
        if (!res) return res;
        if (!m_builder.add_event(event_data))
            return unexpected(error_code::write_failed);
    }

    // Track per-column statistics if enabled
    if (m_opts.collect_statistics)
        m_stats_tracker.process_event(event_data);

    return ok();
}

result<void> writer::flush() {
    if (!m_is_open)
        return unexpected(error_code::writer_not_open);
    return write_record();
}

result<void> writer::write_record() {
    if (m_builder.entry_count() == 0) return ok();

    auto res = m_builder.build();
    if (!res) return res;

    record_info info;
    info.position      = m_sink.position();
    info.length        = m_builder.record_size();
    info.entries        = m_builder.entry_count();
    info.user_word_one = 0;
    info.user_word_two = 0;

    auto data = m_builder.record_data();
    auto wr = m_sink.write(data.data(), static_cast<int>(data.size()));
    if (!wr) return unexpected(wr.error());

    m_record_infos.push_back(info);

    // Save per-record column stats
    if (m_opts.collect_statistics)
        m_record_stats.push_back(m_stats_tracker.finish_record());

    if (m_on_record_written) {
        m_on_record_written(static_cast<int>(m_record_infos.size()) - 1,
                            info.position, info.length, info.entries);
    }

    m_builder.reset();
    return ok();
}

result<void> writer::write_index_table() {
    // Build index bank with schema "position/L,length/I,entries/I,userWordOne/L,userWordTwo/L"
    // group=32111, item=1
    auto index_schema = schema::parse("file::index", 32111, 1,
                                       "position/L,length/I,entries/I,userWordOne/L,userWordTwo/L");
    if (!index_schema) return unexpected(index_schema.error());

    int n = static_cast<int>(m_record_infos.size());
    bank index_bank(*index_schema, n);

    auto [h_pos, h_len, h_ent, h_uw1, h_uw2] =
        index_schema->get_handles("position", "length", "entries", "userWordOne", "userWordTwo")
        .expect("resolve index handles");

    for (int i = 0; i < n; ++i) {
        auto const& ri = m_record_infos[i];
        index_bank.put<int64_t>(h_pos, i, ri.position);
        index_bank.put<int32_t>(h_len, i, ri.length);
        index_bank.put<int32_t>(h_ent, i, ri.entries);
        index_bank.put<int64_t>(h_uw1, i, ri.user_word_one);
        index_bank.put<int64_t>(h_uw2, i, ri.user_word_two);
    }

    // Estimate event size (index + optional stats)
    int estimated_size = 32 * n + 4096;

    // Build stats bank if statistics were collected
    std::optional<schema> stats_schema_opt;
    std::optional<bank> stats_bank_opt;
    if (m_opts.collect_statistics && !m_record_stats.empty()) {
        auto ss = schema::parse("file::column_stats", STATS_GROUP, STATS_ITEM,
                                "record/I,group/I,item/I,col/I,min/D,max/D");
        if (ss) {
            stats_schema_opt = std::move(*ss);
            // Count total rows (one row per column per record)
            int total_stat_rows = 0;
            for (auto const& rs : m_record_stats)
                total_stat_rows += static_cast<int>(rs.columns.size());

            if (total_stat_rows > 0) {
                bank sb(*stats_schema_opt, total_stat_rows);
                auto [h_rec, h_grp, h_itm, h_col, h_min, h_max] =
                    stats_schema_opt->get_handles("record", "group", "item", "col", "min", "max")
                    .expect("resolve stats handles");

                int row = 0;
                for (int ri = 0; ri < static_cast<int>(m_record_stats.size()); ++ri) {
                    for (auto const& [key, entry] : m_record_stats[ri].columns) {
                        int grp = static_cast<int>((key >> 24) & 0xFFFF);
                        int itm = static_cast<int>((key >> 16) & 0xFF);
                        int cidx = static_cast<int>(key & 0xFFFF);
                        sb.put<int32_t>(h_rec, row, ri);
                        sb.put<int32_t>(h_grp, row, grp);
                        sb.put<int32_t>(h_itm, row, itm);
                        sb.put<int32_t>(h_col, row, cidx);
                        sb.put<double>(h_min, row, entry.min_val);
                        sb.put<double>(h_max, row, entry.max_val);
                        row++;
                    }
                }
                stats_bank_opt = std::move(sb);
                estimated_size += total_stat_rows * 40 + 1024;
            }
        }
    }

    // Build event with the index bank (and optional stats bank)
    event_builder ev(estimated_size);
    ev.add(index_bank);
    if (stats_bank_opt)
        ev.add(*stats_bank_opt);

    // Build a record containing the trailer event
    record_builder_options rb_opts;
    rb_opts.max_events = 2;  // need > 1 due to >= boundary check in add_event
    rb_opts.max_record_bytes = estimated_size;
    rb_opts.compression = m_opts.compression;
    record_builder rb(rb_opts);
    rb.add_event(ev.data());

    auto build_res = rb.build();
    if (!build_res) return unexpected(build_res.error());

    // Write trailer record as-is (matching Java HIPO writer behavior)
    auto rec_data = rb.record_data();

    int64_t trailer_pos = m_sink.position();

    auto wr = m_sink.write(rec_data.data(), static_cast<int>(rec_data.size()));
    if (!wr) return unexpected(wr.error());

    // Patch file header: write trailer position at offset 40 (matching Java writer)
    char patch_buf[8];
    write_value<int64_t>(patch_buf, trailer_pos);
    return m_sink.write_at(FH_TRAILER_POS_OFFSET, patch_buf, 8);
}

result<void> writer::close() {
    if (!m_is_open)
        return unexpected(error_code::writer_not_open);

    // Flush remaining events
    auto res = write_record();
    if (!res) return res;

    // Write index table (trailer)
    res = write_index_table();
    if (!res) return res;

    m_sink.close();
    m_is_open = false;
    m_record_infos.clear();
    m_record_stats.clear();
    return ok();
}

void writer::set_user_word_one(int64_t w) noexcept {
    m_builder.set_user_word_one(w);
}

void writer::set_user_word_two(int64_t w) noexcept {
    m_builder.set_user_word_two(w);
}

} // namespace hipo
