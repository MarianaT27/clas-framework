#pragma once

#include <array>
#include <string>
#include <utility>
#include <clas12/Banks.hpp>
#include <clas12/Particle.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>

namespace clas12::framework {

// Forward declarations
class SamplingFractionService;

/**
 * @brief Service for electron selection cuts in CLAS12 analysis
 *
 * This service provides a unified interface for all electron identification cuts:
 * - Chi2 PID cuts (configurable range, default: -5.0 to 5.0)
 * - Momentum cuts (configurable minimum, default: 0.8 GeV)
 * - Vertex cuts (run-group and target-specific)
 * - Sampling fraction cuts (run-group dependent, configurable n-sigma, default: 3.5)
 * - PCAL fiducial cuts (configurable: loose/medium/tight)
 * - Drift chamber edge cuts
 *
 * The service integrates with SamplingFractionService to provide a single
 * point of configuration for all electron cuts.
 *
 * Run Group Dependency:
 * - Vz cuts are run-group and target-dependent (currently using RG-D values)
 * - Sampling fraction fits are run-group dependent:
 *   - RG-D uses poly3 fit function
 *   - RG-A, RG-B, RG-C may use different fit functions
 * - These are configured via the SamplingFractionService
 *
 * Configurable Parameters (via Config struct):
 * - target: clas12::Target (default: LD2)
 * - polarity: clas12::Polarity (default: Outbending)
 * - run_group: clas12::RunGroup (default: RG_D)
 * - min_electron_momentum: default 0.8 GeV
 * - min_chi2pid, max_chi2pid: default -5.0 to 5.0
 * - n_sigma: default 3.5
 * - pcal_level: Loose (9 cm), Medium (14 cm), or Tight (19 cm)
 *
 * Usage:
 * @code
 * auto electron_cuts = service<clas12::framework::ElectronCutsService>("ElectronCutsService");
 * auto sf_service = service<clas12::framework::SamplingFractionService>("SamplingFractionService");
 *
 * // Connect the sampling fraction service
 * electron_cuts->set_sf_service(sf_service);
 *
 * // Configure via struct (preferred)
 * ElectronCutsService::Config config;
 * config.target = clas12::Target::LD2;
 * config.polarity = clas12::Polarity::Outbending;
 * config.run_group = clas12::RunGroup::RG_D;
 * config.min_electron_momentum = 1.0;  // Tighter momentum cut
 * config.pcal_level = ElectronCutsService::PCALFiducialLevel::Medium;
 * electron_cuts->set_config(config);
 *
 * // Or configure via individual setters (legacy)
 * electron_cuts->set_target(clas12::Target::LD2);
 * electron_cuts->set_polarity(clas12::Polarity::Outbending);
 * electron_cuts->set_run_group(clas12::RunGroup::RG_D);
 * electron_cuts->set_min_electron_momentum(0.8);
 *
 * // Apply individual cuts
 * if (electron_cuts->pass_chi2pid_cut(chi2pid)) { ... }
 * @endcode
 */
class ElectronCutsService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "ElectronCutsService"; }

    // ============================================================================
    // Constants
    // ============================================================================

    // Momentum Cuts
    static constexpr double HIGH_MOMENTUM_THRESHOLD = 4.5;  // GeV
    static constexpr double MIN_ELECTRON_MOMENTUM = 0.8;    // GeV

    // Status Cuts
    static constexpr int MIN_FD_STATUS = 2000;
    static constexpr int MAX_FD_STATUS = 4000;

    // PCAL Fiducial Cut Levels
    enum class PCALFiducialLevel {
        Loose = 0,   // 9 cm
        Medium = 1,  // 14 cm
        Tight = 2    // 19 cm
    };

    static constexpr std::array<double, 3> PCAL_FIDUCIAL_CUTS = {9.0, 14.0, 19.0};  // cm

    // Sampling Fraction Cuts
    static constexpr double SF_N_SIGMA = 3.5;

    // Chi2 Cuts
    static constexpr double MIN_CHI2PID = -5.0;
    static constexpr double MAX_CHI2PID = 5.0;

    // Drift Chamber Edge Cuts (cm)
    static constexpr double MIN_EDGE_R1 = 1.680;
    static constexpr double MIN_EDGE_R2 = 2.0;
    static constexpr double MIN_EDGE_R3 = 8.750;

    // Geometry
    static constexpr std::size_t NUM_SECTORS = 6;
    static constexpr std::size_t NUM_REGIONS = 3;

    // ============================================================================
    // Configuration Structure
    // ============================================================================

    struct Config {
        clas12::Target target = clas12::Target::LD2;
        clas12::Polarity polarity = clas12::Polarity::Outbending;
        clas12::RunGroup run_group = clas12::RunGroup::RG_D;
        double n_sigma = SF_N_SIGMA;
        double min_electron_momentum = MIN_ELECTRON_MOMENTUM;
        double min_chi2pid = MIN_CHI2PID;
        double max_chi2pid = MAX_CHI2PID;
        PCALFiducialLevel pcal_level = PCALFiducialLevel::Loose;
    };

    // ============================================================================
    // Nested Structures
    // ============================================================================

    struct VzRange {
        double min;
        double max;

        [[nodiscard]] constexpr bool contains(double vz) const { return min < vz && vz < max; }
    };

    struct EdgeCuts {
        double R1;
        double R2;
        double R3;

        [[nodiscard]] bool passes() const {
            return R1 > MIN_EDGE_R1 && R2 > MIN_EDGE_R2 && R3 > MIN_EDGE_R3;
        }
    };

    /**
     * @brief Structure to hold cut results for easy inspection
     */
    struct CutResults {
        bool chi2pid{false};
        bool momentum{false};
        bool vz{false};
        bool sampling_fraction{false};
        bool pcal_fiducial{false};
        bool dc_chi2ndf{false};

        [[nodiscard]] bool passes_all() const {
            return chi2pid && momentum && vz && sampling_fraction && pcal_fiducial && dc_chi2ndf;
        }
    };

    // ============================================================================
    // Service Interface
    // ============================================================================

    explicit ElectronCutsService(std::string name = "ElectronCutsService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    /// Set the sampling fraction service (call this during algorithm initialization)
    auto set_sf_service(SamplingFractionService* sf_service) -> void {
        m_sf_service = sf_service;
    }

    /// Check if sampling fraction service is available
    [[nodiscard]] auto has_sf_service() const -> bool {
        return m_sf_service != nullptr;
    }

    /// Get the sampling fraction service (may be nullptr)
    [[nodiscard]] auto sf_service() const -> SamplingFractionService* {
        return m_sf_service;
    }

    // ============================================================================
    // Configuration
    // ============================================================================

    /// Set configuration via struct
    void set_config(const Config& config) { m_config = config; }

    /// Get configuration
    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    /// Legacy setters for backward compatibility
    void set_target(clas12::Target target) { m_config.target = target; }

    void set_polarity(clas12::Polarity polarity) { m_config.polarity = polarity; }

    void set_run_group(clas12::RunGroup run_group) { m_config.run_group = run_group; }

    void set_n_sigma(double n_sigma) { m_config.n_sigma = n_sigma; }

    void set_pcal_fiducial_level(PCALFiducialLevel level) { m_config.pcal_level = level; }

    void set_min_electron_momentum(double p_min) { m_config.min_electron_momentum = p_min; }

    void set_chi2pid_range(double min, double max) {
        m_config.min_chi2pid = min;
        m_config.max_chi2pid = max;
    }

    /// Legacy getters for backward compatibility
    [[nodiscard]] auto target() const -> clas12::Target { return m_config.target; }

    [[nodiscard]] auto polarity() const -> clas12::Polarity { return m_config.polarity; }

    [[nodiscard]] auto run_group() const -> clas12::RunGroup { return m_config.run_group; }

    [[nodiscard]] auto n_sigma() const -> double { return m_config.n_sigma; }

    [[nodiscard]] auto pcal_fiducial_level() const -> PCALFiducialLevel { return m_config.pcal_level; }

    [[nodiscard]] auto min_electron_momentum() const -> double { return m_config.min_electron_momentum; }

    [[nodiscard]] auto min_chi2pid() const -> double { return m_config.min_chi2pid; }

    [[nodiscard]] auto max_chi2pid() const -> double { return m_config.max_chi2pid; }

    [[nodiscard]] auto get_pcal_fiducial_cut() const -> double {
        return PCAL_FIDUCIAL_CUTS[static_cast<std::size_t>(m_config.pcal_level)];
    }

    // ============================================================================
    // Individual Cut Functions
    // ============================================================================

    [[nodiscard]] static auto is_valid_sector(int sector) -> bool {
        return sector >= 1 && sector <= static_cast<int>(NUM_SECTORS);
    }

    [[nodiscard]] auto get_vz_range() const -> VzRange {
        return get_vz_range(m_config.run_group, m_config.target);
    }

    [[nodiscard]] static auto get_vz_range(clas12::RunGroup /*run_group*/, clas12::Target target) -> VzRange {
        // RG-D cuts (default for now - other run groups use same cuts)
        // TODO: Add specific cuts for RG-A, RG-B, RG-C if they differ
        static constexpr std::array<std::pair<clas12::Target, VzRange>, 6> RG_D_VZ_RANGES = {
            {{clas12::Target::LD2, {-15.0, 5.0}},
             {clas12::Target::CxC, {-10.6, 5.0}},
             {clas12::Target::Cu, {-10.6, -6.5}},
             {clas12::Target::Sn, {-5.5, 5.0}},
             {clas12::Target::CuSn, {-10.6, 5.0}},
            {clas12::Target::Custom, {-12.0, 50.0}}}};  // Custom target uses wide default cuts

        // For now, all run groups use RG-D cuts
        // This can be extended when cuts for other run groups are defined
        const auto& ranges = RG_D_VZ_RANGES;

        for (const auto& [t, range] : ranges) {
            if (t == target) return range;
        }
        return {-30.0, 30.0};  // Default fallback
    }

    // Legacy overload for backward compatibility
    [[nodiscard]] static auto get_vz_range(clas12::Target target) -> VzRange {
        return get_vz_range(clas12::RunGroup::RG_D, target);
    }

    // Chi2 PID Cut
    [[nodiscard]] auto pass_chi2pid_cut(double chi2pid) const -> bool {
        return m_config.min_chi2pid < chi2pid && chi2pid < m_config.max_chi2pid;
    }

    // Momentum Cut
    [[nodiscard]] auto pass_momentum_cut(double p) const -> bool {
        return p > m_config.min_electron_momentum;
    }

    // Vertex Z Cut
    [[nodiscard]] auto pass_vz_cut(double vz) const -> bool {
        return get_vz_range(m_config.target).contains(vz);
    }

    [[nodiscard]] auto pass_vz_cut(double vz, clas12::Target target) const -> bool {
        return get_vz_range(target).contains(vz);
    }

    // PCAL Fiducial Cuts
    [[nodiscard]] auto pass_pcal_v_cut(double v) const -> bool {
        return v > get_pcal_fiducial_cut();
    }

    [[nodiscard]] auto pass_pcal_w_cut(double w) const -> bool {
        return w > get_pcal_fiducial_cut();
    }

    [[nodiscard]] auto pass_pcal_fiducial_cut(double v, double w) const -> bool {
        const double cut_value = get_pcal_fiducial_cut();
        return v > cut_value && w > cut_value;
    }

    // Drift Chamber Edge Cuts
    [[nodiscard]] static auto pass_dc_edge_cut(double edge1, double edge2, double edge3) -> bool {
        return edge1 > MIN_EDGE_R1 && edge2 > MIN_EDGE_R2 && edge3 > MIN_EDGE_R3;
    }

    [[nodiscard]] static auto pass_dc_edge_cut(const EdgeCuts& edges) -> bool {
        return edges.passes();
    }

    // ============================================================================
    // Sampling Fraction Cut (requires SamplingFractionService)
    // ============================================================================

    /**
     * @brief Apply sampling fraction cut using provided mean and sigma
     *
     * @param SF Sampling fraction (E_total / p)
     * @param sf_mean Mean sampling fraction for this sector/momentum
     * @param sf_sigma Sigma of sampling fraction for this sector/momentum
     * @return true if SF passes the cut
     */
    [[nodiscard]] auto pass_sampling_fraction_cut(
        double SF, double sf_mean, double sf_sigma) const -> bool {
        double cut_low = sf_mean - m_config.n_sigma * sf_sigma;
        double cut_high = sf_mean + m_config.n_sigma * sf_sigma;
        return cut_low < SF && SF < cut_high;
    }

    /**
     * @brief Apply sampling fraction cut using target-averaged parameterizations
     *
     * This method uses the integrated SamplingFractionService to automatically
     * determine the correct mean and sigma based on polarity, sector, and momentum.
     * The parameterizations are target-averaged.
     *
     * @param SF Sampling fraction (E_total / p)
     * @param p Particle momentum (GeV)
     * @param sector Sector number (1-6)
     * @return true if SF passes the cut
     *
     * @note Requires SamplingFractionService to be set via set_sf_service()
     * @note Uses polarity and n_sigma from config
     */
    [[nodiscard]] auto pass_sampling_fraction_cut(double SF, double p, int sector) const -> bool {
        if (!m_sf_service) return false;

        return m_sf_service->passes_cut(SF, p, sector,
                                         m_config.polarity,
                                         m_config.n_sigma);
    }

    /**
     * @brief Get sampling fraction mean and sigma for given momentum and sector
     *
     * @param p Particle momentum (GeV)
     * @param sector Sector number (1-6)
     * @return std::pair<double, double> (mean, sigma) or (0, 0) if service unavailable
     *
     * @note Requires SamplingFractionService to be set via set_sf_service()
     * @note Uses polarity from config
     */
    [[nodiscard]] auto get_sampling_fraction_params(double p, int sector) const -> std::pair<double, double> {
        if (!m_sf_service) return {0.0, 0.0};

        return m_sf_service->get_mean_sigma(p, sector,
                                             m_config.polarity);
    }

    // ============================================================================
    // Complete Electron Selection
    // ============================================================================

    /**
     * @brief Apply all electron cuts and return detailed results
     *
     * Note: This version does NOT include sampling fraction cut.
     * Use with SamplingFractionService for full selection.
     */
    auto apply_basic_cuts(
        const clas12::Particle& electron,
        const clas12::CalorimeterBank& calorimeter,
        const clas12::DCTrajBank& traj) const -> CutResults {

        CutResults results;

        results.chi2pid = pass_chi2pid_cut(electron.chi2pid());
        results.momentum = pass_momentum_cut(electron.p());
        results.vz = pass_vz_cut(electron.vz());
        results.pcal_fiducial = pass_pcal_fiducial_cut(calorimeter.pcal_lv, calorimeter.pcal_lw);
        results.dc_chi2ndf = pass_dc_edge_cut(traj.region1_edge, traj.region2_edge, traj.region3_edge);
        results.sampling_fraction = true;  // Not evaluated without SF service

        return results;
    }

    /**
     * @brief Check if electron passes basic cuts (excluding sampling fraction)
     */
    [[nodiscard]] auto passes_basic_cuts(
        const clas12::Particle& electron,
        const clas12::CalorimeterBank& calorimeter,
        const clas12::DCTrajBank& traj) const -> bool {

        auto results = apply_basic_cuts(electron, calorimeter, traj);
        return results.chi2pid && results.momentum && results.vz &&
               results.pcal_fiducial && results.dc_chi2ndf;
    }

   private:
    Config m_config;
    SamplingFractionService* m_sf_service = nullptr;
};

}  // namespace clas12::framework
