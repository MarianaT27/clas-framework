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
/**
 * <pre>
 *
 * FILE HEADER STRUCTURE ( 56 bytes, 14 integers (32 bit) )
 *
 *    +----------------------------------+
 *  1 |              ID                  | // HIPO: 0x43455248, Evio: 0x4556494F
 *    +----------------------------------+
 *  2 +          File Number             | // split file #
 *    +----------------------------------+
 *  3 +         Header Length            | // 14 (words)
 *    +----------------------------------+
 *  4 +      Record (Index) Count        |
 *    +----------------------------------+
 *  5 +      Index Array Length          | // bytes
 *    +-----------------------+----------+
 *  6 +       Bit Info        | Version  | // version (8 bits)
 *    +-----------------------+----------+
 *  7 +      User Header Length          | // bytes
 *    +----------------------------------+
 *  8 +          Magic Number            | // 0xc0da0100
 *    +----------------------------------+
 *  9 +          User Register           |
 *    +--                              --+
 * 10 +                                  |
 *    +----------------------------------+
 * 11 +         Trailer Position         | // File offset to trailer head (64 bits).
 *    +--                              --+ // 0 = no offset available or no trailer exists.
 * 12 +                                  |
 *    +----------------------------------+
 * 13 +          User Integer 1          |
 *    +----------------------------------+
 * 14 +          User Integer 2          |
 *    +----------------------------------+
 *
 * -------------------
 *   Bit Info Word
 * -------------------
 *     0-7  = version
 *     8    = true if dictionary is included (relevant for first record only)
 *     9    = true if this file has "first" event (in every split file)
 *    10    = File trailer with index array exists
 *    11-19 = reserved
 *    20-21 = pad 1
 *    22-23 = pad 2
 *    24-25 = pad 3 (always 0)
 *    26-27 = reserved
 *    28-31 = general header type: 1 = Evio file
 *                                 2 = Evio extended file
 *                                 5 = HIPO file
 *                                 6 = HIPO extended file
 *
 * </pre>
 *
 * @version 6.0
 * @since 6.0 9/6/17
 */
/*
 * File:   reader.h
 * Author: gavalian
 *
 * Created on April 11, 2017, 2:07 PM
 */

#ifndef HIPOREADER_H
#define HIPOREADER_H

#define HIPO_FILE_HEADER_SIZE 72
/* Constants for endianness of the file */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include "bank.h"
#include "record.h"
#include "utils.h"

namespace hipo {

struct ErrorCode {
    enum Value {
        NO_ERROR = 0,
        FILE_OPEN_ERROR = 1,
        INVALID_HEADER_LENGTH = 2,
        INDEX_READ_ERROR = 3,
        ERROR_NOINDEX = 4,
        ERROR_CORRUPT_FILE = 5
    } value = NO_ERROR;

    ErrorCode(Value v = NO_ERROR) : value(v) {}

    operator Value() const { return value; }

    static std::string toString(Value code) {
        switch (code) {
            case NO_ERROR:
                return "No Error";
            case FILE_OPEN_ERROR:
                return "File Open Error";
            case INVALID_HEADER_LENGTH:
                return "Invalid Header Length";
            case INDEX_READ_ERROR:
                return "Index Read Error";
            case ERROR_NOINDEX:
                return "File has no index (trailer missing)";
            case ERROR_CORRUPT_FILE:
                return "File appears corrupt or not closed properly";
            default:
                return "Unknown Error Code";
        }
    }
};

//  typedef struct fileHeader_t {
struct fileHeader_t {
    int uniqueid{};
    int filenumber{};
    int headerLength{};  // in words (usually 14)
    int recordCount{};
    int indexArrayLength{};  // in bytes
    int bitInfo{};
    int version{};
    int userHeaderLength{};
    int magicNumber{};
    long userRegister{};
    long trailerPosition{};
    long firstRecordPosition{};
};  // fileHeader_t;

//  typedef struct recordInfo_t {
struct recordInfo_t {
    long recordPosition{};
    int recordLength{};
    int recordEntries{};
    long userWordOne{};
    long userWordTwo{};
};  // recordInfo_t;

/**
 * RecordPosition - Detailed record position info matching Java implementation
 * Used for file trailer index tracking with tag-based filtering support
 */
class RecordPosition {
   private:
    long position_{};
    int length_{};
    int count_{};
    long userWordOne_{};
    long userWordTwo_{};

   public:
    RecordPosition() = default;

    RecordPosition(long pos, int len, int cnt)
        : position_(pos), length_(len), count_(cnt) {}

    long getPosition() const { return position_; }
    int getLength() const { return length_; }
    int getCount() const { return count_; }
    long getUserWordOne() const { return userWordOne_; }
    long getUserWordTwo() const { return userWordTwo_; }

    void setPosition(long pos) { position_ = pos; }
    void setLength(int len) { length_ = len; }
    void setCount(int cnt) { count_ = cnt; }
    void setUserWordOne(long word) { userWordOne_ = word; }
    void setUserWordTwo(long word) { userWordTwo_ = word; }

    std::string toString() const {
        char buffer[256];
        std::snprintf(buffer, sizeof(buffer),
                      "pos=%12ld, len=%8d, count=%6d, tag1=%ld, tag2=%ld",
                      position_, length_, count_, userWordOne_, userWordTwo_);
        return std::string(buffer);
    }
};

/**
   * READER index class is used to construct entire events
   * sequence from all records, and provides ability to canAdvance
   * through events where record number is automatically calculated
   * and triggers reading of the next record when events in the current
   * record are exhausted.
   */

class readerIndex {

   private:
    std::vector<int> recordEvents;
    std::vector<long> recordPosition;

    int currentRecord{};
    int currentEvent{};
    int currentRecordEvent{};

   public:
    readerIndex() = default;
    ~readerIndex() = default;

    bool canAdvance();
    bool advance();

    //dglazier
    bool canAdvanceInRecord();
    bool loadRecord(int irec);
    bool gotoEvent(int eventNumber);
    bool gotoRecord(int irec);

    int getEventNumber() { return currentEvent; }

    int getRecordNumber() { return currentRecord; }

    int getRecordEventNumber() { return currentRecordEvent; }

    int getMaxEvents();
    void addSize(int size);

    void addPosition(long position) { recordPosition.push_back(position); }

    long getPosition(int index) { return recordPosition[index]; }

    //dglazier
    int getNRecords() const { return recordEvents.size(); }

    void show();

    void rewind() {
        currentRecord = -1;
        currentEvent = -1;
        currentRecordEvent = -1;
    }

    void clear() {
        recordEvents.clear();
        recordPosition.clear();
    }

    void reset() {
        currentRecord = 0;
        currentEvent = 0;
        currentRecordEvent = 0;
    }
};

class reader {

   private:
    fileHeader_t header{};
    hipo::utils hipoutils;
    std::ifstream inputStream;
    long inputStreamSize{};

    hipo::record inputRecord;
    hipo::readerIndex readerEventIndex;
    std::vector<long> tagsToRead;

    short _verbose = {0};

    std::map<std::string, std::string> userConfig;

    hipo::event event;
    hipo::dictionary factory;

    // New members matching Java HipoReader
    std::vector<RecordPosition> recordPositions;
    ErrorCode readerErrorCode{ErrorCode::NO_ERROR};
    int numberOfEvents{0};

    void readHeader();
    void readIndex();

   public:
    reader();

    reader(const char* file) { open(file); }

    reader(const char* file, std::vector<int> tags) {
        //for(int t = 0; t < tags.size(); t++) setTags(tags[t]);
        for (auto tag : tags)
            setTags(tag);
        open(file);
    }

    reader(const reader& /*r*/) {}

    ~reader();

    void about();

    void rewind() { readerEventIndex.rewind(); }

    void readDictionary(hipo::dictionary& dict);
    void getStructure(hipo::structure& structure, int group, int item);
    void getStructureNoCopy(hipo::structure& structure, int group, int item);

    void readUserConfig(std::map<std::string, std::string>& mapConfig);
    //std::string getUserConfig(const char *key);

    ErrorCode open(const char* filename);

    bool is_open() { return inputStream.is_open(); }

    void setTags(int tag) { tagsToRead.push_back(tag); }

    void setTags(std::vector<long> tags) { tagsToRead = std::move(tags); }

    void setVerbose(short level = 1) { _verbose = level; }

    bool hasNext();
    bool next();
    bool gotoEvent(int eventNumber);
    bool gotoRecord(int irec);
    bool next(hipo::event& dataevent);

    bool next(std::vector<hipo::bank>& list);
    std::vector<hipo::bank> getBanks(std::vector<std::string> names);

    void read(hipo::event& dataevent);
    void printWarning();

    //void  showUserConfig();
    int getNRecords() const { return readerEventIndex.getNRecords() - 1; }

    bool nextInRecord();
    bool loadRecord(int irec);
    bool loadRecord(hipo::record& record, int irec);

    int getEntries() { return readerEventIndex.getMaxEvents(); }

    std::vector<int> getInt(const char* bank, const char* column, int max = -1);
    std::vector<float> getFloat(const char* bank, const char* column, int max = -1);

    // =========================================================================
    // New methods matching Java HipoReader API
    // =========================================================================

    /**
     * Get the reader error code (NO_ERROR, ERROR_NOINDEX, etc.)
     * @return current error code
     */
    ErrorCode getErrorCode() const { return readerErrorCode; }

    /**
     * Get total event count (sum of all records)
     * @return number of events in file
     */
    int getEventCount() const { return numberOfEvents; }

    /**
     * Get list of record positions
     * @return reference to record positions vector
     */
    const std::vector<RecordPosition>& getRecordPositions() const { return recordPositions; }

    // -------------------------------------------------------------------------
    // User Configuration Methods (like Java hasConfig/getConfig/showConfig)
    // -------------------------------------------------------------------------

    /**
     * Check if user configuration contains a key
     * @param name configuration key
     * @return true if key exists
     */
    bool hasConfig(const std::string& name) const {
        return userConfig.find(name) != userConfig.end();
    }

    /**
     * Get user configuration value by key
     * @param name configuration key
     * @return configuration value or empty string if not found
     */
    std::string getConfig(const std::string& name) const {
        auto it = userConfig.find(name);
        return (it != userConfig.end()) ? it->second : "";
    }

    /**
     * Print all user configuration key-value pairs
     */
    void showConfig() const;

    // -------------------------------------------------------------------------
    // Random Access by Event Index (like Java getEvent)
    // -------------------------------------------------------------------------

    /**
     * Get event at specific index (random access)
     * @param dataevent event to fill
     * @param index event index
     * @return true if successful
     */
    bool getEvent(hipo::event& dataevent, int index);

    // -------------------------------------------------------------------------
    // Filtered Event Reading (like Java nextEvent with schemas)
    // -------------------------------------------------------------------------

    /**
     * Get next event that contains all specified banks
     * @param dataevent event to fill
     * @param bankNames list of required bank names
     * @return true if found, false if no more matching events
     */
    bool nextEvent(hipo::event& dataevent, const std::vector<std::string>& bankNames);

    // -------------------------------------------------------------------------
    // Bank Factory (like Java getBank)
    // -------------------------------------------------------------------------

    /**
     * Create a bank instance from schema name
     * @param name bank/schema name
     * @return bank instance (empty if schema not found)
     */
    hipo::bank getBank(const std::string& name);

    // -------------------------------------------------------------------------
    // Debug/Info Display (like Java showRecords/showInfo)
    // -------------------------------------------------------------------------

    /**
     * Print record positions and event counts
     */
    void showRecords() const;

    /**
     * Print file info summary grouped by tags
     */
    void showInfo() const;

    /**
     * Get the dictionary/schema factory
     * @return reference to dictionary
     */
    hipo::dictionary& getSchemaFactory() { return factory; }
    const hipo::dictionary& getSchemaFactory() const { return factory; }
};

class readerstream {
   private:
    hipo::reader hr;
    //hipo::writer hw;
    hipo::dictionary factory;
    std::mutex obj;
    long nProcessed = 0;
    long nDataLimit = -1;

   public:
    //static int eof_printout;

    readerstream() { /*datastream::eof_printout = 0;*/ }

    virtual ~readerstream() {}

    void open(const char* input) {
        hr.open(input);
        hr.readDictionary(factory);
        //hw.addDictionary(factory);
        //hw.open(output);
    }

    void setLimit(long limit) {
        nDataLimit = limit;
    }

    void run(std::function<int(int)>&& function, int nthreads) {
        // Use std::jthread for RAII - threads auto-join on destruction
        std::vector<std::jthread> threads;
        threads.reserve(nthreads);
        for (int i = 0; i < nthreads; i++) {
            threads.emplace_back(function, i);
        }
        printf("-- created denoiser with %zu threads\n", threads.size());
        // No explicit join needed - jthread destructor handles it
    }

    hipo::reader& reader() { return hr; }

    hipo::dictionary& dictionary() { return factory; }

    void pull(hipo::record& record, int index) {
        std::unique_lock<std::mutex> lock(obj);
        hr.loadRecord(record, index);
    }

    void pull(std::vector<hipo::event>& events) {

        std::unique_lock<std::mutex> lock(obj);
        bool finished = false;
        if (nDataLimit > 0) {
            if (nProcessed > nDataLimit) finished = true;
        }

        if (hr.hasNext() == false) {
            printf("\n");
        }

        for (int n = 0; n < (int)events.size(); n++) {
            // write the event in the output if it's not empty
            //if(events[n].getSize()>16){ hw.addEvent(events[n]);}
            // reset event and read next in the file if any left
            events[n].reset();
            if (hr.next() == true && finished == false) {
                hr.read(events[n]);
                nProcessed++;
                if (nProcessed % 250 == 0) {
                    printf(".");
                    fflush(stdout);
                }
                if (nProcessed % 10000 == 0) printf(" : %9lu \n", nProcessed);
            }
        }
    }
};

}  // namespace hipo
#endif /* HIPOREADER_H */
