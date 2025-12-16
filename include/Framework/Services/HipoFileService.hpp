#pragma once

#include <fmt/format.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "hipo4/reader.h"
#include "hipo4/writer.h"
#include "hipo4/dictionary.h"

namespace Framework {

/**
 * @brief Service for managing HIPO file I/O
 *
 * This service provides centralized HIPO file management for:
 * - Reading input HIPO files
 * - Writing output HIPO files with filtered/processed events
 * - Managing HIPO dictionaries and schemas
 *
 * Usage:
 * @code
 * auto hipo_svc = std::make_unique<HipoFileService>();
 * hipo_svc->add_input_file("data/run_5032.hipo");
 * hipo_svc->set_output_file("filtered_events.hipo");
 * app.service_manager().add_service(std::move(hipo_svc));
 * @endcode
 */
class HipoFileService : public Service {
public:
    explicit HipoFileService(std::string name = "HipoFileService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        if (m_log) {
            m_log->info(name(), "Initializing HipoFileService");
            m_log->info(name(), "  Input files: {}", m_input_files.size());
            if (!m_output_file.empty()) {
                m_log->info(name(), "  Output file: {}", m_output_file);
            }
        }

        // Open output file if specified
        if (!m_output_file.empty() && m_enable_output) {
            if (!open_output_file()) {
                return StatusCode::FAILURE();
            }
        }

        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        // Close output file
        if (m_writer) {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            m_writer->close();
            m_writer.reset();

            if (m_log) {
                m_log->info(name(), "Wrote {} events to {}", m_events_written, m_output_file);
            }
        }

        if (m_log) {
            m_log->info(name(), "Finalized - {} events read, {} events written",
                       m_events_read, m_events_written);
        }

        return StatusCode::SUCCESS();
    }

    // ========== Configuration ==========

    /**
     * @brief Set logging service
     */
    auto set_logger(LoggingService* log) -> void { m_log = log; }

    /**
     * @brief Add an input HIPO file
     */
    auto add_input_file(const std::string& filename) -> void {
        m_input_files.push_back(filename);
    }

    /**
     * @brief Set input files from a vector
     */
    auto set_input_files(std::vector<std::string> files) -> void {
        m_input_files = std::move(files);
    }

    /**
     * @brief Get input files
     */
    auto get_input_files() const -> const std::vector<std::string>& {
        return m_input_files;
    }

    /**
     * @brief Set output HIPO file for writing filtered events
     */
    auto set_output_file(const std::string& filename) -> void {
        m_output_file = filename;
    }

    /**
     * @brief Get output file path
     */
    auto get_output_file() const -> const std::string& {
        return m_output_file;
    }

    /**
     * @brief Enable/disable output writing
     */
    auto set_output_enabled(bool enable) -> void {
        m_enable_output = enable;
    }

    /**
     * @brief Check if output is enabled
     */
    auto is_output_enabled() const -> bool {
        return m_enable_output && !m_output_file.empty();
    }

    // ========== File Operations ==========

    /**
     * @brief Open a HIPO file for reading
     * @param filename Path to HIPO file
     * @return Unique pointer to reader, or nullptr on failure
     */
    auto open_reader(const std::string& filename) -> std::unique_ptr<hipo::reader> {
        auto reader = std::make_unique<hipo::reader>();
        reader->open(filename.c_str());
        
        if (m_log) {
            m_log->debug(name(), "Opened reader for: {}", filename);
        }
        
        return reader;
    }

    /**
     * @brief Write an event to the output file
     * @param event The HIPO event to write
     * @return true if successful
     */
    auto write_event(hipo::event& event) -> bool {
        if (!m_writer || !m_enable_output) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_write_mutex);
        m_writer->addEvent(event);
        m_events_written++;
        return true;
    }

    /**
     * @brief Get the dictionary from the first input file
     */
    auto get_dictionary() -> hipo::dictionary* {
        if (m_input_files.empty()) {
            return nullptr;
        }

        if (!m_dictionary) {
            auto reader = open_reader(m_input_files[0]);
            m_dictionary = std::make_unique<hipo::dictionary>();
            reader->readDictionary(*m_dictionary);
        }

        return m_dictionary.get();
    }

    // ========== Statistics ==========

    /**
     * @brief Get number of events read
     */
    auto events_read() const -> size_t { return m_events_read; }

    /**
     * @brief Get number of events written
     */
    auto events_written() const -> size_t { return m_events_written; }

    /**
     * @brief Increment events read counter (called by Application)
     */
    auto increment_events_read() -> void { m_events_read++; }

    /**
     * @brief Get total number of input files
     */
    auto num_input_files() const -> size_t { return m_input_files.size(); }

private:
    std::vector<std::string> m_input_files;
    std::string m_output_file;
    bool m_enable_output = true;

    std::unique_ptr<hipo::writer> m_writer;
    std::unique_ptr<hipo::dictionary> m_dictionary;
    std::mutex m_write_mutex;

    size_t m_events_read = 0;
    size_t m_events_written = 0;

    LoggingService* m_log = nullptr;

    auto open_output_file() -> bool {
        try {
            m_writer = std::make_unique<hipo::writer>();
            
            // Copy dictionary from first input file if available
            if (!m_input_files.empty()) {
                auto dict = get_dictionary();
                if (dict) {
                    auto schema_list = dict->getSchemaList();
                    for (const auto& schema_name : schema_list) {
                        m_writer->getDictionary().addSchema(dict->getSchema(schema_name.c_str()));
                    }
                }
            }
            
            m_writer->open(m_output_file.c_str());
            
            if (m_log) {
                m_log->info(name(), "Opened output file: {}", m_output_file);
            }
            return true;
        } catch (const std::exception& e) {
            if (m_log) {
                m_log->error(name(), "Failed to open output file: {}", e.what());
            }
            return false;
        }
    }
};

}  // namespace Framework
