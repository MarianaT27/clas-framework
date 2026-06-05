#pragma once

#include <fmt/format.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <reader.h>
#include <writer.h>
#include <dictionary.h>
#include <event.h>
#include <bank.h>

namespace framework {

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
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "HipoFileService"; }

    explicit HipoFileService(std::string name = "HipoFileService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        Log::info(name(), "Initializing HipoFileService");
        Log::info(name(), "  Input files: {}", m_input_files.size());
        if (!m_output_file.empty()) Log::info(name(), "  Output file: {}", m_output_file);

        // Open output file if specified
        if (!m_output_file.empty() && m_enable_output) {
            if (!open_output_file()) return StatusCode::FAILURE();
        }

        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        // Close output file
        if (m_writer) {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            m_writer->close();  // new API: void return
            m_writer.reset();

            Log::info(name(), "Wrote {} events to {}", m_events_written, m_output_file);
        }

        Log::info(name(), "Finalized - {} events read, {} events written", m_events_read, m_events_written);

        return StatusCode::SUCCESS();
    }

    // ========== Configuration ==========

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
        auto r = std::make_unique<hipo::reader>();
        r->open(filename.c_str());
        if (!r->is_open()) {
            Log::error(name(), "Failed to open reader for: {}", filename);
            return nullptr;
        }
        Log::debug(name(), "Opened reader for: {}", filename);
        return r;
    }

    /**
     * @brief Write a HIPO event to the output file.
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
     * @brief Write raw event bytes to the output file.
     *
     * Callers receive the byte buffer from framework::Event::get_raw_event_buffer().
     * The hipo::writer::addEvent overload takes a mutable reference, so this
     * helper copies into a stack buffer to honor the const-correct interface.
     */
    auto write_event(const std::vector<char>& event_data) -> bool {
        if (!m_writer || !m_enable_output) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_write_mutex);
        std::vector<char> copy = event_data;
        m_writer->addEvent(copy);
        m_events_written++;
        return true;
    }

    /**
     * @brief Register a custom bank schema to include in the output dictionary.
     *
     * Call before the output file is opened (e.g. in an algorithm's
     * on_initialize). The schema is added to the writer's dictionary so banks
     * of that type can be serialized via write_event(hipo::event&). This lets a
     * skim write user-defined banks (e.g. PI0::event / PI0::photons) instead of
     * the raw-passthrough of all input banks.
     */
    auto register_output_schema(const hipo::schema& s) -> void {
        m_output_schemas.push_back(s);
    }

    /**
     * @brief Peek at the first input file's dictionary for a bank schema.
     *
     * Opens a throwaway hipo::reader on the first file and checks whether the
     * named bank is in its dictionary — without touching the service's own
     * reader/dictionary state. Used by analyses to auto-detect a slim skim
     * (PI0::event present) versus a full HIPO input. Returns false on an empty
     * file list or an unopenable file.
     */
    static auto input_has_bank(const std::vector<std::string>& files,
                               const std::string& bank_name) -> bool {
        if (files.empty()) return false;
        hipo::reader r;
        r.open(files.front().c_str());
        if (!r.is_open()) return false;
        hipo::dictionary dict;
        r.readDictionary(dict);
        return dict.hasSchema(bank_name.c_str());
    }

    /**
     * @brief Peek the run number from the first input file (RUN::config row 0).
     *
     * Opens a throwaway reader on the first file and returns the run number of
     * the first event that carries a RUN::config row — without touching the
     * service's own state. Used to resolve per-run RCDB conditions (target /
     * polarity / beam energy) before the event loop starts. Returns 0 on an
     * empty file list, an unopenable file, or a missing bank.
     */
    static auto peek_run_number(const std::vector<std::string>& files) -> int {
        if (files.empty()) return 0;
        hipo::reader r;
        r.open(files.front().c_str());
        if (!r.is_open()) return 0;
        hipo::dictionary dict;
        r.readDictionary(dict);
        if (!dict.hasSchema("RUN::config")) return 0;
        hipo::schema& sch = dict.getSchema("RUN::config");
        const int run_idx = sch.getEntryOrder("run");
        if (run_idx < 0) return 0;
        hipo::bank cfg(sch);
        hipo::event ev;
        // The first events of a HIPO file are often header/config records whose
        // RUN::config carries run = 0; scan until a real (nonzero) run appears.
        for (int guard = 0; r.next() && guard < 1000; ++guard) {
            r.read(ev);
            ev.getStructure(cfg);
            if (cfg.getRows() > 0) {
                const int run = cfg.getInt(run_idx, 0);
                if (run > 0) return run;
            }
        }
        return 0;
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
            if (!reader) return nullptr;
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
    std::vector<hipo::schema> m_output_schemas;
    std::mutex m_write_mutex;

    size_t m_events_read = 0;
    size_t m_events_written = 0;

    auto open_output_file() -> bool {
        try {
            m_writer = std::make_unique<hipo::writer>();

            // Two output modes:
            //  - Custom-bank output (registered schemas, e.g. the slim pi0
            //    skim): write a MINIMAL dictionary with only those schemas. The
            //    file contains only those banks, so copying the full input
            //    dictionary would just bloat every file's metadata.
            //  - Raw passthrough (no registered schemas): preserve the input
            //    dictionary so the original banks can be serialized/decoded.
            if (!m_output_schemas.empty()) {
                hipo::dictionary dict;
                for (const auto& s : m_output_schemas) dict.addSchema(s);
                m_writer->addDictionary(dict);
            } else if (!m_input_files.empty()) {
                if (auto* dict = get_dictionary()) {
                    m_writer->addDictionary(*dict);
                }
            }

            m_writer->open(m_output_file.c_str());  // new API: void return
            Log::info(name(), "Opened output file: {}", m_output_file);
            return true;
        } catch (const std::exception& e) {
            Log::error(name(), "Failed to open output file: {}", e.what());
            return false;
        }
    }
};

}  // namespace framework
