#pragma once

#include <CLAS/Banks.hpp>
#include <CLAS/Particle.hpp>
#include <CLAS/Target.hpp>
#include <Framework/IService.hpp>
#include <Framework/StatusCode.hpp>
#include <array>
#include <utility>

namespace Framework {

/**
 * @brief Service for electron selection cuts in CLAS12 analysis
 *
 * This service provides configurable electron identification cuts including:
 * - Chi2 PID cuts
 * - Momentum cuts
 * - Vertex cuts (target-specific)
 * - Sampling fraction cuts (sector and momentum dependent)
 * - PCAL fiducial cuts
 * - Drift chamber edge cuts
 *
 * Usage:
 * @code
 * auto electron_cuts = service<Framework::ElectronCutsService>("ElectronCutsService");
 * electron_cuts->set_target(CLAS::Target::LD2);
 * electron_cuts->set_polarity(CLAS::Polarity::Outbending);
 *
 * if (electron_cuts->pass_all_cuts(electron, calorimeter, traj)) {
 *     // Electron passes selection
 * }
 * @endcode
 */
class ElectronCutsService : public Service {
   public:
    // ============================================================================
    // Constants
    // ============================================================================

    // Momentum Cuts
    static constexpr double HIGH_MOMENTUM_THRESHOLD = 4.5;  // GeV
    static constexpr double MIN_ELECTRON_MOMENTUM = 0.8;    // GeV

    // Status Cuts
    static constexpr int MIN_FD_STATUS = 2000;
    static constexpr int MAX_FD_STATUS = 4000;

    // PCAL Fiducial Cuts
    static constexpr double MIN_PCAL_V = 9.0;  // cm
    static constexpr double MIN_PCAL_W = 9.0;  // cm

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

    // ============================================================================
    // Configuration
    // ============================================================================

    void set_target(CLAS::Target target) { m_target = target; }

    void set_polarity(CLAS::Polarity polarity) { m_polarity = polarity; }

    void set_n_sigma(double n_sigma) { m_n_sigma = n_sigma; }

    [[nodiscard]] auto target() const -> CLAS::Target { return m_target; }

    [[nodiscard]] auto polarity() const -> CLAS::Polarity { return m_polarity; }

    [[nodiscard]] auto n_sigma() const -> double { return m_n_sigma; }

    // ============================================================================
    // Individual Cut Functions
    // ============================================================================

    [[nodiscard]] static auto is_valid_sector(int sector) -> bool {
        return sector >= 1 && sector <= static_cast<int>(NUM_SECTORS);
    }

    [[nodiscard]] auto get_vz_range() const -> VzRange {
        return get_vz_range(m_target);
    }

    [[nodiscard]] static auto get_vz_range(CLAS::Target target) -> VzRange {
        static constexpr std::array<std::pair<CLAS::Target, VzRange>, 5> TARGET_VZ_RANGES = {{{CLAS::Target::LD2, {-15.0, 5.0}},
                                                                                              {CLAS::Target::CxC, {-10.6, 5.0}},
                                                                                              {CLAS::Target::Cu, {-10.6, -6.5}},
                                                                                              {CLAS::Target::Sn, {-5.5, 5.0}},
                                                                                              {CLAS::Target::CuSn, {-10.6, 5.0}}}};

        for (const auto& [t, range] : TARGET_VZ_RANGES) {
            if (t == target) return range;
        }
        return {-30.0, 30.0};  // Default
    }

    // Chi2 PID Cut
    [[nodiscard]] auto pass_chi2pid_cut(double chi2pid) const -> bool {
        return MIN_CHI2PID < chi2pid && chi2pid < MAX_CHI2PID;
    }

    // Momentum Cut
    [[nodiscard]] auto pass_momentum_cut(double p) const -> bool {
        return p > MIN_ELECTRON_MOMENTUM;
    }

    // Vertex Z Cut
    [[nodiscard]] auto pass_vz_cut(double vz) const -> bool {
        return get_vz_range().contains(vz);
    }

    [[nodiscard]] auto pass_vz_cut(double vz, CLAS::Target target) const -> bool {
        return get_vz_range(target).contains(vz);
    }

    // PCAL Fiducial Cuts
    [[nodiscard]] static auto pass_pcal_v_cut(double v) -> bool { return v > MIN_PCAL_V; }

    [[nodiscard]] static auto pass_pcal_w_cut(double w) -> bool { return w > MIN_PCAL_W; }

    [[nodiscard]] static auto pass_pcal_fiducial_cut(double v, double w) -> bool {
        return pass_pcal_v_cut(v) && pass_pcal_w_cut(w);
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
     * @brief Apply sampling fraction cut
     *
     * @param SF Sampling fraction (E_total / p)
     * @param p Particle momentum (GeV)
     * @param sector Sector number (1-6)
     * @param sf_mean Mean sampling fraction for this sector/momentum
     * @param sf_sigma Sigma of sampling fraction for this sector/momentum
     * @return true if SF passes the cut
     */
    [[nodiscard]] auto pass_sampling_fraction_cut(
        double SF, double sf_mean, double sf_sigma) const -> bool {
        double cut_low = sf_mean - m_n_sigma * sf_sigma;
        double cut_high = sf_mean + m_n_sigma * sf_sigma;
        return cut_low < SF && SF < cut_high;
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
        const CLAS::Particle& electron,
        const CLAS::CalorimeterBank& calorimeter,
        const CLAS::DCTrajBank& traj) const -> CutResults {

        CutResults results;

        results.chi2pid = pass_chi2pid_cut(electron.chi2pid());
        results.momentum = pass_momentum_cut(electron.p());
        results.vz = pass_vz_cut(electron.vz());
        results.pcal_fiducial = pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw);
        results.dc_chi2ndf = pass_dc_edge_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge);
        results.sampling_fraction = true;  // Not evaluated without SF service

        return results;
    }

    /**
     * @brief Check if electron passes basic cuts (excluding sampling fraction)
     */
    [[nodiscard]] auto passes_basic_cuts(
        const CLAS::Particle& electron,
        const CLAS::CalorimeterBank& calorimeter,
        const CLAS::DCTrajBank& traj) const -> bool {

        auto results = apply_basic_cuts(electron, calorimeter, traj);
        return results.chi2pid && results.momentum && results.vz &&
               results.pcal_fiducial && results.dc_chi2ndf;
    }

   private:
    CLAS::Target m_target = CLAS::Target::LD2;
    CLAS::Polarity m_polarity = CLAS::Polarity::Outbending;
    double m_n_sigma = SF_N_SIGMA;
};

}  // namespace Framework
