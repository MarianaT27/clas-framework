#pragma once

#include <Math/VectorUtil.h>
#include <Core/Banks.hpp>
#include <Core/Particle.hpp>
#include <Core/SamplingFractionCut.hpp>
#include <Core/Target.hpp>
#include <Core/models/RGA_inbending_pass1.hpp>
#include <Core/models/RGA_inbending_pass2.hpp>
#include <Core/models/RGA_outbending_pass1.hpp>
#include <Core/models/RGA_outbending_pass2.hpp>
#include <Core/models/RGC_Summer2022_pass1.hpp>
#include <functional>
#include <string>
#include <vector>
#include "Core/Helpers.hpp"
#include "hipo4/bank.h"

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

class PhotonCuts {
   private:
    static constexpr double o_threshold = 0.78;  // Threshold value for model predictions
    // PCAL Fiducial Cuts
    static constexpr double MIN_PCAL_V = 14.0;  // cm
    static constexpr double MIN_PCAL_W = 14.0;  // cm

    struct calo_row_data {
        double pcal_x = 0;
        double pcal_y = 0;
        double pcal_z = 0;
        double ecin_x = 0;
        double ecin_y = 0;
        double ecin_z = 0;
        double ecout_x = 0;
        double ecout_y = 0;
        double ecout_z = 0;
        double pcal_e = 0;
        double pcal_m2u = 0;
        double pcal_m2v = 0;
        double ecin_e = 0;
        double ecin_m2u = 0;
        double ecin_m2v = 0;
        double ecout_e = 0;
        double ecout_m2u = 0;
        double ecout_m2v = 0;
    };

    // clang-format off
     // Map for the GBT Models to use depending on pass and run number
     static inline std::map<std::tuple<int, int, int>, std::function<double(std::vector<float> const &)>> const modelMap = {
        {{5032, 5332, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Fall2018 RGA Inbending
        {{5032, 5332, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Fall2018 RGA Inbending
        {{5333, 5666, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass1(data); }}, // Fall2018 RGA Outbending
        {{5333, 5666, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass2(data); }}, // Fall2018 RGA Outbending
        {{6616, 6783, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Spring2019 RGA Inbending
        {{6616, 6783, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Spring2019 RGA Inbending
        {{6156, 6603, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Spring2019 RGB Inbending
        {{6156, 6603, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Spring2019 RGB Inbending
        {{11093, 11283, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass1(data); }}, // Fall2019 RGB Outbending
        {{11093, 11283, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass2(data); }}, // Fall2019 RGB Outbending
        {{11284, 11300, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Fall2019 RGB BAND Inbending
        {{11284, 11300, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Fall2019 RGB BAND Inbending
        {{11323, 11571, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Spring2020 RGB Inbending
        {{11323, 11571, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Spring2020 RGB Inbending
        {{16042, 16772, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGC_Summer2022_pass1(data); }}, // Summer2022 RGC Inbending
        {{16042, 16772, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGC_Summer2022_pass1(data); }} // Summer2022 RGC Inbending (no pass2 currently)
     };
    // clang-format on

   public:
    // Stuff for GBTFilter
    static auto select_photons_with_AI(const hipo::bank& RUN_config, const hipo::bank& REC_Particle, const hipo::bank& REC_Calorimeter, int pass) -> std::vector<Core::Particle> {
        // ---------------------------
        //     Stuff for GBTFilter
        // ---------------------------
        int run_number = RUN_config.getInt("run", 0);
        std::vector<int> good_photon_rows;
        auto calo_map = GetCaloMap(REC_Calorimeter);
        for (int i = 0; i < REC_Particle.getRows(); i++) {
            int pid = REC_Particle.getInt("pid", i);
            if (pid == 22) {
                if (Filter(REC_Particle, calo_map, i, run_number, pass)) {
                    good_photon_rows.push_back(i);
                }
            }
        }

        // Get good photons
        std::vector<Core::Particle> good_photons;
        for (const auto& row : good_photon_rows) {
            int pid = REC_Particle.getInt("pid", row);
            int status = REC_Particle.getInt("status", row);
            int charge = REC_Particle.getInt("charge", row);

            double px = REC_Particle.getDouble("px", row);
            double py = REC_Particle.getDouble("py", row);
            double pz = REC_Particle.getDouble("pz", row);
            double vx = REC_Particle.getDouble("vx", row);
            double vy = REC_Particle.getDouble("vy", row);
            double vz = REC_Particle.getDouble("vz", row);
            double vt = REC_Particle.getDouble("vt", row);
            double beta = REC_Particle.getDouble("beta", row);
            double chi2pid = REC_Particle.getDouble("chi2pid", row);
            double E = Core::compute_energy(px, py, pz, pid);

            good_photons.emplace_back(pid, status, row, charge, 0, px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
        }

        return good_photons;
    }

    static auto pass_pcal_fiducial_cut(double v, double w) -> bool {
        return v > MIN_PCAL_V && w > MIN_PCAL_W;
    }

   private:
    static std::map<int, calo_row_data> GetCaloMap(hipo::bank const& bank) {
        std::map<int, calo_row_data> calo_map;
        // Loop over REC::Calorimeter rows
        // Here we use bank.getRows() to purposefully ignore upstream filters
        for (int row = 0; row < bank.getRows(); row++) {
            auto pindex = bank.getShort("pindex", row);
            auto x = bank.getFloat("x", row);
            auto y = bank.getFloat("y", row);
            auto z = bank.getFloat("z", row);
            auto m2u = bank.getFloat("m2u", row);
            auto m2v = bank.getFloat("m2v", row);
            auto layer = bank.getInt("layer", row);
            auto e = bank.getFloat("energy", row);

            // Ensure an entry exists in the map for the given pindex
            if (calo_map.find(pindex) == calo_map.end()) {
                calo_map[pindex] = calo_row_data();
            }

            switch (layer) {
                case 1:  // pcal
                    calo_map[pindex].pcal_x = x;
                    calo_map[pindex].pcal_y = y;
                    calo_map[pindex].pcal_z = z;
                    calo_map[pindex].pcal_e = e;
                    calo_map[pindex].pcal_m2u = m2u;
                    calo_map[pindex].pcal_m2v = m2v;
                    break;
                case 4:  // ecin
                    calo_map[pindex].ecin_x = x;
                    calo_map[pindex].ecin_y = y;
                    calo_map[pindex].ecin_z = z;
                    break;
                case 7:  // ecout
                    calo_map[pindex].ecout_x = x;
                    calo_map[pindex].ecout_y = y;
                    calo_map[pindex].ecout_z = z;
                    break;
            }
        }
        return calo_map;
    }

    static bool Filter(hipo::bank const& particleBank, std::map<int, calo_row_data> calo_map, int const row, int const runnum, int const pass) {

        // Set variables native to the photon we are classifying
        double gPx = particleBank.getFloat("px", row);
        double gPy = particleBank.getFloat("py", row);
        double gPz = particleBank.getFloat("pz", row);

        // Set ML features intrinsic to the photon of interest
        double gE = sqrt(gPx * gPx + gPy * gPy + gPz * gPz);
        double gTheta = acos(gPz / gE);
        double gEpcal = calo_map[row].pcal_e;
        double gm2u = calo_map[row].pcal_m2u;
        double gm2v = calo_map[row].pcal_m2v;

        // Apply PID purity cuts on the photon
        // If they do not pass, then these photons are incompatible with the trained GBT model
        if (PidPurityPhotonFilter(gE, gEpcal, gTheta) == false) return false;

        //Define the variables "m_g" , "m_ch" , "m_nh"
        //Should not be changed because the model was trained with this specific set of inputs
        int const m_g = 3;   // Number of neighboring gammas
        int const m_ch = 2;  // Number of neighboring charged hadrons (protons, pions, kaons)
        int const m_nh = 2;  // Number of neighboring neutral hadrons (neutrons)

        double R_e = 0;
        double dE_e = 0;

        double R_gamma[m_g];      // Angular distance between calo shower centers
        double dE_gamma[m_g];     // Energy difference
        double Epcal_gamma[m_g];  // Energy deposited in the pcal
        double m2u_gamma[m_g];    // Shower shape variables
        double m2v_gamma[m_g];    // Shower shape variables

        double R_ch[m_ch];      // Angular distance between calo shower centers
        double dE_ch[m_ch];     // Energy difference
        double Epcal_ch[m_ch];  // Energy deposited in the pcal
        double m2u_ch[m_ch];    // Shower shape variables
        double m2v_ch[m_ch];    // Shower shape variables

        double R_nh[m_nh];      // Angular distance between calo shower centers
        double dE_nh[m_nh];     // Energy difference
        double Epcal_nh[m_nh];  // Energy deposited in the pcal
        double m2u_nh[m_nh];    // Shower shape variables
        double m2v_nh[m_nh];    // Shower shape variables

        double num_photons_0_1, num_photons_0_2, num_photons_0_35;

        //Initialize the arrays
        for (int i = 0; i < m_g; ++i) {
            R_gamma[i] = 0;
            dE_gamma[i] = 0;
            Epcal_gamma[i] = 0;
            m2u_gamma[i] = 0;
            m2v_gamma[i] = 0;
        }
        for (int i = 0; i < m_ch; ++i) {
            R_ch[i] = 0;
            dE_ch[i] = 0;
            Epcal_ch[i] = 0;
            m2u_ch[i] = 0;
            m2v_ch[i] = 0;
        }
        for (int i = 0; i < m_nh; ++i) {
            R_nh[i] = 0;
            dE_nh[i] = 0;
            Epcal_nh[i] = 0;
            m2u_nh[i] = 0;
            m2v_nh[i] = 0;
        }

        //Set the number of photons within R<0.1, R<0.2, R<0.35
        num_photons_0_1 = 0;
        num_photons_0_2 = 0;
        num_photons_0_35 = 0;

        // Get 3-vector that points to the photon of interest's location in the calorimeter
        auto calo_POI = calo_map.at(row);
        ROOT::Math::XYZVector vPOI = GetParticleCaloVector(calo_POI);

        // Build nearest neighbor event structure
        // Loop over particles in the event
        // Here we loop over particleBank.getRows(), which ignores upstream filters
        // This is critical as the GBTs were trained on identifying nearest neighbors for the whole REC::Particle bank
        // Only considering nearest neighbor particles that pass upstream filters would call the accuracy of the model into question
        for (int inner_row = 0; inner_row < particleBank.getRows(); inner_row++) {
            // Skip over the particle if it is photon we are trying to classify
            if (inner_row == row) continue;

            // Check if the key exists in the calo_map
            // This skips over REC::Particle entries without a REC::Calorimeter entry
            if (calo_map.find(inner_row) == calo_map.end()) continue;
            auto calo_PART = calo_map.at(inner_row);

            auto pid = particleBank.getInt("pid", inner_row);
            auto mass = Core::get_mass(pid);

            // Skip over particle if its mass was undefined
            if (mass == std::numeric_limits<double>::quiet_NaN()) continue;
            auto px = particleBank.getFloat("px", inner_row);
            auto py = particleBank.getFloat("py", inner_row);
            auto pz = particleBank.getFloat("pz", inner_row);
            auto p = sqrt(px * px + py * py + pz * pz);
            auto E = sqrt(p * p + mass * mass);
            auto th = acos(pz / p);
            // Skip over particle if it is not in the forward detector (necessary for model compatibility)
            if (ForwardDetectorFilter(th) == false) continue;

            // Get 3-vector that points to the neighboring particle's location in the calorimeter
            ROOT::Math::XYZVector vPART = GetParticleCaloVector(calo_PART);

            // Get angular distance between photon of interest and particle
            double R = ROOT::Math::VectorUtil::Angle(vPOI, vPART);

            // Logic for filling nearest neighbor variables
            if (pid == 22) {  //photon

                // Apply Photon Purity Cuts to ensure this neighbor can be used in classification
                if (PidPurityPhotonFilter(E, calo_PART.pcal_e, th) == false) continue;

                if (R < 0.1) num_photons_0_1++;
                if (R < 0.2) num_photons_0_2++;
                if (R < 0.35) num_photons_0_35++;

                for (int i = 0; i < m_g; ++i) {
                    if (R < R_gamma[i] || R_gamma[i] == 0) {
                        int j = m_g - 1;
                        while (j > i) {
                            R_gamma[j] = R_gamma[j - 1];
                            dE_gamma[j] = dE_gamma[j - 1];
                            Epcal_gamma[j] = Epcal_gamma[j - 1];
                            m2u_gamma[j] = m2u_gamma[j - 1];
                            m2v_gamma[j] = m2v_gamma[j - 1];
                            j--;
                        }
                        R_gamma[i] = R;
                        dE_gamma[i] = gE - E;
                        Epcal_gamma[i] = calo_PART.pcal_e;
                        m2u_gamma[i] = calo_PART.pcal_m2u;
                        m2v_gamma[i] = calo_PART.pcal_m2v;
                        break;
                    }
                }
            } else if (pid == 11) {  //electron
                if (R < R_e || R_e == 0) {
                    R_e = R;
                    dE_e = gE - E;
                }
            } else if (pid == 2212 || pid == -2212 || pid == 211 || pid == -211 || pid == 321 || pid == -321) {  //charged hadron
                for (int i = 0; i < m_ch; ++i) {
                    if (R < R_ch[i] || R_ch[i] == 0) {
                        int j = m_ch - 1;
                        while (j > i) {
                            R_ch[j] = R_ch[j - 1];
                            dE_ch[j] = dE_ch[j - 1];
                            Epcal_ch[j] = Epcal_ch[j - 1];
                            m2u_ch[j] = m2u_ch[j - 1];
                            m2v_ch[j] = m2v_ch[j - 1];
                            j--;
                        }
                        R_ch[i] = R;
                        dE_ch[i] = gE - E;
                        Epcal_ch[i] = calo_PART.pcal_e;
                        m2u_ch[i] = calo_PART.pcal_m2u;
                        m2v_ch[i] = calo_PART.pcal_m2v;
                        break;
                    }
                }
            } else if (pid == 2112 || pid == -2112) {  //neutral hadron
                for (int i = 0; i < m_nh; ++i) {
                    if (R < R_nh[i] || R_nh[i] == 0) {
                        int j = m_nh - 1;
                        while (j > i) {
                            R_nh[j] = R_nh[j - 1];
                            dE_nh[j] = dE_nh[j - 1];
                            Epcal_nh[j] = Epcal_nh[j - 1];
                            m2u_nh[j] = m2u_nh[j - 1];
                            m2v_nh[j] = m2v_nh[j - 1];
                            j--;
                        }
                        R_nh[i] = R;
                        dE_nh[i] = gE - E;
                        Epcal_nh[i] = calo_PART.pcal_e;
                        m2u_nh[i] = calo_PART.pcal_m2u;
                        m2v_nh[i] = calo_PART.pcal_m2v;
                        break;
                    }
                }
            } else {  //unrecognized OR uncompatible particle type for the trained model
                continue;
            }
        }

        // Create and populate input_data vector for the ML model
        std::vector<float> input_data = {
            static_cast<float>(gE), static_cast<float>(gEpcal), static_cast<float>(gTheta),
            static_cast<float>(gm2u), static_cast<float>(gm2v), static_cast<float>(R_e),
            static_cast<float>(dE_e)};

        for (int i = 0; i < m_g; ++i)
            input_data.push_back(static_cast<float>(R_gamma[i]));
        for (int i = 0; i < m_g; ++i)
            input_data.push_back(static_cast<float>(dE_gamma[i]));
        for (int i = 0; i < m_g; ++i)
            input_data.push_back(static_cast<float>(Epcal_gamma[i]));
        for (int i = 0; i < m_g; ++i)
            input_data.push_back(static_cast<float>(m2u_gamma[i]));
        for (int i = 0; i < m_g; ++i)
            input_data.push_back(static_cast<float>(m2v_gamma[i]));

        for (int i = 0; i < m_ch; ++i)
            input_data.push_back(static_cast<float>(R_ch[i]));
        for (int i = 0; i < m_ch; ++i)
            input_data.push_back(static_cast<float>(dE_ch[i]));
        for (int i = 0; i < m_ch; ++i)
            input_data.push_back(static_cast<float>(Epcal_ch[i]));
        for (int i = 0; i < m_ch; ++i)
            input_data.push_back(static_cast<float>(m2u_ch[i]));
        for (int i = 0; i < m_ch; ++i)
            input_data.push_back(static_cast<float>(m2v_ch[i]));

        for (int i = 0; i < m_nh; ++i)
            input_data.push_back(static_cast<float>(R_nh[i]));
        for (int i = 0; i < m_nh; ++i)
            input_data.push_back(static_cast<float>(dE_nh[i]));
        for (int i = 0; i < m_nh; ++i)
            input_data.push_back(static_cast<float>(Epcal_nh[i]));
        for (int i = 0; i < m_nh; ++i)
            input_data.push_back(static_cast<float>(m2u_nh[i]));
        for (int i = 0; i < m_nh; ++i)
            input_data.push_back(static_cast<float>(m2v_nh[i]));

        input_data.push_back(static_cast<float>(num_photons_0_1));
        input_data.push_back(static_cast<float>(num_photons_0_2));
        input_data.push_back(static_cast<float>(num_photons_0_35));

        return ClassifyPhoton(input_data, runnum, pass);
    }

    static bool PidPurityPhotonFilter(float const E, float const Epcal, float const theta) {
        // Apply standard pid cuts on the photon, compatible to how the models were trained
        // 1. Minimum photon energy cut of 200 MeV
        // 2. Photon must have deposited energy in the PCal
        // 3. Photon must be in the Forward Detector
        if (E < 0.2 || Epcal <= 0 || !(ForwardDetectorFilter(theta))) return false;
        return true;
    }

    static bool ForwardDetectorFilter(float const theta) {
        // Apply forward detector cut
        if (theta * 180 / M_PI < 5 || theta * 180 / M_PI > 35) return false;
        return true;
    }

    static ROOT::Math::XYZVector GetParticleCaloVector(calo_row_data calo_row) {
        // Determine the 3-vector location of where the photon of interest's calo deposition is
        // First we check the pcal coords, then ecin, then ecout
        ROOT::Math::XYZVector v;
        if (calo_row.pcal_x == 0) {
            if (calo_row.ecin_x == 0) {
                v.SetXYZ(calo_row.ecout_x, calo_row.ecout_y, calo_row.ecout_z);
            } else {
                v.SetXYZ(calo_row.ecin_x, calo_row.ecin_y, calo_row.ecin_z);
            }
        } else {
            v.SetXYZ(calo_row.pcal_x, calo_row.pcal_y, calo_row.pcal_z);
        }
        return v;
    }

    static bool ClassifyPhoton(std::vector<float> const& input_data, int const runnum, int const pass) {
        auto modelFunction = getModelFunction(runnum, pass);
        double sigmoid_x = modelFunction(input_data);
        double prediction = 1 / (1 + exp(-sigmoid_x));
        return (prediction > o_threshold);
    }

    static std::function<double(std::vector<float> const&)> getModelFunction(int runnum, int pass) {

        for (const auto& entry : modelMap) {
            if (runnum >= std::get<0>(entry.first) && runnum <= std::get<1>(entry.first) && pass == std::get<2>(entry.first)) {
                return entry.second;
            }
        }

        return [](std::vector<float> const& data) {
            return ApplyCatboostModel_RGA_inbending_pass1(data);
        };
    }


};

}  // namespace Core
