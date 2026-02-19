#include "validator.h"

#include <fmt/format.h>

namespace hipo {

result<void> validator::validate_file_header(const char* buf, int64_t file_size) {
    if (file_size < FILE_HEADER_SIZE)
        return unexpected(error_code::file_too_small);

    auto unique_word = read_value<uint32_t>(buf + FH_UNIQUE_WORD_OFFSET);
    if (unique_word != HIPO_FILE_UNIQUE_WORD)
        return unexpected(error_code::invalid_magic_number);

    auto magic = read_value<int32_t>(buf + FH_MAGIC_NUMBER_OFFSET);
    if (magic != static_cast<int32_t>(HEADER_MAGIC) &&
        magic != static_cast<int32_t>(0x0001dac0))
        return unexpected(error_code::invalid_endianness);

    bool swapped = (magic == static_cast<int32_t>(0x0001dac0));

    auto header_len = maybe_swap32(read_value<int32_t>(buf + FH_HEADER_LENGTH_OFFSET), swapped);
    if (header_len < FILE_HEADER_WORDS)
        return unexpected(error_code::invalid_header_length);

    auto bit_info = maybe_swap32(read_value<int32_t>(buf + FH_BIT_INFO_OFFSET), swapped);
    auto htype = static_cast<header_type>((bit_info >> BITINFO_HEADER_TYPE_SHIFT) & 0xF);
    // Accept 0 (unset, common in C++ v1 files), hipo_file (5), and hipo_ext_file
    if (htype != header_type::hipo_file && htype != header_type::hipo_ext_file &&
        static_cast<int>(htype) != 0)
        return unexpected(error_code::invalid_header_type);

    return ok();
}

result<void> validator::validate_record_header(const record_header& hdr, int64_t position, int64_t file_size) {
    if (hdr.magic_number != static_cast<int32_t>(HEADER_MAGIC))
        return unexpected(error_code::invalid_magic_number);

    if (hdr.header_length < RECORD_HEADER_WORDS)
        return unexpected(error_code::invalid_header_length);

    int64_t record_end = position + static_cast<int64_t>(hdr.record_length) * 4;
    if (record_end > file_size)
        return unexpected(error_code::record_bounds_exceeded);

    if (hdr.index_data_length != hdr.event_count * 4)
        return unexpected(error_code::record_invalid_index);

    if (hdr.compressed_length > hdr.record_length * 4)
        return unexpected(error_code::record_bounds_exceeded);

    return ok();
}

result<void> validator::validate_trailer(const record_header& hdr, int64_t position, int64_t file_size) {
    auto base = validate_record_header(hdr, position, file_size);
    if (!base) return base;

    // Trailer has header_type::hipo_trailer in bit_info bits[8-11]
    auto htype = static_cast<header_type>((hdr.bit_info >> 8) & 0xF);
    if (htype != header_type::hipo_trailer)
        return unexpected(error_code::invalid_trailer);

    return ok();
}

result<std::vector<validation_issue>> validator::validate_file(std::string_view path) {
    std::vector<validation_issue> issues;

    auto src = file_source::open(path);
    if (!src) {
        issues.push_back({severity::error, fmt::format("cannot open file: {}", path), 0});
        return issues;
    }

    int64_t file_size = src->size();

    // Read and validate file header
    char fh_buf[FILE_HEADER_SIZE];
    auto read_res = src->read_at(0, fh_buf, FILE_HEADER_SIZE);
    if (!read_res || *read_res < FILE_HEADER_SIZE) {
        issues.push_back({severity::error, "cannot read file header", 0});
        return issues;
    }

    auto fh_valid = validate_file_header(fh_buf, file_size);
    if (!fh_valid) {
        issues.push_back({severity::error,
            fmt::format("invalid file header: {}", fh_valid.error()), 0});
        return issues;
    }

    // Parse file header fields
    auto magic = read_value<int32_t>(fh_buf + FH_MAGIC_NUMBER_OFFSET);
    bool swapped = (magic == static_cast<int32_t>(0x0001dac0));

    auto header_len_words = read_value<int32_t>(fh_buf + FH_HEADER_LENGTH_OFFSET);
    auto index_len   = read_value<int32_t>(fh_buf + FH_INDEX_ARRAY_LEN_OFFSET);
    auto uh_len      = read_value<int32_t>(fh_buf + FH_USER_HEADER_LEN_OFFSET);
    auto bit_info    = read_value<int32_t>(fh_buf + FH_BIT_INFO_OFFSET);

    header_len_words = maybe_swap32(header_len_words, swapped);
    index_len        = maybe_swap32(index_len, swapped);
    uh_len           = maybe_swap32(uh_len, swapped);
    bit_info         = maybe_swap32(bit_info, swapped);

    int uh_padding = pad4(uh_len);
    int64_t first_record_pos = header_len_words * 4LL + index_len + uh_len + uh_padding;

    // Check trailer position
    auto trailer_pos = maybe_swap64(read_value<int64_t>(fh_buf + FH_TRAILER_POS_OFFSET), swapped);

    bool has_trailer_with_index = (bit_info >> BITINFO_TRAILER_WITH_INDEX_BIT) & 1;

    if (has_trailer_with_index && trailer_pos > 0) {
        if (trailer_pos >= file_size) {
            issues.push_back({severity::error,
                fmt::format("trailer position {} exceeds file size {}", trailer_pos, file_size),
                FH_TRAILER_POS_OFFSET});
        }
    }

    // Scan records
    record rec;
    int64_t pos = first_record_pos;
    int record_count = 0;

    while (pos + RECORD_HEADER_SIZE <= file_size) {
        auto res = rec.read_from(*src, pos, file_size);
        if (!res) {
            issues.push_back({severity::warning,
                fmt::format("record {} at offset {} failed: {}",
                            record_count, pos, res.error()),
                pos});
            break;
        }

        auto& hdr = rec.header();
        auto hdr_valid = validate_record_header(hdr, pos, file_size);
        if (!hdr_valid) {
            issues.push_back({severity::warning,
                fmt::format("record {} header invalid: {}", record_count,
                            hdr_valid.error()),
                pos});
        }

        pos += static_cast<int64_t>(hdr.record_length) * 4;
        ++record_count;

        // Check if we hit the trailer position
        if (has_trailer_with_index && trailer_pos > 0 && pos >= trailer_pos)
            break;
    }

    return issues;
}

result<void> validator::validate_schema_consistency(const dictionary& d1, const dictionary& d2) {
    auto names = d1.schema_names();
    for (auto const& name : names) {
        auto s1 = d1.find(name);
        auto s2 = d2.find(name);
        if (!s1 || !s2) continue;

        auto const& sc1 = s1->get();
        auto const& sc2 = s2->get();

        if (sc1.group() != sc2.group() || sc1.item() != sc2.item())
            return unexpected(error_code::schema_inconsistent);

        if (sc1.entries() != sc2.entries())
            return unexpected(error_code::schema_inconsistent);

        for (int i = 0; i < sc1.entries(); ++i) {
            if (sc1.entry(i).name != sc2.entry(i).name ||
                sc1.entry(i).type != sc2.entry(i).type)
                return unexpected(error_code::schema_inconsistent);
        }
    }
    return ok();
}

} // namespace hipo
