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
/*******************************************************************************
 * File:   bank.h
 * Author: gavalian
 *
 * Created on April 12, 2017, 10:14 AM
 */

#ifndef HIPO_BANK_H
#define HIPO_BANK_H
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <functional>
#include <utility>
#include "dictionary.h"
#include "node.h"

namespace hipo {

  class Parser;

  class structure {

    private:
      
      std::vector<char> structureBuffer;
      char *structureAddress{};
      void setAddress(const char *address);

      // Used by the copy ctor/assignment to point `structureAddress` at THIS
      // object's buffer when the source pointed at its own owned buffer, or to
      // copy an external (initNoCopy) pointer verbatim.
      void rebindAddressFrom(const structure& o) {
          if (o.structureAddress == nullptr) {
              structureAddress = nullptr;
          } else if (!o.structureBuffer.empty() &&
                     o.structureAddress == o.structureBuffer.data()) {
              structureAddress = structureBuffer.data();  // owned -> our copy
          } else {
              structureAddress = o.structureAddress;       // external (initNoCopy)
          }
      }

    protected:

      void initStructureBySize(int __group, int __item, int __type, int __size);
      std::vector<char>  &getStructureBuffer(){ return structureBuffer;}
      int                 getStructureBufferSize(){ return 8+getSize();}
      int                 dataOffset = 8;
      //std::vector<char> structureBuffer;
      friend class tuple;

  public:

      structure(){ structureAddress = nullptr;}
      structure(int size){ allocate(size);}
      structure(int __group, int __item, std::string &str);

      // Copy/move must rebind `structureAddress`: it is a raw pointer that
      // points either into our OWNED `structureBuffer` (after init /
      // initStructureBySize) or to external memory (after initNoCopy). The
      // implicitly-generated special members would copy the pointer verbatim,
      // leaving it dangling into the source object once the source dies — which
      // corrupts any code that returns/stores a populated bank by value (e.g.
      // framework::Event::get_bank caching a bank read from a ByPage record).
      structure(const structure& o)
          : structureBuffer(o.structureBuffer), dataOffset(o.dataOffset) {
          rebindAddressFrom(o);
      }
      structure& operator=(const structure& o) {
          if (this != &o) {
              structureBuffer = o.structureBuffer;
              dataOffset      = o.dataOffset;
              rebindAddressFrom(o);
          }
          return *this;
      }
      // std::vector move keeps the same heap allocation, so a pointer into the
      // owned buffer stays valid after the move; an external pointer is copied
      // as-is. Either way, null the source so it cannot alias our buffer.
      structure(structure&& o) noexcept
          : structureBuffer(std::move(o.structureBuffer)),
            structureAddress(o.structureAddress),
            dataOffset(o.dataOffset) {
          o.structureAddress = nullptr;
      }
      structure& operator=(structure&& o) noexcept {
          if (this != &o) {
              structureBuffer  = std::move(o.structureBuffer);
              structureAddress = o.structureAddress;
              dataOffset       = o.dataOffset;
              o.structureAddress = nullptr;
          }
          return *this;
      }

      virtual     ~structure()= default;
      bool         allocate(int size);
      
      int          getSize() const noexcept{
        int length = *reinterpret_cast<uint32_t *>(structureAddress+4);
	       return length&STRUCT_SIZE_MASK;
         //return getHeaderSize()+getDataSize();
      }

      int         getHeaderSize() const noexcept {
         int length = *reinterpret_cast<uint32_t *>(structureAddress+4);
         return (length>>STRUCT_FORMAT_SHIFT)&STRUCT_FORMAT_BYTE;
      }
      
      int         getDataSize() const noexcept {         
	       return getSize()-getHeaderSize();
      }

      int          getType() const;
      int          getGroup() const;
      int          getItem() const;
      void         init(const char *buffer, int size);
      void         initNoCopy(const char *buffer, int size);

      const char    *getAddress() const;
      virtual void   show() const;
      void           setSize(int size);
      void           setHeaderSize(int size);
      void           setDataSize(int size);

      int          getIntAt   ( int index) const noexcept {
        return *reinterpret_cast<int32_t*>(&structureAddress[index+dataOffset]);
      }
    
      int16_t      getShortAt ( int index) const noexcept {
        return *reinterpret_cast<int16_t*>(&structureAddress[index+dataOffset]);
      }
      int8_t       getByteAt  ( int index) const noexcept {
        return *reinterpret_cast<int8_t*>(&structureAddress[index+dataOffset]);
      }
      float        getFloatAt ( int index) const noexcept {
        return *reinterpret_cast<float*>(&structureAddress[index+dataOffset]);
      }
      double       getDoubleAt( int index) const noexcept {
        return *reinterpret_cast<double*>(&structureAddress[index+dataOffset]);
      }
      long         getLongAt  ( int index) const noexcept {
        return *reinterpret_cast<int64_t*>(&structureAddress[index+dataOffset]);
      }

      std::string  getStringAt(int index);

      void         putIntAt(int index, int value){
        *reinterpret_cast<int32_t*>(&structureAddress[index+dataOffset]) = value;
      }

      void         putShortAt(int index, int16_t value){
        *reinterpret_cast<int16_t*>(&structureAddress[index+dataOffset]) = value;
      }

      void         putByteAt(int index, int8_t value){
        *reinterpret_cast<int8_t*>(&structureAddress[index+dataOffset]) = value;
      }

      void         putFloatAt(int index, float value){
        *reinterpret_cast<float*>(&structureAddress[index+dataOffset]) = value;
      }

      void         putDoubleAt(int index, double value){
        *reinterpret_cast<double*>(&structureAddress[index+dataOffset]) = value;
      }

      void         putLongAt(int index, int64_t value){
        *reinterpret_cast<int64_t*>(&structureAddress[index+dataOffset]) = value;
      }

      void         putStringAt(int index, std::string &str);

      virtual void notify(){}
      friend class event;
  };

  class composite : public node {
    /**
     * @brief This is composite bank with type = 10, who knows why
     * Class is used to store formated data structures without dictionary.
     * Can be passed around in events and parsed on the other side.
     */
    private:

      std::vector<char>  typeChars;
      std::vector<int>   offsets;
      std::vector<int>   types;
      int                rowOffset = 0;

     // void parse(std::string format);
      int  getTypeSize(int type);

    public:

      composite(){};
      composite(int size){ allocate(size);};
      composite(int group, int item, int size){ /*initStructureBySize(group, item, 10, size);*/};
      composite(const char *format){}
      composite(int group, int item, const char *format, int capacity);
      
      void       parse(std::string format);
      void       parse(int group, int item, std::string format, int maxrows);
      virtual   ~composite(){}
      
      /// @returns the number of bank rows. This is the number of **all** of the rows, not
      ///          the reduced number if, _e.g._, `hipo::bank::rowlist::filter` was called; for the latter, use
      ///          `hipo::bank::getRowList().size()`.
      int      getRows() const noexcept { return dataLength()/rowOffset;}

      int      getEntries() const noexcept { return offsets.size();}
      int      getEntryType(int index) const noexcept { return types[index];}
      void     setRows(int rows);// { setDataLength(rows*rowOffset);}
      
      int      getRowSize() const noexcept { return rowOffset;}

      int      getInt    ( int element, int row) const noexcept;
      int64_t  getLong   ( int element, int row) const noexcept;
      float    getFloat  ( int element, int row) const noexcept;
      void     putInt    ( int element, int row, int value);
      void     putLong   ( int element, int row, int64_t value);
      void     putFloat  ( int element, int row, float value);
      virtual void notify();
      void     print();
      void     reset();
  };
  //typedef std::auto_ptr<hipo::generic_node> node_pointer;

    class bank : public structure {

    public:

      /// `rowlist` encapsulates a list of rows for this `bank`, providing a way to iterate over them
      class rowlist {

      public:
        using list_t = std::vector<int>;

        rowlist()  = default;
        ~rowlist() = default;

        /// initialize with a full list with specified number of rows
        /// @param numRows if negative, use the owner `bank` to set the number of rows, otherwise use `numRows`
        void reset(int numRows = -1);
        bool isInitialized() const { return m_init; }

        /// @returns reference to the immutable list
        list_t const& getList() const;
        /// @param list set the list to this list
        void setList(list_t const& list);

        /// filter the list according to a function
        /// @param func a function which takes a `hipo::bank` reference and an `int` row number and returns a `double`;
        /// if the returned `bool` is `true`, the row is accepted
        void filter(std::function<bool(bank&, int)> func);
        /// filter the list according to an expression
        /// @param expression the filter expression
        void filter(char const* expression);
        /// filter the list according to a parsed expression
        /// @param p the parser with the filter expression
        void filter(Parser& p);

        /// @returns create a list of numbers from 0 to `num`
        /// @param num the size of the list
        static list_t createFullList(int num);

        /// @param ownerBank set the owner bank
        void setOwnerBank(bank* const ownerBank) { m_owner_bank = ownerBank; }

      private:
        bool m_init{false};
        list_t m_list{};
        bank* m_owner_bank{nullptr};

        bool ownerBankIsUnknown(std::string_view caller = "");

      };

    private:

      schema  bankSchema;
      int     bankRows{-1};
      rowlist bankRowList;

      // ---- Phase 4 path B (lazy per-column inflate) ----
      // When a bank is bound to a hipo::bypagerecord by event::getStructure
      // in ByPage mode, we DO NOT pre-inflate its column data. Instead the
      // bank holds a back-pointer; the first call to any per-column getter
      // (getInt/getFloat/etc. for that column) triggers a lazy inflate of
      // just that column's page and memcpys the bytes into the bank's
      // structure buffer. Subsequent getter calls on the same column read
      // directly from the buffer with no extra work.
      //
      // Fields are mutable so const noexcept getters can call ensureColumn().
      mutable class bypagerecord*       bp_ref_{nullptr}; // non-owning; lifetime: bypagerecord outlives the bank
      mutable uint32_t                  bp_event_idx_{0};
      mutable int                       bp_bank_idx_{0};
      mutable std::vector<char>         col_filled_;      // 0 = not yet inflated; 1 = filled

    public:

        // Internal: called by event::getStructure(bank&) in ByPage mode.
        // The bank reserves its structure buffer at full size (already
        // done by the preceding initStructureBySize) but defers the
        // per-column memcpys to ensureColumn() on first access.
        void setByPageRef(class bypagerecord* p, uint32_t event_idx, int bank_idx);
        void clearByPageRef() { bp_ref_ = nullptr; }
        bool hasByPageRef() const { return bp_ref_ != nullptr; }

        // First call inflates+copies the column's bytes; later calls are
        // a single branch check. Cheap (O(rows × bytes_per_row) memcpy
        // per column, one time). noexcept: swallows bypagerecord errors;
        // the caller will read zero-initialized bytes if inflation fails.
        void ensureColumn(int item) const noexcept;

        bank();
        // constructor initializes the nodes in the bank
        // and they will be filled automatically by reader.next()
        // method.
        bank(const schema& __schema){
          bankSchema = __schema;
          bankRows   = -1;
        }

        bank(const schema& __schema, int __rows){
          bankSchema = __schema;
          bankRows   = __rows;
          int size   = bankSchema.getSizeForRows(__rows);
          initStructureBySize(bankSchema.getGroup(),bankSchema.getItem(), 11, size);
          bankRowList.reset(bankRows);
        }

        // Copy/move rebind the structure buffer pointer (via the base class)
        // AND re-point bankRowList's owner back-pointer at the new object.
        // The ByPage lazy-binding members are value-copied (bp_ref_ is a
        // non-owning pointer whose target outlives the bank).
        bank(const bank& o)
          : structure(o), bankSchema(o.bankSchema), bankRows(o.bankRows),
            bankRowList(o.bankRowList), bp_ref_(o.bp_ref_),
            bp_event_idx_(o.bp_event_idx_), bp_bank_idx_(o.bp_bank_idx_),
            col_filled_(o.col_filled_) {
          bankRowList.setOwnerBank(this);
        }
        bank& operator=(const bank& o) {
          if (this != &o) {
            structure::operator=(o);
            bankSchema    = o.bankSchema;
            bankRows      = o.bankRows;
            bankRowList   = o.bankRowList;
            bp_ref_       = o.bp_ref_;
            bp_event_idx_ = o.bp_event_idx_;
            bp_bank_idx_  = o.bp_bank_idx_;
            col_filled_   = o.col_filled_;
            bankRowList.setOwnerBank(this);
          }
          return *this;
        }
        bank(bank&& o) noexcept
          : structure(std::move(o)), bankSchema(std::move(o.bankSchema)),
            bankRows(o.bankRows), bankRowList(std::move(o.bankRowList)),
            bp_ref_(o.bp_ref_), bp_event_idx_(o.bp_event_idx_),
            bp_bank_idx_(o.bp_bank_idx_), col_filled_(std::move(o.col_filled_)) {
          bankRowList.setOwnerBank(this);
        }
        bank& operator=(bank&& o) noexcept {
          if (this != &o) {
            structure::operator=(std::move(o));
            bankSchema    = std::move(o.bankSchema);
            bankRows      = o.bankRows;
            bankRowList   = std::move(o.bankRowList);
            bp_ref_       = o.bp_ref_;
            bp_event_idx_ = o.bp_event_idx_;
            bp_bank_idx_  = o.bp_bank_idx_;
            col_filled_   = std::move(o.col_filled_);
            bankRowList.setOwnerBank(this);
          }
          return *this;
        }

        ~bank() override;

        schema  &getSchema() { return bankSchema;}

        int    getRows()  const noexcept{ return bankRows;}
        void   setRows(   int rows);
        int    getInt(    int item, int index) const noexcept;
        int    getShort(  int item, int index) const noexcept;
        int    getByte(   int item, int index) const noexcept;
        float  getFloat(  int item, int index) const noexcept;
        double getDouble( int item, int index) const noexcept;
        long   getLong(   int item, int index) const noexcept;

        std::vector<int>    getInt(    int item) const noexcept;
        std::vector<float>  getFloat(  int item) const noexcept;
        std::vector<double> getDouble( int item) const noexcept;

        template<typename T = double> T get(int item, int index) const noexcept {
          ensureColumn(item);
          auto type   = bankSchema.getEntryType(item);
          auto offset = bankSchema.getOffset(item, index, bankRows);
          switch(type) {
            case kByte:   return (int) getByteAt(offset);
            case kShort:  return (int) getShortAt(offset);
            case kInt:    return getIntAt(offset);
            case kFloat:  return getFloatAt(offset);
            case kDouble: return getDoubleAt(offset);
            case kLong:   return getLongAt(offset);
            default:
              printf("---> error(get) : unknown type for [%s] type = %d\n", bankSchema.getEntryName(item).c_str(), type);
          }
          return 0;
        }

        int    getInt(    const char *name, int index) const noexcept;
        
        int    getShort(  const char *name, int index) const noexcept;
        int    getByte(   const char *name, int index) const noexcept;
        float  getFloat(  const char *name, int index) const noexcept;
        double getDouble( const char *name, int index) const noexcept;
        long   getLong(   const char *name, int index) const noexcept;
        
        std::vector<int>    getInt(    const char *name) const noexcept;
        std::vector<float>  getFloat(  const char *name) const noexcept;
        std::vector<double> getDouble( const char *name) const noexcept;

        template<typename T = double> T get(const char *name, int index) const noexcept {
          return get<T>(bankSchema.getEntryOrder(name), index);
        }

        void    putInt(    const char *name, int index, int32_t value);
        void    putShort(  const char *name, int index, int16_t value);
        void    putByte(   const char *name, int index, int8_t value);
        void    putFloat(  const char *name, int index, float value);
        void    putDouble( const char *name, int index, double value);
        void    putLong(   const char *name, int index, int64_t value);
        template<typename T> void put(const char *name, int index, T value) {
          put(bankSchema.getEntryOrder(name), index, value);
        }

        void    putInt(int item, int index, int32_t value);
        void    putShort(int item, int index, int16_t value);
        void    putByte(int item, int index, int8_t value);
        void    putFloat(int item, int index, float value);
        void    putDouble(int item, int index, double value);
        void    putLong(int item, int index, int64_t value);

        // ---- Array accessors (for columns declared with /T#N syntax) ----
        //
        // The schema entry's `arrayLength` field must equal `len` or these
        // throw `length_mismatch_error`. `dest`/`src` must point to at
        // least `len` elements. Element order within a cell is
        // element-major (consecutive bytes). Column-major across rows.

        void    getIntArray   (int item, int row, int32_t *dest, int len) const;
        void    getShortArray (int item, int row, int16_t *dest, int len) const;
        void    getByteArray  (int item, int row, int8_t  *dest, int len) const;
        void    getFloatArray (int item, int row, float   *dest, int len) const;
        void    getDoubleArray(int item, int row, double  *dest, int len) const;
        void    getLongArray  (int item, int row, int64_t *dest, int len) const;

        void    getIntArray   (const char *name, int row, int32_t *dest, int len) const;
        void    getShortArray (const char *name, int row, int16_t *dest, int len) const;
        void    getByteArray  (const char *name, int row, int8_t  *dest, int len) const;
        void    getFloatArray (const char *name, int row, float   *dest, int len) const;
        void    getDoubleArray(const char *name, int row, double  *dest, int len) const;
        void    getLongArray  (const char *name, int row, int64_t *dest, int len) const;

        std::vector<int32_t> getIntArray   (int item, int row) const;
        std::vector<int16_t> getShortArray (int item, int row) const;
        std::vector<int8_t>  getByteArray  (int item, int row) const;
        std::vector<float>   getFloatArray (int item, int row) const;
        std::vector<double>  getDoubleArray(int item, int row) const;
        std::vector<int64_t> getLongArray  (int item, int row) const;

        void    putIntArray   (int item, int row, const int32_t *src, int len);
        void    putShortArray (int item, int row, const int16_t *src, int len);
        void    putByteArray  (int item, int row, const int8_t  *src, int len);
        void    putFloatArray (int item, int row, const float   *src, int len);
        void    putDoubleArray(int item, int row, const double  *src, int len);
        void    putLongArray  (int item, int row, const int64_t *src, int len);

        void    putIntArray   (const char *name, int row, const int32_t *src, int len);
        void    putShortArray (const char *name, int row, const int16_t *src, int len);
        void    putByteArray  (const char *name, int row, const int8_t  *src, int len);
        void    putFloatArray (const char *name, int row, const float   *src, int len);
        void    putDoubleArray(const char *name, int row, const double  *src, int len);
        void    putLongArray  (const char *name, int row, const int64_t *src, int len);

        template<typename T> void put(int item, int index, T value) {
          auto type = bankSchema.getEntryType(item);
          switch(type) {
            case kByte:   putByte(item,   index, static_cast<int8_t>(value));  break;
            case kShort:  putShort(item,  index, static_cast<int16_t>(value)); break;
            case kInt:    putInt(item,    index, static_cast<int32_t>(value)); break;
            case kFloat:  putFloat(item,  index, static_cast<float>(value));   break;
            case kDouble: putDouble(item, index, static_cast<double>(value));  break;
            case kLong:   putLong(item,   index, static_cast<int64_t>(value)); break;
            default:
              printf("---> error(put) : unknown type for [%s] type = %d\n", bankSchema.getEntryName(item).c_str(), type);
          }
        }

        /// @returns an immutable list of available rows for this bank. This list may be a subset of the full
        /// list of rows, if for example the bank was filtered (see `hipo::bank::rowlist::filter`); _cf._ `hipo::bank::rowlist::getFullRowList`
        rowlist::list_t const& getRowList() const;

        /// @returns an immutable list of **all** rows in the bank; _cf._ `hipo::bank::rowlist::getRowList`. This method
        /// may be less efficient than simply using a `for` loop from `0` to `getRows()`
        rowlist::list_t const getFullRowList() const;

        /// @returns a reference to the mutable `hipo::bank::rowlist` owned by this bank. For example, use this method to
        /// call `hipo::bank::rowlist::filter`.
        rowlist& getMutableRowList();

        /// @returns a `hipo::bank::rowlist` for this bank, for rows `r` such that `getInt(column,r) == row`
        /// @param row the value to check
        /// @param column the column to check (must be an integer-type column, _e.g._, that of `"pindex"`)
        rowlist::list_t const getRowListLinked(int const row, int const column) const;

        /// show this bank's contents; only the rows in its current `rowlist` instance are shown
        void show() const override;

        /// show this bank's contents
        /// @param showAllRows if `true`, show **all** this bank's rows, otherwise just the rows in its `rowlist` instance,
        /// which may have been reduced by, _e.g._, `hipo::bank::rowlist::filter`
        void show(bool const showAllRows) const;

        /// print a stored value
        /// @param schemaEntry the schema entry number
        /// @param row the row number
        void printValue(int schemaEntry, int row) const;

        /// @brief calculate a checksum for this bank; useful for comparing two banks' equality
        /// @param checkAllRows if true, checks all rows rather than just those from `getRowList()`
        /// @returns the checksum
        std::size_t checksum(bool checkAllRows=false) const;

        void reset();

        void notify() override;

  };
    /////////////////////////////////////
    //inlined getters

    inline float  bank::getFloat(int item, int index) const noexcept{
      ensureColumn(item);
      if(bankSchema.getEntryType(item)==kFloat){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getFloatAt(offset);
      }
      return 0.0;
    }

    inline double  bank::getDouble(int item, int index) const noexcept{
      ensureColumn(item);
      if(bankSchema.getEntryType(item)==kDouble){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getDoubleAt(offset);
      }
      if(bankSchema.getEntryType(item)==kFloat){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getFloatAt(offset);
      }
      return 0.0;
    }

    inline long bank::getLong(int item, int index) const noexcept{
      ensureColumn(item);
      if(bankSchema.getEntryType(item)==kLong){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getLongAt(offset);
      }
      return 0;
    }

    inline int    bank::getInt(int item, int index) const noexcept{
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      switch(type){
      case kByte:  return (int) getByteAt(offset);
      case kShort: return (int) getShortAt(offset);
      case kInt:   return getIntAt(offset);
      default: printf("---> error : requested INT for [%s] type = %d\n",
		      bankSchema.getEntryName(item).c_str(),type); break;
      }
      return 0;
    }

  inline std::vector<int>    bank::getInt(int item) const noexcept{
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);

      std::vector<int> row;
      int nrows = getRows();

      for(int j = 0; j < nrows; j++){
        int offset = bankSchema.getOffset(item, j, bankRows);
      switch(type){
          case kByte:  row.push_back((int) getByteAt(offset)); break;
          case kShort: row.push_back((int) getShortAt(offset)); break;
          case kInt:   row.push_back((int) getIntAt(offset)); break;
      default: printf("---> error : requested INT for [%s] type = %d\n",
		      bankSchema.getEntryName(item).c_str(),type); break;
      }
      }
      return row;
    }

    inline int    bank::getShort(int item, int index) const noexcept{
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      switch(type){
      case kByte:  return (int) getByteAt(offset);
      case kShort: return (int) getShortAt(offset);
      default: printf("---> error : requested SHORT for [%s] type = %d\n",
		      bankSchema.getEntryName(item).c_str(),type); break;
      }
      return 0;
    }

    inline int    bank::getByte(int item, int index) const noexcept{
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      switch(type){
      case kByte: return (int) getByteAt(offset);
      default: printf("---> error : requested BYTE for [%s] type = %d\n",
		      bankSchema.getEntryName(item).c_str(),type); break;
      }
      return 0;
    }
    inline int    bank::getInt(const char *name, int index) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      switch(type){
      case kByte:  return (int) getByteAt(offset);
      case kShort: return (int) getShortAt(offset);
      case kInt:   return getIntAt(offset);
      default: printf("---> error : requested INT for [%s] type = %d\n",name,type); break;
      }
      return 0;
    }

    inline std::vector<int>    bank::getInt(const char *name) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      std::vector<int> row;

      int nrows = getRows();
      for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
         switch(type){
             case kByte:  row.push_back((int) getByteAt(offset)); break;
             case kShort: row.push_back((int) getShortAt(offset)); break;
             case kInt:   row.push_back((int) getIntAt(offset)); break;
             default: printf("---> error : requested INT for [%s] type = %d\n",name,type); break;
         }
      }
      return row;
    }

    inline int    bank::getShort(const char *name, int index) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      switch(type){
      case kByte:  return (int) getByteAt(offset);
      case kShort: return (int) getShortAt(offset);
      default: printf("---> error : requested SHORT for [%s] type = %d\n",
		      bankSchema.getEntryName(item).c_str(),type); break;
      }
      return 0;
    }
    inline int    bank::getByte(const char *name, int index) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      switch(type){
      case kByte: return (int) getByteAt(offset);
      default: printf("---> error : requested BYTE for [%s] type = %d\n",
		      bankSchema.getEntryName(item).c_str(),type); break;
      }
      return 0;
    }

    inline float  bank::getFloat(const char *name, int index) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      if(bankSchema.getEntryType(item)==kFloat){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getFloatAt(offset);
      }
      return 0.0;
    }

  inline std::vector<float>  bank::getFloat(const char *name) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      std::vector<float> row;
      int nrows = getRows();
      if(bankSchema.getEntryType(item)==kFloat){
        for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
          row.push_back( getFloatAt(offset));
        }
      }
      return row;
    }

    inline std::vector<float>  bank::getFloat(int item) const noexcept{
      ensureColumn(item);
      std::vector<float> row;
      int nrows = getRows();
      if(bankSchema.getEntryType(item)==kFloat){
        for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
          row.push_back( getFloatAt(offset));
        }
      }
      return row;
    }
    inline double  bank::getDouble(const char *name, int index) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      if(bankSchema.getEntryType(item)==kDouble){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getDoubleAt(offset);
      }
      if(bankSchema.getEntryType(item)==kFloat){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return (double) getFloatAt(offset);
      }
      return 0.0;
    }

    inline std::vector<double>  bank::getDouble(int item) const noexcept{
      ensureColumn(item);
      std::vector<double> row;
      int nrows = getRows();

      if(bankSchema.getEntryType(item)==kDouble){
        for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
          row.push_back(getDoubleAt(offset));
        }
      }
      if(bankSchema.getEntryType(item)==kFloat){
        for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
          row.push_back((double) getFloatAt(offset));
        }
      }
      return row;
    }

    inline std::vector<double>  bank::getDouble(const char *name) const noexcept{
      std::vector<double> row;
      int nrows = getRows();
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);

      if(bankSchema.getEntryType(item)==kDouble){
        for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
          row.push_back(getDoubleAt(offset));
        }
      }
      if(bankSchema.getEntryType(item)==kFloat){
        for(int j = 0; j < nrows; j++){
          int offset = bankSchema.getOffset(item, j, bankRows);
          row.push_back((double) getFloatAt(offset));
        }
      }
      return row;
    }

    inline long bank::getLong(const char *name, int index) const noexcept{
      int item = bankSchema.getEntryOrder(name);
      ensureColumn(item);
      if(bankSchema.getEntryType(item)==kLong){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getLongAt(offset);
      }
      return 0;
    }
  inline void    bank::putInt(int item, int index, int32_t value){
      if(bankSchema.getEntryArrayLength(item) != 1)
        throw hipo::length_mismatch_error("bank::putInt called on array column '" +
          bankSchema.getEntryName(item) + "' — use putIntArray instead");
      int offset = bankSchema.getOffset(item, index, bankRows);
      putIntAt(offset,value);
    }
    inline void    bank::putShort(int item, int index, int16_t value){
      if(bankSchema.getEntryArrayLength(item) != 1)
        throw hipo::length_mismatch_error("bank::putShort called on array column '" +
          bankSchema.getEntryName(item) + "' — use putShortArray instead");
      int offset = bankSchema.getOffset(item, index, bankRows);
      putShortAt(offset,value);
    }
    inline void    bank::putByte(int item, int index, int8_t value){
      if(bankSchema.getEntryArrayLength(item) != 1)
        throw hipo::length_mismatch_error("bank::putByte called on array column '" +
          bankSchema.getEntryName(item) + "' — use putByteArray instead");
      int offset = bankSchema.getOffset(item, index, bankRows);
      putByteAt(offset,value);
    }
    inline  void    bank::putFloat(int item, int index, float value){
      if(bankSchema.getEntryArrayLength(item) != 1)
        throw hipo::length_mismatch_error("bank::putFloat called on array column '" +
          bankSchema.getEntryName(item) + "' — use putFloatArray instead");
      int offset = bankSchema.getOffset(item, index, bankRows);
      putFloatAt(offset,value);
    }
    inline void    bank::putDouble(int item, int index, double value){
      if(bankSchema.getEntryArrayLength(item) != 1)
        throw hipo::length_mismatch_error("bank::putDouble called on array column '" +
          bankSchema.getEntryName(item) + "' — use putDoubleArray instead");
      int offset = bankSchema.getOffset(item, index, bankRows);
      putDoubleAt(offset,value);
    }
    inline void    bank::putLong(int item, int index, int64_t value){
      if(bankSchema.getEntryArrayLength(item) != 1)
        throw hipo::length_mismatch_error("bank::putLong called on array column '" +
          bankSchema.getEntryName(item) + "' — use putLongArray instead");
      int offset = bankSchema.getOffset(item, index, bankRows);
      putLongAt(offset,value);
    }

    using banklist=std::vector<bank>;

    // @returns the index of the bank in `banklist` `banks` which has the name `bankName`; if there is more
    // than one, only the index of the first such bank will be returned. A runtime exception is thrown
    // if the bank is not found
    // @param banks the `banklist` to search
    // @param bankName the bank name
    banklist::size_type getBanklistIndex(banklist& banks, std::string const& bankName) noexcept(false);

}
#endif /* BANK_H */
