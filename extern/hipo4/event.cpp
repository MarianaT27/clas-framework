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
  * File:   event.h
  * Author: gavalian
  *
  * Created on April 12, 2017, 10:14 AM
  */

#include "event.h"
#include "bybankrecord.h"
#include "bypagerecord.h"
#include "constants.h"

namespace hipo {


    event::event(){
        #if __cplusplus > 199711L
            //printf("\n*****>>>>> compiled with c++11 support.\n");
        #endif
        // default allocation size for the event is 128 Kb
        dataBuffer.resize(128*1024);
        reset();
    }

    event::event(int size){
        dataBuffer.resize(size);
        reset();
    }

    // Out-of-line so std::shared_ptr<bybankrecord> sees the full type
    // (event.h only forward-declares bybankrecord to break the include cycle).
    event::event(const event&) = default;
    event::event(event&&) noexcept = default;
    event& event::operator=(const event&) = default;
    event& event::operator=(event&&) noexcept = default;
    event::~event()= default;

    void event::initByBank(std::shared_ptr<hipo::bybankrecord> rec, uint32_t event_idx) {
        bbRecord_     = std::move(rec);
        bbEventIdx_   = event_idx;
        bbSynthDone_  = false;
        // Clear ByPage state — only one alternate mode at a time.
        bpRecord_.reset();
        bpSynthDone_ = false;
        // Keep dataBuffer allocation around so reset() / future Bytes-mode
        // events don't reallocate. The bytes inside become stale until the
        // synthesizer runs or the event is re-initialized.
    }

    void event::initByPage(std::shared_ptr<hipo::bypagerecord> rec, uint32_t event_idx) {
        bpRecord_     = std::move(rec);
        bpEventIdx_   = event_idx;
        bpSynthDone_  = false;
        // Clear ByBank state — only one alternate mode at a time.
        bbRecord_.reset();
        bbSynthDone_ = false;
    }

    void event::ensureBytesMode() {
        if (bbRecord_) {
            synthesizeFromByBank();
            bbRecord_.reset();
            bbSynthDone_ = false;
        } else if (bpRecord_) {
            synthesizeFromByPage();
            bpRecord_.reset();
            bpSynthDone_ = false;
        }
    }

    // Reassemble the event's full byte representation into dataBuffer
    // from the bybankrecord's lazy bank streams. Paid only once per
    // event, when a caller asks for raw bytes (getEventBuffer, getSize,
    // show, ...). The bytes-walking methods then continue with the
    // existing Bytes-mode logic.
    void event::synthesizeFromByBank() {
        if (bbSynthDone_ || !bbRecord_) return;
        const int num_banks = bbRecord_->num_banks();

        uint32_t total_struct_bytes = 0;
        for (int b = 0; b < num_banks; ++b) {
            if (bbRecord_->has(bbEventIdx_, b)) {
                total_struct_bytes += BANK_STRUCTURE_SIZE + bbRecord_->bank_size(bbEventIdx_, b);
            }
        }
        const uint32_t event_size = EVENT_HEADER_SIZE + total_struct_bytes;

        if (dataBuffer.size() < event_size) dataBuffer.resize(event_size + 1024);

        // Event header
        dataBuffer[0] = 'E'; dataBuffer[1] = 'V';
        dataBuffer[2] = 'N'; dataBuffer[3] = 'T';
        std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &event_size, sizeof(event_size));
        uint32_t tag = bbRecord_->event_tag(bbEventIdx_);
        std::memcpy(&dataBuffer[EH_TAG_OFFSET], &tag, sizeof(tag));
        uint32_t zero = 0;
        std::memcpy(&dataBuffer[EH_RESERVED_OFFSET], &zero, sizeof(zero));

        // Structures in sorted (group, item) order.
        uint32_t pos = EVENT_HEADER_SIZE;
        for (int b = 0; b < num_banks; ++b) {
            if (!bbRecord_->has(bbEventIdx_, b)) continue;
            auto desc      = bbRecord_->descriptor(b);
            uint32_t blen  = bbRecord_->bank_size(bbEventIdx_, b);
            std::memcpy(&dataBuffer[pos + 0], &desc.group, 2);
            dataBuffer[pos + 2] = static_cast<char>(desc.item);
            dataBuffer[pos + 3] = static_cast<char>(desc.data_type);
            uint32_t size_word = blen & STRUCT_SIZE_MASK;
            std::memcpy(&dataBuffer[pos + 4], &size_word, sizeof(size_word));
            if (blen > 0) {
                const auto& stream = bbRecord_->bank_stream(b);
                auto range = bbRecord_->bank_byte_range(bbEventIdx_, b);
                std::memcpy(&dataBuffer[pos + BANK_STRUCTURE_SIZE],
                            stream.data() + range.first, blen);
            }
            pos += BANK_STRUCTURE_SIZE + blen;
        }
        bbSynthDone_ = true;
    }

    void event::synthesizeFromByPage() {
        if (bpSynthDone_ || !bpRecord_) return;
        const int num_banks = bpRecord_->num_banks();

        // First pass: total bytes (event header + each present bank's
        // 8-byte structure header + bank data).
        uint32_t total_struct_bytes = 0;
        for (int b = 0; b < num_banks; ++b) {
            if (!bpRecord_->has(bpEventIdx_, b)) continue;
            const uint32_t R = bpRecord_->event_row_count(bpEventIdx_, b);
            uint32_t row_size = 0;
            const int ncols = bpRecord_->num_columns(b);
            for (int c = 0; c < ncols; ++c) row_size += bpRecord_->column(b, c).bytes_per_row;
            total_struct_bytes += BANK_STRUCTURE_SIZE + R * row_size;
        }
        const uint32_t event_size = EVENT_HEADER_SIZE + total_struct_bytes;
        if (dataBuffer.size() < event_size) dataBuffer.resize(event_size + 1024);

        dataBuffer[0] = 'E'; dataBuffer[1] = 'V';
        dataBuffer[2] = 'N'; dataBuffer[3] = 'T';
        std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &event_size, sizeof(event_size));
        uint32_t tag = 0;  // ByPage doesn't track per-event tags (Lz4ByBank does, but the
                            // writer here doesn't surface it; safe to write 0).
        std::memcpy(&dataBuffer[EH_TAG_OFFSET], &tag, sizeof(tag));
        uint32_t zero = 0;
        std::memcpy(&dataBuffer[EH_RESERVED_OFFSET], &zero, sizeof(zero));

        uint32_t pos = EVENT_HEADER_SIZE;
        for (int b = 0; b < num_banks; ++b) {
            if (!bpRecord_->has(bpEventIdx_, b)) continue;
            const auto desc   = bpRecord_->descriptor(b);
            const uint32_t R  = bpRecord_->event_row_count(bpEventIdx_, b);
            const int ncols   = bpRecord_->num_columns(b);
            uint32_t row_size = 0;
            for (int c = 0; c < ncols; ++c) row_size += bpRecord_->column(b, c).bytes_per_row;
            const uint32_t bank_data_size = R * row_size;
            // Structure header.
            std::memcpy(&dataBuffer[pos + 0], &desc.group, 2);
            dataBuffer[pos + 2] = static_cast<char>(desc.item);
            dataBuffer[pos + 3] = static_cast<char>(desc.data_type);
            uint32_t size_word = bank_data_size & STRUCT_SIZE_MASK;
            std::memcpy(&dataBuffer[pos + 4], &size_word, sizeof(size_word));
            // Per-column copy.
            uint32_t col_off = 0;
            for (int c = 0; c < ncols; ++c) {
                const auto ci = bpRecord_->column(b, c);
                if (R > 0) {
                    auto v = bpRecord_->column_for_event(bpEventIdx_, b, c);  // lazy inflate
                    std::memcpy(&dataBuffer[pos + BANK_STRUCTURE_SIZE + R * col_off],
                                v.data, v.byte_len);
                }
                col_off += ci.bytes_per_row;
            }
            pos += BANK_STRUCTURE_SIZE + bank_data_size;
        }
        bpSynthDone_ = true;
    }

    // Phase 4 Path B: ByPage-only zero-copy column view. Returns the
    // page's decompressed bytes reinterpreted as T*, plus the element
    // count (= row_count × array_length, so a `cov/F#6` column for an
    // event with 4 rows returns 24). Caller is responsible for matching
    // T to the column's actual type. In non-ByPage modes returns
    // {nullptr, 0} — callers should fall back to the bank.getXxx loop.
    template<typename T>
    std::pair<const T*, int> event::getColumnView(int group, int item, int col_idx) {
        if (!bpRecord_) return {nullptr, 0};
        const int bidx = bpRecord_->bank_index(static_cast<uint16_t>(group),
                                               static_cast<uint8_t>(item));
        if (bidx < 0 || !bpRecord_->has(bpEventIdx_, bidx)) return {nullptr, 0};
        auto v = bpRecord_->column_for_event(bpEventIdx_, bidx, col_idx);
        const auto ci = bpRecord_->column(bidx, col_idx);
        const int element_count = static_cast<int>(v.row_count) * ci.array_length;
        return {reinterpret_cast<const T*>(v.data), element_count};
    }
    // Explicit instantiations for the HIPO scalar types. The template body
    // lives in the .cpp (it touches bypagerecord) so callers don't need
    // bypagerecord.h to use it.
    template std::pair<const int8_t*,  int> event::getColumnView<int8_t>(int, int, int);
    template std::pair<const int16_t*, int> event::getColumnView<int16_t>(int, int, int);
    template std::pair<const int32_t*, int> event::getColumnView<int32_t>(int, int, int);
    template std::pair<const int64_t*, int> event::getColumnView<int64_t>(int, int, int);
    template std::pair<const float*,   int> event::getColumnView<float>(int, int, int);
    template std::pair<const double*,  int> event::getColumnView<double>(int, int, int);

    void   event::getStructure(hipo::bank &b){
        // ByPage fast path (default now): set up the bank's lazy
        // per-column binding instead of pre-inflating all columns.
        // bank.getXxx() calls trigger ensureColumn() for each touched
        // column — analyses that only read a few columns out of many
        // inflate only those columns' page streams.
        if(bpRecord_){
          const hipo::schema& sch = b.getSchema();
          const int bidx = bpRecord_->bank_index(
              static_cast<uint16_t>(sch.getGroup()),
              static_cast<uint8_t>(sch.getItem()));
          if(bidx < 0 || !bpRecord_->has(bpEventIdx_, bidx)){
            b.initStructureBySize(sch.getGroup(), sch.getItem(), 11, 0);
            b.notify();
            return;
          }
          const uint32_t R = bpRecord_->event_row_count(bpEventIdx_, bidx);
          uint32_t row_size = 0;
          const int ncols = bpRecord_->num_columns(bidx);
          for(int c = 0; c < ncols; ++c)
            row_size += bpRecord_->column(bidx, c).bytes_per_row;
          const uint32_t bank_data_size = R * row_size;
          // Allocate full structure buffer and write the structure header.
          // Column data is left uninitialized — ensureColumn() fills each
          // column lazily on first touch.
          b.initStructureBySize(sch.getGroup(), sch.getItem(), 11,
                                static_cast<int>(bank_data_size));
          b.notify();  // notify() clears any prior ByPage binding
          b.setByPageRef(bpRecord_.get(), bpEventIdx_, bidx);
          return;
        }
        getStructure(b,b.getSchema().getGroup(),b.getSchema().getItem());
    }

    void   event::read(hipo::bank &b){
        getStructure(b);
    }


    void event::getStructure(hipo::structure &str, int group, int item){
       // Phase 4 ByPage path A: reassemble the requested bank from its
       // per-column streams. Inflates ALL columns of THIS bank only (other
       // banks stay compressed). Callers that need partial-column reads
       // should use event::getColumnView<T> instead — it stays at
       // per-column granularity.
       if(bpRecord_){
         int bidx = bpRecord_->bank_index(static_cast<uint16_t>(group),
                                          static_cast<uint8_t>(item));
         if(bidx < 0 || !bpRecord_->has(bpEventIdx_, bidx)){
           str.initStructureBySize(group, item, 1, 0);
           str.notify();
           return;
         }
         auto desc       = bpRecord_->descriptor(bidx);
         const uint32_t R = bpRecord_->event_row_count(bpEventIdx_, bidx);
         const int ncols  = bpRecord_->num_columns(bidx);
         uint32_t row_size = 0;
         for(int c = 0; c < ncols; ++c) row_size += bpRecord_->column(bidx, c).bytes_per_row;
         const uint32_t bank_data_size = R * row_size;
         const size_t need = BANK_STRUCTURE_SIZE + bank_data_size;
         if(bbScratch_.size() < need) bbScratch_.resize(need);
         std::memcpy(&bbScratch_[0], &desc.group, 2);
         bbScratch_[2] = static_cast<char>(desc.item);
         bbScratch_[3] = static_cast<char>(desc.data_type);
         uint32_t size_word = bank_data_size & STRUCT_SIZE_MASK;
         std::memcpy(&bbScratch_[4], &size_word, sizeof(size_word));
         uint32_t col_off = 0;
         for(int c = 0; c < ncols; ++c){
           const auto ci = bpRecord_->column(bidx, c);
           if(R > 0){
             auto v = bpRecord_->column_for_event(bpEventIdx_, bidx, c); // lazy inflate
             std::memcpy(&bbScratch_[BANK_STRUCTURE_SIZE + R * col_off],
                         v.data, v.byte_len);
           }
           col_off += ci.bytes_per_row;
         }
         str.init(bbScratch_.data(), static_cast<int>(need));
         str.notify();
         return;
       }
       // Phase 3 v2 hot path: when this event was produced from an
       // Lz4ByBank record, bypass dataBuffer entirely and reach into the
       // shared bybankrecord. The (group, item) lookup, presence check
       // and bank_stream() inflate all hit the lazy-cached state — banks
       // the analysis never asks for stay compressed for the record's
       // lifetime.
       if(bbRecord_){
         int bidx = bbRecord_->bank_index(static_cast<uint16_t>(group),
                                          static_cast<uint8_t>(item));
         if(bidx < 0 || !bbRecord_->has(bbEventIdx_, bidx)){
           str.initStructureBySize(group,item,1,0);
           str.notify();
           return;
         }
         auto desc       = bbRecord_->descriptor(bidx);
         uint32_t blen   = bbRecord_->bank_size(bbEventIdx_, bidx);
         size_t need     = BANK_STRUCTURE_SIZE + blen;
         if(bbScratch_.size() < need) bbScratch_.resize(need);
         // Synthesize [8-byte header | data bytes] into the reusable
         // scratch then hand it to structure::init (which copies into
         // the structure's own buffer — bbScratch_ can be overwritten
         // on the next call without invalidating the structure).
         std::memcpy(&bbScratch_[0], &desc.group, 2);
         bbScratch_[2] = static_cast<char>(desc.item);
         bbScratch_[3] = static_cast<char>(desc.data_type);
         uint32_t size_word = blen & STRUCT_SIZE_MASK;
         std::memcpy(&bbScratch_[4], &size_word, sizeof(size_word));
         if(blen > 0){
           const auto& stream = bbRecord_->bank_stream(bidx); // lazy inflate
           auto range = bbRecord_->bank_byte_range(bbEventIdx_, bidx);
           std::memcpy(&bbScratch_[BANK_STRUCTURE_SIZE],
                       stream.data() + range.first, blen);
         }
         str.init(bbScratch_.data(), static_cast<int>(need));
         str.notify();
         return;
       }
       std::pair<int,int> index = getStructurePosition(group,item);
       if(index.first>0){
         str.init(&dataBuffer[index.first], index.second + BANK_STRUCTURE_SIZE);
         str.notify();
       } else {
         str.initStructureBySize(group,item,1,0);
         str.notify();
         //printf("*** error *** : structure (%5d,%5d) does not exist\n", group,item);
       }
    }

    void event::getStructure4(hipo::structure &str, int group, int item){
       if(bbRecord_){
         // Same lazy ByBank dispatch as getStructure(...).
         getStructure(str, group, item);
         return;
       }
       std::pair<int,int> index = getStructurePosition4(group,item);
       //printf("\n\n>>> GET STRUCTURE %d %d -> %d %d \n",group,item, index.first, index.second);
       if(index.first>0){
         str.init(&dataBuffer[index.first], index.second + BANK_STRUCTURE_SIZE);
         str.notify();
       } else {
         str.initStructureBySize(group,item,1,0);
         str.notify();
         //printf("*** error *** : structure (%5d,%5d) does not exist\n", group,item);
       }
    }

    void     event::getStructureNoCopy(const char *buffer, hipo::structure &str, int group, int item){
       std::pair<int,int> index = getStructurePosition(buffer,group,item);
       if(index.first>0){
         str.initNoCopy(&buffer[index.first], index.second + BANK_STRUCTURE_SIZE);
         str.notify();
       } else {
         str.initStructureBySize(group,item,1,0);
         str.notify();
         //printf("*** error *** : structure (%5d,%5d) does not exist\n", group,item);
       }
    }
    void     event::getStructure(const char *buffer, hipo::structure &str, int group, int item){
       std::pair<int,int> index = getStructurePosition(buffer,group,item);
       if(index.first>0){
         str.init(&buffer[index.first], index.second + BANK_STRUCTURE_SIZE);
         str.notify();
       } else {
         str.initStructureBySize(group,item,1,0);
         str.notify();
         //printf("*** error *** : structure (%5d,%5d) does not exist\n", group,item);
       }
    }
    void   event::override(hipo::structure &str){

    }

    void   event::replace(hipo::bank &bank){
      ensureBytesMode();
      std::pair<int,int> index = getStructurePosition(&dataBuffer[0],
            bank.getSchema().getGroup(),bank.getSchema().getItem());
      if(index.first>0){
          int oSize =   index.second + BANK_STRUCTURE_SIZE;
          int sSize = bank.getSize() + BANK_STRUCTURE_SIZE;
          if(oSize==sSize){
             std::memcpy(&dataBuffer[index.first],&(bank.getStructureBuffer()[0]),sSize);
          } else {
            printf("[event::replace] error in replacing the bank %s\n",bank.getSchema().getName().c_str());
          }
      }       
    }
    void   event::remove(hipo::bank &str){
       remove(str.getSchema().getGroup(),str.getSchema().getItem());
    }

    void   event::remove(int group, int item){
       ensureBytesMode();
       //int str_size = str.getStructureBufferSize();
       //int data_size = str.getSize();
       std::pair<int,int> index = getStructurePosition(&dataBuffer[0],group,item);
       printf("-removing %d, %d\n",index.first,index.second);
       if(index.first>0){
	        int eventSize = getSize();
	        int    toCopy = eventSize - index.first - (index.second + BANK_STRUCTURE_SIZE);
	        int   newSize = eventSize - (index.second + BANK_STRUCTURE_SIZE);
	        std::memmove(&dataBuffer[index.first],&dataBuffer[index.first+BANK_STRUCTURE_SIZE+index.second],toCopy);
          printf(" first = %d, second = %d\n",index.first,index.second);
          printf(" \t memcopy = %d, %d, ncopy %d , size = %d , new size = %d\n",
          index.first, index.first+BANK_STRUCTURE_SIZE+index.second, toCopy, eventSize, newSize
          );
	          uint32_t newSize_u = static_cast<uint32_t>(newSize);
	          std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &newSize_u, sizeof(newSize_u));
       }
       //printf("size = %d, new size = %d, structure removed size = %d, position = %d, to copy = %d\n",
       //   eventSize, newSize, index.second +8, index.first, toCopy );
    }

    void    event::addStructure(hipo::structure &str){
        ensureBytesMode();
        int str_size = str.getStructureBufferSize();
        int data_size = str.getSize();
        int evt_size = getSize();
	      int evt_capacity = dataBuffer.size();

        //if(dataBuffer.size()<= () ){
        //  dataBuffer.resize(size+1024);
        //}
        if(data_size>0){
          //printf(" writing structure %d/%d - size %d, data size %d\n",str.getGroup(),str.getItem(),str_size,data_size);
	        if((evt_size + str_size)<evt_capacity){
	           memcpy(&dataBuffer[evt_size], &str.getStructureBuffer()[0],str_size);
	            uint32_t new_size = static_cast<uint32_t>(evt_size + str_size);
              std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &new_size, sizeof(new_size));
	          } else {
	              printf("event::add : error adding structure with size = %5d (capacity = %5d, size = %5d)\n",
		              str_size,evt_capacity, evt_size);
	          }
        }
    }

  void   event::add(hipo::node &_n){
      ensureBytesMode();
      int _n_data_length = _n.dataLength();
      if(_n_data_length==0) return;

      int ev_size = getSize();
      int _n_size = _n.size() + BANK_STRUCTURE_SIZE;
      int ev_capacity = dataBuffer.size();
      if((ev_size + _n_size)<ev_capacity){
        memcpy(&dataBuffer[ev_size], _n.pointer(),_n_size);
        uint32_t new_size = static_cast<uint32_t>(ev_size + _n_size);
        std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &new_size, sizeof(new_size));
      } else {
        printf("event::add : error adding structure with size = %5d (capacity = %5d, size = %5d)\n",
          _n_size,ev_capacity, ev_size);
      }
  }
  void   event::write(hipo::node &node){
      add(node);
  }

  void   event::read(hipo::node  &node, int group, int item){

  }

void event::get(hipo::node &_n, int group, int item){
       std::pair<int,int> index = getStructurePosition(group,item);
       printf(" found the structure : %d/%d - properties = %d, %d\n",group,item,index.first,index.second);
       if(index.first>0){
         _n.init(&dataBuffer[index.first], index.second + BANK_STRUCTURE_SIZE);
         printf("calling notify.....\n");
         _n.notify();
       } else {
         _n.initEmpty();
         _n.notify();
         //printf("*** error *** : structure (%5d,%5d) does not exist\n", group,item);
       }
    }

    int  event::getTag(){
      // ByBank-mode fast path: pull tag straight from the descriptor table
      // without synthesizing the full event bytes.
      if(bbRecord_ && !bbSynthDone_) return static_cast<int>(bbRecord_->event_tag(bbEventIdx_));
      // ByPage doesn't store per-event tags in the writer; always 0 until
      // synthesized (where we also write 0).
      if(bpRecord_ && !bpSynthDone_) return 0;
      uint32_t eventTag_u;
      std::memcpy(&eventTag_u, &dataBuffer[EH_TAG_OFFSET], sizeof(eventTag_u));
      return static_cast<int>(eventTag_u);
    }

    void  event::setTag(int tag){
      // setTag mutates: promote ByBank/ByPage → Bytes before writing.
      ensureBytesMode();
      uint32_t tag_u = static_cast<uint32_t>(tag);
      std::memcpy(&dataBuffer[EH_TAG_OFFSET], &tag_u, sizeof(tag_u));
    }

    void event::init(std::vector<char> &buffer){
        // init() puts the event back in Bytes mode.
        bbRecord_.reset(); bbSynthDone_ = false;
        bpRecord_.reset(); bpSynthDone_ = false;
        dataBuffer.resize(buffer.size());
        std::memcpy(&dataBuffer[0],&buffer[0],buffer.size());
    }

    std::pair<int,int>  event::getStructurePosition(const char *buffer, int group, int item){
      int position = EVENT_HEADER_SIZE;
      uint32_t eventSize_u;
      std::memcpy(&eventSize_u, &buffer[EH_SIZE_OFFSET], sizeof(eventSize_u));
      int eventSize = static_cast<int>(eventSize_u);
      while(position+BANK_STRUCTURE_SIZE<eventSize){
          uint16_t   gid;
          std::memcpy(&gid, &buffer[position], sizeof(gid));
          uint8_t    iid = static_cast<uint8_t>(buffer[position+2]);
          int     length;
          std::memcpy(&length, &buffer[position+4], sizeof(length));
          if(gid==group&&iid==item) return std::make_pair(position,length);
          position += (length + BANK_STRUCTURE_SIZE);
      }
      return std::make_pair(-1,0);
    }

    std::pair<int,int>  event::getStructurePosition(int group, int item){
      // ByBank/ByPage events have no native byte position; synthesize so
      // the legacy walker can find the structure.
      if(bbRecord_ && !bbSynthDone_) synthesizeFromByBank();
      if(bpRecord_ && !bpSynthDone_) synthesizeFromByPage();
      int position = EVENT_HEADER_SIZE;
      uint32_t eventSize_u;
      std::memcpy(&eventSize_u, &dataBuffer[EH_SIZE_OFFSET], sizeof(eventSize_u));
      int eventSize = static_cast<int>(eventSize_u);
      while(position+BANK_STRUCTURE_SIZE<eventSize){
          uint16_t   gid;
          std::memcpy(&gid, &dataBuffer[position], sizeof(gid));
          uint8_t    iid = static_cast<uint8_t>(dataBuffer[position+2]);
          int       word;
          std::memcpy(&word, &dataBuffer[position+4], sizeof(word));
          int length = word&STRUCT_SIZE_MASK;
          if(gid==group&&iid==item) return std::make_pair(position,length);
          position += (length + BANK_STRUCTURE_SIZE);
      }
      return std::make_pair(-1,0);
    }

    std::pair<int,int>  event::getStructurePosition4(int group, int item){
      if(bbRecord_ && !bbSynthDone_) synthesizeFromByBank();
      if(bpRecord_ && !bpSynthDone_) synthesizeFromByPage();
      int position = EVENT_HEADER_SIZE;
      uint32_t eventSize_u;
      std::memcpy(&eventSize_u, &dataBuffer[EH_SIZE_OFFSET], sizeof(eventSize_u));
      int eventSize = static_cast<int>(eventSize_u);
      while(position+BANK_STRUCTURE_SIZE<eventSize){
          uint16_t   gid;
          std::memcpy(&gid, &dataBuffer[position], sizeof(gid));
          uint8_t    iid = static_cast<uint8_t>(dataBuffer[position+2]);
          int     length;
          std::memcpy(&length, &dataBuffer[position+4], sizeof(length));
          if(gid==group&&iid==item) return std::make_pair(position,length);
          position += (length + BANK_STRUCTURE_SIZE);
      }
      return std::make_pair(-1,0);
    }
/*
    std::pair<int,int>  event::getStructurePosition(const char *buffer, int group, int item){
      int position = 16;
      int eventSize = *(reinterpret_cast<uint32_t*>(&buffer[4]));
      while(position+8<eventSize){
          uint16_t   gid = *(reinterpret_cast<uint16_t*>(&buffer[position]));
          uint8_t    iid = *(reinterpret_cast<uint8_t*>(&buffer[position+2]));
          uint8_t   type = *(reinterpret_cast<uint8_t*>(&buffer[position+3]));
          int     length = *(reinterpret_cast<int*>(&buffer[position+4]));
          //printf("group = %4d , item = %4d\n",(unsigned int) gid, (unsigned int) iid);
          if(gid==group&&iid==item) return std::make_pair(position,length);
          position += (length + 8);
      }
      return std::make_pair(-1,0);
    }
*/
    void event::init(const char *buffer, int size){
       // init() puts the event back in Bytes mode.
       bbRecord_.reset(); bbSynthDone_ = false;
       bpRecord_.reset(); bpSynthDone_ = false;
       if(dataBuffer.size()<=static_cast<size_t>(size)){
         dataBuffer.resize(size+1024);
       }
       std::memcpy(&dataBuffer[0],buffer,size);
       uint32_t size_u = static_cast<uint32_t>(size);
       std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &size_u, sizeof(size_u));
    }

    int event::getSize(){
      if(bbRecord_ && !bbSynthDone_){
        uint32_t total = EVENT_HEADER_SIZE;
        const int B = bbRecord_->num_banks();
        for(int b = 0; b < B; ++b){
          if(bbRecord_->has(bbEventIdx_, b)){
            total += BANK_STRUCTURE_SIZE + bbRecord_->bank_size(bbEventIdx_, b);
          }
        }
        return static_cast<int>(total);
      }
      if(bpRecord_ && !bpSynthDone_){
        // Same idea: sum bank structure header + R * row_size per present bank.
        uint32_t total = EVENT_HEADER_SIZE;
        const int B = bpRecord_->num_banks();
        for(int b = 0; b < B; ++b){
          if(!bpRecord_->has(bpEventIdx_, b)) continue;
          const uint32_t R = bpRecord_->event_row_count(bpEventIdx_, b);
          uint32_t row_size = 0;
          const int ncols = bpRecord_->num_columns(b);
          for(int c = 0; c < ncols; ++c) row_size += bpRecord_->column(b, c).bytes_per_row;
          total += BANK_STRUCTURE_SIZE + R * row_size;
        }
        return static_cast<int>(total);
      }
      uint32_t size_u;
      std::memcpy(&size_u, &dataBuffer[EH_SIZE_OFFSET], sizeof(size_u));
      return static_cast<int>(size_u);
    }

    void event::reset(){
        bbRecord_.reset(); bbSynthDone_ = false;
        bpRecord_.reset(); bpSynthDone_ = false;
        dataBuffer[0] = 'E'; dataBuffer[1] = 'V';
        dataBuffer[2] = 'N'; dataBuffer[3] = 'T';
        uint32_t val;
        val = EVENT_HEADER_SIZE; std::memcpy(&dataBuffer[EH_SIZE_OFFSET], &val, sizeof(val));
        val =  0; std::memcpy(&dataBuffer[EH_TAG_OFFSET], &val, sizeof(val));
        val =  0; std::memcpy(&dataBuffer[EH_RESERVED_OFFSET], &val, sizeof(val));
    }
    std::vector<char> &event::getEventBuffer(){
       // Force synthesis on first byte-buffer request; subsequent calls
       // are zero-cost.
       if(bbRecord_ && !bbSynthDone_) synthesizeFromByBank();
       if(bpRecord_ && !bpSynthDone_) synthesizeFromByPage();
       return dataBuffer;
    }
    /*
    template<class T>   node<T> event::getNode(){
        node<T> en;
        en.setLength(4);
        en.setAddress(NULL);
    } */
    void event::show(){
        if(bbRecord_ && !bbSynthDone_) synthesizeFromByBank();
        if(bpRecord_ && !bpSynthDone_) synthesizeFromByPage();
        printf(" EVENT  SIZE = %d\n",getSize());
        int position = EVENT_HEADER_SIZE;
        uint32_t eventSize_u;
        std::memcpy(&eventSize_u, &dataBuffer[EH_SIZE_OFFSET], sizeof(eventSize_u));
        int eventSize = static_cast<int>(eventSize_u);
        while(position+BANK_STRUCTURE_SIZE<eventSize){
            uint16_t   gid;
            std::memcpy(&gid, &dataBuffer[position], sizeof(gid));
            uint8_t    iid = static_cast<uint8_t>(dataBuffer[position+2]);
            uint8_t   type = static_cast<uint8_t>(dataBuffer[position+3]);
	    int     sizeWord;
	    std::memcpy(&sizeWord, &dataBuffer[position+4], sizeof(sizeWord));
            int     length = sizeWord&STRUCT_SIZE_MASK;
	    int     format = (sizeWord>>STRUCT_FORMAT_SHIFT)&STRUCT_FORMAT_BYTE;
            //printf("group = %4d , item = %4d\n",(unsigned int) gid, (unsigned int) iid);
            //if(gid==group&&iid==item) return std::make_pair(position,length);
            printf("%12s node [%9d %4d] type = %12d, fotmat size = %3d , length = %12d\n"," ",gid,iid,type,format,length);
            position += (length + BANK_STRUCTURE_SIZE);
        }
    }



    void     event::get(const char *buffer, hipo::node &_n, int group, int item){
       std::pair<int,int> index = getStructurePosition(buffer,group,item);
       if(index.first>0){
         _n.init(&buffer[index.first], index.second + BANK_STRUCTURE_SIZE);
         //_n.notify();
       } else {
         _n.initEmpty();
         _n.notify();
         //printf("*** error *** : structure (%5d,%5d) does not exist\n", group,item);
       }
    }
}
