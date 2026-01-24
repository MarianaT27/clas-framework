#pragma once

/**
 * @file ElectronCuts.hpp
 * @brief Electron selection cuts for CLAS12 analysis
 *
 * This header provides the clas12::ElectronCuts class for electron identification.
 * For service-based cuts in the framework, use Framework::ElectronCutsService.
 *
 * @see Framework::ElectronCutsService
 * @see Framework::SamplingFractionService
 */

#include <clas12/Banks.hpp>
#include <clas12/Particle.hpp>
#include <clas12/Target.hpp>
#include <array>
#include <utility>

namespace clas12 {

/**
 * @brief Comprehensive electron selection cuts for CLAS12 analysis
 *
 * This class implements the full suite of electron identification cuts including:
 * - Chi2 PID cuts
 * - Momentum cuts
 * - Vertex cuts (target-specific)
 * - PCAL fiducial cuts
 * - Drift chamber edge cuts
 *
 * For sampling fraction cuts, use Framework::SamplingFractionService.
 */
class ElectronCuts {
   public:
    // ============================================================================
    // Constants
    // ============================================================================

    static constexpr double HIGH_MOMENTUM_THRESHOLD = 4.5;
    static constexpr double MIN_ELECTRON_MOMENTUM = 0.8;
    static constexpr double MIN_PCAL_V = 9.0;
    static constexpr double MIN_PCAL_W = 9.0;
    static constexpr double SF_N_SIGMA = 3.5;
    static constexpr double MIN_CHI2PID = -5.0;
    static constexpr double MAX_CHI2PID = 5.0;
    static constexpr double MIN_EDGE_R1 = 1.680;
    static constexpr double MIN_EDGE_R2 = 2.0;
    static constexpr double MIN_EDGE_R3 = 8.750;
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

        [[nodiscard]] bool passes_chi2ndf_cut() const {
            return R1 > MIN_EDGE_R1 && R2 > MIN_EDGE_R2 && R3 > MIN_EDGE_R3;
        }
    };

    struct CutResults {
        bool chi2pid{false};
        bool momentum{false};
        bool vz{false};
        bool sampling_fraction{false};
        bool pcal_fiducial{false};
        bool dc_chi2ndf{false};

        [[nodiscard]] bool passes_all_required() const {
            return chi2pid && momentum && vz && sampling_fraction && pcal_fiducial && dc_chi2ndf;
        }

        [[nodiscard]] bool passes_basic() const {
            return chi2pid && momentum && vz && pcal_fiducial && dc_chi2ndf;
        }
    };

    // ============================================================================
    // Static Data
    // ============================================================================

    static constexpr std::array<std::pair<clas12::Target, VzRange>, 5> TARGET_VZ_RANGES = {{{clas12::Target::LD2, {-15.0, 5.0}},
                                                                                          {clas12::Target::CxC, {-10.6, 5.0}},
                                                                                          {clas12::Target::Cu, {-10.6, -6.5}},
                                                                                          {clas12::Target::Sn, {-5.5, 5.0}},
                                                                                          {clas12::Target::CuSn, {-10.6, 5.0}}}};

    static constexpr VzRange DEFAULT_VZ_RANGE = {-30.0, 30.0};

    // ============================================================================
    // Static Methods - Individual Cuts
    // ============================================================================

    [[nodiscard]] static auto is_valid_sector(int sector) -> bool {
        return sector >= 1 && sector <= static_cast<int>(NUM_SECTORS);
    }

    [[nodiscard]] static auto get_vz_range(clas12::Target target) -> VzRange {
        for (const auto& [t, range] : TARGET_VZ_RANGES) {
            if (t == target) return range;
        }
        return DEFAULT_VZ_RANGE;
    }

    [[nodiscard]] static auto pass_electron_vz_cut(double vz, clas12::Target target) -> bool {
        return get_vz_range(target).contains(vz);
    }

    [[nodiscard]] static auto pass_pcal_v_cut(double v) -> bool { return v > MIN_PCAL_V; }

    [[nodiscard]] static auto pass_pcal_w_cut(double w) -> bool { return w > MIN_PCAL_W; }

    [[nodiscard]] static auto pass_pcal_fiducial_cut(double v, double w) -> bool {
        return pass_pcal_v_cut(v) && pass_pcal_w_cut(w);
    }

    [[nodiscard]] static auto pass_chi2pid_cut(double chi2pid) -> bool {
        return MIN_CHI2PID < chi2pid && chi2pid < MAX_CHI2PID;
    }

    [[nodiscard]] static auto pass_electron_momentum_cut(double p) -> bool {
        return p > MIN_ELECTRON_MOMENTUM;
    }

    [[nodiscard]] static auto pass_chi2ndf_cut(double edge1, double edge2, double edge3) -> bool {
        return edge1 > MIN_EDGE_R1 && edge2 > MIN_EDGE_R2 && edge3 > MIN_EDGE_R3;
    }

    [[nodiscard]] static auto pass_chi2ndf_cut(const EdgeCuts& edges) -> bool {
        return edges.passes_chi2ndf_cut();
    }

    /**
     * @brief Apply sampling fraction cut given pre-computed bounds
     *
     * Use Framework::SamplingFractionService to compute sf_min and sf_max.
     */
    [[nodiscard]] static auto pass_sampling_fraction_cut(double SF, double sf_min, double sf_max) -> bool {
        return sf_min < SF && SF < sf_max;
    }

    // ============================================================================
    // Combined Cut Methods
    // ============================================================================

    /**
     * @brief Apply basic cuts (excluding sampling fraction)
     */
    static auto apply_basic_cuts(
        const clas12::Particle& electron,
        const clas12::CalorimeterBank& calorimeter,
        const clas12::DCTrajBank& traj,
        clas12::Target target) -> CutResults {

        CutResults results;

        results.chi2pid = pass_chi2pid_cut(electron.chi2pid());
        results.momentum = pass_electron_momentum_cut(electron.p());
        results.vz = pass_electron_vz_cut(electron.vz(), target);
        results.pcal_fiducial = pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw);
        results.dc_chi2ndf = pass_chi2ndf_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge);
        results.sampling_fraction = true;  // Not evaluated

        return results;
    }

    /**
     * @brief Check if electron passes basic cuts (excluding sampling fraction)
     */
    [[nodiscard]] static auto passes_basic_cuts(
        const clas12::Particle& electron,
        const clas12::CalorimeterBank& calorimeter,
        const clas12::DCTrajBank& traj,
        clas12::Target target) -> bool {

        return apply_basic_cuts(electron, calorimeter, traj, target).passes_basic();
    }
};

}  // namespace clas12
