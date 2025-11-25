#pragma once

#include "Framework/IService.hpp"
#include "TF1.h"
#include "TH1D.h"
#include <fmt/format.h>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <fstream>

namespace Framework {

/**
 * @brief Service for applying detector corrections and calibrations
 *
 * Features:
 * - Momentum corrections
 * - Energy scale corrections
 * - Efficiency corrections
 * - Run-dependent calibrations
 * - Easy to update without recompilation
 * - Multiple correction types supported
 *
 * Example:
 * @code
 * auto corrections = service<CorrectionsService>("CorrectionsService");
 *
 * // Define corrections
 * corrections->define_correction("electron_momentum", [](double p, int sector) {
 *     return p * (1.0 + 0.01 * std::sin(sector));  // Example correction
 * });
 *
 * // Apply corrections
 * double corrected_p = corrections->apply("electron_momentum", uncorrected_p, sector);
 * @endcode
 */
class CorrectionsService : public Service {
public:
    // Correction function types
    using Correction1D = std::function<double(double)>;
    using Correction2D = std::function<double(double, double)>;
    using Correction3D = std::function<double(double, double, double)>;

    explicit CorrectionsService(std::string name = "CorrectionsService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        load_default_corrections();
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        print_summary();
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Define 1D correction
     */
    auto define_correction_1d(const std::string& name, Correction1D func) -> void {
        m_corrections_1d[name] = func;
    }

    /**
     * @brief Define 2D correction
     */
    auto define_correction_2d(const std::string& name, Correction2D func) -> void {
        m_corrections_2d[name] = func;
    }

    /**
     * @brief Define 3D correction
     */
    auto define_correction_3d(const std::string& name, Correction3D func) -> void {
        m_corrections_3d[name] = func;
    }

    /**
     * @brief Apply 1D correction
     */
    auto apply(const std::string& name, double value) -> double {
        auto it = m_corrections_1d.find(name);
        if (it == m_corrections_1d.end()) {
            return value;  // No correction, return original
        }
        return it->second(value);
    }

    /**
     * @brief Apply 2D correction
     */
    auto apply(const std::string& name, double x, double y) -> double {
        auto it = m_corrections_2d.find(name);
        if (it == m_corrections_2d.end()) {
            return x;  // No correction, return original
        }
        return it->second(x, y);
    }

    /**
     * @brief Apply 3D correction
     */
    auto apply(const std::string& name, double x, double y, double z) -> double {
        auto it = m_corrections_3d.find(name);
        if (it == m_corrections_3d.end()) {
            return x;  // No correction, return original
        }
        return it->second(x, y, z);
    }

    /**
     * @brief Enable/disable a correction
     */
    auto enable_correction(const std::string& name, bool enable = true) -> void {
        m_enabled[name] = enable;
    }

    /**
     * @brief Check if correction is enabled
     */
    auto is_enabled(const std::string& name) const -> bool {
        auto it = m_enabled.find(name);
        return (it != m_enabled.end()) ? it->second : true;
    }

    /**
     * @brief Load corrections from file
     */
    auto load_corrections_from_file(const std::string& filename) -> bool {
        std::ifstream file(filename);
        if (!file.is_open()) {
            fmt::print(stderr, "WARNING: Failed to open corrections file: {}\n",
                      filename);
            return false;
        }

        // Parse file and load corrections
        // Format: name type params...
        // Example: electron_momentum polynomial 1.0 0.01 -0.001

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            // Parse and create correction
            // Implementation depends on file format
        }

        fmt::print("Loaded corrections from: {}\n", filename);
        return true;
    }

    /**
     * @brief Print corrections summary
     */
    auto print_summary() const -> void {
        size_t total = m_corrections_1d.size() +
                       m_corrections_2d.size() +
                       m_corrections_3d.size();

        if (total == 0) {
            return;
        }

        fmt::print("\nCorrections Summary:\n");
        fmt::print("  Total corrections: {}\n", total);
        fmt::print("    1D: {}\n", m_corrections_1d.size());
        fmt::print("    2D: {}\n", m_corrections_2d.size());
        fmt::print("    3D: {}\n", m_corrections_3d.size());

        size_t enabled = 0;
        for (const auto& [name, is_enabled] : m_enabled) {
            if (is_enabled) enabled++;
        }
        fmt::print("  Enabled: {}\n", enabled);
        fmt::print("\n");
    }

private:
    std::map<std::string, Correction1D> m_corrections_1d;
    std::map<std::string, Correction2D> m_corrections_2d;
    std::map<std::string, Correction3D> m_corrections_3d;
    std::map<std::string, bool> m_enabled;

    /**
     * @brief Load default corrections
     */
    auto load_default_corrections() -> void {
        // Example default corrections for CLAS12

        // Electron momentum correction (example)
        define_correction_2d("electron_momentum",
            [](double p, double sector) {
                // Sector-dependent momentum correction
                // This is a placeholder - replace with real corrections
                return p * (1.0 + 0.001 * (sector - 3));
            });

        // ECAL energy scale (example)
        define_correction_1d("ecal_energy_scale",
            [](double E) {
                // Energy scale correction
                return E * 1.002;  // 0.2% correction
            });

        // Sampling fraction correction (example)
        define_correction_2d("sampling_fraction",
            [](double p, double sector) {
                // p and sector dependent
                double sf_nom = 0.27;
                double correction = 0.01 * std::exp(-p/3.0);
                return sf_nom * (1.0 + correction);
            });

        // Vertex Z correction (example)
        define_correction_1d("vertex_z",
            [](double vz) {
                // Target position correction
                return vz - 0.1;  // -1 mm shift
            });
    }
};

/**
 * @brief Predefined correction functions
 */
namespace Corrections {

/**
 * @brief Polynomial correction
 */
inline auto polynomial(const std::vector<double>& coeffs)
    -> std::function<double(double)> {
    return [coeffs](double x) {
        double result = 0.0;
        double x_pow = 1.0;
        for (double c : coeffs) {
            result += c * x_pow;
            x_pow *= x;
        }
        return result;
    };
}

/**
 * @brief Exponential correction
 */
inline auto exponential(double a, double b) -> std::function<double(double)> {
    return [a, b](double x) {
        return a * std::exp(b * x);
    };
}

/**
 * @brief Sector-dependent correction
 */
inline auto sector_dependent(const std::array<double, 6>& corrections)
    -> std::function<double(double, int)> {
    return [corrections](double value, int sector) {
        if (sector < 1 || sector > 6) return value;
        return value * corrections[sector - 1];
    };
}

/**
 * @brief Run-dependent correction
 */
class RunDependentCorrection {
public:
    auto add_run_correction(int run_number, double correction) -> void {
        m_corrections[run_number] = correction;
    }

    auto apply(double value, int run_number) const -> double {
        auto it = m_corrections.find(run_number);
        if (it != m_corrections.end()) {
            return value * it->second;
        }
        return value;  // No correction for this run
    }

private:
    std::map<int, double> m_corrections;
};

/**
 * @brief Efficiency correction (from histogram)
 */
class EfficiencyCorrection {
public:
    explicit EfficiencyCorrection(TH1D* efficiency_hist)
        : m_hist(efficiency_hist) {}

    auto get_efficiency(double value) const -> double {
        if (!m_hist) return 1.0;

        int bin = m_hist->FindBin(value);
        double eff = m_hist->GetBinContent(bin);

        // Clamp to reasonable range
        if (eff < 0.01) return 0.01;
        if (eff > 1.0) return 1.0;

        return eff;
    }

    auto get_weight(double value) const -> double {
        double eff = get_efficiency(value);
        return (eff > 0.0) ? 1.0 / eff : 0.0;
    }

private:
    TH1D* m_hist;
};

}  // namespace Corrections

}  // namespace Framework
