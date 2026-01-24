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
#include <concepts>
#include <string_view>
#include <iterator>
#include <algorithm>
#include "dictionary.h"
#include "node.h"

// Platform-specific terminal width detection
#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace hipo {

  // Forward declarations
  class bank;
  template<typename T> class ColumnAccessor;
  template<typename T> class ColumnView;

  class structure {

    private:
      
      std::vector<char> structureBuffer;
      char *structureAddress{};
      void setAddress(const char *address);

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

      virtual     ~structure()= default;
      bool         allocate(int size);
      
      int          getSize() const noexcept{
        uint32_t length;
        std::memcpy(&length, structureAddress+4, sizeof(uint32_t));
        return length&0x00ffffff;
      }

      int         getHeaderSize() const noexcept {
         uint32_t length;
         std::memcpy(&length, structureAddress+4, sizeof(uint32_t));
         return (length>>24)&0x000000ff;
      }
      
      int         getDataSize() const noexcept {         
	       return getSize()-getHeaderSize();
      }

      int          getType() const;
      int          getGroup() const;
      int          getItem() const;
      void         init(const char *buffer, int size);
      void         initNoCopy(const char *buffer, int size);

      const char    *getAddress();
      virtual void   show() const;
      void           setSize(int size);
      void           setHeaderSize(int size);
      void           setDataSize(int size);

      int          getIntAt   ( int index) const noexcept {
        int32_t value;
        std::memcpy(&value, &structureAddress[index+dataOffset], sizeof(int32_t));
        return value;
      }
    
      int16_t      getShortAt ( int index) const noexcept {
        int16_t value;
        std::memcpy(&value, &structureAddress[index+dataOffset], sizeof(int16_t));
        return value;
      }
      int8_t       getByteAt  ( int index) const noexcept {
        int8_t value;
        std::memcpy(&value, &structureAddress[index+dataOffset], sizeof(int8_t));
        return value;
      }
      float        getFloatAt ( int index) const noexcept {
        float value;
        std::memcpy(&value, &structureAddress[index+dataOffset], sizeof(float));
        return value;
      }
      double       getDoubleAt( int index) const noexcept {
        double value;
        std::memcpy(&value, &structureAddress[index+dataOffset], sizeof(double));
        return value;
      }
      long         getLongAt  ( int index) const noexcept {
        int64_t value;
        std::memcpy(&value, &structureAddress[index+dataOffset], sizeof(int64_t));
        return value;
      }

      std::string  getStringAt(int index);

      void         putIntAt(int index, int value){
        int32_t val = value;
        std::memcpy(&structureAddress[index+dataOffset], &val, sizeof(int32_t));
      }

      void         putShortAt(int index, int16_t value){
        std::memcpy(&structureAddress[index+dataOffset], &value, sizeof(int16_t));
      }

      void         putByteAt(int index, int8_t value){
        std::memcpy(&structureAddress[index+dataOffset], &value, sizeof(int8_t));
      }

      void         putFloatAt(int index, float value){
        std::memcpy(&structureAddress[index+dataOffset], &value, sizeof(float));
      }

      void         putDoubleAt(int index, double value){
        std::memcpy(&structureAddress[index+dataOffset], &value, sizeof(double));
      }

      void         putLongAt(int index, int64_t value){
        std::memcpy(&structureAddress[index+dataOffset], &value, sizeof(int64_t));
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

        /// @returns create a list of numbers from 0 to `num`
        /// @param num the size of the list
        static list_t createFullList(int num);

        /// @param ownerBank set the owner bank
        void setOwnerBank(bank* const ownerBank) { m_owner_bank = ownerBank; }

      private:
        bool m_init{false};
        list_t m_list{};
        bank* m_owner_bank{nullptr};

        static list_t s_number_list_init(list_t::size_type num = 500);
        static list_t s_number_list;

        bool ownerBankIsUnknown(std::string_view caller = "");

      };

    private:

      schema  bankSchema;
      int     bankRows{-1};
      rowlist bankRowList;

    public:

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

        ~bank() override;

        [[nodiscard]] schema& getSchema() noexcept { return bankSchema;}
        [[nodiscard]] const schema& getSchema() const noexcept { return bankSchema;}

        /// Get number of rows in the bank
        [[nodiscard]] int getRows() const noexcept { return bankRows; }
        /// Modern alias for getRows()
        [[nodiscard]] int rows() const noexcept { return bankRows; }

        void   setRows(int rows);

        [[nodiscard]] int    getInt(    int item, int index) const noexcept;
        [[nodiscard]] int    getShort(  int item, int index) const noexcept;
        [[nodiscard]] int    getByte(   int item, int index) const noexcept;
        [[nodiscard]] float  getFloat(  int item, int index) const noexcept;
        [[nodiscard]] double getDouble( int item, int index) const noexcept;
        [[nodiscard]] long   getLong(   int item, int index) const noexcept;

        [[nodiscard]] std::vector<int>    getInt(    int item) const noexcept;
        [[nodiscard]] std::vector<float>  getFloat(  int item) const noexcept;
        [[nodiscard]] std::vector<double> getDouble( int item) const noexcept;

        template<typename T = double>
        [[nodiscard]] T get(int item, int index) const noexcept {
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

        template<typename T = double>
        [[nodiscard]] T get(int item, int index, int type, int offset) const noexcept {
          switch(type) {
            case kByte:   return getByteAt(offset);
            case kShort:  return getShortAt(offset);
            case kInt:    return getIntAt(offset);
            case kFloat:  return getFloatAt(offset);
            case kDouble: return getDoubleAt(offset);
            case kLong:   return getLongAt(offset);
            default:
              printf("---> error(get) : unknown type for [%s] type = %d\n", bankSchema.getEntryName(item).c_str(), type);
          }
          return 0;
        }

        [[nodiscard]] int    getInt(    const char *name, int index) const noexcept;
        [[nodiscard]] int    getShort(  const char *name, int index) const noexcept;
        [[nodiscard]] int    getByte(   const char *name, int index) const noexcept;
        [[nodiscard]] float  getFloat(  const char *name, int index) const noexcept;
        [[nodiscard]] double getDouble( const char *name, int index) const noexcept;
        [[nodiscard]] long   getLong(   const char *name, int index) const noexcept;

        [[nodiscard]] std::vector<int>    getInt(    const char *name) const noexcept;
        [[nodiscard]] std::vector<float>  getFloat(  const char *name) const noexcept;
        [[nodiscard]] std::vector<double> getDouble( const char *name) const noexcept;

        template<typename T = double>
        [[nodiscard]] T get(const char *name, int index) const noexcept {
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

        void reset();

        void notify() override;

        /// Create a high-performance column accessor for tight loops.
        /// Pre-computes offset information to avoid repeated string lookups.
        /// @param column_index the column index (use schema.getEntryOrder() once)
        /// @returns ColumnAccessor for direct indexed access
        template<typename T>
        [[nodiscard]] ColumnAccessor<T> column(int column_index) const noexcept {
            return ColumnAccessor<T>(this, column_index);
        }

        /// Create a high-performance column accessor by name.
        /// @param name the column name (lookup done once at creation)
        /// @returns ColumnAccessor for direct indexed access
        template<typename T>
        [[nodiscard]] ColumnAccessor<T> column(std::string_view name) const {
            auto idx = bankSchema.getEntryOrderOpt(name);
            if (!idx) {
                throw ColumnNotFound(bankSchema.getName(), name);
            }
            return ColumnAccessor<T>(this, *idx);
        }

        /// Create a zero-copy view over a column for iteration.
        /// @param column_index the column index
        /// @returns ColumnView providing iterator access without allocation
        template<typename T>
        [[nodiscard]] ColumnView<T> view(int column_index) const noexcept {
            return ColumnView<T>(this, column_index);
        }

        /// Create a zero-copy view over a column by name.
        /// @param name the column name
        /// @returns ColumnView providing iterator access without allocation
        template<typename T>
        [[nodiscard]] ColumnView<T> view(std::string_view name) const {
            auto idx = bankSchema.getEntryOrderOpt(name);
            if (!idx) {
                throw ColumnNotFound(bankSchema.getName(), name);
            }
            return ColumnView<T>(this, *idx);
        }

        // =========================================================================
        // Bank Metadata
        // =========================================================================

        /// Get the bank/schema name
        [[nodiscard]] std::string name() const { return bankSchema.getName(); }

        /// Get number of columns in the bank
        [[nodiscard]] int columns() const noexcept { return bankSchema.getEntries(); }

        /// Check if bank has any data
        [[nodiscard]] bool empty() const noexcept { return bankRows == 0; }

        // =========================================================================
        // Pretty Printing
        // =========================================================================

        /// Print bank contents as a formatted table
        /// @param max_rows Maximum rows to display (-1 for all)
        void print(int max_rows = -1) const { print_table(false, max_rows); }

        /// Print all bank contents (ignoring row filters)
        /// @param max_rows Maximum rows to display (-1 for all)
        void print_all(int max_rows = -1) const { print_table(true, max_rows); }

    private:
        // =========================================================================
        // Pretty Printing Helpers
        // =========================================================================

        [[nodiscard]] static int detect_terminal_width() {
            if (const char* env = std::getenv("COLUMNS")) {
                if (int w = std::atoi(env); w > 0) return w;
            }
#ifdef TIOCGWINSZ
            struct winsize ws{};
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
                return ws.ws_col;
            }
#endif
            return 120;
        }

        [[nodiscard]] std::string type_name(int column) const {
            switch (bankSchema.getEntryType(column)) {
                case kByte: return "byte";
                case kShort: return "short";
                case kInt: return "int";
                case kFloat: return "float";
                case kDouble: return "double";
                case kLong: return "long";
                default: return "?";
            }
        }

        [[nodiscard]] int column_width(int column) const {
            const auto name_len = static_cast<int>(bankSchema.getEntryName(column).length());
            switch (bankSchema.getEntryType(column)) {
                case kByte: return std::max(name_len, 4);
                case kShort: return std::max(name_len, 6);
                case kInt: return std::max(name_len, 8);
                case kFloat: return std::max(name_len, 9);
                case kDouble: return std::max(name_len, 9);
                case kLong: return std::max(name_len, 12);
                default: return name_len;
            }
        }

        void print_value_formatted(int column, int row, int width) const {
            switch (bankSchema.getEntryType(column)) {
                case kByte: std::printf("%*d ", width, get<int8_t>(column, row)); break;
                case kShort: std::printf("%*d ", width, get<int16_t>(column, row)); break;
                case kInt: std::printf("%*d ", width, get<int32_t>(column, row)); break;
                case kFloat: std::printf("%*.3f ", width, get<float>(column, row)); break;
                case kDouble: std::printf("%*.3f ", width, get<double>(column, row)); break;
                case kLong: std::printf("%*lld ", width, static_cast<long long>(get<int64_t>(column, row))); break;
                default: std::printf("%*s ", width, "?");
            }
        }

        void print_border(const std::vector<int>& widths, const char* left, const char* mid,
                          const char* right, const char* fill) const {
            std::printf("%s", left);
            for (int k = 0; k < 6; ++k) std::printf("%s", fill);
            std::printf("%s", mid);
            for (size_t i = 0; i < widths.size(); ++i) {
                for (int j = 0; j < widths[i] + 2; ++j) std::printf("%s", fill);
                std::printf("%s", i < widths.size() - 1 ? mid : right);
            }
            std::printf("\n");
        }

        void print_table(bool show_all, int max_rows) const;

  };

  //=============================================================================
  // ColumnAccessor - High-performance pre-computed column access
  //=============================================================================
  /// Provides O(1) indexed access to bank columns without repeated string lookups.
  /// Use this in tight loops where you access the same column many times.
  ///
  /// Example:
  /// @code
  /// auto px = particles.column<float>("px");
  /// auto py = particles.column<float>("py");
  /// for (int i = 0; i < particles.getRows(); ++i) {
  ///     float momentum = std::sqrt(px[i]*px[i] + py[i]*py[i]);
  /// }
  /// @endcode
  template<typename T>
  class ColumnAccessor {
  private:
      const bank* m_bank;
      int m_column_index;
      int m_base_offset;   // Pre-computed: schema entry offset
      int m_type_size;     // Pre-computed: size of type
      int m_type;          // Column type for validation

  public:
      ColumnAccessor(const bank* b, int col_idx) noexcept
          : m_bank(b), m_column_index(col_idx) {
          auto [base_off, type_sz] = b->getSchema().getOffsetParts(col_idx);
          m_base_offset = base_off;
          m_type_size = type_sz;
          m_type = b->getSchema().getEntryType(col_idx);
      }

      /// Access element at row index - O(1) operation
      [[nodiscard]] T operator[](int row) const noexcept {
          int offset = m_bank->getRows() * m_base_offset + row * m_type_size;
          return m_bank->template get<T>(m_column_index, row, m_type, offset);
      }

      /// Get the number of rows
      [[nodiscard]] int size() const noexcept { return m_bank->getRows(); }

      /// Get the column index
      [[nodiscard]] int columnIndex() const noexcept { return m_column_index; }
  };

  //=============================================================================
  // ColumnView - Zero-copy iterable view over a column
  //=============================================================================
  /// Provides iterator-based access to bank columns without vector allocation.
  /// Use this when you need to iterate over all values in a column.
  ///
  /// Example:
  /// @code
  /// for (float px : particles.view<float>("px")) {
  ///     // process px
  /// }
  /// @endcode
  template<typename T>
  class ColumnView {
  private:
      const bank* m_bank;
      int m_column_index;
      int m_base_offset;
      int m_type_size;
      int m_type;
      int m_rows;

  public:
      class iterator {
      public:
          using iterator_category = std::random_access_iterator_tag;
          using value_type = T;
          using difference_type = std::ptrdiff_t;
          using pointer = const T*;
          using reference = T;  // Returns by value since we compute on-the-fly

      private:
          const ColumnView* m_view;
          int m_row;

      public:
          iterator(const ColumnView* view, int row) noexcept
              : m_view(view), m_row(row) {}

          [[nodiscard]] T operator*() const noexcept {
              return (*m_view)[m_row];
          }

          iterator& operator++() noexcept { ++m_row; return *this; }
          iterator operator++(int) noexcept { auto tmp = *this; ++m_row; return tmp; }
          iterator& operator--() noexcept { --m_row; return *this; }
          iterator operator--(int) noexcept { auto tmp = *this; --m_row; return tmp; }

          iterator& operator+=(difference_type n) noexcept { m_row += n; return *this; }
          iterator& operator-=(difference_type n) noexcept { m_row -= n; return *this; }

          [[nodiscard]] iterator operator+(difference_type n) const noexcept {
              return iterator(m_view, m_row + n);
          }
          [[nodiscard]] iterator operator-(difference_type n) const noexcept {
              return iterator(m_view, m_row - n);
          }
          [[nodiscard]] difference_type operator-(const iterator& other) const noexcept {
              return m_row - other.m_row;
          }

          [[nodiscard]] T operator[](difference_type n) const noexcept {
              return (*m_view)[m_row + n];
          }

          [[nodiscard]] bool operator==(const iterator& other) const noexcept {
              return m_row == other.m_row;
          }
          [[nodiscard]] bool operator!=(const iterator& other) const noexcept {
              return m_row != other.m_row;
          }
          [[nodiscard]] bool operator<(const iterator& other) const noexcept {
              return m_row < other.m_row;
          }
          [[nodiscard]] bool operator<=(const iterator& other) const noexcept {
              return m_row <= other.m_row;
          }
          [[nodiscard]] bool operator>(const iterator& other) const noexcept {
              return m_row > other.m_row;
          }
          [[nodiscard]] bool operator>=(const iterator& other) const noexcept {
              return m_row >= other.m_row;
          }
      };

      ColumnView(const bank* b, int col_idx) noexcept
          : m_bank(b), m_column_index(col_idx), m_rows(b->getRows()) {
          auto [base_off, type_sz] = b->getSchema().getOffsetParts(col_idx);
          m_base_offset = base_off;
          m_type_size = type_sz;
          m_type = b->getSchema().getEntryType(col_idx);
      }

      /// Access element at row index
      [[nodiscard]] T operator[](int row) const noexcept {
          int offset = m_rows * m_base_offset + row * m_type_size;
          return m_bank->template get<T>(m_column_index, row, m_type, offset);
      }

      [[nodiscard]] iterator begin() const noexcept { return iterator(this, 0); }
      [[nodiscard]] iterator end() const noexcept { return iterator(this, m_rows); }

      [[nodiscard]] int size() const noexcept { return m_rows; }
      [[nodiscard]] bool empty() const noexcept { return m_rows == 0; }

      [[nodiscard]] T front() const noexcept { return (*this)[0]; }
      [[nodiscard]] T back() const noexcept { return (*this)[m_rows - 1]; }
  };

    /////////////////////////////////////
    //inlined getters

    inline float  bank::getFloat(int item, int index) const noexcept{
      if(bankSchema.getEntryType(item)==kFloat){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getFloatAt(offset);
      }
      return 0.0;
    }

    inline double  bank::getDouble(int item, int index) const noexcept{
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
      if(bankSchema.getEntryType(item)==kLong){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getLongAt(offset);
      }
      return 0;
    }

    inline int    bank::getInt(int item, int index) const noexcept{
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
      if(bankSchema.getEntryType(item)==kFloat){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getFloatAt(offset);
      }
      return 0.0;
    }

  inline std::vector<float>  bank::getFloat(const char *name) const noexcept{
      int item = bankSchema.getEntryOrder(name);
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
      if(bankSchema.getEntryType(item)==kLong){
        int offset = bankSchema.getOffset(item, index, bankRows);
        return getLongAt(offset);
      }
      return 0;
    }
  inline void    bank::putInt(int item, int index, int32_t value){
    //int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      putIntAt(offset,value);
    }
    inline void    bank::putShort(int item, int index, int16_t value){
      //int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      putShortAt(offset,value);
    }
    inline void    bank::putByte(int item, int index, int8_t value){
      //int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      putByteAt(offset,value);
    }
    inline  void    bank::putFloat(int item, int index, float value){
      //int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      //printf("---- put float %f at position = %d\n",value,offset);
      putFloatAt(offset,value);
    }
    inline void    bank::putDouble(int item, int index, double value){
      //int type = bankSchema.getEntryType(item);
      int offset = bankSchema.getOffset(item, index, bankRows);
      putDoubleAt(offset,value);
    }
    inline void    bank::putLong(int item, int index, int64_t value){
      //int type = bankSchema.getEntryType(item);
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
