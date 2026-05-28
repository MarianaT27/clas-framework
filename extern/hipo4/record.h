//******************************************************************************
//*       ██╗  ██╗██╗██████╗  ██████╗     ██╗  ██╗    ██████╗                  *
//*       ██║  ██║██║██╔══██╗██╔═══██╗    ██║  ██║   ██╔═████╗                 *
//*       ███████║██║██████╔╝██║   ██║    ███████║   ██║██╔██║                 *
//*       ██╔══██║██║██╔═══╝ ██║   ██║    ╚════██║   ████╔╝██║                 *
//*       ██║  ██║██║██║     ╚██████╔╝         ██║██╗╚██████╔╝                 *
//*       ╚═╝  ╚═╝╚═╝╚═╝      ╚═════╝          ╚═╝╚═╝ ╚═════╝                  *
//************************ Jefferson National Lab (2017) ***********************
/*
 *   Copyright (c) 2017.  Jefferson Lab (JLab). All rights reserved. Permission
 *   to use, copy, modify, and distribute  this software and its documentation
 *   for educational, research, and not-for-profit purposes, without fee and
 *   without a signed licensing agreement.
 *
 *   IN NO EVENT SHALL JLAB BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL
 *   INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
 *   OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF JLAB HAS
 *   BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *   JLAB SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *   PURPOSE. THE HIPO DATA FORMAT SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 *   ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". JLAB HAS NO OBLIGATION TO
 *   PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *   This software was developed under the United States Government license.
 *   For more information contact author at gavalian@jlab.org
 *   Department of Experimental Nuclear Physics, Jefferson Lab.
 */

/*
 * File:   record.h
 * Author: gavalian
 *
 * Created on April 11, 2017, 4:47 PM
 */

#ifndef HIPORECORD_H
#define HIPORECORD_H

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "event.h"
#include "utils.h"

namespace hipo {

    class bybankrecord;  // forward decl for ByBank-direct mode (see below)
    class bypagerecord;  // forward decl for ByPage-direct mode (Phase 4)
    class dictionary;    // forward decl for ByPage schema lookups


    typedef struct recordHeader_t {
      int signatureString{}; // 1) identifier string is HREC (int = 0x43455248
      int recordLength{}; // 2) TOTAL Length of the RECORD, includes INDEX array
      int recordDataLength{}; // 3) Length of the DATA uncompressed
      int recordDataLengthCompressed{}; // 4) compressed length of the DATA buffer
      int numberOfEvents{} ; // 5) number of event, data buckets in DATA buffer
      int headerLength{} ; // 6) Length of the buffer represengin HEADER for the record
      int indexDataLength{} ; // 7) Length of the index buffer (in bytes)
      int userHeaderLength{}; // user header length in bytes
      int userHeaderLengthPadding{}; // the padding added to user header Length
      int bitInfo{};
      int compressionType{};
      int compressedLengthPadding{};
      int dataEndianness{};
    } recordHeader_t;

    class data {
      private:
      const char  *data_ptr{};
      int          data_size{};
      int          data_endianness{};
      int          data_offset{};
      int          data_type;
      
      public:
        data(){ data_ptr = nullptr; data_size = 0;}
        ~data()= default;

        void setDataPtr(const char *__ptr){ data_ptr = __ptr;}
        void setDataSize(int __size){ data_size = __size;}
        void setDataOffset(int __offset) { data_offset = __offset;}
        void setDataEndianness(int __endianness) { data_endianness = __endianness;}
        void setDataType(int __type){ data_type = __type;}
        const uint32_t   *getEvioPtr(){ return reinterpret_cast<const uint32_t *>(data_ptr);}
        int         getEvioSize(){ return (int) data_size/4 ;}
        const char *getDataPtr(){ return data_ptr;}
        int         getDataSize(){ return data_size;}
        int         getDataType(){ return data_type;}
        int         getDataEndianness(){ return data_endianness;}
        int         getDataOffset(){ return data_offset;}
    };

    class dataframe {
       private:
         std::vector<char>  dataBuffer;
         recordHeader_t     recordHeader{};
         int                maxEvents = 50;
         int                maxSize   = 512*1024;

       public:
         dataframe(){};
         dataframe(int max_events, int max_size);
         ~dataframe(){};
         
         void         init(const char *ptr);
         bool         addEvent(hipo::event &event);
         int          getEventAt(int pos, hipo::event &event);
         int          count();
         int          size();
         void         reset();
         const char  *buffer(){ return &dataBuffer[0];}
         
         void summary();
    };

    class record {

      private:

        //std::vector< std::vector<char> > eventBuffer;
        std::vector<char>  recordHeaderBuffer;
        recordHeader_t     recordHeader{};

        std::vector<char>  recordBuffer;
        std::vector<char>  recordCompressedBuffer;

        // ByBank-direct mode: when enabled, tag-5 records bypass the v1
        // eager-decode path (decodeByBank) and produce a shared_ptr<bybankrecord>
        // accessible via byBank(). The lazy per-bank inflate then happens
        // through bybankrecord::bank_stream() when the user touches a bank.
        // Default OFF for backward-compat with callers that depend on
        // recordBuffer being filled after readRecord.
        bool                            useByBankDirect_{false};
        std::shared_ptr<bybankrecord>   currentByBank_;

        // Phase 4: same pattern for Lz4ByPage (tag 6).
        bool                            useByPageDirect_{false};
        std::shared_ptr<bypagerecord>   currentByPage_;
        // Dictionary required by bypagerecord to resolve column type sizes.
        // Reader sets this at open() time. Lifetime: caller-owned.
        const hipo::dictionary*         schemaDict_{nullptr};

        hipo::benchmark                 readBenchmark;
        hipo::benchmark                 unzipBenchmark;
        hipo::benchmark                 indexBenchmark;

        char *getUncompressed(const char *data, int dataLength, int dataLengthUncompressed);
        int   getUncompressed(const char *data, char *dest, int dataLength, int dataLengthUncompressed);
        void  showBuffer(const char *data, int wrapping, int maxsize);

        // Lz4Chunked decoder (compression tag 4). Parses the directory
        // table at recordCompressedBuffer[0..], inflates each chunk, and
        // populates recordBuffer with the [cumulative_event_positions |
        // concatenated_events] layout the legacy getData() path expects.
        // Big-endian records are rejected explicitly. Returns true on
        // success, false on any LZ4 or layout error.
        bool  decodeChunked(int section_bytes);

        // Lz4ByBank decoder (compression tag 5). Parses the by-bank
        // directory via hipo::bybankrecord, decompresses every bank
        // stream, then reassembles each event's [EH | structure...] blob
        // into recordBuffer using the same legacy layout getData() expects.
        // v1 is eager (decompresses all banks); a future v2 will surface
        // hipo::bybankrecord directly to callers for lazy per-bank inflate.
        bool  decodeByBank(int section_bytes);

    public:

        record();
        ~record();

        void   read(hipo::bank &b, int event);
        void   readRecord(std::ifstream &stream, long position, int dataOffset);
        void   readRecord__(std::ifstream &stream, long position, long recordLength);
        bool   readRecord(std::ifstream &stream, long position, int dataOffset, long inputSize);
        int    getEventCount();
        int    getRecordSizeCompressed();
        
        void   getColumn(hipo::data &data,int column, hipo::bank &bank, int event);
        void   getColumn(hipo::data &data,const char* column, hipo::bank &bank, int event);
        
        void   readEvent( std::vector<char> &vec, int index);
        void   readHipoEvent(hipo::event &event, int index);
        
        void   getData(   hipo::data &data, int index);
        //void   getBank(   hipo::bank &bank, int index);

        void   getEventsMap(std::vector<std::pair<int,int>> &emap);

        hipo::benchmark  &getReadBenchmark(){ return readBenchmark;}
        hipo::benchmark  &getUnzipBenchmark(){ return unzipBenchmark;}
        hipo::benchmark  &getIndexBenchmark(){ return indexBenchmark;}

        // Toggle ByBank-direct mode (Phase 3 v2). When ON, readRecord()
        // on a tag-5 record parses to currentByBank_ and skips the
        // decodeByBank() reassembly. The user retrieves the bybankrecord
        // via byBank() and threads it into hipo::event::initByBank().
        void setUseByBankDirect(bool enable) { useByBankDirect_ = enable; }
        std::shared_ptr<bybankrecord> byBank() const { return currentByBank_; }

        // Phase 4: same toggle for Lz4ByPage (tag 6). Requires
        // setDictionary() to have been called first.
        void setUseByPageDirect(bool enable) { useByPageDirect_ = enable; }
        std::shared_ptr<bypagerecord> byPage() const { return currentByPage_; }
        void setDictionary(const hipo::dictionary* d) { schemaDict_ = d; }
    };
}
#endif /* HIPORECORD_H */
