#pragma once

#include <Core/Banks.hpp>
#include <Core/Particle.hpp>
#include <Core/SamplingFractionCut.hpp>
#include <Core/Target.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Core {

/**
 * @brief Base class for physics cuts on particles
 *
 * Provides a clean interface for defining and applying cuts to particles.
 * Cuts can be combined using logical operators.
 */
class Cut {
   public:
    using CutFunction = std::function<bool(const Core::Particle&)>;

    explicit Cut(std::string name, CutFunction func)
        : m_name(std::move(name)), m_cut_func(std::move(func)) {}

    /**
     * @brief Apply the cut to a particle
     */
    bool operator()(const Core::Particle& particle) const {
        return m_cut_func(particle);
    }

    /**
     * @brief Get cut name for logging
     */
    const std::string& name() const { return m_name; }

    /**
     * @brief Combine cuts with AND logic
     */
    Cut operator&&(const Cut& other) const {
        return Cut(
            m_name + " && " + other.m_name,
            [f1 = m_cut_func, f2 = other.m_cut_func](const Core::Particle& p) {
                return f1(p) && f2(p);
            });
    }

    /**
     * @brief Combine cuts with OR logic
     */
    Cut operator||(const Cut& other) const {
        return Cut(
            m_name + " || " + other.m_name,
            [f1 = m_cut_func, f2 = other.m_cut_func](const Core::Particle& p) {
                return f1(p) || f2(p);
            });
    }

    /**
     * @brief Negate a cut
     */
    Cut operator!() const {
        return Cut(
            "!(" + m_name + ")",
            [f = m_cut_func](const Core::Particle& p) { return !f(p); });
    }

   private:
    std::string m_name;
    CutFunction m_cut_func;
};

/**
 * @brief Collection of standard particle cuts
 */
namespace Cuts {

/**
     * @brief Cut on particle momentum range
     */
inline Cut MomentumRange(double min_p, double max_p) {
    return Cut(
        "p ∈ [" + std::to_string(min_p) + ", " + std::to_string(max_p) + "]",
        [min_p, max_p](const Core::Particle& p) {
            double momentum = p.p();
            return momentum >= min_p && momentum <= max_p;
        });
}

/**
     * @brief Cut on vertex z-position range
     */
inline Cut VertexZRange(double min_vz, double max_vz) {
    return Cut(
        "vz ∈ [" + std::to_string(min_vz) + ", " + std::to_string(max_vz) + "]",
        [min_vz, max_vz](const Core::Particle& p) {
            double vz = p.vz();
            return vz >= min_vz && vz <= max_vz;
        });
}

/**
     * @brief Cut on chi2pid range
     */
inline Cut Chi2PIDRange(double min_chi2, double max_chi2) {
    return Cut(
        "χ²_PID ∈ [" + std::to_string(min_chi2) + ", " + std::to_string(max_chi2) + "]",
        [min_chi2, max_chi2](const Core::Particle& p) {
            double chi2 = p.chi2pid();
            return chi2 >= min_chi2 && chi2 <= max_chi2;
        });
}

/**
     * @brief Cut on theta angle range (degrees)
     */
inline Cut ThetaRange(double min_theta, double max_theta) {
    return Cut(
        "θ ∈ [" + std::to_string(min_theta) + "°, " + std::to_string(max_theta) + "°]",
        [min_theta, max_theta](const Core::Particle& p) {
            double theta = p.theta();
            return theta >= min_theta && theta <= max_theta;
        });
}

/**
     * @brief Cut on beta range
     */
inline Cut BetaRange(double min_beta, double max_beta) {
    return Cut(
        "β ∈ [" + std::to_string(min_beta) + ", " + std::to_string(max_beta) + "]",
        [min_beta, max_beta](const Core::Particle& p) {
            double beta = p.beta();
            return beta >= min_beta && beta <= max_beta;
        });
}

/**
     * @brief Cut on particle charge
     */
inline Cut Charge(int required_charge) {
    return Cut(
        "q = " + std::to_string(required_charge),
        [required_charge](const Core::Particle& p) {
            return p.charge() == required_charge;
        });
}

/**
     * @brief Cut requiring forward-going particles
     */
inline Cut Forward(double max_theta = 40.0) {
    return Cut(
        "Forward (θ < " + std::to_string(max_theta) + "°)",
        [max_theta](const Core::Particle& p) {
            return p.theta() < max_theta;
        });
}

}  // namespace Cuts

/**
 * @brief Advanced cut for electron ID using calorimeter
 */
class ElectronIDCut {
   public:
    struct Config {
        double min_sampling_fraction = 0.18;
        double max_sampling_fraction = 0.30;
        double min_nphe = 2.0;
        double min_pcal_energy = 0.06;  // GeV
        bool require_htcc = true;
        bool require_pcal = true;
    };

    explicit ElectronIDCut(Config config = {}) : m_config(config) {}

    /**
     * @brief Apply electron ID cuts
     *
     * @param particle The particle to check
     * @param calorimeter Calorimeter bank data
     * @param cherenkov Cherenkov bank data
     * @return true if particle passes electron ID cuts
     */
    bool operator()(const Core::Particle& particle,
                    const Core::CalorimeterBank& calorimeter,
                    const Core::CherenkovBank& cherenkov) const {

        // Total calorimeter energy
        double E_tot = calorimeter.pcal.energy +
                       calorimeter.inner.energy +
                       calorimeter.outer.energy;

        // Sampling fraction
        double SF = E_tot / particle.p();

        // Check HTCC
        if (m_config.require_htcc) {
            if (std::isnan(cherenkov.nphe) || cherenkov.nphe < m_config.min_nphe) {
                return false;
            }
        }

        // Check PCAL energy
        if (m_config.require_pcal) {
            if (calorimeter.pcal.energy < m_config.min_pcal_energy) {
                return false;
            }
        }

        // Check sampling fraction
        if (SF < m_config.min_sampling_fraction || SF > m_config.max_sampling_fraction) {
            return false;
        }

        return true;
    }

   private:
    Config m_config;
};

/**
 * @brief Apply a cut to a vector of particles and return survivors
 */
inline auto apply_cut(const std::vector<Core::Particle>& particles, const Cut& cut)
    -> std::vector<Core::Particle> {
    std::vector<Core::Particle> survivors;
    survivors.reserve(particles.size());

    for (const auto& particle : particles) {
        if (cut(particle)) {
            survivors.push_back(particle);
        }
    }

    return survivors;
}

/**
 * @brief Apply multiple cuts in sequence
 */
inline auto apply_cuts(const std::vector<Core::Particle>& particles,
                       const std::vector<Cut>& cuts)
    -> std::vector<Core::Particle> {
    auto result = particles;

    for (const auto& cut : cuts) {
        result = apply_cut(result, cut);
        if (result.empty()) break;
    }

    return result;
}

/**
 * @brief Comprehensive electron selection cuts for CLAS12 analysis
 *
 * This class implements the full suite of electron identification cuts including:
 * - Chi2 PID cuts
 * - Momentum cuts
 * - Vertex cuts (target-specific)
 * - Sampling fraction cuts (sector and momentum dependent)
 * - PCAL fiducial cuts
 * - Drift chamber edge cuts (chi2/ndf based and theta-dependent)
 */
class ElectronCuts {
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

        [[nodiscard]] bool passes_chi2ndf_cut() const {
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

        [[nodiscard]] bool passes_all_required() const {
            return chi2pid && momentum && vz && sampling_fraction && pcal_fiducial && dc_chi2ndf;
        }
    };

    // ============================================================================
    // Static Data (Target Types)
    // ============================================================================
    // Note: Using Core::Target and Core::Polarity from Target.hpp

    static constexpr std::array<std::pair<Core::Target, VzRange>, 5> TARGET_VZ_RANGES = {{{Core::Target::LD2, {-15.0, 5.0}},
                                                                                          {Core::Target::CxC, {-10.6, 5.0}},
                                                                                          {Core::Target::Cu, {-10.6, -6.5}},
                                                                                          {Core::Target::Sn, {-5.5, 5.0}},
                                                                                          {Core::Target::CuSn, {-10.6, 5.0}}}};

    static constexpr VzRange DEFAULT_VZ_RANGE = {-30.0, 30.0};

    // ============================================================================
    // Static Methods - Individual Cut Functions
    // ============================================================================

    // Sector Validation
    [[nodiscard]] static auto is_valid_sector(int sector) -> bool {
        return sector >= 1 && sector <= static_cast<int>(NUM_SECTORS);
    }

    // Vertex Z Cuts
    [[nodiscard]] static auto get_vz_range(Core::Target target) -> VzRange {
        for (const auto& [t, range] : TARGET_VZ_RANGES) {
            if (t == target) return range;
        }
        return DEFAULT_VZ_RANGE;
    }

    [[nodiscard]] static auto pass_electron_vz_cut(double vz, Core::Target target) -> bool {
        return get_vz_range(target).contains(vz);
    }

    // PCAL Fiducial Cuts
    [[nodiscard]] static auto pass_pcal_v_cut(double v) -> bool { return v > MIN_PCAL_V; }

    [[nodiscard]] static auto pass_pcal_w_cut(double w) -> bool { return w > MIN_PCAL_W; }

    [[nodiscard]] static auto pass_pcal_fiducial_cut(double v, double w) -> bool {
        return pass_pcal_v_cut(v) && pass_pcal_w_cut(w);
    }

    // Chi2 PID Cuts
    [[nodiscard]] static auto pass_chi2pid_cut(double chi2pid) -> bool {
        return MIN_CHI2PID < chi2pid && chi2pid < MAX_CHI2PID;
    }

    // Momentum Cuts
    [[nodiscard]] static auto pass_electron_momentum_cut(double p) -> bool {
        return p > MIN_ELECTRON_MOMENTUM;
    }

    // Drift Chamber Edge Cuts (chi2/ndf based)
    [[nodiscard]] static auto pass_chi2ndf_cut(double edge1, double edge2, double edge3) -> bool {
        return edge1 > MIN_EDGE_R1 && edge2 > MIN_EDGE_R2 && edge3 > MIN_EDGE_R3;
    }

    [[nodiscard]] static auto pass_chi2ndf_cut(const EdgeCuts& edges) -> bool {
        return edges.passes_chi2ndf_cut();
    }

    /**
     * @brief Apply sampling fraction cut (Full production version with target/polarity)
     *
     * This uses the full SamplingFractionCut class with target and polarity dependence.
     *
     * @param SF Sampling fraction (E_total / p)
     * @param p Particle momentum (GeV)
     * @param sector Sector number (1-6)
     * @param target Target material
     * @param polarity Torus polarity
     * @param sf_cut Reference to SamplingFractionCut object
     * @param n_sigma Number of sigma for the cut (default 3.5)
     * @return true if SF passes the cut
     */
    [[nodiscard]] static auto pass_sampling_fraction_cut(
        double SF,
        double p,
        int sector,
        Core::Target target,
        Core::Polarity polarity,
        const Core::SamplingFractionCut& sf_cut,
        double n_sigma = SF_N_SIGMA) -> bool {

        if (!is_valid_sector(sector)) return false;

        const std::string target_str = Core::to_string(target);
        const std::string polarity_str = Core::to_string(polarity);

        const double param_minus = sf_cut.evaluate_minus_cut(target_str, polarity_str, sector, p, n_sigma);
        const double param_plus = sf_cut.evaluate_plus_cut(target_str, polarity_str, sector, p, n_sigma);

        return param_minus < SF && SF < param_plus;
    }

    // ============================================================================
    // Complete Electron Selection
    // ============================================================================

    /**
     * @brief Apply all electron cuts and return detailed results (with SamplingFractionCut)
     *
     * @param electron The electron particle
     * @param calorimeter Calorimeter bank data
     * @param traj DC trajectory data
     * @param target Target type
     * @param polarity Torus polarity
     * @param sf_cut Reference to SamplingFractionCut object
     * @param theta1 Local theta in DC region 1 (degrees)
     * @param theta2 Local theta in DC region 2 (degrees)
     * @param theta3 Local theta in DC region 3 (degrees)
     * @return CutResults structure with individual cut results
     */
    static auto apply_all_cuts(
        const Core::Particle& electron,
        const Core::CalorimeterBank& calorimeter,
        const Core::DCTrajBank& traj,
        Core::Target target,
        Core::Polarity polarity,
        const Core::SamplingFractionCut& sf_cut) -> CutResults {

        CutResults results;

        // Calculate derived quantities
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / electron.p();
        const double p_e = electron.p();
        const int sector = calorimeter.pcal.sector;
        const double edge1 = traj.region1.edge;
        const double edge2 = traj.region2.edge;
        const double edge3 = traj.region3.edge;

        // Apply individual cuts
        results.chi2pid = pass_chi2pid_cut(electron.chi2pid());
        results.momentum = pass_electron_momentum_cut(p_e);
        results.vz = pass_electron_vz_cut(electron.vz(), target);
        results.sampling_fraction = pass_sampling_fraction_cut(SF, p_e, sector, target, polarity, sf_cut);
        results.pcal_fiducial = pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw);
        results.dc_chi2ndf = pass_chi2ndf_cut(edge1, edge2, edge3);

        return results;
    }

    /**
     * @brief Simplified electron selection (all required cuts with full SF cut)
     *
     * @return true if electron passes all required cuts (excluding theta cuts)
     */
    static auto select_electron(
        const Core::Particle& electron,
        const Core::CalorimeterBank& calorimeter,
        const Core::DCTrajBank& traj,
        Core::Target target,
        Core::Polarity polarity,
        const Core::SamplingFractionCut& sf_cut) -> bool {

        auto results = apply_all_cuts(electron, calorimeter, traj, target, polarity, sf_cut);
        return results.passes_all_required();
    }
};

}  // namespace Core
