#pragma once

#include <algorithm>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

// JSON parsing (using nlohmann/json if available, otherwise simple parser)
#ifdef CLAS12_HAS_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace clas12::framework {

/**
 * @brief Information about a single run
 */
struct RunInfo {
    int run_number = 0;            ///< Run number
    bool is_good = true;           ///< Overall quality flag
    double luminosity = 0.0;       ///< Integrated luminosity [nb^-1] or [fb^-1]
    double beam_energy = 10.6;     ///< Beam energy [GeV]
    std::string target;            ///< Target type (e.g., "LD2", "Cu")
    std::string polarity;          ///< Torus polarity ("inbending", "outbending")
    std::set<std::string> flags;   ///< Quality flags (e.g., "beam_stable", "torus_ok")
    std::string comment;           ///< Optional comment

    /// Check if run has a specific flag
    [[nodiscard]] auto has_flag(const std::string& flag) const -> bool {
        return flags.find(flag) != flags.end();
    }

    /// Check if run has all specified flags
    [[nodiscard]] auto has_all_flags(const std::set<std::string>& required_flags) const -> bool {
        for (const auto& flag : required_flags) {
            if (flags.find(flag) == flags.end()) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief Service for managing good run lists and data quality filtering
 *
 * GoodRunListService provides functionality for:
 * - Loading run quality information from JSON or CSV files
 * - Filtering events by run quality criteria
 * - Luminosity-weighted normalization
 * - Run range selection
 *
 * This follows the GoodRunsList pattern used in ATLAS, CMS, and other LHC experiments.
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_grl_svc = &require_service<GoodRunListService>("GoodRunListService");
 *
 * // In algorithm process():
 * int run_number = event.run_number();
 * if (!m_grl_svc->is_good_run(run_number)) {
 *     return StatusCode::FILTERED();
 * }
 *
 * // Get luminosity weight
 * double lumi_weight = m_grl_svc->luminosity_weight(run_number);
 * h_mass->Fill(mass, lumi_weight);
 * @endcode
 *
 * JSON file format:
 * @code
 * {
 *   "runs": [
 *     {
 *       "run_number": 16042,
 *       "is_good": true,
 *       "luminosity": 1.234,
 *       "beam_energy": 10.6,
 *       "target": "LD2",
 *       "polarity": "inbending",
 *       "flags": ["beam_stable", "torus_ok", "solenoid_ok"]
 *     }
 *   ]
 * }
 * @endcode
 *
 * CSV file format:
 * @code
 * run_number,is_good,luminosity,beam_energy,target,polarity,flags
 * 16042,1,1.234,10.6,LD2,inbending,"beam_stable,torus_ok"
 * @endcode
 */
class GoodRunListService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "GoodRunListService"; }

    explicit GoodRunListService(std::string name = "GoodRunListService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        // Load GRL file if specified
        if (!m_grl_file.get().empty()) {
            auto sc = load_from_file(m_grl_file.get());
            if (sc.is_failure()) {
                Log::error(this->name(), "Failed to load GRL from: {}", m_grl_file.get());
                return sc;
            }
        }

        // Parse required flags from comma-separated string
        parse_required_flags(m_required_flags_str.get());

        Log::info(this->name(), "GoodRunListService initialized");
        Log::info(this->name(), "  Loaded {} runs, {} good",
                  m_run_map.size(), count_good_runs());
        Log::info(this->name(), "  Run range: [{}, {}]",
                  m_min_run.get(), m_max_run.get());
        if (!m_required_flags.empty()) {
            Log::info(this->name(), "  Required flags: {}",
                      fmt::join(m_required_flags, ", "));
        }
        Log::info(this->name(), "  Total luminosity: {:.3f}",
                  total_luminosity());

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "GoodRunListService finalized. "
                                "Events checked: {}, Good runs: {}, Bad runs: {}",
                  m_events_checked, m_good_events, m_bad_events);
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Path to good run list file (JSON or CSV)
    Property<std::string> m_grl_file{"grl_file", "", "Path to good run list file"};

    /// Required quality flags (comma-separated)
    Property<std::string> m_required_flags_str{"required_flags", "",
                                               "Required quality flags (comma-separated)"};

    /// Minimum run number (0 = no minimum)
    Property<int> m_min_run{"min_run", 0, "Minimum run number (0 = no limit)"};

    /// Maximum run number (0 = no maximum)
    Property<int> m_max_run{"max_run", 0, "Maximum run number (0 = no limit)"};

    /// Whether to accept unknown runs (not in GRL)
    Property<bool> m_accept_unknown{"accept_unknown", false,
                                    "Accept runs not in GRL as good"};

    /// Target filter (empty = all targets)
    Property<std::string> m_target_filter{"target", "", "Filter by target type"};

    /// Polarity filter (empty = all)
    Property<std::string> m_polarity_filter{"polarity", "", "Filter by torus polarity"};

    // ========================================================================
    // File Loading API
    // ========================================================================

    /**
     * @brief Load good run list from file (auto-detects format)
     *
     * @param filepath Path to GRL file (JSON or CSV)
     * @return StatusCode SUCCESS or FAILURE
     */
    auto load_from_file(const std::string& filepath) -> StatusCode {
        if (filepath.empty()) {
            return StatusCode::failure("Empty filepath");
        }

        // Detect format from extension
        if (filepath.ends_with(".json")) {
            return load_from_json(filepath);
        } else if (filepath.ends_with(".csv")) {
            return load_from_csv(filepath);
        } else {
            // Try JSON first, then CSV
            auto sc = load_from_json(filepath);
            if (sc.is_success()) return sc;
            return load_from_csv(filepath);
        }
    }

    /**
     * @brief Load from JSON file
     */
    auto load_from_json(const std::string& filepath) -> StatusCode {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return StatusCode::failure("Cannot open file: " + filepath);
        }

#ifdef CLAS12_HAS_NLOHMANN_JSON
        try {
            nlohmann::json j;
            file >> j;

            if (!j.contains("runs")) {
                return StatusCode::failure("JSON missing 'runs' array");
            }

            for (const auto& run_json : j["runs"]) {
                RunInfo info;
                info.run_number = run_json.value("run_number", 0);
                info.is_good = run_json.value("is_good", true);
                info.luminosity = run_json.value("luminosity", 0.0);
                info.beam_energy = run_json.value("beam_energy", 10.6);
                info.target = run_json.value("target", "");
                info.polarity = run_json.value("polarity", "");
                info.comment = run_json.value("comment", "");

                if (run_json.contains("flags")) {
                    for (const auto& flag : run_json["flags"]) {
                        info.flags.insert(flag.get<std::string>());
                    }
                }

                m_run_map[info.run_number] = info;
            }

            return StatusCode::SUCCESS();
        } catch (const std::exception& e) {
            return StatusCode::failure("JSON parse error: " + std::string(e.what()));
        }
#else
        // Simple JSON parser for basic format
        return parse_simple_json(file);
#endif
    }

    /**
     * @brief Load from CSV file
     */
    auto load_from_csv(const std::string& filepath) -> StatusCode {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return StatusCode::failure("Cannot open file: " + filepath);
        }

        std::string line;
        bool header_read = false;

        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;

            if (!header_read) {
                // Skip header line
                header_read = true;
                continue;
            }

            RunInfo info;
            if (parse_csv_line(line, info)) {
                m_run_map[info.run_number] = info;
            }
        }

        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Run Query API
    // ========================================================================

    /**
     * @brief Check if a run is good
     *
     * A run is considered good if:
     * - It exists in the GRL and is_good is true (or accept_unknown is true)
     * - It falls within the specified run range
     * - It has all required quality flags
     * - It matches target/polarity filters (if set)
     *
     * @param run_number Run number to check
     * @return True if run passes all criteria
     */
    [[nodiscard]] auto is_good_run(int run_number) const -> bool {
        m_events_checked++;

        // Check run range
        if (m_min_run.get() > 0 && run_number < m_min_run.get()) {
            m_bad_events++;
            return false;
        }
        if (m_max_run.get() > 0 && run_number > m_max_run.get()) {
            m_bad_events++;
            return false;
        }

        // Look up run info
        auto it = m_run_map.find(run_number);
        if (it == m_run_map.end()) {
            // Run not in GRL
            if (m_accept_unknown.get()) {
                m_good_events++;
                return true;
            }
            m_bad_events++;
            return false;
        }

        const auto& info = it->second;

        // Check basic is_good flag
        if (!info.is_good) {
            m_bad_events++;
            return false;
        }

        // Check required flags
        if (!m_required_flags.empty() && !info.has_all_flags(m_required_flags)) {
            m_bad_events++;
            return false;
        }

        // Check target filter
        if (!m_target_filter.get().empty() && info.target != m_target_filter.get()) {
            m_bad_events++;
            return false;
        }

        // Check polarity filter
        if (!m_polarity_filter.get().empty() && info.polarity != m_polarity_filter.get()) {
            m_bad_events++;
            return false;
        }

        m_good_events++;
        return true;
    }

    /**
     * @brief Get run information
     *
     * @param run_number Run number
     * @return RunInfo if found, nullopt otherwise
     */
    [[nodiscard]] auto get_run_info(int run_number) const -> std::optional<RunInfo> {
        auto it = m_run_map.find(run_number);
        if (it != m_run_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Get all run numbers in GRL
     */
    [[nodiscard]] auto all_runs() const -> std::vector<int> {
        std::vector<int> runs;
        runs.reserve(m_run_map.size());
        for (const auto& [run, info] : m_run_map) {
            runs.push_back(run);
        }
        std::sort(runs.begin(), runs.end());
        return runs;
    }

    /**
     * @brief Get all good run numbers
     */
    [[nodiscard]] auto good_runs() const -> std::vector<int> {
        std::vector<int> runs;
        for (const auto& [run, info] : m_run_map) {
            if (info.is_good) {
                runs.push_back(run);
            }
        }
        std::sort(runs.begin(), runs.end());
        return runs;
    }

    // ========================================================================
    // Luminosity API
    // ========================================================================

    /**
     * @brief Get total integrated luminosity of all good runs
     */
    [[nodiscard]] auto total_luminosity() const -> double {
        double total = 0.0;
        for (const auto& [run, info] : m_run_map) {
            if (info.is_good && passes_filters(info)) {
                total += info.luminosity;
            }
        }
        return total;
    }

    /**
     * @brief Get luminosity for a specific run
     *
     * @param run_number Run number
     * @return Luminosity value, or 0 if run not found
     */
    [[nodiscard]] auto luminosity(int run_number) const -> double {
        auto it = m_run_map.find(run_number);
        if (it != m_run_map.end()) {
            return it->second.luminosity;
        }
        return 0.0;
    }

    /**
     * @brief Get luminosity weight for normalization
     *
     * Returns lumi(run) / total_lumi so that sum of weights = 1
     *
     * @param run_number Run number
     * @return Normalized luminosity weight
     */
    [[nodiscard]] auto luminosity_weight(int run_number) const -> double {
        double total = total_luminosity();
        if (total <= 0.0) return 1.0;
        return luminosity(run_number) / total;
    }

    // ========================================================================
    // Filter Configuration
    // ========================================================================

    /**
     * @brief Add a required quality flag
     */
    auto require_flag(const std::string& flag) -> GoodRunListService& {
        m_required_flags.insert(flag);
        return *this;
    }

    /**
     * @brief Remove a required flag
     */
    auto remove_required_flag(const std::string& flag) -> GoodRunListService& {
        m_required_flags.erase(flag);
        return *this;
    }

    /**
     * @brief Set run range
     */
    auto set_run_range(int min_run, int max_run) -> GoodRunListService& {
        m_min_run.set(min_run);
        m_max_run.set(max_run);
        return *this;
    }

    /**
     * @brief Exclude specific runs
     */
    auto exclude_runs(const std::vector<int>& runs) -> GoodRunListService& {
        for (int run : runs) {
            m_excluded_runs.insert(run);
        }
        return *this;
    }

    // ========================================================================
    // Manual Run Management
    // ========================================================================

    /**
     * @brief Add or update run info
     */
    auto add_run(const RunInfo& info) -> void {
        m_run_map[info.run_number] = info;
    }

    /**
     * @brief Mark a run as good/bad
     */
    auto set_run_quality(int run_number, bool is_good) -> void {
        auto it = m_run_map.find(run_number);
        if (it != m_run_map.end()) {
            it->second.is_good = is_good;
        } else {
            RunInfo info;
            info.run_number = run_number;
            info.is_good = is_good;
            m_run_map[run_number] = info;
        }
    }

    /**
     * @brief Clear all run info
     */
    auto clear() -> void {
        m_run_map.clear();
        m_required_flags.clear();
        m_excluded_runs.clear();
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    [[nodiscard]] auto num_runs() const -> size_t { return m_run_map.size(); }
    [[nodiscard]] auto events_checked() const -> size_t { return m_events_checked; }
    [[nodiscard]] auto good_events() const -> size_t { return m_good_events; }
    [[nodiscard]] auto bad_events() const -> size_t { return m_bad_events; }

   private:
    std::map<int, RunInfo> m_run_map;
    std::set<std::string> m_required_flags;
    std::set<int> m_excluded_runs;

    // Statistics (mutable for const methods)
    mutable size_t m_events_checked = 0;
    mutable size_t m_good_events = 0;
    mutable size_t m_bad_events = 0;

    /**
     * @brief Count good runs in GRL
     */
    [[nodiscard]] auto count_good_runs() const -> size_t {
        size_t count = 0;
        for (const auto& [run, info] : m_run_map) {
            if (info.is_good) count++;
        }
        return count;
    }

    /**
     * @brief Check if run passes all filters
     */
    [[nodiscard]] auto passes_filters(const RunInfo& info) const -> bool {
        if (!m_target_filter.get().empty() && info.target != m_target_filter.get()) {
            return false;
        }
        if (!m_polarity_filter.get().empty() && info.polarity != m_polarity_filter.get()) {
            return false;
        }
        if (!m_required_flags.empty() && !info.has_all_flags(m_required_flags)) {
            return false;
        }
        return true;
    }

    /**
     * @brief Parse comma-separated flags string
     */
    auto parse_required_flags(const std::string& flags_str) -> void {
        if (flags_str.empty()) return;

        std::istringstream ss(flags_str);
        std::string flag;
        while (std::getline(ss, flag, ',')) {
            // Trim whitespace
            auto start = flag.find_first_not_of(" \t");
            auto end = flag.find_last_not_of(" \t");
            if (start != std::string::npos) {
                m_required_flags.insert(flag.substr(start, end - start + 1));
            }
        }
    }

    /**
     * @brief Parse a CSV line into RunInfo
     */
    auto parse_csv_line(const std::string& line, RunInfo& info) -> bool {
        std::istringstream ss(line);
        std::string field;
        int col = 0;

        while (std::getline(ss, field, ',')) {
            // Handle quoted fields
            if (!field.empty() && field[0] == '"') {
                field = field.substr(1);
                if (!field.empty() && field.back() == '"') {
                    field.pop_back();
                }
            }

            switch (col) {
                case 0: info.run_number = std::stoi(field); break;
                case 1: info.is_good = (field == "1" || field == "true"); break;
                case 2: info.luminosity = std::stod(field); break;
                case 3: info.beam_energy = std::stod(field); break;
                case 4: info.target = field; break;
                case 5: info.polarity = field; break;
                case 6: {
                    // Parse flags (comma-separated within quotes)
                    std::istringstream flag_ss(field);
                    std::string flag;
                    while (std::getline(flag_ss, flag, ',')) {
                        auto start = flag.find_first_not_of(" \t");
                        auto end = flag.find_last_not_of(" \t");
                        if (start != std::string::npos) {
                            info.flags.insert(flag.substr(start, end - start + 1));
                        }
                    }
                    break;
                }
                default: break;
            }
            col++;
        }

        return info.run_number > 0;
    }

    /**
     * @brief Simple JSON parser (fallback when nlohmann/json not available)
     */
    auto parse_simple_json(std::ifstream& file) -> StatusCode {
        // Basic JSON parsing for simple GRL format
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        // Find "runs" array
        size_t runs_pos = content.find("\"runs\"");
        if (runs_pos == std::string::npos) {
            return StatusCode::failure("JSON missing 'runs' array");
        }

        // Find array start
        size_t array_start = content.find('[', runs_pos);
        if (array_start == std::string::npos) {
            return StatusCode::failure("Invalid JSON: missing array");
        }

        // Parse each object in array
        size_t pos = array_start + 1;
        while (pos < content.size()) {
            size_t obj_start = content.find('{', pos);
            if (obj_start == std::string::npos) break;

            size_t obj_end = content.find('}', obj_start);
            if (obj_end == std::string::npos) break;

            std::string obj_str = content.substr(obj_start, obj_end - obj_start + 1);

            RunInfo info;
            info.run_number = extract_json_int(obj_str, "run_number");
            info.is_good = extract_json_bool(obj_str, "is_good", true);
            info.luminosity = extract_json_double(obj_str, "luminosity");
            info.beam_energy = extract_json_double(obj_str, "beam_energy", 10.6);
            info.target = extract_json_string(obj_str, "target");
            info.polarity = extract_json_string(obj_str, "polarity");

            if (info.run_number > 0) {
                m_run_map[info.run_number] = info;
            }

            pos = obj_end + 1;
        }

        return StatusCode::SUCCESS();
    }

    // Helper functions for simple JSON parsing
    static auto extract_json_int(const std::string& obj, const std::string& key) -> int {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return 0;
        pos = obj.find(':', pos);
        if (pos == std::string::npos) return 0;
        return std::atoi(obj.c_str() + pos + 1);
    }

    static auto extract_json_double(const std::string& obj, const std::string& key,
                                    double default_val = 0.0) -> double {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return default_val;
        pos = obj.find(':', pos);
        if (pos == std::string::npos) return default_val;
        return std::atof(obj.c_str() + pos + 1);
    }

    static auto extract_json_bool(const std::string& obj, const std::string& key,
                                  bool default_val = false) -> bool {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return default_val;
        pos = obj.find(':', pos);
        if (pos == std::string::npos) return default_val;
        size_t val_start = obj.find_first_not_of(" \t\n", pos + 1);
        return obj.substr(val_start, 4) == "true";
    }

    static auto extract_json_string(const std::string& obj, const std::string& key) -> std::string {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = obj.find(':', pos);
        if (pos == std::string::npos) return "";
        size_t str_start = obj.find('"', pos + 1);
        if (str_start == std::string::npos) return "";
        size_t str_end = obj.find('"', str_start + 1);
        if (str_end == std::string::npos) return "";
        return obj.substr(str_start + 1, str_end - str_start - 1);
    }
};

}  // namespace clas12::framework
