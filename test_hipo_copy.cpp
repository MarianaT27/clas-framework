// Minimal test: read events from input HIPO file and write to output
// Compile: g++ -std=c++20 -I extern/hipo4 -o test_hipo_copy test_hipo_copy.cpp extern/hipo4/*.cpp -llz4 -lz -lfmt
#include <cstdio>
#include "reader.h"
#include "writer.h"
#include "record.h"
#include "event_builder.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <input.hipo> <output.hipo> [max_events]\n", argv[0]);
        return 1;
    }

    int max_events = (argc > 3) ? std::atoi(argv[3]) : 10;

    // Open input
    auto rdr = hipo::reader::open(argv[1]);
    if (!rdr) { printf("Failed to open input\n"); return 1; }
    printf("Input: %d records, dict size %zu\n", rdr->num_records(), rdr->dict().size());

    // Open output writer with same dictionary
    hipo::writer wr;
    wr.set_dictionary(rdr->dict());
    auto res = wr.open(argv[2]);
    if (!res) { printf("Failed to open output\n"); return 1; }

    // Read events from first record and write them
    hipo::record rec;
    int total_written = 0;

    for (int r = 0; r < rdr->num_records() && total_written < max_events; ++r) {
        auto load_res = rdr->load_record(rec, r);
        if (!load_res) { printf("Failed to load record %d\n", r); continue; }

        int nevents = rec.event_count();
        printf("Record %d: %d events\n", r, nevents);

        for (int e = 0; e < nevents && total_written < max_events; ++e) {
            auto ev = rec.event_data(e);
            if (!ev) { printf("  Failed to get event %d\n", e); continue; }

            printf("  Event %d: %zu bytes, first 8: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                   e, ev->size(),
                   (unsigned char)(*ev)[0], (unsigned char)(*ev)[1],
                   (unsigned char)(*ev)[2], (unsigned char)(*ev)[3],
                   (unsigned char)(*ev)[4], (unsigned char)(*ev)[5],
                   (unsigned char)(*ev)[6], (unsigned char)(*ev)[7]);

            // Convert to EVNT format: replace first 4 bytes with "EVNT" magic
            std::vector<char> evnt_data(ev->begin(), ev->end());
            evnt_data[0] = 'E'; evnt_data[1] = 'V';
            evnt_data[2] = 'N'; evnt_data[3] = 'T';
            auto write_res = wr.add_event(std::span<const char>(evnt_data));
            if (!write_res) { printf("  Failed to write event\n"); continue; }
            total_written++;
        }
    }

    auto close_res = wr.close();
    if (!close_res) { printf("Failed to close writer\n"); return 1; }

    printf("\nWrote %d events to %s\n", total_written, argv[2]);
    return 0;
}
