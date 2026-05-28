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

#ifndef HIPO_EVENT_H
#define HIPO_EVENT_H

#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "bank.h"
#include "node.h"

// if the library is compiled with C++11
// support we will use unordered map which
// is faster than standard map
#if __cplusplus > 199711L
#include <unordered_map>
#endif


namespace hipo {

  class bybankrecord;  // forward decl for ByBank mode (Phase 3 v2)
  class bypagerecord;  // forward decl for ByPage mode (Phase 4)

  //typedef std::auto_ptr<hipo::generic_node> node_pointer;

  class event {

    private:
        std::vector<char> dataBuffer;

        // Phase 3 v2: dual-mode storage. Bytes mode is the default
        // (dataBuffer holds raw event bytes). ByBank mode holds a
        // shared pointer to the bybankrecord that produced this event
        // plus the event index within that record. Bank lookups
        // dispatch through the record's lazy-inflate API.
        std::shared_ptr<bybankrecord> bbRecord_;
        uint32_t                      bbEventIdx_{0};
        // True once the lazy synthesizer has reassembled the event's
        // full bytes into dataBuffer (paid only when callers ask for
        // raw bytes via getEventBuffer/getSize/show etc.).
        bool                          bbSynthDone_{false};
        // Per-getStructure scratch buffer for the [8-byte header | data
        // bytes] reassembly. Reused across calls; sized on demand.
        std::vector<char>             bbScratch_;

        // Phase 4: ByPage mode. Same idea but with finer per-(bank,col,page)
        // lazy inflate. getStructure path A inflates ALL columns of the
        // requested bank; getColumnView path B inflates only the column
        // the caller asks for.
        std::shared_ptr<bypagerecord> bpRecord_;
        uint32_t                      bpEventIdx_{0};
        bool                          bpSynthDone_{false};

        // Lazy: reassemble bbRecord_/bbEventIdx_ into dataBuffer so
        // getEventBuffer()/getSize() and any bytes-walking method works.
        void synthesizeFromByBank();
        // Same for ByPage (more expensive — inflates every column of every
        // bank in this event).
        void synthesizeFromByPage();
        // Force ByBank/ByPage events back into Bytes mode before a mutation
        // (addStructure/replace/remove/...) modifies dataBuffer.
        void ensureBytesMode();
    public:

        event();
        event(int size);
        event(const event&);
        event(event&&) noexcept;
        event& operator=(const event&);
        event& operator=(event&&) noexcept;
        virtual ~event();

        void   show();
        void   init(std::vector<char> &buffer);
        void   init(const char *buffer, int size);
        void   getStructure(hipo::structure &str, int group, int item);
        void   getStructure4(hipo::structure &str, int group, int item);
        
        int    getTag();
        void   setTag(int tag);
        void   getStructure(hipo::bank &b);
        void   read(hipo::bank &b);
        void   addStructure(hipo::structure &str);
        void   override(hipo::structure &str);
        void   remove(hipo::bank &str);
        void   remove(int group, int item);
        void   replace(hipo::bank  &bank);
        
        void   add(hipo::node &_n);
        void   get(hipo::node &_n, int group, int item);

        std::pair<int,int>  getStructurePosition(int group, int item);
        std::pair<int,int>  getStructurePosition4(int group, int item);

        std::vector<char>  &getEventBuffer();
        int                 getSize();
        void                reset();

        // Phase 3 v2: switch this event into ByBank mode. The event
        // becomes a lightweight view into an existing bybankrecord;
        // bank lookups use the record's lazy-inflate API. Any prior
        // Bytes-mode state is cleared.
        void initByBank(std::shared_ptr<bybankrecord> rec, uint32_t event_idx);
        bool isByBank() const { return bbRecord_ != nullptr; }

        // Phase 4: same for ByPage. getStructure(bank) does an
        // all-columns inflate of the requested bank; for the partial-
        // column win, prefer the new getColumnView<T>() below.
        void initByPage(std::shared_ptr<bypagerecord> rec, uint32_t event_idx);
        bool isByPage() const { return bpRecord_ != nullptr; }

        // Phase 4 Path B: lazy partial-column read. In ByPage mode this
        // inflates *only* the column's page containing this event — banks
        // and other columns of the same bank stay compressed. In Bytes or
        // ByBank mode it falls back to walking the bank's structure bytes
        // (correct but no partial-column win).
        //
        // Returns (pointer-to-first-element, element-count). Pointer
        // stays valid until the next event/record advance.
        // Explicit instantiations: int8_t, int16_t, int32_t, int64_t,
        // float, double — the HIPO scalar types.
        template<typename T>
        std::pair<const T*, int> getColumnView(int group, int item, int col_idx);
        void                write(hipo::node &node);
        void                read(hipo::node  &node, int group, int item);

        //*******************************************************************
        //** static methods for reading structures from event structure
        //** from the memory. It does not have to copy event into separate
        //** buffer.
        //*******************************************************************
        static std::pair<int,int>
              getStructurePosition(const char *buffer, int group, int item);
        //static std::pair<int,int>
        //      getStructurePosition5(const char *buffer, int group, int item);
        //static std::pair<int,int>  getStructurePosition(const char *buffer, int group, int item);
        static void
              getStructure(const char *buffer, hipo::structure &str, int group, int item);
        static void   get(const char *buffer, hipo::node &_n, int group, int item);      
        static void
              getStructureNoCopy(const char *buffer, hipo::structure &str, int group, int item);
    };
}

#endif /* EVENT_H */
