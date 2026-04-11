#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <clas12/Banks.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief Detector regions for fiducial cuts
 */
enum class DetectorRegion {
    PCAL,     ///< Pre-shower calorimeter
    ECin,     ///< Inner electromagnetic calorimeter
    ECout,    ///< Outer electromagnetic calorimeter
    DC_R1,    ///< Drift chamber region 1
    DC_R2,    ///< Drift chamber region 2
    DC_R3,    ///< Drift chamber region 3
    HTCC,     ///< High threshold Cherenkov counter
    LTCC,     ///< Low threshold Cherenkov counter
    FTOF,     ///< Forward time-of-flight
    CTOF,     ///< Central time-of-flight
    CND,      ///< Central neutron detector
    FD,       ///< Forward detector (composite)
    CD        ///< Central detector (composite)
};

/**
 * @brief Parameters for a fiducial cut
 */
struct FiducialCutParams {
    std::string name;           ///< Cut name for logging/histograms
    std::string description;    ///< Human-readable description
    bool enabled = true;        ///< Whether cut is active
    std::map<std::string, double> parameters;  ///< Cut parameters (e.g., min_edge, etc.)
};

/**
 * @brief Result of fiducial cut evaluation
 */
struct FiducialResult {
    bool passed = true;         ///< Overall pass/fail
    std::map<DetectorRegion, bool> region_results;  ///< Per-region results
    std::string failed_cut;     ///< Name of first failed cut (if any)

    /// Check if specific region passed
    [[nodiscard]] auto passed_region(DetectorRegion region) const -> bool {
        auto it = region_results.find(region);
        return it == region_results.end() || it->second;
    }
};

/**
 * @brief Service for unified detector acceptance and fiducial cuts
 *
 * FiducialService provides a centralized, configurable system for detector
 * fiducial cuts. It consolidates scattered implementations from ElectronCuts
 * and similar classes into a single service that:
 *
 * - Supports sector-dependent cut parameters
 * - Allows TOML configuration of all cuts
 * - Provides acceptance weights for efficiency corrections
 * - Enables easy extension to new detector regions
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_fiducial_svc = &require_service<FiducialService>("FiducialService");
 *
 * // In algorithm process():
 * auto result = m_fiducial_svc->evaluate(electron, calorimeter, dc_traj);
 * if (!result.passed) {
 *     // Fill histogram showing which cut failed
 *     h_failed_cut->Fill(result.failed_cut);
 *     return StatusCode::FILTERED();
 * }
 * @endcode
 *
 * TOML Configuration:
 * @code
 * [FiducialService]
 * pcal_lv_min = 9.0
 * pcal_lw_min = 9.0
 * dc_r1_edge_min = 1.68
 * dc_r2_edge_min = 2.0
 * dc_r3_edge_min = 8.75
 * sector_dependent = true
 * @endcode
 */
class FiducialService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "FiducialService"; }

    /// Number of CLAS12 forward detector sectors
    static constexpr int NUM_SECTORS = 6;

    explicit FiducialService(std::string name = "FiducialService")
        : Service(std::move(name)) {
        // Initialize default sector parameters
        for (int sector = 1; sector <= NUM_SECTORS; ++sector) {
            m_sector_params[sector] = SectorParams{};
        }
    }

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "FiducialService initialized");
        Log::info(this->name(), "  PCAL: lv_min={}, lw_min={}",
                  m_pcal_lv_min.get(), m_pcal_lw_min.get());
        Log::info(this->name(), "  DC edge: R1>{}, R2>{}, R3>{}",
                  m_dc_r1_edge_min.get(), m_dc_r2_edge_min.get(), m_dc_r3_edge_min.get());
        Log::info(this->name(), "  Sector-dependent cuts: {}",
                  m_sector_dependent.get() ? "enabled" : "disabled");
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "FiducialService finalized. "
                                "Evaluations: {}, Passed: {}, Failed: {}",
                  m_total_evaluations, m_total_passed, m_total_failed);
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// PCAL minimum lv coordinate [cm]
    Property<double> m_pcal_lv_min{"pcal_lv_min", 9.0, "Minimum PCAL lv coordinate [cm]"};

    /// PCAL minimum lw coordinate [cm]
    Property<double> m_pcal_lw_min{"pcal_lw_min", 9.0, "Minimum PCAL lw coordinate [cm]"};

    /// PCAL minimum lu coordinate [cm] (optional, not used by default)
    Property<double> m_pcal_lu_min{"pcal_lu_min", 0.0, "Minimum PCAL lu coordinate [cm]"};

    /// DC Region 1 minimum edge distance [cm]
    Property<double> m_dc_r1_edge_min{"dc_r1_edge_min", 1.68, "DC R1 minimum edge [cm]"};

    /// DC Region 2 minimum edge distance [cm]
    Property<double> m_dc_r2_edge_min{"dc_r2_edge_min", 2.0, "DC R2 minimum edge [cm]"};

    /// DC Region 3 minimum edge distance [cm]
    Property<double> m_dc_r3_edge_min{"dc_r3_edge_min", 8.75, "DC R3 minimum edge [cm]"};

    /// Whether to use sector-dependent cut parameters
    Property<bool> m_sector_dependent{"sector_dependent", false,
                                      "Use sector-dependent fiducial cuts"};

    /// HTCC minimum number of photoelectrons (if enabled)
    Property<double> m_htcc_nphe_min{"htcc_nphe_min", 2.0, "Minimum HTCC nphe"};

    /// Enable HTCC fiducial cut
    Property<bool> m_htcc_enabled{"htcc_enabled", false, "Enable HTCC fiducial cut"};

    // ========================================================================
    // Primary Evaluation API
    // ========================================================================

    /**
     * @brief Evaluate all fiducial cuts for a particle
     *
     * @param particle Reconstructed particle
     * @param calo Calorimeter bank data
     * @param traj DC trajectory bank data
     * @param sector Detector sector (1-6)
     * @return FiducialResult with pass/fail and detailed region info
     */
    [[nodiscard]] auto evaluate(
        const clas12::Particle& particle,
        const clas12::CalorimeterBank& calo,
        const clas12::DCTrajBank& traj,
        int sector = -1) const -> FiducialResult {

        FiducialResult result;
        result.passed = true;

        // Get sector from calorimeter if not provided
        if (sector <= 0) {
            sector = calo.pcal_sector;
        }

        // Get parameters (sector-dependent if enabled)
        const auto& params = get_sector_params(sector);

        // Evaluate PCAL fiducial
        bool pcal_pass = evaluate_pcal_fiducial(calo, params);
        result.region_results[DetectorRegion::PCAL] = pcal_pass;
        if (!pcal_pass) {
            result.passed = false;
            if (result.failed_cut.empty()) result.failed_cut = "PCAL";
        }

        // Evaluate DC edge cuts
        bool dc_r1_pass = evaluate_dc_edge(traj.region1_edge, params.dc_r1_edge_min);
        bool dc_r2_pass = evaluate_dc_edge(traj.region2_edge, params.dc_r2_edge_min);
        bool dc_r3_pass = evaluate_dc_edge(traj.region3_edge, params.dc_r3_edge_min);

        result.region_results[DetectorRegion::DC_R1] = dc_r1_pass;
        result.region_results[DetectorRegion::DC_R2] = dc_r2_pass;
        result.region_results[DetectorRegion::DC_R3] = dc_r3_pass;

        if (!dc_r1_pass) {
            result.passed = false;
            if (result.failed_cut.empty()) result.failed_cut = "DC_R1";
        }
        if (!dc_r2_pass) {
            result.passed = false;
            if (result.failed_cut.empty()) result.failed_cut = "DC_R2";
        }
        if (!dc_r3_pass) {
            result.passed = false;
            if (result.failed_cut.empty()) result.failed_cut = "DC_R3";
        }

        // Update statistics
        m_total_evaluations++;
        if (result.passed) {
            m_total_passed++;
        } else {
            m_total_failed++;
        }

        return result;
    }

    /**
     * @brief Evaluate PCAL fiducial cut only
     */
    [[nodiscard]] auto evaluate_pcal(const clas12::CalorimeterBank& calo, int sector = -1) const -> bool {
        if (sector <= 0) sector = calo.pcal_sector;
        const auto& params = get_sector_params(sector);
        return evaluate_pcal_fiducial(calo, params);
    }

    /**
     * @brief Evaluate DC edge cuts only
     */
    [[nodiscard]] auto evaluate_dc(const clas12::DCTrajBank& traj, int sector = -1) const -> bool {
        const auto& params = get_sector_params(sector);
        return evaluate_dc_edge(traj.region1_edge, params.dc_r1_edge_min) &&
               evaluate_dc_edge(traj.region2_edge, params.dc_r2_edge_min) &&
               evaluate_dc_edge(traj.region3_edge, params.dc_r3_edge_min);
    }

    /**
     * @brief Evaluate HTCC fiducial (if enabled)
     */
    [[nodiscard]] auto evaluate_htcc(const clas12::CherenkovBank& htcc) const -> bool {
        if (!m_htcc_enabled.get()) return true;
        return htcc.nphe >= m_htcc_nphe_min.get();
    }

    // ========================================================================
    // Individual Cut Methods (for compatibility with ElectronCuts)
    // ========================================================================

    /**
     * @brief Check PCAL lv cut
     */
    [[nodiscard]] auto pass_pcal_lv_cut(double lv, int sector = -1) const -> bool {
        const auto& params = get_sector_params(sector);
        return lv > params.pcal_lv_min;
    }

    /**
     * @brief Check PCAL lw cut
     */
    [[nodiscard]] auto pass_pcal_lw_cut(double lw, int sector = -1) const -> bool {
        const auto& params = get_sector_params(sector);
        return lw > params.pcal_lw_min;
    }

    /**
     * @brief Check DC edge cut for a specific region
     */
    [[nodiscard]] auto pass_dc_edge_cut(double edge, int region, int sector = -1) const -> bool {
        const auto& params = get_sector_params(sector);
        switch (region) {
            case 1: return edge > params.dc_r1_edge_min;
            case 2: return edge > params.dc_r2_edge_min;
            case 3: return edge > params.dc_r3_edge_min;
            default: return true;
        }
    }

    // ========================================================================
    // Sector-Dependent Configuration
    // ========================================================================

    /**
     * @brief Sector-specific fiducial cut parameters
     */
    struct SectorParams {
        double pcal_lv_min = 9.0;
        double pcal_lw_min = 9.0;
        double pcal_lu_min = 0.0;
        double dc_r1_edge_min = 1.68;
        double dc_r2_edge_min = 2.0;
        double dc_r3_edge_min = 8.75;
    };

    /**
     * @brief Set sector-specific parameters
     *
     * @param sector Sector number (1-6)
     * @param params Parameters for this sector
     */
    auto set_sector_params(int sector, const SectorParams& params) -> void {
        if (sector >= 1 && sector <= NUM_SECTORS) {
            m_sector_params[sector] = params;
        }
    }

    /**
     * @brief Get sector-specific parameters
     */
    [[nodiscard]] auto get_sector_params(int sector) const -> const SectorParams& {
        if (m_sector_dependent.get() && sector >= 1 && sector <= NUM_SECTORS) {
            auto it = m_sector_params.find(sector);
            if (it != m_sector_params.end()) {
                return it->second;
            }
        }
        return m_default_params;
    }

    /**
     * @brief Update default parameters from properties
     */
    auto update_default_params() -> void {
        m_default_params.pcal_lv_min = m_pcal_lv_min.get();
        m_default_params.pcal_lw_min = m_pcal_lw_min.get();
        m_default_params.pcal_lu_min = m_pcal_lu_min.get();
        m_default_params.dc_r1_edge_min = m_dc_r1_edge_min.get();
        m_default_params.dc_r2_edge_min = m_dc_r2_edge_min.get();
        m_default_params.dc_r3_edge_min = m_dc_r3_edge_min.get();
    }

    // ========================================================================
    // Acceptance Weights (for efficiency corrections)
    // ========================================================================

    /**
     * @brief Get acceptance weight for a particle
     *
     * Returns 1.0 if particle passes fiducial, 0.0 if fails.
     * Future: Could return smooth edge weights for better systematic control.
     *
     * @param particle Reconstructed particle
     * @param calo Calorimeter data
     * @param traj DC trajectory data
     * @return Acceptance weight (0.0 or 1.0)
     */
    [[nodiscard]] auto acceptance_weight(
        const clas12::Particle& particle,
        const clas12::CalorimeterBank& calo,
        const clas12::DCTrajBank& traj) const -> double {
        return evaluate(particle, calo, traj).passed ? 1.0 : 0.0;
    }

    /**
     * @brief Get smooth acceptance weight near fiducial edges
     *
     * Uses sigmoid function to provide smooth transition at edges.
     * Useful for systematic uncertainty studies.
     *
     * @param edge_distance Distance from fiducial edge
     * @param edge_width Width of transition region
     * @return Smooth weight (0-1)
     */
    [[nodiscard]] static auto smooth_edge_weight(double edge_distance, double edge_min,
                                                  double edge_width = 0.5) -> double {
        // Sigmoid transition
        double x = (edge_distance - edge_min) / edge_width;
        return 1.0 / (1.0 + std::exp(-x));
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    [[nodiscard]] auto total_evaluations() const -> size_t { return m_total_evaluations; }
    [[nodiscard]] auto total_passed() const -> size_t { return m_total_passed; }
    [[nodiscard]] auto total_failed() const -> size_t { return m_total_failed; }
    [[nodiscard]] auto efficiency() const -> double {
        return m_total_evaluations > 0 ?
               static_cast<double>(m_total_passed) / m_total_evaluations : 0.0;
    }

   private:
    // Default parameters (used when sector-dependent is disabled)
    SectorParams m_default_params{
        9.0,   // pcal_lv_min
        9.0,   // pcal_lw_min
        0.0,   // pcal_lu_min
        1.68,  // dc_r1_edge_min
        2.0,   // dc_r2_edge_min
        8.75   // dc_r3_edge_min
    };

    // Sector-specific parameters
    std::map<int, SectorParams> m_sector_params;

    // Statistics (mutable for const methods)
    mutable size_t m_total_evaluations = 0;
    mutable size_t m_total_passed = 0;
    mutable size_t m_total_failed = 0;

    /**
     * @brief Evaluate PCAL fiducial cut
     */
    [[nodiscard]] auto evaluate_pcal_fiducial(const clas12::CalorimeterBank& calo,
                                              const SectorParams& params) const -> bool {
        return calo.pcal_lv > params.pcal_lv_min &&
               calo.pcal_lw > params.pcal_lw_min;
    }

    /**
     * @brief Evaluate DC edge cut
     */
    [[nodiscard]] static auto evaluate_dc_edge(double edge, double min_edge) -> bool {
        return edge > min_edge;
    }
};

}  // namespace clas12::framework
