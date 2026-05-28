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
  * File:   record.cpp
  * Author: gavalian
  *
  * Created on April 11, 2017, 4:47 PM
  */

#include "record.h"
#include "bybankrecord.h"
#include "bypagerecord.h"
#include "constants.h"
#include "hipoexceptions.h"

#include <future>

#ifdef __LZ4__
#include <lz4.h>
#endif

namespace hipo {


    record::record()= default;


    record::~record()= default;

    /**
     * Read
     */
    void record::readRecord(std::ifstream &stream, long position, int dataOffset){

        recordHeaderBuffer.resize(80);
        stream.seekg(position,std::ios::beg);

        stream.read( (char *) &recordHeaderBuffer[0],80);
        recordHeader.recordLength     = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_RECORD_LENGTH_OFFSET]));
        recordHeader.headerLength     = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_HEADER_LENGTH_OFFSET]));
        recordHeader.numberOfEvents   = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_EVENT_COUNT_OFFSET]));
        recordHeader.bitInfo          = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_BIT_INFO_OFFSET]));
        recordHeader.signatureString  = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_MAGIC_NUMBER_OFFSET]));
        recordHeader.recordDataLength = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_DATA_LENGTH_OFFSET]));
        recordHeader.userHeaderLength = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_USER_HEADER_LEN_OFFSET]));
        int compressedWord            = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_COMP_WORD_OFFSET]));

        if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC) recordHeader.dataEndianness = 0;
        if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC_BE) recordHeader.dataEndianness = 1;

        if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC_BE){
          recordHeader.recordLength     = __builtin_bswap32(recordHeader.recordLength);
          recordHeader.headerLength     = __builtin_bswap32(recordHeader.headerLength);
          recordHeader.numberOfEvents   = __builtin_bswap32(recordHeader.numberOfEvents);
          recordHeader.recordDataLength = __builtin_bswap32(recordHeader.recordDataLength);
          recordHeader.userHeaderLength = __builtin_bswap32(recordHeader.userHeaderLength);
          recordHeader.bitInfo          = __builtin_bswap32(recordHeader.bitInfo);
          compressedWord                = __builtin_bswap32(compressedWord);
        }

        int compressedDataLengthPadding = (recordHeader.bitInfo>>BITINFO_PAD3_SHIFT)&BITINFO_PAD_MASK;
        int headerLengthBytes     = recordHeader.headerLength*4;
        int dataBufferLengthBytes = recordHeader.recordLength * 4 - headerLengthBytes;

        recordHeader.userHeaderLengthPadding    = (recordHeader.bitInfo>>BITINFO_PAD1_SHIFT)&BITINFO_PAD_MASK;
        recordHeader.recordDataLengthCompressed = compressedWord&COMP_LENGTH_MASK;
        recordHeader.compressionType            = (compressedWord>>COMP_TYPE_SHIFT)&COMP_TYPE_BYTE;
        recordHeader.indexDataLength            = 4*recordHeader.numberOfEvents;

        long  dataposition = position + headerLengthBytes;

        // ByBank-direct fast path: read compressed payload into an
        // uninitialized unique_ptr<char[]> (no zero-init, no double copy
        // through recordCompressedBuffer) and hand ownership to the
        // bybankrecord. Saves the v2 reader hundreds of microseconds per
        // record on multi-MB payloads.
        if(useByBankDirect_
           && recordHeader.compressionType == static_cast<int>(compression_type::lz4_by_bank)){
          currentByBank_.reset();
          std::unique_ptr<char[]> bbBuf(new char[dataBufferLengthBytes]);
          stream.seekg(dataposition, std::ios::beg);
          stream.read(bbBuf.get(), dataBufferLengthBytes);
          currentByBank_ = hipo::bybankrecord::parse_take_raw(
              recordHeader,
              std::move(bbBuf),
              dataBufferLengthBytes,
              dataBufferLengthBytes,
              compressedDataLengthPadding);
          return;
        }
        // Phase 4: ByPage-direct fast path, same pattern.
        if(useByPageDirect_
           && recordHeader.compressionType == static_cast<int>(compression_type::lz4_by_page)){
          if(!schemaDict_){
            throw hipo::record_error("record::readRecord (ByPage): no dictionary set");
          }
          currentByPage_.reset();
          std::unique_ptr<char[]> pBuf(new char[dataBufferLengthBytes]);
          stream.seekg(dataposition, std::ios::beg);
          stream.read(pBuf.get(), dataBufferLengthBytes);
          currentByPage_ = hipo::bypagerecord::parse_take_raw(
              recordHeader,
              std::move(pBuf),
              dataBufferLengthBytes,
              dataBufferLengthBytes,
              compressedDataLengthPadding,
              *schemaDict_);
          return;
        }

        if(static_cast<size_t>(dataBufferLengthBytes)>recordCompressedBuffer.size()){
          int newSize = dataBufferLengthBytes + 5*1024;
          recordCompressedBuffer.resize(newSize);
        }
        stream.seekg(dataposition,std::ios::beg);
        stream.read( (&recordCompressedBuffer[0]), dataBufferLengthBytes);
        //showBuffer(compressedBuffer, 10, 200);
        //printf("position = %ld data position = %ld \n",position, dataposition);
        int decompressedLength = recordHeader.indexDataLength +
                                 recordHeader.userHeaderLength +
                                 recordHeader.userHeaderLengthPadding +
                                 recordHeader.recordDataLength;

        if(recordBuffer.size()<static_cast<size_t>(decompressedLength)){
          //printf(" resizing internal buffer from %lu to %d\n", recordBuffer.size(), recordHeader.recordDataLength);
          recordBuffer.resize(decompressedLength+1024);
        }
        //readBenchmark.pause();
        //for(int i = 0; i < recordBuffer.size(); i++) recordBuffer[i] = 0;
        //printf("****************** BEFORE padding = %d\n", compressedDataLengthPadding);
        //showBuffer(&recordBuffer[0], 10, 200);
        //unzipBenchmark.resume();
        if(recordHeader.compressionType == static_cast<int>(compression_type::lz4_by_bank)){
          // useByBankDirect_ case handled earlier (fast path). This is the
          // v1 eager-decode fallback for callers without ByBank-direct.
          currentByBank_.reset();
          if(!decodeByBank(dataBufferLengthBytes - compressedDataLengthPadding)){
            throw hipo::record_error("Lz4ByBank decode failed");
          }
          return;
        }
        currentByBank_.reset();
        currentByPage_.reset();
        if(recordHeader.compressionType == static_cast<int>(compression_type::lz4_chunked)){
          if(!decodeChunked(dataBufferLengthBytes - compressedDataLengthPadding)){
            throw hipo::record_error("Lz4Chunked decode failed");
          }
          return; // decodeChunked filled recordBuffer + offsets; skip legacy index rewrite
        }
        if(recordHeader.compressionType==0){
          // printf("compression type = 0 data length = %d\n",decompressedLength);
          memcpy((&recordBuffer[0]),(&recordCompressedBuffer[0]),decompressedLength);
        } else {
          getUncompressed((&recordCompressedBuffer[0]) , (&recordBuffer[0]),
				      dataBufferLengthBytes-compressedDataLengthPadding,
					         decompressedLength);
        }
        //unzipBenchmark.pause();
        //printf("******************\n");
        //showBuffer(&recordBuffer[0], 10, 200);
        //char *uncompressedBuffer  = getUncompressed(compressedBuffer,dataBufferLengthBytes,recordHeader.recordDataLength);
        //printf(" decompression size = %d  error = %d\n", unc_result,
	       //unc_result - decompressedLength);
        //free(compressedBuffer);
        /**
         * converting index array from lengths of each buffer in the
         * record to relative positions in the record stream.
         */

        int eventPosition = 0;
        for(int i = 0; i < recordHeader.numberOfEvents; i++){
            auto *ptr = reinterpret_cast<int*>(&recordBuffer[i*4]);
            int size = *ptr;
            if(recordHeader.dataEndianness==1) size = __builtin_bswap32(size);
            eventPosition += size;
            *ptr = eventPosition;
        }
	      //printf("final position = %d\n",eventPosition);
    }

    bool  record::readRecord(std::ifstream &stream, long position, int dataOffset, long inputSize){

      readBenchmark.resume();
      if((position+80)>=inputSize) return false;

      recordHeaderBuffer.resize(80);
      stream.seekg(position,std::ios::beg);

      stream.read( (char *) &recordHeaderBuffer[0],80);
      recordHeader.recordLength     = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_RECORD_LENGTH_OFFSET]));
      recordHeader.headerLength     = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_HEADER_LENGTH_OFFSET]));
      recordHeader.numberOfEvents   = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_EVENT_COUNT_OFFSET]));
      recordHeader.bitInfo          = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_BIT_INFO_OFFSET]));
      recordHeader.signatureString  = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_MAGIC_NUMBER_OFFSET]));
      recordHeader.recordDataLength = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_DATA_LENGTH_OFFSET]));
      recordHeader.userHeaderLength = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_USER_HEADER_LEN_OFFSET]));
      int compressedWord            = *(reinterpret_cast<int *>(&recordHeaderBuffer[RH_COMP_WORD_OFFSET]));

      if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC) recordHeader.dataEndianness = 0;
      if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC_BE) recordHeader.dataEndianness = 1;

      if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC_BE){
        recordHeader.recordLength = __builtin_bswap32(recordHeader.recordLength);
        recordHeader.headerLength = __builtin_bswap32(recordHeader.headerLength);
        recordHeader.numberOfEvents = __builtin_bswap32(recordHeader.numberOfEvents);
        recordHeader.recordDataLength = __builtin_bswap32(recordHeader.recordDataLength);
        recordHeader.userHeaderLength = __builtin_bswap32(recordHeader.userHeaderLength);
        recordHeader.bitInfo          = __builtin_bswap32(recordHeader.bitInfo);
        compressedWord                = __builtin_bswap32(compressedWord);
      }

      int compressedDataLengthPadding = (recordHeader.bitInfo>>BITINFO_PAD3_SHIFT)&BITINFO_PAD_MASK;
      int headerLengthBytes           = recordHeader.headerLength*4;
      int dataBufferLengthBytes       = recordHeader.recordLength * 4 - headerLengthBytes;

      recordHeader.userHeaderLengthPadding    = (recordHeader.bitInfo>>BITINFO_PAD1_SHIFT)&BITINFO_PAD_MASK;
      recordHeader.recordDataLengthCompressed = compressedWord&COMP_LENGTH_MASK;
      recordHeader.compressionType            = (compressedWord>>COMP_TYPE_SHIFT)&COMP_TYPE_BYTE;
      recordHeader.indexDataLength            = 4*recordHeader.numberOfEvents;

      long  dataposition = position + headerLengthBytes;

      if(position+dataBufferLengthBytes+recordHeader.headerLength>inputSize){
        printf("**** warning : record at position %ld is incomplete.",position);
         return false;
      }

      // ByBank-direct fast path (see void readRecord above for rationale).
      if(useByBankDirect_
         && recordHeader.compressionType == static_cast<int>(compression_type::lz4_by_bank)){
        currentByBank_.reset();
        try {
          std::unique_ptr<char[]> bbBuf(new char[dataBufferLengthBytes]);
          stream.seekg(dataposition, std::ios::beg);
          stream.read(bbBuf.get(), dataBufferLengthBytes);
          readBenchmark.pause();
          unzipBenchmark.resume();
          currentByBank_ = hipo::bybankrecord::parse_take_raw(
              recordHeader,
              std::move(bbBuf),
              dataBufferLengthBytes,
              dataBufferLengthBytes,
              compressedDataLengthPadding);
        } catch (const std::exception& e) {
          printf("record::readRecord(byBankDirect): %s\n", e.what());
          unzipBenchmark.pause();
          return false;
        }
        unzipBenchmark.pause();
        return true;
      }
      // Phase 4: ByPage-direct fast path.
      if(useByPageDirect_
         && recordHeader.compressionType == static_cast<int>(compression_type::lz4_by_page)){
        if(!schemaDict_){
          throw hipo::record_error("record::readRecord (ByPage): no dictionary set");
        }
        currentByPage_.reset();
        try {
          std::unique_ptr<char[]> pBuf(new char[dataBufferLengthBytes]);
          stream.seekg(dataposition, std::ios::beg);
          stream.read(pBuf.get(), dataBufferLengthBytes);
          readBenchmark.pause();
          unzipBenchmark.resume();
          currentByPage_ = hipo::bypagerecord::parse_take_raw(
              recordHeader,
              std::move(pBuf),
              dataBufferLengthBytes,
              dataBufferLengthBytes,
              compressedDataLengthPadding,
              *schemaDict_);
        } catch (const std::exception& e) {
          printf("record::readRecord(byPageDirect): %s\n", e.what());
          unzipBenchmark.pause();
          return false;
        }
        unzipBenchmark.pause();
        return true;
      }

      if(static_cast<size_t>(dataBufferLengthBytes)>recordCompressedBuffer.size()){
        int newSize = dataBufferLengthBytes + 500*1024;
        recordCompressedBuffer.resize(newSize);
      }
      stream.seekg(dataposition,std::ios::beg);
      stream.read( (&recordCompressedBuffer[0]), dataBufferLengthBytes);

      int decompressedLength = recordHeader.indexDataLength +
                               recordHeader.userHeaderLength +
                               recordHeader.userHeaderLengthPadding +
                               recordHeader.recordDataLength;

      if(recordBuffer.size()<static_cast<size_t>(decompressedLength)){
        // Buffer resizes by 20% --- modified by me
        int resize_fraction = (int) (decompressedLength*0.2);
        //printf(" resizing internal buffer from %lu to %d\n", recordBuffer.size(), recordHeader.recordDataLength);
        recordBuffer.resize(decompressedLength + resize_fraction);
      }
      //for(int i = 0; i < recordBuffer.size(); i++) recordBuffer[i] = 0;
      //printf("****************** BEFORE padding = %d\n", compressedDataLengthPadding);
      //showBuffer(&recordBuffer[0], 10, 200);
      readBenchmark.pause();
      unzipBenchmark.resume();
      if(recordHeader.compressionType == static_cast<int>(compression_type::lz4_by_bank)){
        // useByBankDirect_ case handled in the fast path above.
        currentByBank_.reset();
        bool ok = decodeByBank(dataBufferLengthBytes - compressedDataLengthPadding);
        unzipBenchmark.pause();
        return ok;
      }
      currentByBank_.reset();
      currentByPage_.reset();
      if(recordHeader.compressionType == static_cast<int>(compression_type::lz4_chunked)){
        bool ok = decodeChunked(dataBufferLengthBytes - compressedDataLengthPadding);
        unzipBenchmark.pause();
        return ok;
      }
      if(recordHeader.compressionType==0){
        // printf("compression type = 0 data length = %d\n",decompressedLength);
        memcpy((&recordBuffer[0]),(&recordCompressedBuffer[0]),decompressedLength);
      } else {
        getUncompressed((&recordCompressedBuffer[0]) , (&recordBuffer[0]),
            dataBufferLengthBytes-compressedDataLengthPadding,
                 decompressedLength);
      }
      unzipBenchmark.pause();
      //printf("******************\n");
      //showBuffer(&recordBuffer[0], 10, 200);
      //char *uncompressedBuffer  = getUncompressed(compressedBuffer,dataBufferLengthBytes,recordHeader.recordDataLength);
      //printf(" decompression size = %d  error = %d\n", unc_result,
       //unc_result - decompressedLength);
      //free(compressedBuffer);
      /**
       * converting index array from lengths of each buffer in the
       * record to relative positions in the record stream.
       */
      indexBenchmark.resume();
      int eventPosition = 0;
      for(int i = 0; i < recordHeader.numberOfEvents; i++){
          auto *ptr = reinterpret_cast<int*>(&recordBuffer[i*4]);
          int size = *ptr;
          if(recordHeader.dataEndianness==1) size = __builtin_bswap32(size);
          eventPosition += size;
          *ptr = eventPosition;
      }
      indexBenchmark.pause();
      return true;
    }

    int   record::getRecordSizeCompressed(){
      return recordHeader.recordLength;
    }

    void  record::readRecord__(std::ifstream &stream, long position, long recordLength){

      stream.seekg(position,std::ios::beg);

      if(static_cast<size_t>(recordLength)>recordCompressedBuffer.size()){
        int newSize = recordLength + 5*1024;
        //printf("---> resizing internal compressed buffer size to from %ld to %d\n",
           //recordCompressedBuffer.size(), newSize);
           //printf("---> after printout\n");
        recordCompressedBuffer.resize(newSize);
      }
      //printf(" trying seeksg\n");
      stream.seekg( position,std::ios::beg);
      //printf(" trying read\n");
      stream.read( (&recordCompressedBuffer[0]), recordLength);
      //printf(" readin was successfull....\n");
      //stream.read( (char *) &recordHeaderBuffer[0],80);
      recordHeader.recordLength    = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_RECORD_LENGTH_OFFSET]));
      recordHeader.headerLength    = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_HEADER_LENGTH_OFFSET]));
      recordHeader.numberOfEvents  = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_EVENT_COUNT_OFFSET]));
      recordHeader.bitInfo         = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_BIT_INFO_OFFSET]));
      recordHeader.signatureString  = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_MAGIC_NUMBER_OFFSET]));
      recordHeader.recordDataLength = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_DATA_LENGTH_OFFSET]));
      recordHeader.userHeaderLength = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_USER_HEADER_LEN_OFFSET]));
      int compressedWord            = *(reinterpret_cast<int *>(&recordCompressedBuffer[RH_COMP_WORD_OFFSET]));

      if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC) recordHeader.dataEndianness = 0;
      if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC_BE) recordHeader.dataEndianness = 1;

      if(static_cast<uint32_t>(recordHeader.signatureString)==HEADER_MAGIC_BE){
        recordHeader.recordLength = __builtin_bswap32(recordHeader.recordLength);
        recordHeader.headerLength = __builtin_bswap32(recordHeader.headerLength);
        recordHeader.numberOfEvents = __builtin_bswap32(recordHeader.numberOfEvents);
        recordHeader.recordDataLength = __builtin_bswap32(recordHeader.recordDataLength);
        recordHeader.userHeaderLength = __builtin_bswap32(recordHeader.userHeaderLength);
        recordHeader.bitInfo          = __builtin_bswap32(recordHeader.bitInfo);
        compressedWord                = __builtin_bswap32(compressedWord);
      }

      int compressedDataLengthPadding = (recordHeader.bitInfo>>BITINFO_PAD3_SHIFT)&BITINFO_PAD_MASK;
      int headerLengthBytes     = recordHeader.headerLength*4;
      int dataBufferLengthBytes = recordHeader.recordLength * 4 - headerLengthBytes;

      recordHeader.userHeaderLengthPadding = (recordHeader.bitInfo>>BITINFO_PAD1_SHIFT)&BITINFO_PAD_MASK;
      recordHeader.recordDataLengthCompressed = compressedWord&COMP_LENGTH_MASK;
      recordHeader.compressionType            = (compressedWord>>COMP_TYPE_SHIFT)&COMP_TYPE_BYTE;
      recordHeader.indexDataLength            = 4*recordHeader.numberOfEvents;

      /*printf(" allocating buffer for record, size = %d, padding = %d length = %d type = %d nevents = %d data length = %d\n",
        dataBufferLengthBytes,
        compressedDataLengthPadding, recordHeader.recordDataLengthCompressed*4,
        recordHeader.compressionType, recordHeader.numberOfEvents, recordHeader.recordDataLength);
        */
      //char *compressedBuffer    = (char*) malloc(dataBufferLengthBytes);


      //dataBufferLengthBytes    -= compressedDataLengthPadding;
      //printf(" record header intialized....\n");
      long  dataposition = position + headerLengthBytes;
      //printf("position = %ld data position = %ld\n",position, dataposition);
      //stream.seekg(dataposition,std::ios::beg);
      //stream.read( compressedBuffer, dataBufferLengthBytes);
      //stream.read( (&recordCompressedBuffer[0]), dataBufferLengthBytes);
      //showBuffer(compressedBuffer, 10, 200);
      //printf("position = %ld data position = %ld \n",position, dataposition);
      int decompressedLength = recordHeader.indexDataLength +
                               recordHeader.userHeaderLength +
                               recordHeader.userHeaderLengthPadding +
                               recordHeader.recordDataLength;
      //printf(" decompressed length = %d\n",decompressedLength);
      if(recordBuffer.size()<static_cast<size_t>(decompressedLength)){
        //printf(" resizing internal buffer from %lu to %d\n", recordBuffer.size(), recordHeader.recordDataLength);
        recordBuffer.resize(decompressedLength+1024);
      }
      //for(int i = 0; i < recordBuffer.size(); i++) recordBuffer[i] = 0;
      //printf("****************** BEFORE padding = %d\n", compressedDataLengthPadding);
      //showBuffer(&recordBuffer[0], 10, 200);
      if(recordHeader.compressionType==0){
        //printf("compression type = 0 data length = %d\n",decompressedLength);
        memcpy((&recordBuffer[0]),(&recordCompressedBuffer[0]),decompressedLength);
      } else {
        //printf(" running deompression %d %d %d\n",dataBufferLengthBytes-compressedDataLengthPadding,decompressedLength,dataposition);
        getUncompressed((&recordCompressedBuffer[RECORD_HEADER_SIZE]) , (&recordBuffer[0]),
            dataBufferLengthBytes-compressedDataLengthPadding,
                 decompressedLength);
      }
      //printf("******************\n");
      //showBuffer(&recordBuffer[0], 10, 200);
      //char *uncompressedBuffer  = getUncompressed(compressedBuffer,dataBufferLengthBytes,recordHeader.recordDataLength);
      //printf(" decompression size = %d  error = %d\n", unc_result,
       //unc_result - decompressedLength);
      //free(compressedBuffer);
      /**
       * converting index array from lengths of each buffer in the
       * record to relative positions in the record stream.
       */
       //printf(" deompression ..... ok \n");
      int eventPosition = dataposition;
      for(int i = 0; i < recordHeader.numberOfEvents; i++){
          auto *ptr = reinterpret_cast<int*>(&recordBuffer[i*4]);
          int size = *ptr;
          if(recordHeader.dataEndianness==1) size = __builtin_bswap32(size);
          eventPosition += size;
          *ptr = eventPosition;
      }
      //printf("final position = %d\n",eventPosition);
    }
    /**
     * returns number of events in the record.
     */
    int   record::getEventCount(){
      return recordHeader.numberOfEvents;
    }
    /**
     * reads content of the event with given index into a vector
     * vector will be resized to fit the data. The resulting
     * size of the vector can be used to veryfy the successfull read.
    */
    void  record::readEvent( std::vector<char> &vec, int index){

    }

    /**
     * returns a data object that points to the event inside of the
     * record. For given index the data object will be filled with the
     * pointer to the position in the buffer where the event starts and
     * with the size indicating length of the event.
    */
    void  record::getData(hipo::data &data, int index){
        int first_position = 0;
        if(index > 0){
          first_position  = *(reinterpret_cast<uint32_t *>(&recordBuffer[(index -1)*4]));
        }
        int last_position = *(reinterpret_cast<uint32_t *>(&recordBuffer[index*4]));
        int offset        = recordHeader.indexDataLength
                          + recordHeader.userHeaderLength
                          + recordHeader.userHeaderLengthPadding;
        data.setDataPtr(&recordBuffer[first_position+offset]);
        data.setDataSize(last_position-first_position);
        data.setDataOffset(first_position + offset);
    }
void   record::read(hipo::bank &b, int event){
   hipo::data event_data;
   getData(event_data,event);
   hipo::event::getStructure(event_data.getDataPtr(),b,b.getSchema().getGroup(),b.getSchema().getItem());
}

void   record::getColumn(hipo::data &data,const char* column, hipo::bank &bank, int event){
  int order = bank.getSchema().getEntryOrder(column);
  getColumn(data,order,bank,event);
}

void   record::getColumn(hipo::data &data,int column, hipo::bank &bank, int event){
    hipo::data event_data;
    getData(event_data,event);
    std::pair<int,int> pos = hipo::event::getStructurePosition(event_data.getDataPtr(),
         bank.getSchema().getGroup(), bank.getSchema().getItem());

    //printf(" returned position = %8d, %8d\n",pos.first,pos.second);

    if(pos.second<=0) {
      data.setDataPtr(NULL);
      data.setDataSize(0);
      data.setDataType(0);
      return;
    }
    //printf("----- hok ---- we are here now ----\n");
    int size = *reinterpret_cast<const uint32_t *>(&event_data.getDataPtr()[pos.first+4]);
    //printf("----- hok ---- one more step ----\n");
    int bankRows = size/bank.getSchema().getRowLength();
    //printf(" returned sizes = %8d, %8d\n",size,bankRows);
    data.setDataSize(bankRows);
    data.setDataType(bank.getSchema().getEntryType(column));

    //printf(" returned type = %d\n",data.getDataType());
    int offset   = bank.getSchema().getOffset(column,0,bankRows);
    //printf("============ offset = %8d\n",offset);
    data.setDataPtr(&(event_data.getDataPtr()[pos.first+offset+8]));
}

void record::getEventsMap(std::vector<std::pair<int,int>> &emap){
   emap.clear();
   int offset        = recordHeader.indexDataLength
                          + recordHeader.userHeaderLength
                          + recordHeader.userHeaderLengthPadding;
                          /*
   printf(" offset for the record = %8d, %8d, %8d, %8d\n",offset,
          recordHeader.indexDataLength
        , recordHeader.userHeaderLength
        , recordHeader.userHeaderLengthPadding
        );*/

   int nevents = recordHeader.numberOfEvents;
   for(int index = 0; index < nevents; index++){
      int first_position = 0;
        if(index > 0){
          first_position  = *(reinterpret_cast<uint32_t *>(&recordBuffer[(index -1)*4]));
        }
        int last_position = *(reinterpret_cast<uint32_t *>(&recordBuffer[index*4]));
        emap.push_back(std::make_pair(first_position+offset,last_position+offset));
   }
}
    void  record::readHipoEvent(hipo::event &event, int index){
          hipo::data event_data;
          getData(event_data,index);
          event.init(event_data.getDataPtr(), event_data.getDataSize());
    }
    /**
     * prints the content of given buffer in HEX format. Used for debugging.
     */
    void  record::showBuffer(const char *data, int wrapping, int maxsize)
    {
      for(int i = 0; i < maxsize; i++){
        printf("%X ", STRUCT_FORMAT_BYTE&((unsigned int) data[i]));
        if( (i+1)%wrapping==0) printf("\n");
      }
      printf("\n");
    }
    /**
     * Decode an Lz4Chunked record's compressed-payload section into recordBuffer.
     *
     * The section sits in recordCompressedBuffer[0..section_bytes]; trailing
     * compressed-data padding has already been stripped by the caller. The
     * output layout in recordBuffer matches what the legacy getData() path
     * expects: [4*event_count bytes of cumulative event end-positions]
     * followed by [concatenated event bytes].
     *
     * Improvement over hipo-rs: each chunk's LZ4_decompress_safe runs in
     * parallel via std::async when num_chunks > 1. Sequential fallback for
     * num_chunks == 1.
     */
    bool record::decodeChunked(int section_bytes){
#ifndef __LZ4__
        printf("record::decodeChunked: LZ4 not available\n");
        return false;
#else
        if (recordHeader.dataEndianness == 1) {
            printf("record::decodeChunked: big-endian Lz4Chunked records are not supported\n");
            return false;
        }

        const int event_count = recordHeader.numberOfEvents;
        if (section_bytes < 8) {
            printf("record::decodeChunked: chunk-table header truncated\n");
            return false;
        }
        const uint32_t num_chunks = *reinterpret_cast<uint32_t*>(&recordCompressedBuffer[0]);
        // events_per_chunk at +4 is informational; per-chunk decompressed sizes drive decode.

        const int event_sizes_off  = 8;
        const int comp_sizes_off   = event_sizes_off  + 4*event_count;
        const int decomp_sizes_off = comp_sizes_off   + 4*static_cast<int>(num_chunks);
        const int payload_off      = decomp_sizes_off + 4*static_cast<int>(num_chunks);

        if (section_bytes < payload_off) {
            printf("record::decodeChunked: chunk-table truncated (sizes)\n");
            return false;
        }

        // Pass 1: total decompressed event bytes from event_sizes table.
        uint32_t total_event_bytes = 0;
        for (int e = 0; e < event_count; ++e) {
            total_event_bytes +=
                *reinterpret_cast<uint32_t*>(&recordCompressedBuffer[event_sizes_off + e*4]);
        }

        // Allocate recordBuffer: [4*event_count cumulative-position table | events]
        const size_t total_bytes = static_cast<size_t>(4*event_count) + total_event_bytes;
        if (recordBuffer.size() < total_bytes) {
            recordBuffer.resize(total_bytes + 1024);
        }

        // Pass 2: write cumulative positions to recordBuffer[0..4*event_count].
        uint32_t cum = 0;
        for (int e = 0; e < event_count; ++e) {
            cum += *reinterpret_cast<uint32_t*>(&recordCompressedBuffer[event_sizes_off + e*4]);
            *reinterpret_cast<uint32_t*>(&recordBuffer[e*4]) = cum;
        }

        // Pass 3: split source & destination spans per chunk for parallel LZ4 decompress.
        struct ChunkJob {
            const char *src;
            char       *dst;
            int         comp;
            int         decomp;
        };
        std::vector<ChunkJob> jobs;
        jobs.reserve(num_chunks);
        int src_off = payload_off;
        int dst_off = 4*event_count;
        for (uint32_t c = 0; c < num_chunks; ++c) {
            uint32_t comp =
                *reinterpret_cast<uint32_t*>(&recordCompressedBuffer[comp_sizes_off   + c*4]);
            uint32_t decomp =
                *reinterpret_cast<uint32_t*>(&recordCompressedBuffer[decomp_sizes_off + c*4]);
            jobs.push_back({&recordCompressedBuffer[src_off], &recordBuffer[dst_off],
                            static_cast<int>(comp), static_cast<int>(decomp)});
            src_off += static_cast<int>(comp);
            dst_off += static_cast<int>(decomp);
        }

        auto inflate = [](const ChunkJob& j) -> int {
            return LZ4_decompress_safe(j.src, j.dst, j.comp, j.decomp);
        };

        if (num_chunks <= 1) {
            for (const auto& j : jobs) {
                int r = inflate(j);
                if (r != j.decomp) {
                    printf("record::decodeChunked: LZ4 chunk decompress failed (got %d, expected %d)\n",
                           r, j.decomp);
                    return false;
                }
            }
        } else {
            // Improvement over hipo-rs: fan out each chunk to std::async. The
            // pool defaults to "deferred or async" — most stdlibs pick async
            // when there's a free thread, deferred otherwise. For small N
            // this is still faster than serial; for large N a real pool wins
            // more, but adding the ThreadPool dep here is out of scope.
            std::vector<std::future<int>> futures;
            futures.reserve(jobs.size());
            for (const auto& j : jobs) {
                futures.push_back(std::async(std::launch::async, inflate, j));
            }
            for (size_t i = 0; i < futures.size(); ++i) {
                int r = futures[i].get();
                if (r != jobs[i].decomp) {
                    printf("record::decodeChunked: LZ4 chunk %zu decompress failed (got %d, expected %d)\n",
                           i, r, jobs[i].decomp);
                    return false;
                }
            }
        }

        // Update header fields that getData() reads. The data section in our
        // layout starts at offset = indexDataLength + userHeaderLength + padding
        // = 4*event_count + 0 + 0.
        recordHeader.indexDataLength         = 4*event_count;
        recordHeader.userHeaderLength        = 0;
        recordHeader.userHeaderLengthPadding = 0;
        recordHeader.recordDataLength        = static_cast<int>(total_event_bytes);
        return true;
#endif
    }

    /**
     * Decode an Lz4ByBank record's compressed-payload section into recordBuffer.
     *
     * Parses the directory through hipo::bybankrecord (which validates layout
     * and copies an owned section), eagerly inflates every bank stream, then
     * reassembles each event's structure list into the legacy
     * [cumulative_event_positions | concatenated_events] layout that getData()
     * expects.
     *
     * NOTE: This v1 inflates every bank stream up-front, sacrificing the
     * lazy-per-bank-decompression win that hipo-rs gets on its read path. A
     * future v2 will surface hipo::bybankrecord directly to callers via a
     * dual-mode hipo::event, matching the Rust impl.
     */
    bool record::decodeByBank(int section_bytes){
        try {
            auto rec = hipo::bybankrecord::parse(recordHeader,
                                                  recordCompressedBuffer.data(),
                                                  section_bytes, /*padding=*/0);
            const int event_count = static_cast<int>(rec->event_count());

            // Pass 1: per event, compute reconstituted size = EVENT_HEADER_SIZE +
            //   Σ (BANK_STRUCTURE_SIZE + bank_size) for every bank present.
            std::vector<uint32_t> event_sizes(event_count, 0);
            uint32_t total_event_bytes = 0;
            const int num_banks = rec->num_banks();
            for (int e = 0; e < event_count; ++e) {
                uint32_t es = EVENT_HEADER_SIZE;
                for (int b = 0; b < num_banks; ++b) {
                    if (rec->has(e, b)) {
                        es += BANK_STRUCTURE_SIZE + rec->bank_size(e, b);
                    }
                }
                event_sizes[e]    = es;
                total_event_bytes += es;
            }

            // Allocate recordBuffer for [4*event_count cumulative-position table | events].
            const size_t total_bytes = static_cast<size_t>(4*event_count) + total_event_bytes;
            if (recordBuffer.size() < total_bytes) recordBuffer.resize(total_bytes + 1024);

            // Pass 2: write cumulative positions + reassemble each event in place.
            uint32_t cum = 0;
            uint32_t evt_write_off = 4 * event_count;
            for (int e = 0; e < event_count; ++e) {
                cum += event_sizes[e];
                std::memcpy(&recordBuffer[e*4], &cum, 4);

                // Event header — "EVNT\0" magic + size + tag + reserved.
                char* evt = &recordBuffer[evt_write_off];
                evt[0] = 'E'; evt[1] = 'V'; evt[2] = 'N'; evt[3] = 'T';
                std::memcpy(evt + EH_SIZE_OFFSET, &event_sizes[e], 4);
                uint32_t tag = rec->event_tag(e);
                std::memcpy(evt + EH_TAG_OFFSET, &tag, 4);
                uint32_t zero = 0;
                std::memcpy(evt + EH_RESERVED_OFFSET, &zero, 4);

                // Walk banks in sorted directory order; emit structures.
                uint32_t struct_off = EVENT_HEADER_SIZE;
                for (int b = 0; b < num_banks; ++b) {
                    if (!rec->has(e, b)) continue;
                    auto desc      = rec->descriptor(b);
                    uint32_t blen  = rec->bank_size(e, b);
                    char* hdr = evt + struct_off;
                    std::memcpy(hdr + 0, &desc.group, 2);
                    hdr[2] = static_cast<char>(desc.item);
                    hdr[3] = static_cast<char>(desc.data_type);
                    // size_word: low 24 bits = data length, upper 8 = 0 (bank type)
                    uint32_t size_word = blen & STRUCT_SIZE_MASK;
                    std::memcpy(hdr + 4, &size_word, 4);

                    if (blen > 0) {
                        const auto& stream = rec->bank_stream(b);
                        auto range = rec->bank_byte_range(e, b);
                        std::memcpy(hdr + BANK_STRUCTURE_SIZE,
                                    stream.data() + range.first, blen);
                    }
                    struct_off += BANK_STRUCTURE_SIZE + blen;
                }

                evt_write_off += event_sizes[e];
            }

            recordHeader.indexDataLength         = 4 * event_count;
            recordHeader.userHeaderLength        = 0;
            recordHeader.userHeaderLengthPadding = 0;
            recordHeader.recordDataLength        = static_cast<int>(total_event_bytes);
            return true;
        } catch (const std::exception& e) {
            printf("record::decodeByBank: %s\n", e.what());
            return false;
        }
    }

    /**
     * decompresses the buffer given with pointed *data, into a destination array
     * provided. The arguments indicate the compressed data length (dataLength),
     * and maximum decompressed length.
     * returns the number of bytes that were decompressed by LZ4
     */
    int  record::getUncompressed(const char *data,  char *dest, int dataLength, int dataLengthUncompressed){
      #ifdef __LZ4__
        int result = LZ4_decompress_safe(data,dest,dataLength,dataLengthUncompressed);
        //int result = LZ4_decompress_fast(data,dest,dataLengthUncompressed);
        return result;
        #endif

        #ifndef __LZ4__
          printf("\n   >>>>> LZ4 compression is not supported.");
          printf("\n   >>>>> check if libz4 is installed on your system.");
          printf("\n   >>>>> recompile the library with liblz4 installed.\n");
          return NULL;
        #endif

    }
    /**
     * deompresses the content of given buffer ( *data), into a newly allocated
     * memory. User is responsible for free-ing the allocated memory.
     */
    char *record::getUncompressed(const char *data, int dataLength,
                                  int dataLengthUncompressed){

      #ifdef __LZ4__
        char *output = (char *) malloc(dataLengthUncompressed);
        LZ4_decompress_safe(data,output,dataLength,dataLengthUncompressed);
        return output;
        //printf(" FIRST (%d) = %x %x %x %x\n",result);//,destUnCompressed[0],destUnCompressed[1],
        //destUnCompressed[2],destUnCompressed[3]);
        //LZ4_decompress_fast(buffer,destUnCompressed,decompressedLength);
        //LZ4_uncompress(buffer,destUnCompressed,decompressedLength);
        #endif

        #ifndef __LZ4__
          printf("\n   >>>>> LZ4 compression is not supported.");
          printf("\n   >>>>> check if libz4 is installed on your system.");
          printf("\n   >>>>> recompile the library with liblz4 installed.\n");
          return nullptr;
        #endif

    }



dataframe::dataframe(int max_events, int max_size){
   maxEvents = max_events; maxSize = max_size;
   dataBuffer.resize(RECORD_HEADER_SIZE+maxSize);
   reset();
}

void dataframe::reset(){
   *(reinterpret_cast<int *>(&dataBuffer[RH_RECORD_LENGTH_OFFSET]))   = RECORD_HEADER_SIZE; // record length 1
   *(reinterpret_cast<int *>(&dataBuffer[RH_RECORD_NUMBER_OFFSET]))  = 1; // record number 2
   *(reinterpret_cast<int *>(&dataBuffer[RH_HEADER_LENGTH_OFFSET]))  = RECORD_HEADER_WORDS; // header length words 3
   *(reinterpret_cast<int *>(&dataBuffer[RH_EVENT_COUNT_OFFSET]))    = 0; // event count 4
   *(reinterpret_cast<int *>(&dataBuffer[RH_INDEX_ARRAY_LEN_OFFSET]))= 0; // index array length 5
   *(reinterpret_cast<int *>(&dataBuffer[RH_BIT_INFO_OFFSET]))       = 5; // verion + bitinfo 6
   *(reinterpret_cast<int *>(&dataBuffer[RH_USER_HEADER_LEN_OFFSET]))= 0; // user header length
   *(reinterpret_cast<int *>(&dataBuffer[RH_MAGIC_NUMBER_OFFSET]))   = HEADER_MAGIC;
   *(reinterpret_cast<int *>(&dataBuffer[RH_DATA_LENGTH_OFFSET]))    = 0; // uncompressed data length
   *(reinterpret_cast<int *>(&dataBuffer[RH_COMP_WORD_OFFSET]))      = 0; // compressed data length
}

bool         dataframe::addEvent(hipo::event &event){ 
  int fsize = size();
  if( (event.getSize() + fsize + RECORD_HEADER_SIZE ) > maxSize) return false;
  int evCount = count();
  if(evCount>=maxEvents) return false;
  memcpy(&dataBuffer[fsize],&(event.getEventBuffer()[0]), event.getSize());
  int datasize = *(reinterpret_cast<int *>(&dataBuffer[RH_DATA_LENGTH_OFFSET]));
  *(reinterpret_cast<int *>(&dataBuffer[RH_EVENT_COUNT_OFFSET])) = evCount + 1;
  *(reinterpret_cast<int *>(&dataBuffer[RH_DATA_LENGTH_OFFSET])) = datasize + event.getSize();
  *(reinterpret_cast<int *>(&dataBuffer[RH_COMP_WORD_OFFSET])) = datasize + event.getSize();
  *(reinterpret_cast<int *>(&dataBuffer[RH_RECORD_LENGTH_OFFSET]))  = fsize + event.getSize();
  return true;
}

int          dataframe::getEventAt(int pos, hipo::event &event){ 
    int eventsize = *(reinterpret_cast<int *>(&dataBuffer[pos+4]));
    event.init(&dataBuffer[pos],eventsize);
    return pos+eventsize;
}

void          dataframe::init(const char *ptr){
    int size = *(reinterpret_cast<const int *>(ptr));
    if(static_cast<size_t>(size)>dataBuffer.size()) dataBuffer.resize(size+RECORD_HEADER_SIZE);
    memcpy(&dataBuffer[0],ptr,size);
}

int          dataframe::count(){ return *(reinterpret_cast<int *>(&dataBuffer[RH_EVENT_COUNT_OFFSET]));}
int          dataframe::size(){ return  (*(reinterpret_cast<int *>(&dataBuffer[RH_RECORD_LENGTH_OFFSET])));}

void         dataframe::summary(){
  printf("## data frame summary:\n");
  printf("frame size       : %d\n",*(reinterpret_cast<int *>(&dataBuffer[RH_RECORD_LENGTH_OFFSET])));
  printf("magic word       : %X\n",*(reinterpret_cast<int *>(&dataBuffer[RH_MAGIC_NUMBER_OFFSET])));
  printf("number of events : %d\n",*(reinterpret_cast<int *>(&dataBuffer[RH_EVENT_COUNT_OFFSET])));
  printf("data length      : %d\n",*(reinterpret_cast<int *>(&dataBuffer[RH_DATA_LENGTH_OFFSET]))); 
}

}
