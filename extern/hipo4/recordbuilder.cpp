/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "recordbuilder.h"
#include "constants.h"
#include "dictionary.h"
#include "hipoexceptions.h"

#include <algorithm>
#include <cstdint>
#include <unordered_set>

#ifdef __LZ4__
#include <lz4.h>
#endif

namespace hipo {
  /**
  * Default constructor sets number of max events to 100000
  * and the buffer size to 8MB.
  */
  recordbuilder::recordbuilder(){
    bufferIndex.resize(4*defaultNumberOfEvents);
    bufferEvents.resize(defaultRecordSize);
    bufferData.resize(defaultRecordSize+4*defaultNumberOfEvents);
    bufferRecord.resize(defaultRecordSize+4*defaultNumberOfEvents+512*1024);
    bufferIndexEntries   = 0;
    bufferEventsPosition = 0;
  }
  /**
   * Constructor with custom max event size and maximum record
   * size provided by user.
   */
  recordbuilder::recordbuilder(int maxEvents, int maxLength){
    bufferIndex.resize(4*maxEvents);
    bufferEvents.resize(maxLength);
    bufferData.resize(  maxLength+4*maxEvents + 1024);
    bufferRecord.resize(maxLength+4*maxEvents+512*1024);
    bufferIndexEntries   = 0;
    bufferEventsPosition = 0;
  }

  /**
   * add event object to the record builder buffer.
  */
  bool recordbuilder::addEvent(hipo::event &evnt){
      return addEvent(evnt.getEventBuffer(),0,evnt.getSize());
  }
  /**
   * add a content of a vector to the record builder buffer.
   * offset in the buffer and number of bytes to add provided
   * by user.
   */
  bool recordbuilder::addEvent(std::vector<char> &vec, int start, int length){
      if(static_cast<size_t>(bufferEventsPosition+length)>=bufferEvents.size()) return false;
      if(static_cast<size_t>((bufferIndexEntries+1)*4)>=bufferIndex.size()) return false;
      *reinterpret_cast<int*>(&bufferIndex[bufferIndexEntries*4]) = length;
      bufferIndexEntries++;
      memcpy(&bufferEvents[bufferEventsPosition],&vec[start],length);
      bufferEventsPosition += length;
      return true;
  }
  /**
   * Resets the counters for number of events and sets the
   * position for writing new events to the begining of the
   * event buffer.
   */
  void recordbuilder::reset(){
    bufferIndexEntries   = 0;
    bufferEventsPosition = 0;
  }
  /**
   * returns record length in bytes rounded to first integer.
   * the length comes out divisible by 4.
   */
  int  recordbuilder::getRecordLengthRounding(int bufferSize){
    if(bufferSize%4==0) return 0;
    int nwords = bufferSize/4;
    int nbytes = 4*(nwords+1);
    return (nbytes-bufferSize);
  }
  /**
   * Returns number of events in the record.
   */
  int  recordbuilder::getEntries(){
    int nentries = *reinterpret_cast<int*>(&bufferRecord[RH_EVENT_COUNT_OFFSET]);
    return nentries;
  }
  /**
   * returns the size of the record.
   */
  int  recordbuilder::getRecordSize(){
      int size = *reinterpret_cast<int*>(&bufferRecord[RH_RECORD_LENGTH_OFFSET]);
      return size*4;
  }

  long recordbuilder::getUserWordOne(){
    long wOne = *reinterpret_cast<long*>(&bufferRecord[RH_USER_WORD1_OFFSET]);
    return wOne;
  }

  long recordbuilder::getUserWordTwo(){
    long wTwo = *reinterpret_cast<long*>(&bufferRecord[RH_USER_WORD2_OFFSET]);
    return wTwo;
  }

  void recordbuilder::setUserWordOne(long userWordOne){
    bufferUserWordOne = userWordOne;
  }

  void recordbuilder::setUserWordTwo(long userWordTwo){
    bufferUserWordTwo = userWordTwo;
  } 

  void recordbuilder::build(){
      if (activeSpec.type == compression_type::lz4_chunked) {
          buildChunked();
          return;
      }
      if (activeSpec.type == compression_type::lz4_by_bank) {
          buildByBank();
          return;
      }
      if (activeSpec.type == compression_type::lz4_by_page) {
          buildByPage();
          return;
      }
      int  indexSize = bufferIndexEntries*4;
      int eventsSize = bufferEventsPosition;
      memcpy(&bufferData[0],&bufferIndex[0],indexSize);
      memcpy(&bufferData[indexSize],&bufferEvents[0],eventsSize);
      int uncompressedSize = indexSize+eventsSize;
      int   compressedSize = compressRecord(uncompressedSize);
      int         rounding = getRecordLengthRounding(compressedSize);
      int   compressedSizeToWrite = compressedSize + rounding;
      int   compressedSizeToWriteWords =  compressedSizeToWrite/4;
      int            recordLength = compressedSizeToWrite/4+RECORD_HEADER_WORDS;

      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_LENGTH_OFFSET, recordLength); // (1) - record length in words (includes header)
      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_NUMBER_OFFSET, 0); // (2) - record #
      hipo::utils::writeInt(&bufferRecord[0], RH_HEADER_LENGTH_OFFSET, RECORD_HEADER_WORDS); // (3) - record header lenght (in words)
      hipo::utils::writeInt(&bufferRecord[0], RH_EVENT_COUNT_OFFSET, bufferIndexEntries); // (4) event count in the record
      hipo::utils::writeInt(&bufferRecord[0], RH_INDEX_ARRAY_LEN_OFFSET, bufferIndexEntries*4); // (5) length of index array in bytes
      int versionWord = (rounding<<BITINFO_PAD3_SHIFT)|(HIPO_VERSION);
      hipo::utils::writeInt(&bufferRecord[0], RH_BIT_INFO_OFFSET, versionWord); // (6) record version number
      hipo::utils::writeInt(&bufferRecord[0], RH_USER_HEADER_LEN_OFFSET, 0); // (7) user header length bytes
      hipo::utils::writeInt(&bufferRecord[0], RH_MAGIC_NUMBER_OFFSET, HEADER_MAGIC); // (8) magic word
      hipo::utils::writeInt(&bufferRecord[0], RH_DATA_LENGTH_OFFSET, eventsSize); // (9) magic word
      uint32_t type_bits = static_cast<uint32_t>(activeSpec.type);
      int compressionWord = (type_bits<<COMP_TYPE_SHIFT)|(COMP_LENGTH_MASK&compressedSizeToWriteWords);
      hipo::utils::writeInt(&bufferRecord[0], RH_COMP_WORD_OFFSET, compressionWord);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD1_OFFSET, bufferUserWordOne);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD2_OFFSET, bufferUserWordTwo);

      //printf("record::build uncompressed size = %8d, compressed size = %8d, rounding = %4d , compressed FULL = %6d, record size = %6d, version = %X, size = %5X\n",
      //      uncompressedSize,compressedSize, rounding,compressedSizeToWrite, recordLength*4,versionWord,compressionWord);
  }

  /**
   * Build the record using the Lz4Chunked compressed layout (compression tag 4).
   *
   * Each event is grouped into chunks of `events_per_chunk` events. Each chunk
   * is independently LZ4-compressed. The on-disk layout of the compressed
   * payload section (matches hipo-rs):
   *
   *   u32 num_chunks
   *   u32 events_per_chunk            (informational)
   *   event_count × u32 event_sizes   (OUTSIDE any LZ4 stream)
   *   N × u32 compressed_chunk_sizes
   *   N × u32 decompressed_chunk_sizes
   *   concatenated LZ4 chunk payloads
   *
   * Big-endian records are not supported by this layout; the writer is always
   * little-endian. (See hipo-rs wire/record.rs for the spec.)
   */
  void recordbuilder::buildChunked(){
#ifndef __LZ4__
      printf("recordbuilder::buildChunked: LZ4 not available\n");
      return;
#else
      const uint32_t events_per_chunk = std::max(1u, activeSpec.events_per_chunk);
      const int      event_count      = bufferIndexEntries;
      const int      num_chunks       = (event_count + events_per_chunk - 1) / events_per_chunk;

      const int directory_size = 8 + 4*event_count + 4*num_chunks + 4*num_chunks;
      const int event_sizes_off  = RECORD_HEADER_SIZE + 8;
      const int comp_sizes_off   = event_sizes_off + 4*event_count;
      const int decomp_sizes_off = comp_sizes_off + 4*num_chunks;
      const int payload_off      = RECORD_HEADER_SIZE + directory_size;

      // Pessimistic upper bound for the output buffer: directory + LZ4_compressBound per chunk.
      // We don't know per-chunk decompressed size until we walk events, so use total events size.
      size_t needed_cap = static_cast<size_t>(payload_off) + LZ4_compressBound(bufferEventsPosition) + 1024;
      if (bufferRecord.size() < needed_cap) {
          bufferRecord.resize(needed_cap);
      }

      // Directory header
      *reinterpret_cast<uint32_t*>(&bufferRecord[RECORD_HEADER_SIZE + 0]) = static_cast<uint32_t>(num_chunks);
      *reinterpret_cast<uint32_t*>(&bufferRecord[RECORD_HEADER_SIZE + 4]) = events_per_chunk;

      // Event sizes table (copy from bufferIndex which already holds per-event sizes as i32)
      memcpy(&bufferRecord[event_sizes_off], &bufferIndex[0], 4*event_count);

      int chunk_dst_off    = payload_off;
      int event_start_byte = 0;
      int event_start_idx  = 0;

      for (int c = 0; c < num_chunks; ++c) {
          const int chunk_events = std::min(static_cast<int>(events_per_chunk),
                                            event_count - event_start_idx);
          uint32_t decomp_size = 0;
          for (int e = 0; e < chunk_events; ++e) {
              decomp_size += *reinterpret_cast<uint32_t*>(&bufferIndex[(event_start_idx + e)*4]);
          }

          int dst_capacity = static_cast<int>(bufferRecord.size()) - chunk_dst_off;
          int comp_size = LZ4_compress_fast(
              &bufferEvents[event_start_byte],
              &bufferRecord[chunk_dst_off],
              static_cast<int>(decomp_size),
              dst_capacity,
              1);
          if (comp_size <= 0) {
              printf("recordbuilder::buildChunked: LZ4_compress_fast failed for chunk %d\n", c);
              return;
          }

          *reinterpret_cast<uint32_t*>(&bufferRecord[comp_sizes_off   + c*4]) = static_cast<uint32_t>(comp_size);
          *reinterpret_cast<uint32_t*>(&bufferRecord[decomp_sizes_off + c*4]) = decomp_size;

          chunk_dst_off    += comp_size;
          event_start_byte += static_cast<int>(decomp_size);
          event_start_idx  += chunk_events;
      }

      const int compressed_payload_size = chunk_dst_off - RECORD_HEADER_SIZE;
      const int rounding                = getRecordLengthRounding(compressed_payload_size);
      const int compressed_to_write     = compressed_payload_size + rounding;
      const int compressed_words        = compressed_to_write / 4;
      const int recordLength            = compressed_words + RECORD_HEADER_WORDS;
      const int total_data_length       = event_start_byte;  // sum of all event_sizes

      // Zero-pad any rounding bytes so the file output is deterministic.
      for (int p = 0; p < rounding; ++p) bufferRecord[chunk_dst_off + p] = 0;

      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_LENGTH_OFFSET,   recordLength);
      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_NUMBER_OFFSET,   0);
      hipo::utils::writeInt(&bufferRecord[0], RH_HEADER_LENGTH_OFFSET,   RECORD_HEADER_WORDS);
      hipo::utils::writeInt(&bufferRecord[0], RH_EVENT_COUNT_OFFSET,     event_count);
      // Chunked has no separate "index array" — event sizes live inside the chunked directory.
      hipo::utils::writeInt(&bufferRecord[0], RH_INDEX_ARRAY_LEN_OFFSET, 0);
      const int versionWord = (rounding << BITINFO_PAD3_SHIFT) | HIPO_VERSION;
      hipo::utils::writeInt(&bufferRecord[0], RH_BIT_INFO_OFFSET,        versionWord);
      hipo::utils::writeInt(&bufferRecord[0], RH_USER_HEADER_LEN_OFFSET, 0);
      hipo::utils::writeInt(&bufferRecord[0], RH_MAGIC_NUMBER_OFFSET,    HEADER_MAGIC);
      hipo::utils::writeInt(&bufferRecord[0], RH_DATA_LENGTH_OFFSET,     total_data_length);
      const uint32_t type_bits = static_cast<uint32_t>(compression_type::lz4_chunked);
      const int compressionWord = (type_bits << COMP_TYPE_SHIFT) | (COMP_LENGTH_MASK & compressed_words);
      hipo::utils::writeInt(&bufferRecord[0], RH_COMP_WORD_OFFSET,       compressionWord);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD1_OFFSET,     bufferUserWordOne);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD2_OFFSET,     bufferUserWordTwo);
#endif
  }
  /**
   * Build the record using the Lz4ByBank compressed layout (compression tag 5).
   *
   * Walks each accumulated event's bank structures to:
   *   1. Discover the unique (group, item, data_type) bank descriptors.
   *   2. Pack a per-event bank-presence bit matrix.
   *   3. Concatenate the data bytes of each bank across all events into one
   *      stream per bank, then LZ4-compress each stream independently.
   *
   * On-disk layout of the compressed-payload section (matches hipo-rs):
   *   u32 num_banks (B)
   *   u32 event_count (E)
   *   B × { u16 group, u8 item, u8 data_type }
   *   B × u32 compressed_bank_sizes
   *   B × u32 decompressed_bank_sizes
   *   E × u32 event_tags
   *   E × ceil(B/8) bytes presence_matrix (row-major, bit b in event-e)
   *   B × E × u32 event_bank_sizes
   *   concat B × LZ4 bank streams
   *
   * Limitations of this initial port:
   *   - Composite banks (type=10) — only their data bytes are preserved.
   *     Format prefix bytes are lost (full composite support is a follow-up).
   *   - Per-structure size_word's upper 8 bits (format-descriptor length)
   *     are not preserved across the round-trip; assumed zero for bank
   *     structures (type=11), which is the common case.
   */
  void recordbuilder::buildByBank(){
#ifndef __LZ4__
      printf("recordbuilder::buildByBank: LZ4 not available\n");
      return;
#else
      const int event_count = bufferIndexEntries;

      // Pass 1: walk every event, discover banks and collect per-(bank,event) sizes.
      struct bank_info {
          uint16_t group;
          uint8_t  item;
          uint8_t  data_type;
      };
      std::vector<bank_info> banks;
      auto bank_index_of = [&](uint16_t g, uint8_t i) -> int {
          for (size_t k = 0; k < banks.size(); ++k) {
              if (banks[k].group == g && banks[k].item == i) return static_cast<int>(k);
          }
          return -1;
      };

      // First pass: identify banks (sorted afterwards) and collect per-event tags + structure offsets.
      std::vector<uint32_t> event_tags(event_count, 0);
      // For each event, list of (bank_global_idx_in_banks, data_offset_in_bufferEvents, data_size).
      struct per_event_bank {
          int      bank_pos; // index into the `banks` vector (later remapped to sorted index)
          uint32_t data_off; // offset within bufferEvents where the data bytes start
          uint32_t data_len; // length in bytes
      };
      std::vector<std::vector<per_event_bank>> per_event(event_count);

      uint32_t event_running_off = 0;
      for (int e = 0; e < event_count; ++e) {
          const uint32_t evt_size_field = *reinterpret_cast<uint32_t*>(&bufferIndex[e*4]);
          const uint32_t evt_start = event_running_off;

          // Pull the EH tag (offset EH_TAG_OFFSET into the event header).
          uint32_t tag = 0;
          std::memcpy(&tag, &bufferEvents[evt_start + EH_TAG_OFFSET], sizeof(tag));
          event_tags[e] = tag;

          // Walk structures.
          uint32_t pos = EVENT_HEADER_SIZE;
          while (pos + BANK_STRUCTURE_SIZE < evt_size_field) {
              uint16_t gid;
              std::memcpy(&gid, &bufferEvents[evt_start + pos], sizeof(gid));
              uint8_t  iid = static_cast<uint8_t>(bufferEvents[evt_start + pos + 2]);
              uint8_t  type = static_cast<uint8_t>(bufferEvents[evt_start + pos + 3]);
              uint32_t word;
              std::memcpy(&word, &bufferEvents[evt_start + pos + 4], sizeof(word));
              uint32_t data_len = word & STRUCT_SIZE_MASK;

              // Bounds check: a malformed structure header (data_len that
              // overruns the event) would later read past valid event data
              // and trip libstdc++'s _GLIBCXX_ASSERTIONS bounds check. Skip
              // and bail out of this event's walk.
              if (static_cast<uint64_t>(pos) + BANK_STRUCTURE_SIZE + data_len > evt_size_field) {
                  break;
              }

              int idx = bank_index_of(gid, iid);
              if (idx < 0) {
                  idx = static_cast<int>(banks.size());
                  banks.push_back({gid, iid, type});
              }
              // Lz4ByBank's wire format assumes each (group, item) appears
              // at most once per event (it stores ONE byte range per
              // (bank, event)). Some files violate this (e.g. legacy
              // events with duplicate skim tags). Silently keep the first
              // occurrence and skip the rest — recording duplicates would
              // overwrite event_bank_sizes with the last data_len while
              // the byte-copy loop still writes EVERY entry's bytes,
              // overrunning the bank's pre-sized buffer.
              bool dup = false;
              for (const auto& prior : per_event[e]) {
                  if (prior.bank_pos == idx) { dup = true; break; }
              }
              if (!dup) {
                  per_event[e].push_back({idx,
                                          evt_start + pos + BANK_STRUCTURE_SIZE,
                                          data_len});
              }
              pos += data_len + BANK_STRUCTURE_SIZE;
          }
          event_running_off += evt_size_field;
      }

      const int num_banks = static_cast<int>(banks.size());

      // Sort descriptors by (group, item) for reproducible on-disk order. Build
      // an index remap so subsequent lookups go through sorted positions.
      std::vector<int> sort_order(num_banks);
      for (int i = 0; i < num_banks; ++i) sort_order[i] = i;
      std::sort(sort_order.begin(), sort_order.end(),
                [&](int a, int b) {
                    if (banks[a].group != banks[b].group) return banks[a].group < banks[b].group;
                    return banks[a].item < banks[b].item;
                });
      std::vector<int> orig_to_sorted(num_banks);
      for (int s = 0; s < num_banks; ++s) orig_to_sorted[sort_order[s]] = s;
      std::vector<bank_info> sorted_banks(num_banks);
      for (int s = 0; s < num_banks; ++s) sorted_banks[s] = banks[sort_order[s]];

      // event_bank_sizes flat layout [b * E + e]
      const size_t ebs_count = static_cast<size_t>(num_banks) * event_count;
      std::vector<uint32_t> event_bank_sizes(ebs_count, 0);
      for (int e = 0; e < event_count; ++e) {
          for (const auto& peb : per_event[e]) {
              int s = orig_to_sorted[peb.bank_pos];
              event_bank_sizes[static_cast<size_t>(s) * event_count + e] = peb.data_len;
          }
      }

      // presence matrix (row-major, ceil(B/8) bytes per event)
      const size_t bytes_per_row = (num_banks + 7) / 8;
      std::vector<uint8_t> presence(static_cast<size_t>(event_count) * bytes_per_row, 0);
      for (int e = 0; e < event_count; ++e) {
          for (const auto& peb : per_event[e]) {
              int s = orig_to_sorted[peb.bank_pos];
              size_t byte = static_cast<size_t>(e) * bytes_per_row + s / 8;
              uint8_t bit = static_cast<uint8_t>(s % 8);
              presence[byte] |= static_cast<uint8_t>(1u << bit);
          }
      }

      // Build per-bank uncompressed concatenated streams. We do this with a
      // single flat buffer per bank — `decomp_sizes[b]` = sum of sizes for bank b.
      std::vector<uint32_t> decomp_sizes(num_banks, 0);
      for (int b = 0; b < num_banks; ++b) {
          uint64_t sum = 0;
          for (int e = 0; e < event_count; ++e) {
              sum += event_bank_sizes[static_cast<size_t>(b) * event_count + e];
          }
          decomp_sizes[b] = static_cast<uint32_t>(sum);
      }

      std::vector<std::vector<char>> uncompressed(num_banks);
      for (int b = 0; b < num_banks; ++b) {
          uncompressed[b].resize(decomp_sizes[b]);
      }

      // Fill per-bank streams by walking events in order.
      std::vector<size_t> write_off(num_banks, 0);
      for (int e = 0; e < event_count; ++e) {
          for (const auto& peb : per_event[e]) {
              int s = orig_to_sorted[peb.bank_pos];
              if (peb.data_len == 0) continue;
              std::memcpy(&uncompressed[s][write_off[s]],
                          &bufferEvents[peb.data_off],
                          peb.data_len);
              write_off[s] += peb.data_len;
          }
      }

      // LZ4-compress each bank stream into a temp vector. Improvement over
      // hipo-rs: this is trivially parallelizable (independent streams) but
      // for an initial port we keep it sequential — std::async or ThreadPool
      // can be wired in as a follow-up.
      std::vector<std::vector<char>> compressed(num_banks);
      std::vector<uint32_t> comp_sizes(num_banks, 0);
      for (int b = 0; b < num_banks; ++b) {
          if (decomp_sizes[b] == 0) {
              comp_sizes[b] = 0;
              continue;
          }
          int bound = LZ4_compressBound(static_cast<int>(decomp_sizes[b]));
          compressed[b].resize(bound);
          int cs = LZ4_compress_fast(uncompressed[b].data(),
                                     compressed[b].data(),
                                     static_cast<int>(decomp_sizes[b]),
                                     bound, 1);
          if (cs <= 0) {
              printf("recordbuilder::buildByBank: LZ4_compress_fast failed for bank %d\n", b);
              return;
          }
          comp_sizes[b] = static_cast<uint32_t>(cs);
          compressed[b].resize(cs);
      }

      // Layout the compressed-payload section in bufferRecord (after the
      // 56-byte record header).
      const size_t desc_off      = 8;
      const size_t desc_bytes    = 4 * num_banks;
      const size_t comp_size_off = desc_off + desc_bytes;
      const size_t decomp_size_off = comp_size_off + 4 * num_banks;
      const size_t tags_off      = decomp_size_off + 4 * num_banks;
      const size_t presence_off  = tags_off + 4 * event_count;
      const size_t ebs_off       = presence_off + event_count * bytes_per_row;
      const size_t streams_off   = ebs_off + 4 * num_banks * event_count;

      // Total compressed payload bytes (before any alignment padding).
      uint64_t total_comp = 0;
      for (auto cs : comp_sizes) total_comp += cs;
      const size_t payload_total = streams_off + total_comp;

      const size_t needed_cap = static_cast<size_t>(RECORD_HEADER_SIZE) + payload_total + 1024;
      if (bufferRecord.size() < needed_cap) bufferRecord.resize(needed_cap);

      char* out = &bufferRecord[RECORD_HEADER_SIZE];

      // u32 num_banks
      uint32_t v = static_cast<uint32_t>(num_banks);
      std::memcpy(out + 0, &v, 4);
      // u32 event_count
      v = static_cast<uint32_t>(event_count);
      std::memcpy(out + 4, &v, 4);
      // descriptors
      for (int b = 0; b < num_banks; ++b) {
          char* d = out + desc_off + b * 4;
          uint16_t g = sorted_banks[b].group;
          std::memcpy(d, &g, 2);
          d[2] = static_cast<char>(sorted_banks[b].item);
          d[3] = static_cast<char>(sorted_banks[b].data_type);
      }
      // sizes
      for (int b = 0; b < num_banks; ++b) {
          std::memcpy(out + comp_size_off   + b * 4, &comp_sizes[b],   4);
          std::memcpy(out + decomp_size_off + b * 4, &decomp_sizes[b], 4);
      }
      // event tags
      for (int e = 0; e < event_count; ++e) {
          std::memcpy(out + tags_off + e * 4, &event_tags[e], 4);
      }
      // presence
      std::memcpy(out + presence_off, presence.data(), event_count * bytes_per_row);
      // event_bank_sizes
      for (int b = 0; b < num_banks; ++b) {
          char* dst = out + ebs_off + b * 4 * event_count;
          std::memcpy(dst, &event_bank_sizes[static_cast<size_t>(b) * event_count], 4 * event_count);
      }
      // concatenated compressed streams
      size_t off = streams_off;
      for (int b = 0; b < num_banks; ++b) {
          std::memcpy(out + off, compressed[b].data(), comp_sizes[b]);
          off += comp_sizes[b];
      }

      // Header
      const int compressed_payload_size = static_cast<int>(payload_total);
      const int rounding                = getRecordLengthRounding(compressed_payload_size);
      const int compressed_to_write     = compressed_payload_size + rounding;
      const int compressed_words        = compressed_to_write / 4;
      const int recordLength            = compressed_words + RECORD_HEADER_WORDS;
      // For Lz4ByBank, "recordDataLength" doesn't have a single canonical
      // meaning (it would be the sum of all uncompressed bank streams).
      uint64_t total_decomp = 0;
      for (auto ds : decomp_sizes) total_decomp += ds;

      for (int p = 0; p < rounding; ++p) bufferRecord[RECORD_HEADER_SIZE + payload_total + p] = 0;

      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_LENGTH_OFFSET,   recordLength);
      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_NUMBER_OFFSET,   0);
      hipo::utils::writeInt(&bufferRecord[0], RH_HEADER_LENGTH_OFFSET,   RECORD_HEADER_WORDS);
      hipo::utils::writeInt(&bufferRecord[0], RH_EVENT_COUNT_OFFSET,     event_count);
      hipo::utils::writeInt(&bufferRecord[0], RH_INDEX_ARRAY_LEN_OFFSET, 0);
      const int versionWord = (rounding << BITINFO_PAD3_SHIFT) | HIPO_VERSION;
      hipo::utils::writeInt(&bufferRecord[0], RH_BIT_INFO_OFFSET,        versionWord);
      hipo::utils::writeInt(&bufferRecord[0], RH_USER_HEADER_LEN_OFFSET, 0);
      hipo::utils::writeInt(&bufferRecord[0], RH_MAGIC_NUMBER_OFFSET,    HEADER_MAGIC);
      hipo::utils::writeInt(&bufferRecord[0], RH_DATA_LENGTH_OFFSET,     static_cast<int>(total_decomp));
      const uint32_t type_bits = static_cast<uint32_t>(compression_type::lz4_by_bank);
      const int compressionWord = (type_bits << COMP_TYPE_SHIFT) | (COMP_LENGTH_MASK & compressed_words);
      hipo::utils::writeInt(&bufferRecord[0], RH_COMP_WORD_OFFSET,       compressionWord);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD1_OFFSET,     bufferUserWordOne);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD2_OFFSET,     bufferUserWordTwo);
#endif
  }

  /**
   * Build the record using the Lz4ByPage compressed layout (compression tag 6).
   *
   * RNTuple-style per-(bank, column, page) compression. For each bank
   * type, each schema column gets its own LZ4 stream; each column's data
   * is further split into pages of `events_per_page` events, with each
   * page independently compressed. Wire layout (LE u32 unless noted):
   *
   *   u32 num_banks                                B
   *   u32 event_count                              E
   *   u32 events_per_page                          P
   *   B × { u16 group, u8 item, u8 type, u32 num_columns }
   *   E × ceil(B/8) bytes presence_matrix
   *   B × E × u32 event_row_counts
   *   u32 total_streams (T = Σ_b num_cols[b] × ceil(E/P))
   *   T × { u16 bank_idx, u16 col_idx, u32 first_event,
   *         u32 page_event_count, u32 comp_size, u32 decomp_size }
   *   concat T × LZ4 page payloads
   *
   * Requires the writer to have pushed its hipo::dictionary via
   * setDictionary(); we need the schema to know per-column row strides.
   */
  void recordbuilder::buildByPage(){
#ifndef __LZ4__
      printf("recordbuilder::buildByPage: LZ4 not available\n");
      return;
#else
      if (activeDict == nullptr) {
          throw hipo::record_error(
              "recordbuilder::buildByPage: dictionary not set "
              "(call recordbuilder::setDictionary() first)");
      }
      const int      event_count     = bufferIndexEntries;
      const uint32_t events_per_page = std::max(1u, activeSpec.events_per_page);
      const uint32_t num_pages_per_col = (event_count + events_per_page - 1) / events_per_page;

      // Pass 1: walk events, collect bank discovery + per-event row counts &
      // pointers into bufferEvents.
      struct bank_info {
          uint16_t group;
          uint8_t  item;
          uint8_t  data_type;
      };
      std::vector<bank_info> banks;
      auto bank_pos_of = [&](uint16_t g, uint8_t i) -> int {
          for (size_t k = 0; k < banks.size(); ++k) {
              if (banks[k].group == g && banks[k].item == i) return static_cast<int>(k);
          }
          return -1;
      };

      struct per_event_bank {
          int      bank_pos;     // index into `banks` (later remapped to sorted)
          uint32_t data_off;     // start of bank's data bytes in bufferEvents
          uint32_t num_rows;     // rows in this bank for this event
      };
      std::vector<std::vector<per_event_bank>> per_event(event_count);

      // (group<<8 | item) tuples whose schema isn't in the dictionary —
      // they can't be packed column-wise, so we skip them and log once.
      std::unordered_set<uint32_t> skipped_unknown;

      uint32_t event_running_off = 0;
      for (int e = 0; e < event_count; ++e) {
          const uint32_t evt_size_field = *reinterpret_cast<uint32_t*>(&bufferIndex[e*4]);
          const uint32_t evt_start = event_running_off;

          uint32_t pos = EVENT_HEADER_SIZE;
          while (pos + BANK_STRUCTURE_SIZE < evt_size_field) {
              uint16_t gid;
              std::memcpy(&gid, &bufferEvents[evt_start + pos], sizeof(gid));
              uint8_t  iid  = static_cast<uint8_t>(bufferEvents[evt_start + pos + 2]);
              uint8_t  type = static_cast<uint8_t>(bufferEvents[evt_start + pos + 3]);
              uint32_t word;
              std::memcpy(&word, &bufferEvents[evt_start + pos + 4], sizeof(word));
              const uint32_t data_len = word & STRUCT_SIZE_MASK;

              // Bounds: a malformed/corrupt data_len that overruns the
              // event would later read past valid event data and trip
              // libstdc++'s _GLIBCXX_ASSERTIONS bounds check. Bail.
              if (static_cast<uint64_t>(pos) + BANK_STRUCTURE_SIZE + data_len > evt_size_field) {
                  break;
              }

              int idx = bank_pos_of(gid, iid);
              if (idx < 0) {
                  // Lz4ByPage needs a schema to know per-column layout.
                  // Banks not in the writer's dictionary cannot be packed
                  // column-wise — skip them (they will not appear in the
                  // ByPage output). This handles real files that carry
                  // unregistered banks (e.g. private skim tags) AND the
                  // (0,0,0,0) zero-padding case some events end with.
                  const hipo::schema* sch = activeDict->findByGroupItem(gid, iid);
                  if (!sch) {
                      skipped_unknown.insert((static_cast<uint32_t>(gid) << 8) | iid);
                      pos += data_len + BANK_STRUCTURE_SIZE;
                      continue;
                  }
                  idx = static_cast<int>(banks.size());
                  banks.push_back({gid, iid, type});
              }

              // Lz4ByPage's wire format assumes each (group, item) appears
              // at most once per event. Skip duplicates — see buildByBank
              // for the rationale (write_off OOB on the per-bank stream).
              bool dup = false;
              for (const auto& prior : per_event[e]) {
                  if (prior.bank_pos == idx) { dup = true; break; }
              }
              if (dup) { pos += data_len + BANK_STRUCTURE_SIZE; continue; }

              // O(1) schema lookup by (group, item) for num_rows compute.
              uint32_t num_rows = 0;
              {
                  const hipo::schema* sch = activeDict->findByGroupItem(gid, iid);
                  if (sch && data_len > 0) {
                      const int row_size = const_cast<hipo::schema*>(sch)->getRowLength();
                      if (row_size > 0) num_rows = data_len / row_size;
                  }
              }

              per_event[e].push_back({idx, evt_start + pos + BANK_STRUCTURE_SIZE, num_rows});
              pos += data_len + BANK_STRUCTURE_SIZE;
          }
          event_running_off += evt_size_field;
      }

      // Dedup warnings across all records in this writer's lifetime. Each
      // unique (group, item) is reported only the FIRST time it's skipped,
      // so a file with many records carrying the same handful of unknown
      // banks produces a handful of lines, not thousands.
      static thread_local std::unordered_set<uint32_t> warned_unknown;
      for (uint32_t k : skipped_unknown) {
          if (warned_unknown.insert(k).second) {
              fprintf(stderr, "recordbuilder::buildByPage: skipping unknown bank (%u,%u) — not in dictionary; will not appear in ByPage output\n",
                      static_cast<uint32_t>((k >> 8) & 0xFFFFFFu),
                      static_cast<uint32_t>(k & 0xFF));
          }
      }

      const int num_banks = static_cast<int>(banks.size());

      // Sort descriptors by (group, item) for reproducible on-disk order.
      std::vector<int> sort_order(num_banks);
      for (int i = 0; i < num_banks; ++i) sort_order[i] = i;
      std::sort(sort_order.begin(), sort_order.end(),
                [&](int a, int b) {
                    if (banks[a].group != banks[b].group) return banks[a].group < banks[b].group;
                    return banks[a].item < banks[b].item;
                });
      std::vector<int> orig_to_sorted(num_banks);
      for (int s = 0; s < num_banks; ++s) orig_to_sorted[sort_order[s]] = s;
      std::vector<bank_info> sorted_banks(num_banks);
      for (int s = 0; s < num_banks; ++s) sorted_banks[s] = banks[sort_order[s]];

      // Resolve sorted bank → schema and column metadata (O(1) per bank).
      // Banks without a schema were already filtered out during the walk;
      // we expect findByGroupItem to always succeed here.
      std::vector<const hipo::schema*> sorted_schemas(num_banks, nullptr);
      std::vector<int> sorted_num_cols(num_banks, 0);
      for (int s = 0; s < num_banks; ++s) {
          const hipo::schema* sch = activeDict->findByGroupItem(
              sorted_banks[s].group, sorted_banks[s].item);
          if (!sch) {
              // Should not happen given the walk filter; defend anyway.
              throw hipo::record_error(
                  "recordbuilder::buildByPage: schema disappeared mid-build for bank (" +
                  std::to_string(sorted_banks[s].group) + "," +
                  std::to_string(sorted_banks[s].item) + ")");
          }
          sorted_schemas[s]   = sch;
          sorted_num_cols[s]  = const_cast<hipo::schema*>(sch)->getEntries();
      }

      // Per-(bank, event) row counts (flat) and presence bitmap.
      std::vector<uint32_t> row_counts(static_cast<size_t>(num_banks) * event_count, 0);
      const size_t bytes_per_row = (num_banks + 7) / 8;
      std::vector<uint8_t> presence(static_cast<size_t>(event_count) * bytes_per_row, 0);
      for (int e = 0; e < event_count; ++e) {
          for (const auto& peb : per_event[e]) {
              const int s = orig_to_sorted[peb.bank_pos];
              row_counts[static_cast<size_t>(s) * event_count + e] = peb.num_rows;
              const size_t byte = static_cast<size_t>(e) * bytes_per_row + s / 8;
              presence[byte] |= static_cast<uint8_t>(1u << (s % 8));
          }
      }

      // Index per_event for O(1) lookup: per-event, per-sorted-bank → per_event_bank*.
      // For each (event, sorted_bank), find the per_event_bank entry (or nullptr).
      // num_banks × event_count entries.
      std::vector<const per_event_bank*> peb_index(
          static_cast<size_t>(num_banks) * event_count, nullptr);
      for (int e = 0; e < event_count; ++e) {
          for (const auto& peb : per_event[e]) {
              const int s = orig_to_sorted[peb.bank_pos];
              peb_index[static_cast<size_t>(s) * event_count + e] = &peb;
          }
      }

      // Build per-(bank, col, page) uncompressed bytes & LZ4-compress.
      // We stream straight into compressed[] vectors, one per stream.
      const uint32_t total_streams = [&]{
          uint32_t t = 0;
          for (int s = 0; s < num_banks; ++s) t += sorted_num_cols[s] * num_pages_per_col;
          return t;
      }();

      struct stream_meta {
          uint16_t bank_idx;
          uint16_t col_idx;
          uint32_t first_event;
          uint32_t page_event_count;
          uint32_t comp_size;
          uint32_t decomp_size;
      };
      std::vector<stream_meta> metas;
      metas.reserve(total_streams);
      std::vector<std::vector<char>> compressed_pages;
      compressed_pages.reserve(total_streams);

      std::vector<char> scratch;   // reused page-uncompressed buffer
      for (int s = 0; s < num_banks; ++s) {
          const hipo::schema& sch = *sorted_schemas[s];
          const int ncols = sorted_num_cols[s];
          for (int c = 0; c < ncols; ++c) {
              const auto& e_meta = const_cast<hipo::schema&>(sch).getEntry(c);
              const uint32_t col_offset      = static_cast<uint32_t>(e_meta.offset);
              const uint32_t col_bytes_per_row = static_cast<uint32_t>(e_meta.typeSize) *
                                                  static_cast<uint32_t>(e_meta.arrayLength);

              for (uint32_t pg = 0; pg < num_pages_per_col; ++pg) {
                  const uint32_t first = pg * events_per_page;
                  const uint32_t last  = std::min<uint32_t>(first + events_per_page,
                                                            static_cast<uint32_t>(event_count));
                  // Compute decompressed size for this page.
                  uint32_t decomp_size = 0;
                  for (uint32_t e = first; e < last; ++e) {
                      decomp_size += row_counts[static_cast<size_t>(s) * event_count + e]
                                      * col_bytes_per_row;
                  }
                  scratch.resize(decomp_size);
                  // Fill scratch with concatenated column-c bytes for events [first, last).
                  size_t wo = 0;
                  for (uint32_t e = first; e < last; ++e) {
                      const per_event_bank* peb = peb_index[static_cast<size_t>(s) * event_count + e];
                      if (!peb || peb->num_rows == 0) continue;
                      const uint32_t col_byte_off = peb->num_rows * col_offset;
                      const uint32_t col_byte_len = peb->num_rows * col_bytes_per_row;
                      std::memcpy(&scratch[wo], &bufferEvents[peb->data_off + col_byte_off],
                                  col_byte_len);
                      wo += col_byte_len;
                  }

                  // LZ4-compress.
                  std::vector<char> comp;
                  if (decomp_size == 0) {
                      // Empty page: zero-byte compressed payload.
                      metas.push_back({static_cast<uint16_t>(s), static_cast<uint16_t>(c),
                                       first, last - first, 0, 0});
                      compressed_pages.push_back(std::move(comp));
                      continue;
                  }
                  const int bound = LZ4_compressBound(static_cast<int>(decomp_size));
                  comp.resize(bound);
                  int cs = LZ4_compress_fast(scratch.data(), comp.data(),
                                              static_cast<int>(decomp_size), bound, 1);
                  if (cs <= 0) {
                      throw hipo::record_error(
                          "recordbuilder::buildByPage: LZ4_compress_fast failed for stream " +
                          std::to_string(metas.size()));
                  }
                  comp.resize(cs);
                  metas.push_back({static_cast<uint16_t>(s), static_cast<uint16_t>(c),
                                   first, last - first,
                                   static_cast<uint32_t>(cs), decomp_size});
                  compressed_pages.push_back(std::move(comp));
              }
          }
      }

      // Lay out the compressed-payload section in bufferRecord.
      const size_t desc_off      = 12;
      const size_t desc_bytes    = 8 * num_banks;
      const size_t presence_off  = desc_off + desc_bytes;
      const size_t presence_bytes = static_cast<size_t>(event_count) * bytes_per_row;
      const size_t rowcounts_off = presence_off + presence_bytes;
      const size_t rowcounts_bytes = static_cast<size_t>(4) * num_banks * event_count;
      const size_t streams_hdr_off = rowcounts_off + rowcounts_bytes;
      const size_t stream_desc_off = streams_hdr_off + 4;
      const size_t stream_desc_bytes = static_cast<size_t>(20) * total_streams;
      const size_t streams_off = stream_desc_off + stream_desc_bytes;
      uint64_t total_comp = 0;
      for (auto& m : metas) total_comp += m.comp_size;
      const size_t payload_total = streams_off + total_comp;

      const size_t needed_cap = static_cast<size_t>(RECORD_HEADER_SIZE) + payload_total + 1024;
      if (bufferRecord.size() < needed_cap) bufferRecord.resize(needed_cap);

      char* out = &bufferRecord[RECORD_HEADER_SIZE];
      uint32_t v = static_cast<uint32_t>(num_banks);
      std::memcpy(out + 0, &v, 4);
      v = static_cast<uint32_t>(event_count);
      std::memcpy(out + 4, &v, 4);
      v = events_per_page;
      std::memcpy(out + 8, &v, 4);

      // Bank descriptors.
      for (int s = 0; s < num_banks; ++s) {
          char* d = out + desc_off + s * 8;
          uint16_t g = sorted_banks[s].group;
          std::memcpy(d + 0, &g, 2);
          d[2] = static_cast<char>(sorted_banks[s].item);
          d[3] = static_cast<char>(sorted_banks[s].data_type);
          uint32_t nc = static_cast<uint32_t>(sorted_num_cols[s]);
          std::memcpy(d + 4, &nc, 4);
      }

      // Presence matrix.
      std::memcpy(out + presence_off, presence.data(), presence_bytes);

      // Per-(bank, event) row counts. Flat layout matches reader.
      for (int s = 0; s < num_banks; ++s) {
          std::memcpy(out + rowcounts_off + static_cast<size_t>(s) * event_count * 4,
                      &row_counts[static_cast<size_t>(s) * event_count],
                      static_cast<size_t>(4) * event_count);
      }

      // Stream descriptors header + entries.
      uint32_t t32 = static_cast<uint32_t>(total_streams);
      std::memcpy(out + streams_hdr_off, &t32, 4);
      for (uint32_t i = 0; i < total_streams; ++i) {
          char* d = out + stream_desc_off + static_cast<size_t>(i) * 20;
          const auto& m = metas[i];
          std::memcpy(d + 0,  &m.bank_idx, 2);
          std::memcpy(d + 2,  &m.col_idx, 2);
          std::memcpy(d + 4,  &m.first_event, 4);
          std::memcpy(d + 8,  &m.page_event_count, 4);
          std::memcpy(d + 12, &m.comp_size, 4);
          std::memcpy(d + 16, &m.decomp_size, 4);
      }

      // Concatenated compressed streams.
      size_t off = streams_off;
      for (uint32_t i = 0; i < total_streams; ++i) {
          if (metas[i].comp_size == 0) continue;
          std::memcpy(out + off, compressed_pages[i].data(), metas[i].comp_size);
          off += metas[i].comp_size;
      }

      // Header.
      const int compressed_payload_size = static_cast<int>(payload_total);
      const int rounding                = getRecordLengthRounding(compressed_payload_size);
      const int compressed_to_write     = compressed_payload_size + rounding;
      const int compressed_words        = compressed_to_write / 4;
      const int recordLength            = compressed_words + RECORD_HEADER_WORDS;
      uint64_t total_decomp = 0;
      for (auto& m : metas) total_decomp += m.decomp_size;

      for (int p = 0; p < rounding; ++p) bufferRecord[RECORD_HEADER_SIZE + payload_total + p] = 0;

      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_LENGTH_OFFSET,   recordLength);
      hipo::utils::writeInt(&bufferRecord[0], RH_RECORD_NUMBER_OFFSET,   0);
      hipo::utils::writeInt(&bufferRecord[0], RH_HEADER_LENGTH_OFFSET,   RECORD_HEADER_WORDS);
      hipo::utils::writeInt(&bufferRecord[0], RH_EVENT_COUNT_OFFSET,     event_count);
      hipo::utils::writeInt(&bufferRecord[0], RH_INDEX_ARRAY_LEN_OFFSET, 0);
      const int versionWord = (rounding << BITINFO_PAD3_SHIFT) | HIPO_VERSION;
      hipo::utils::writeInt(&bufferRecord[0], RH_BIT_INFO_OFFSET,        versionWord);
      hipo::utils::writeInt(&bufferRecord[0], RH_USER_HEADER_LEN_OFFSET, 0);
      hipo::utils::writeInt(&bufferRecord[0], RH_MAGIC_NUMBER_OFFSET,    HEADER_MAGIC);
      hipo::utils::writeInt(&bufferRecord[0], RH_DATA_LENGTH_OFFSET,     static_cast<int>(total_decomp));
      const uint32_t type_bits = static_cast<uint32_t>(compression_type::lz4_by_page);
      const int compressionWord = (type_bits << COMP_TYPE_SHIFT) | (COMP_LENGTH_MASK & compressed_words);
      hipo::utils::writeInt(&bufferRecord[0], RH_COMP_WORD_OFFSET,       compressionWord);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD1_OFFSET,     bufferUserWordOne);
      hipo::utils::writeLong(&bufferRecord[0], RH_USER_WORD2_OFFSET,     bufferUserWordTwo);
#endif
  }

  /**
   * Compresses the constructed buffer with LZ4 into internal buffer that
   * will be written to the output.
   */
  int  recordbuilder::compressRecord(int src_size){

    #ifdef __LZ4__
    //(const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);
      int result = LZ4_compress_fast(&bufferData[0],&bufferRecord[RECORD_HEADER_SIZE],src_size,bufferRecord.size(),1);

      //int   result = LZ4_decompress_safe(data,output,dataLength,dataLengthUncompressed);
      //int   result = LZ4_decompress_fast(data,output,dataLengthUncompressed);
      return result;
      //printf(" FIRST (%d) = %x %x %x %x\n",result);//,destUnCompressed[0],destUnCompressed[1],
      //destUnCompressed[2],destUnCompressed[3]);
      //LZ4_decompress_fast(buffer,destUnCompressed,decompressedLength);
      //LZ4_uncompress(buffer,destUnCompressed,decompressedLength);
      #endif

      #ifndef __LZ4__
        printf("\n   >>>>> LZ4 compression is not supported.");
        printf("\n   >>>>> check if libz4 is installed on your system.");
        printf("\n   >>>>> recompile the library with liblz4 installed.\n");
        return 0;
      #endif
  }
}
