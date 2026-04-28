#pragma once

#include <fmt/format.h>
#include <any>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <clas12/Target.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace framework {

/**
 * @brief Interval of Validity for calibration constants
 *
 * Represents the run range for which a set of calibration constants
 * is valid. This follows the IOV pattern used in ATLAS/CMS.
 */
struct IntervalOfValidity {
    int run_min;  // First valid run (inclusive)
    int run_max;  // Last valid run (inclusive)

    auto contains(int run) const -> bool {
        return run >= run_min && run <= run_max;
    }
};

/**
 * @brief Service for managing calibration constants and conditions data
 *
 * This service provides access to run-dependent calibrations such as:
 * - Sampling fraction parameters
 * - Time offsets
 * - Energy scale corrections
 * - Fiducial cut parameters
 *
 * Data is organized by IOV (Interval of Validity) and automatically
 * selected based on run number.
 */
class ConditionsService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "ConditionsService"; }

    explicit ConditionsService(std::string name = "ConditionsService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        // Initialize default conditions
        load_default_conditions();
        Log::info(name(), "Initialized");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        Log::info(name(), "Finalized");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set the current run number
     * @param run Run number
     */
    auto set_run_number(int run) -> void {
        m_current_run = run;
        Log::info(name(), "Set run number to {}", run);
    }

    /**
     * @brief Get the current run number
     * @return int Run number
     */
    auto run_number() const -> int {
        return m_current_run;
    }

    /**
     * @brief Get sampling fraction parameters for current run
     * @param target Target type
     * @param polarity Beam polarity
     * @param sector Detector sector (1-6)
     * @return std::vector<double> SF polynomial parameters
     */
    auto get_sampling_fraction_params(Target target,
                                      Polarity polarity,
                                      int sector) -> std::vector<double> {
        // For now, return empty vector - would be populated from conditions DB
        // In a full implementation, this would query by IOV and return
        // the polynomial coefficients for sampling fraction cuts
        (void)target;
        (void)polarity;
        (void)sector;  // Suppress unused warnings
        return std::vector<double>{};
    }

    /**
     * @brief Get time offset for a detector
     * @param detector_name Detector name (e.g., "FTOF", "ECAL")
     * @param sector Sector number
     * @return double Time offset in ns
     */
    auto get_time_offset(const std::string& detector_name, int sector) -> double {
        std::string key = fmt::format("time_offset_{}_{}", detector_name, sector);
        return get_value<double>(key, 0.0);
    }

    /**
     * @brief Set a calibration constant
     * @tparam T Value type
     * @param key Calibration key
     * @param value Value to store
     * @param iov Interval of validity
     */
    template <typename T>
    auto set_value(const std::string& key, T value, IntervalOfValidity iov) -> void {
        ConditionData data;
        data.value = value;
        data.iov = iov;
        m_conditions[key] = data;
    }

    /**
     * @brief Get a calibration constant for current run
     * @tparam T Value type
     * @param key Calibration key
     * @param default_value Default if not found
     * @return T The calibration value
     */
    template <typename T>
    auto get_value(const std::string& key, T default_value = T{}) -> T {
        auto it = m_conditions.find(key);
        if (it == m_conditions.end()) {
            return default_value;
        }

        // Check if IOV is valid for current run
        if (!it->second.iov.contains(m_current_run)) {
            Log::warning(name(), "No valid IOV for '{}' at run {}", key, m_current_run);
            return default_value;
        }

        try {
            return std::any_cast<T>(it->second.value);
        } catch (const std::bad_any_cast&) {
            Log::error(name(), "Type mismatch for condition '{}'", key);
            return default_value;
        }
    }

   private:
    struct ConditionData {
        std::any value;
        IntervalOfValidity iov;
    };

    int m_current_run = 0;
    std::map<std::string, ConditionData> m_conditions;

    auto load_default_conditions() -> void {
        // Load default time offsets, energy scales, etc.
        // These would normally come from a database

        // Default IOV covering all runs
        IntervalOfValidity default_iov{0, 999999};

        // Example: Default time offsets for FTOF
        for (int sector = 1; sector <= 6; ++sector) {
            std::string key = fmt::format("time_offset_FTOF_{}", sector);
            set_value(key, 0.0, default_iov);
        }

        // Example: Energy scale factors
        for (int sector = 1; sector <= 6; ++sector) {
            std::string key = fmt::format("energy_scale_ECAL_{}", sector);
            set_value(key, 1.0, default_iov);
        }
    }
};

}  // namespace framework
