/*
 * File:   hipoexceptions.h
 * Author: gavalian
 *
 * Created on April 11, 2017, 2:06 PM
 * Modernized for C++20
 */

#ifndef HIPOEXCEPTIONS_H
#define HIPOEXCEPTIONS_H

#include <exception>
#include <string>
#include <string_view>

namespace hipo {

/// Base exception class for all HIPO errors
class HipoException : public std::exception {
protected:
    std::string m_message;

public:
    explicit HipoException(std::string_view msg) : m_message(msg) {}

    [[nodiscard]] const char* what() const noexcept override {
        return m_message.c_str();
    }
};

/// Thrown when a HIPO file cannot be opened or has wrong format
class FileOpenError : public HipoException {
public:
    explicit FileOpenError(std::string_view filename)
        : HipoException(std::string("Failed to open HIPO file: ") + std::string(filename)) {}

    FileOpenError() : HipoException("Failed to open HIPO file") {}
};

/// Thrown when the file format is invalid
class InvalidFileFormat : public HipoException {
public:
    InvalidFileFormat() : HipoException("Invalid HIPO file format") {}

    explicit InvalidFileFormat(std::string_view details)
        : HipoException(std::string("Invalid HIPO file format: ") + std::string(details)) {}
};

/// Thrown when a record cannot be parsed
class RecordParseError : public HipoException {
public:
    RecordParseError() : HipoException("Record parsing error") {}

    explicit RecordParseError(std::string_view details)
        : HipoException(std::string("Record parsing error: ") + std::string(details)) {}
};

/// Thrown when a requested schema is not found in the dictionary
class SchemaNotFound : public HipoException {
public:
    explicit SchemaNotFound(std::string_view name)
        : HipoException(std::string("Schema not found: ") + std::string(name)) {}
};

/// Thrown when a requested column is not found in a bank
class ColumnNotFound : public HipoException {
public:
    ColumnNotFound(std::string_view bank_name, std::string_view column_name)
        : HipoException(std::string("Column '") + std::string(column_name) +
                        "' not found in bank '" + std::string(bank_name) + "'") {}

    explicit ColumnNotFound(std::string_view column_name)
        : HipoException(std::string("Column not found: ") + std::string(column_name)) {}
};

/// Thrown when an index is out of bounds
class IndexOutOfBounds : public HipoException {
public:
    IndexOutOfBounds(int index, int max)
        : HipoException(std::string("Index ") + std::to_string(index) +
                        " out of bounds (max: " + std::to_string(max) + ")") {}
};

}  // namespace hipo

// Legacy compatibility aliases (deprecated)
using HipoWrongFile = hipo::InvalidFileFormat;
using HipoRecordError = hipo::RecordParseError;

#endif /* HIPOEXCEPTIONS_H */
