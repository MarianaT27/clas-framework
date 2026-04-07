// Test using the OLD JLab hipo4 library to write a file
// This should produce files compatible with hipo-utils
#include <cstdio>
#include "reader.h"
#include "writer.h"
#include "event.h"
#include "bank.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <input.hipo> <output.hipo> [max_events]\n", argv[0]);
        return 1;
    }
    int max_events = (argc > 3) ? std::atoi(argv[3]) : 10;

    // Use OLD reader to read input
    hipo::reader reader;
    reader.open(argv[1]);
    printf("Input opened\n");

    // Read dictionary
    hipo::dictionary dict;
    reader.readDictionary(dict);
    printf("Dictionary: %d schemas\n", (int)dict.getSchemaList().size());

    // Use OLD writer
    hipo::writer writer;
    writer.addDictionary(dict);
    writer.open(argv[2]);
    printf("Output opened\n");

    hipo::event evt;
    int written = 0;
    while (reader.next() && written < max_events) {
        reader.read(evt);
        writer.addEvent(evt);
        if (written < 3) {
            printf("  Event %d: size=%d\n", written, evt.getSize());
        }
        written++;
    }

    writer.close();
    printf("Wrote %d events to %s\n", written, argv[2]);
    return 0;
}
