/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   recordbuilder.h
 * Author: gavalian
 *
 * Created on April 11, 2017, 4:47 PM
 */

#ifndef HIPORECORDBUILDER_H
#define HIPORECORDBUILDER_H

#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "event.h"
#include "utils.h"
#include "constants.h"

namespace hipo {

    class dictionary;  // fwd decl for Phase 4 setDictionary

    class recordbuilder {

      private:
        const int  defaultNumberOfEvents = 100000;
        const int  defaultRecordSize     = 8*1024*1024;
        //std::vector< std::vector<char> > eventBuffer;
        std::vector<char>  bufferIndex;
        std::vector<char>  bufferEvents;
        std::vector<char>  bufferData;
        std::vector<char>  bufferRecord;

        int                bufferIndexEntries;
        int                bufferEventsPosition;
	long               bufferUserWordOne = 0;
	long               bufferUserWordTwo = 0;

        compression_spec   activeSpec{compression_spec::lz4()};
        // Phase 4 (Lz4ByPage): the writer pushes its dictionary here so
        // buildByPage can resolve per-column type sizes. nullptr ⇒ ByPage
        // builds fail with a record_error. Lifetime: caller (the writer)
        // must keep the dictionary alive for the builder's lifetime.
        const hipo::dictionary* activeDict{nullptr};

        int                compressRecord(int src_size);
        int                getRecordLengthRounding(int bufferSize);

        // Phase 2: Lz4Chunked builder. Layout matches hipo-rs:
        // [u32 num_chunks | u32 events_per_chunk | event_count×u32 event_sizes
        //  | N×u32 compressed_chunk_sizes | N×u32 decompressed_chunk_sizes
        //  | concatenated LZ4 chunk payloads]
        void               buildChunked();

        // Phase 3: Lz4ByBank builder. Layout matches hipo-rs.
        void               buildByBank();

        // Phase 4: Lz4ByPage builder. RNTuple-style per-(bank,col,page) LZ4.
        void               buildByPage();

    public:

        recordbuilder(int maxEvents, int maxLength);
        recordbuilder();
        virtual ~recordbuilder(){};

        bool addEvent(std::vector<char> &vec, int start, int length);
        bool addEvent(hipo::event &evnt);

        long getUserWordOne();
        long getUserWordTwo();
	void setUserWordOne(long userWordOne);
	void setUserWordTwo(long userWordTwo);

        int  getRecordSize();
        int  getEntries();
        std::vector<char> &getRecordBuffer(){ return bufferRecord;};
        void reset();
        void build();

        // Configure compression for subsequent build() calls. Default is
        // compression_spec::lz4() to keep round-trip with older C++ readers.
        void setCompression(compression_spec spec) { activeSpec = spec; }
        compression_spec getCompression() const    { return activeSpec; }

        // Phase 4: Lz4ByPage requires per-column type info from the schema
        // dictionary to slice column bytes from each event's bank data.
        // Caller's dictionary must outlive the builder.
        void setDictionary(const hipo::dictionary* dict) { activeDict = dict; }
    };
}
#endif /* HIPORECORD_H */
