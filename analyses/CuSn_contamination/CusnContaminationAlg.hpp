#pragma once

/**
 * @file CusnContaminationAlg.hpp
 * @brief Algorithm for Cu/Sn contamination analysis
 */

#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <type_traits>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/HistogramReflect.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceReflect.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include "Math/GenVector/PxPyPzE4D.h"

namespace CuSnContamination {

using namespace framework;

// Momentum binning shared by every species' vz-vs-theta breakdown.
static constexpr int N_P_BINS = 20;
static constexpr std::array<double, N_P_BINS + 1> p_bin_edges = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0};

// Filesystem/ROOT-safe edge label: 0.5 -> "0p5", 10.0 -> "10p0".
inline auto fmt_p_edge(double v) -> std::string {
    int tenths = static_cast<int>(std::lround(v * 10));
    return std::to_string(tenths / 10) + "p" + std::to_string(tenths % 10);
}

/// Configuration for CuSnContaminationAlg.
struct CuSnContaminationConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.

    double example_cut = 1.0;
    bool enable_histograms = true;
};

static_assert(std::is_aggregate_v<CuSnContaminationConfig>, "CuSnContaminationConfig must stay a simple aggregate for PFR reflection");

/**
 * @brief Per-species set of vertex/kinematics histograms.
 *
 * Booked once per particle species (electron, pi+, pi-). The MC histograms
 * live under `/MC/<species>/`, the reconstructed ones under `/Data/<species>/`.
 */
struct SpeciesHists {
    /// MC-truth histograms, registered under `/MC/<species>/<member>`.
    struct McH {
        H1Auto p{"p;p;Events", 0.0, 10.0, 100};
        H1Auto phi{"#phi;#phi;Events", -180.0, 180.0, 100};
        H1Auto theta{"#theta;#theta;Events", 0.0, 90.0, 100};
        H1Auto vz{"vz;vz;Events", -13.0, 0.0, 100};
        // MC-truth-target gated: reconstructed vz for events whose MC vertex is in the region.
        H1Auto vz_Cu{"vz_Cu;vz_Cu;Events", -13.0, 0.0, 100};
        H1Auto vz_Sn{"vz_Sn;vz_Sn;Events", -13.0, 0.0, 100};
        H1Auto vz_CuSn{"vz_CuSn;vz_CuSn;Events", -13.0, 0.0, 100};
        H2Auto vz_vs_theta{"vz vs #theta;#theta [deg];vz [cm]", 0.0, 40.0, 100, -13.0, 0.0, 100};
        H2Auto vz_vs_p{"vz vs p;p [GeV/c];vz [cm]", 0.0, 10.0, 100, -13.0, 0.0, 100};
        // Per-target vz vs theta: reconstructed vz vs theta, gated by MC truth vertex region.
        H2Auto vz_Cu_vs_theta{"Cu vz vs #theta;#theta [deg];vz [cm]", 0.0, 40.0, 100, -13.0, 0.0, 100};
        H2Auto vz_Sn_vs_theta{"Sn vz vs #theta;#theta [deg];vz [cm]", 0.0, 40.0, 100, -13.0, 0.0, 100};
        H2Auto vz_CuSn_vs_theta{"CuSn vz vs #theta;#theta [deg];vz [cm]", 0.0, 40.0, 100, -13.0, 0.0, 100};
    };

    /// Reconstructed-vz-gated histograms, registered under `/Data/<species>/<member>`.
    struct DataH {
        // Reconstructed-vz gated: reconstructed vz for tracks whose own vz is in the region.
        H1Auto vz_Cu{"vz_Cu;vz_Cu;Events", -13.0, 0.0, 100};
        H1Auto vz_Sn{"vz_Sn;vz_Sn;Events", -13.0, 0.0, 100};
        H1Auto vz_CuSn{"vz_CuSn;vz_CuSn;Events", -13.0, 0.0, 100};
    };

    explicit SpeciesHists(std::string species) : m_species(std::move(species)) {}

    std::string m_species;
    McH mc;
    DataH data;

    // Variable-sized arrays of histograms need indirection: heap-allocate so
    // each slot has a stable address for register_hist.
    std::array<std::unique_ptr<H2<>>, N_P_BINS> h_vz_vs_theta_pbin{};
    // Reconstructed vz sliced per momentum bin, for the cut sample.
    // Used to diagnose p-dependent vz bias (apparent broadening vs real
    // intrinsic resolution at a given p).
    std::array<std::unique_ptr<H1<>>, N_P_BINS> h_rec_vz_pbin{};

    /// Register the reflected MC/Data named histograms with the service.
    /// (The per-p-bin arrays are booked explicitly by the algorithm — only
    /// some species get them — via book_pbins/book_rec_pbins below.)
    auto register_with(HistogramService& hist) -> void {
        register_fields(hist, "/MC/" + m_species, mc);
        register_fields(hist, "/Data/" + m_species, data);
    }

    /// Register the per-momentum-bin vz-vs-theta histograms with the service.
    auto book_pbins(HistogramService& hist) -> void {
        for (int i = 0; i < N_P_BINS; ++i) {
            auto plo = fmt_p_edge(p_bin_edges[i]);
            auto phi = fmt_p_edge(p_bin_edges[i + 1]);
            auto title = "vz vs #theta (" + plo + "<p<" + phi + " GeV/c);#theta [deg];vz [cm]";
            h_vz_vs_theta_pbin[i] = std::make_unique<H2<>>(
                mh::regular(0.0, 40.0, 100), mh::regular(-13.0, 0.0, 100)
            );
            hist.register_hist("/MC/" + m_species + "/vz_vs_theta_p" + plo + "_" + phi, title, *h_vz_vs_theta_pbin[i]);
        }
    }

    /// Register per-momentum-bin reconstructed-vz histograms.
    auto book_rec_pbins(HistogramService& hist) -> void {
        for (int i = 0; i < N_P_BINS; ++i) {
            auto plo = fmt_p_edge(p_bin_edges[i]);
            auto phi = fmt_p_edge(p_bin_edges[i + 1]);
            auto title = "rec vz (" + plo + "<p<" + phi + " GeV/c);vz [cm];Events";
            h_rec_vz_pbin[i] = std::make_unique<H1<>>(mh::regular(-13.0, 0.0, 100));
            hist.register_hist("/Data/" + m_species + "/vz_p" + plo + "_" + phi, title, *h_rec_vz_pbin[i]);
        }
    }

    /// Fill the MC-truth histograms from one generated particle.
    auto fill_mc(double px, double py, double pz, double vz, double mass) -> void {
        double E = std::sqrt(px * px + py * py + pz * pz + mass * mass);
        ROOT::Math::PxPyPzE4D p4(px, py, pz, E);
        double theta_deg = p4.Theta() * 180.0 / M_PI;
        double mom = p4.P();
        mc.p.fill(mom);
        mc.phi.fill(p4.Phi() * 180.0 / M_PI);
        mc.theta.fill(theta_deg);
        mc.vz.fill(vz);
        mc.vz_vs_theta.fill(theta_deg, vz);
        mc.vz_vs_p.fill(mom, vz);
        for (int i = 0; i < N_P_BINS; ++i) {
            if (mom >= p_bin_edges[i] && mom < p_bin_edges[i + 1]) {
                h_vz_vs_theta_pbin[i]->fill(theta_deg, vz);
                break;
            }
        }
    }

    /// Fill the reconstructed vz histograms for one reconstructed track.
    /// `mc_vz` is the event's MC vertex z (only used when `has_mc`).
    auto fill_rec(double rec_vz, double theta_deg, double mc_vz, bool has_mc) -> void {
        // RG-D vz ranges: Cu [-10.6, -6.5], Sn [-5.5, 5.0], CuSn [-10.6, 5.0]
        if (-10.6 < rec_vz && rec_vz < -6.5) data.vz_Cu.fill(rec_vz);
        if (-5.5 < rec_vz && rec_vz < 5.0) data.vz_Sn.fill(rec_vz);

        data.vz_CuSn.fill(rec_vz);
        if (has_mc) {
            if (-10.6 < mc_vz && mc_vz < -6.5) {
                mc.vz_Cu.fill(rec_vz);
                mc.vz_Cu_vs_theta.fill(theta_deg, rec_vz);
            }
            if (-5.5 < mc_vz && mc_vz < 5.0) {
                mc.vz_Sn.fill(rec_vz);
                mc.vz_Sn_vs_theta.fill(theta_deg, rec_vz);
            }
            mc.vz_CuSn.fill(rec_vz);
            mc.vz_CuSn_vs_theta.fill(theta_deg, rec_vz);
        }
    }

    /// Fill the per-momentum-bin reconstructed-vz histogram, if booked.
    auto fill_rec_pbin(double rec_vz, double p) -> void {
        for (int i = 0; i < N_P_BINS; ++i) {
            if (p >= p_bin_edges[i] && p < p_bin_edges[i + 1]) {
                if (h_rec_vz_pbin[i]) { h_rec_vz_pbin[i]->fill(rec_vz); }
                break;
            }
        }
    }
};

/**
 * @brief Reconstruction-vs-MC residuals for one species.
 *
 * Booked once per pion species (pi+, pi-). All histograms live under
 * `/Residual/<species>/`. Residual conventions:
 *   - dp/p = (p_rec - p_mc) / p_mc          (dimensionless)
 *   - dphi = (phi_rec - phi_mc) wrapped to [-180, 180]  (deg)
 *   - dtheta = theta_rec - theta_mc          (deg)
 *   - dvz   = vz_rec - vz_mc                 (cm)
 * Per quantity: an integrated 1-D and two 2-D (vs rec p, vs rec theta).
 */
struct ResidualHists {
    H1Auto dp_over_p{"#Delta p/p;#Delta p/p;Events", -0.05, 0.05, 100};
    H1Auto dphi{"#Delta #phi;#Delta #phi [deg];Events", -5.0, 5.0, 100};
    H1Auto dtheta{"#Delta #theta;#Delta #theta [deg];Events", -2.0, 2.0, 100};
    H1Auto dvz{"#Delta v_{z};#Delta v_{z} [cm];Events", -5.0, 5.0, 100};
    H2Auto dp_over_p_vs_p{"#Delta p/p vs p;p [GeV];#Delta p/p", 0.0, 10.0, 50, -0.05, 0.05, 100};
    H2Auto dphi_vs_p{"#Delta #phi vs p;p [GeV];#Delta #phi [deg]", 0.0, 10.0, 50, -5.0, 5.0, 100};
    H2Auto dtheta_vs_p{"#Delta #theta vs p;p [GeV];#Delta #theta [deg]", 0.0, 10.0, 50, -2.0, 2.0, 100};
    H2Auto dvz_vs_p{"#Delta v_{z} vs p;p [GeV];#Delta v_{z} [cm]", 0.0, 10.0, 50, -5.0, 5.0, 100};
    H2Auto dp_over_p_vs_theta{"#Delta p/p vs #theta;#theta [deg];#Delta p/p", 0.0, 40.0, 50, -0.05, 0.05, 100};
    H2Auto dphi_vs_theta{"#Delta #phi vs #theta;#theta [deg];#Delta #phi [deg]", 0.0, 40.0, 50, -5.0, 5.0, 100};
    H2Auto dtheta_vs_theta{"#Delta #theta vs #theta;#theta [deg];#Delta #theta [deg]", 0.0, 40.0, 50, -2.0, 2.0, 100};
    H2Auto dvz_vs_theta{"#Delta v_{z} vs #theta;#theta [deg];#Delta v_{z} [cm]", 0.0, 40.0, 50, -5.0, 5.0, 100};

    /// Fill all residual histograms for one (rec, mc) pair. Angles in degrees.
    auto fill(double p_rec, double theta_rec, double phi_rec, double vz_rec,
              double p_mc, double theta_mc, double phi_mc, double vz_mc) -> void {
        if (p_mc <= 0.0) { return; }
        double dp_over_p_val = (p_rec - p_mc) / p_mc;
        double dphi_val = phi_rec - phi_mc;
        while (dphi_val > 180.0) { dphi_val -= 360.0; }
        while (dphi_val < -180.0) { dphi_val += 360.0; }
        double dtheta_val = theta_rec - theta_mc;
        double dvz_val = vz_rec - vz_mc;

        dp_over_p.fill(dp_over_p_val);
        dphi.fill(dphi_val);
        dtheta.fill(dtheta_val);
        dvz.fill(dvz_val);

        dp_over_p_vs_p.fill(p_rec, dp_over_p_val);
        dphi_vs_p.fill(p_rec, dphi_val);
        dtheta_vs_p.fill(p_rec, dtheta_val);
        dvz_vs_p.fill(p_rec, dvz_val);

        dp_over_p_vs_theta.fill(theta_rec, dp_over_p_val);
        dphi_vs_theta.fill(theta_rec, dphi_val);
        dtheta_vs_theta.fill(theta_rec, dtheta_val);
        dvz_vs_theta.fill(theta_rec, dvz_val);
    }
};

/**
 * @brief MC-truth PID composition of reconstructed pion candidates.
 *
 * Records the MC truth PID of every track selected as a pion (passed all
 * cuts). Sentinel pid=0 means the rec track has no MC association — no
 * real MC particle is ever written with pid=0, so the bin is unambiguous.
 * Booked both integrated and per reconstructed-momentum bin to expose how
 * the non-pion contamination changes with p.
 */
struct PidCompositionHists {
    // |pid| <= 2212 covers every CLAS12 species; 1-unit bins resolve each.
    static constexpr double PID_LO = -3000.5;
    static constexpr double PID_HI = 3000.5;
    static constexpr int    PID_N_BINS = 6001;

    H1Auto mc_pid{"MC PID of selected pion candidates;MC PID;Events", PID_LO, PID_HI, PID_N_BINS};
    std::array<std::unique_ptr<H1<>>, N_P_BINS> h_mc_pid_pbin{};

    auto book_pbins(HistogramService& hist, const std::string& species) -> void {
        for (int i = 0; i < N_P_BINS; ++i) {
            auto plo = fmt_p_edge(p_bin_edges[i]);
            auto phi = fmt_p_edge(p_bin_edges[i + 1]);
            auto title = "MC PID (" + plo + "<p<" + phi + " GeV/c);MC PID;Events";
            h_mc_pid_pbin[i] = std::make_unique<H1<>>(mh::regular(PID_LO, PID_HI, PID_N_BINS));
            hist.register_hist("/PidContam/" + species + "/mc_pid_p" + plo + "_" + phi, title, *h_mc_pid_pbin[i]);
        }
    }

    /// `mc_pid` should be 0 when the rec track has no MC link.
    auto fill(int mc_pid_val, double p_rec) -> void {
        mc_pid.fill(static_cast<double>(mc_pid_val));
        for (int i = 0; i < N_P_BINS; ++i) {
            if (p_rec >= p_bin_edges[i] && p_rec < p_bin_edges[i + 1]) {
                if (h_mc_pid_pbin[i]) { h_mc_pid_pbin[i]->fill(static_cast<double>(mc_pid_val)); }
                break;
            }
        }
    }
};

/**
 * @brief Track-quality histograms (chi2pid, REC::Track chi2/NDF) per p bin.
 *
 * Same structure as the vz per-p-bin diagnostics: an integrated 1-D for
 * each quantity plus one 1-D per momentum bin. `chi2` and `NDF` come from
 * the DC row of REC::Track via `pion.track()`; when no DC row exists the
 * TrackBank's chi2 is NaN and those slots are skipped (chi2pid is on
 * REC::Particle so it always fills).
 */
struct TrackQualityHists {
    H1Auto chi2pid{"chi2pid;#chi^{2}_{PID};Events", -10.0, 10.0, 200};
    H1Auto chi2{"REC::Track #chi^{2};#chi^{2};Events", 0.0, 200.0, 200};
    H1Auto ndf{"REC::Track NDF;NDF;Events", 0.0, 50.0, 50};
    std::array<std::unique_ptr<H1<>>, N_P_BINS> h_chi2pid_pbin{};
    std::array<std::unique_ptr<H1<>>, N_P_BINS> h_chi2_pbin{};
    std::array<std::unique_ptr<H1<>>, N_P_BINS> h_ndf_pbin{};

    auto book_pbins(HistogramService& hist, const std::string& species) -> void {
        for (int i = 0; i < N_P_BINS; ++i) {
            auto plo = fmt_p_edge(p_bin_edges[i]);
            auto phi = fmt_p_edge(p_bin_edges[i + 1]);
            auto suffix = "_p" + plo + "_" + phi;
            auto prange = " (" + plo + "<p<" + phi + " GeV/c)";

            h_chi2pid_pbin[i] = std::make_unique<H1<>>(mh::regular(-10.0, 10.0, 200));
            hist.register_hist("/TrackQuality/" + species + "/chi2pid" + suffix,
                               "chi2pid" + prange + ";#chi^{2}_{PID};Events",
                               *h_chi2pid_pbin[i]);

            h_chi2_pbin[i] = std::make_unique<H1<>>(mh::regular(0.0, 200.0, 200));
            hist.register_hist("/TrackQuality/" + species + "/chi2" + suffix,
                               "REC::Track #chi^{2}" + prange + ";#chi^{2};Events",
                               *h_chi2_pbin[i]);

            h_ndf_pbin[i] = std::make_unique<H1<>>(mh::regular(0.0, 50.0, 50));
            hist.register_hist("/TrackQuality/" + species + "/ndf" + suffix,
                               "REC::Track NDF" + prange + ";NDF;Events",
                               *h_ndf_pbin[i]);
        }
    }

    /// `chi2` may be NaN (no DC row in REC::Track); those slots are skipped.
    auto fill(double chi2pid_val, double chi2_val, int ndf_val, double p_rec) -> void {
        chi2pid.fill(chi2pid_val);
        const bool has_trk = !std::isnan(chi2_val);
        if (has_trk) {
            chi2.fill(chi2_val);
            ndf.fill(static_cast<double>(ndf_val));
        }
        for (int i = 0; i < N_P_BINS; ++i) {
            if (p_rec >= p_bin_edges[i] && p_rec < p_bin_edges[i + 1]) {
                if (h_chi2pid_pbin[i]) { h_chi2pid_pbin[i]->fill(chi2pid_val); }
                if (has_trk) {
                    if (h_chi2_pbin[i]) { h_chi2_pbin[i]->fill(chi2_val); }
                    if (h_ndf_pbin[i])  { h_ndf_pbin[i]->fill(static_cast<double>(ndf_val)); }
                }
                break;
            }
        }
    }
};

/**
 * @brief Algorithm for Cu/Sn contamination analysis
 */
class CuSnContaminationAlg : public Algorithm {
   public:
    using Config = CuSnContaminationConfig;
    using StatusCode = framework::StatusCode;

    explicit CuSnContaminationAlg(std::string name = "CuSnContaminationAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        wire_services(*this);  // reflects ServiceRef members (refl-cpp)
        declare_optional_bank("MC::Particle");
        // MC::RecMatch carries the (REC::Particle -> MC::Particle) link
        // used by `get_mc_particle_index` below; the framework warns
        // upfront if the input file's dictionary lacks it.
        declare_optional_bank("MC::RecMatch");
        declare_config(m_config);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        if (m_config.enable_histograms) {
            // Named MC/Data histograms for every species (matches the original
            // declare_species_histograms set).
            m_e.register_with(*m_hist);
            m_e_nocuts.register_with(*m_hist);
            m_pip.register_with(*m_hist);
            m_pim.register_with(*m_hist);
            m_pip_nocuts.register_with(*m_hist);
            m_pim_nocuts.register_with(*m_hist);
            m_pip_q_hi.register_with(*m_hist);
            m_pip_q_mid.register_with(*m_hist);
            m_pip_q_lo.register_with(*m_hist);
            m_pim_q_hi.register_with(*m_hist);
            m_pim_q_mid.register_with(*m_hist);
            m_pim_q_lo.register_with(*m_hist);
            m_pip_truth.register_with(*m_hist);
            m_pim_truth.register_with(*m_hist);
            for (auto& s : m_pip_cut_steps) { s.register_with(*m_hist); }
            for (auto& s : m_pim_cut_steps) { s.register_with(*m_hist); }

            // Per-p-bin MC vz-vs-theta arrays (only these species in the original).
            m_e.book_pbins(*m_hist);
            m_pip.book_pbins(*m_hist);
            m_pim.book_pbins(*m_hist);
            m_e_nocuts.book_pbins(*m_hist);
            m_pip_nocuts.book_pbins(*m_hist);
            m_pim_nocuts.book_pbins(*m_hist);
            // Per-p-bin reconstructed-vz arrays for the cut sample (diagnostic).
            m_pip.book_rec_pbins(*m_hist);
            m_pim.book_rec_pbins(*m_hist);
            m_pim_nocuts.book_rec_pbins(*m_hist);
            m_pip_nocuts.book_rec_pbins(*m_hist);
            m_pip_truth.book_rec_pbins(*m_hist);
            m_pim_truth.book_rec_pbins(*m_hist);
            for (auto& s : m_pip_cut_steps) { s.book_rec_pbins(*m_hist); }
            for (auto& s : m_pim_cut_steps) { s.book_rec_pbins(*m_hist); }

            register_fields(*m_hist, "/Residual/PiPlus", m_pip_res);
            register_fields(*m_hist, "/Residual/PiMinus", m_pim_res);
            register_fields(*m_hist, "/Residual/PiPlus_Truth", m_pip_res_truth);
            register_fields(*m_hist, "/Residual/PiMinus_Truth", m_pim_res_truth);

            register_fields(*m_hist, "/PidContam/PiPlus", m_pip_pid);
            register_fields(*m_hist, "/PidContam/PiMinus", m_pim_pid);
            m_pip_pid.book_pbins(*m_hist, "PiPlus");
            m_pim_pid.book_pbins(*m_hist, "PiMinus");

            register_fields(*m_hist, "/TrackQuality/PiPlus", m_pip_trkq);
            register_fields(*m_hist, "/TrackQuality/PiMinus", m_pim_trkq);
            m_pip_trkq.book_pbins(*m_hist, "PiPlus");
            m_pim_trkq.book_pbins(*m_hist, "PiMinus");
        }
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        double mc_vz = 0;
        const bool has_mc = event.has_bank("MC::Particle");
        if (has_mc && m_config.enable_histograms) {
            const auto& MC_particles = event.get_bank("MC::Particle");
            for (int i = 0; i < MC_particles.getRows(); ++i) {
                int pid = MC_particles.getInt("pid", i);
                SpeciesHists* hists = nullptr;
                double mass = 0.0;
                if (pid == 11) {
                    hists = &m_e;
                    mass = Constants::ELECTRONMASS;
                } else if (pid == 211) {
                    hists = &m_pip;
                    mass = Constants::PIMASS;
                } else if (pid == -211) {
                    hists = &m_pim;
                    mass = Constants::PIMASS;
                } else {
                    continue;
                }

                double px = static_cast<double>(MC_particles.getFloat("px", i));
                double py = static_cast<double>(MC_particles.getFloat("py", i));
                double pz = static_cast<double>(MC_particles.getFloat("pz", i));
                double vz = static_cast<double>(MC_particles.getFloat("vz", i));
                hists->fill_mc(px, py, pz, vz, mass);

                // The scattered electron defines the event's MC interaction vertex.
                if (pid == 11) mc_vz = vz;
            }
        }

        auto electron_opt = EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();

        if (m_config.enable_histograms) {
            m_e.fill_rec(electron_opt->vz(), electron_opt->theta(), mc_vz, has_mc);

            auto electron = event.find_trigger_electron();
            if (electron) m_e_nocuts.fill_rec(electron->vz(), electron->theta(), mc_vz, has_mc);

            auto fill_residual = [&](const auto& rec_particle, int mc_idx,
                                     ResidualHists& dst) {
                const auto& mc_bank = event.get_bank("MC::Particle");
                auto mc_px = static_cast<double>(mc_bank.getFloat("px", mc_idx));
                auto mc_py = static_cast<double>(mc_bank.getFloat("py", mc_idx));
                auto mc_pz = static_cast<double>(mc_bank.getFloat("pz", mc_idx));
                auto mc_vz_track = static_cast<double>(mc_bank.getFloat("vz", mc_idx));
                double mc_p_mag = std::sqrt((mc_px * mc_px) + (mc_py * mc_py) + (mc_pz * mc_pz));
                double mc_pt = std::sqrt((mc_px * mc_px) + (mc_py * mc_py));
                double mc_theta_deg = std::atan2(mc_pt, mc_pz) * 180.0 / M_PI;
                double mc_phi_deg = std::atan2(mc_py, mc_px) * 180.0 / M_PI;
                dst.fill(rec_particle.p(), rec_particle.theta(), rec_particle.phi(), rec_particle.vz(),
                         mc_p_mag, mc_theta_deg, mc_phi_deg, mc_vz_track);
            };

            // Per-cut-step diagnostics: retrieve the cumulative samples
            // produced by PionSelectionAlg (forward-only by construction)
            // and fill the same vz / vz-per-p-bin histograms so we can
            // watch the distribution evolve cut-by-cut.
            for (std::size_t i = 0; i < PION_CUT_STEP_LABELS.size(); ++i) {
                auto pip_key = TypedKey<std::vector<Particle>>(
                    fmt::format("PiPlus_{}", PION_CUT_STEP_LABELS[i]));
                if (auto pip = event.retrieve_typed(pip_key)) {
                    for (const auto& pion : *pip) {
                        m_pip_cut_steps[i].fill_rec(pion.vz(), pion.theta(), mc_vz, has_mc);
                        m_pip_cut_steps[i].fill_rec_pbin(pion.vz(), pion.p());
                    }
                }
                auto pim_key = TypedKey<std::vector<Particle>>(
                    fmt::format("PiMinus_{}", PION_CUT_STEP_LABELS[i]));
                if (auto pim = event.retrieve_typed(pim_key)) {
                    for (const auto& pion : *pim) {
                        m_pim_cut_steps[i].fill_rec(pion.vz(), pion.theta(), mc_vz, has_mc);
                        m_pim_cut_steps[i].fill_rec_pbin(pion.vz(), pion.p());
                    }
                }
            }

            if (has_mc) {
                const auto& MC_particles = event.get_bank("MC::Particle");
                for (const auto& p : event.all_particles()) {
                    auto match = get_mc_particle_index_diag(event, p.index());
                    if (match.mc_index < 0) { continue; }
                    int mc_pid = MC_particles.getInt("pid", match.mc_index);
                    SpeciesHists* truth = nullptr;
                    ResidualHists* res = nullptr;
                    if (mc_pid == 211) {
                        truth = &m_pip_truth;
                        res = &m_pip_res_truth;
                    } else if (mc_pid == -211) {
                        truth = &m_pim_truth;
                        res = &m_pim_res_truth;
                    } else {
                        continue;
                    }
                    truth->fill_rec(p.vz(), p.theta(), mc_vz, has_mc);
                    truth->fill_rec_pbin(p.vz(), p.p());
                    fill_residual(p, match.mc_index, *res);
                }

                for (auto p : event.pions_plus()) {
                    auto match = get_mc_particle_index_diag(event, p.index());
                    if (match.mc_index >= 0) {
                        int pid = event.get_bank("MC::Particle").getInt("pid", match.mc_index);
                        if (pid == 211) {
                            m_pip_nocuts.fill_rec(p.vz(), p.theta(), mc_vz, has_mc);
                            m_pip_nocuts.fill_rec_pbin(p.vz(), p.p());
                        }
                    }
                }
                for (auto p : event.pions_minus()) {
                    auto match = get_mc_particle_index_diag(event, p.index());
                    if (match.mc_index >= 0) {
                        int pid = event.get_bank("MC::Particle").getInt("pid", match.mc_index);
                        if (pid == -211) {
                            m_pim_nocuts.fill_rec(p.vz(), p.theta(), mc_vz, has_mc);
                            m_pim_nocuts.fill_rec_pbin(p.vz(), p.p());
                        }
                    }
                }
            }

            auto quality_slice = [](float q, SpeciesHists& hi, SpeciesHists& mid, SpeciesHists& lo) -> SpeciesHists& {
                if (q >= 0.92f) { return hi; }
                if (q >= 0.50f) { return mid; }
                return lo;
            };

            if (auto pip = event.retrieve_typed(Keys::SelectedPiPlus)) {
                for (const auto& pion : *pip) {
                    double theta_deg = pion.theta();
                    m_pip.fill_rec(pion.vz(), theta_deg, mc_vz, has_mc);
                    m_pip.fill_rec_pbin(pion.vz(), pion.p());
                    auto trk = pion.track();
                    m_pip_trkq.fill(pion.chi2pid(), static_cast<double>(trk.chi2), trk.NDF, pion.p());
                    auto match = get_mc_particle_index_diag(event, pion.index(), 0.0f);
                    auto& slice = quality_slice(match.quality, m_pip_q_hi, m_pip_q_mid, m_pip_q_lo);
                    slice.fill_rec(pion.vz(), theta_deg, mc_vz, has_mc);
                    if (has_mc) {
                        int mc_pid = 0;
                        if (match.mc_index >= 0) {
                            mc_pid = event.get_bank("MC::Particle").getInt("pid", match.mc_index);
                        }
                        m_pip_pid.fill(mc_pid, pion.p());

                        if (match.mc_index >= 0 && match.quality >= 0.98f) {
                            fill_residual(pion, match.mc_index, m_pip_res);
                        }
                    }
                }
            }
            if (auto pim = event.retrieve_typed(Keys::SelectedPiMinus)) {
                for (const auto& pion : *pim) {
                    double theta_deg = pion.theta();
                    m_pim.fill_rec(pion.vz(), theta_deg, mc_vz, has_mc);
                    m_pim.fill_rec_pbin(pion.vz(), pion.p());
                    auto trk = pion.track();
                    m_pim_trkq.fill(pion.chi2pid(), static_cast<double>(trk.chi2), trk.NDF, pion.p());
                    auto match = get_mc_particle_index_diag(event, pion.index(), 0.0f);
                    auto& slice = quality_slice(match.quality, m_pim_q_hi, m_pim_q_mid, m_pim_q_lo);
                    slice.fill_rec(pion.vz(), theta_deg, mc_vz, has_mc);
                    if (has_mc) {
                        int mc_pid = 0;
                        if (match.mc_index >= 0) {
                            mc_pid = event.get_bank("MC::Particle").getInt("pid", match.mc_index);
                        }

                        m_pim_pid.fill(mc_pid, pion.p());
                        if (match.mc_index >= 0 && match.quality >= 0.98f) {
                            fill_residual(pion, match.mc_index, m_pim_res);
                        }
                    }
                }
            }
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

   private:
    FRAMEWORK_REFLECT_FRIEND;  // let refl-cpp reflect the private ServiceRef members

    Config m_config;

    ServiceRef<HistogramService> m_hist;

    SpeciesHists m_e{"Electron"};
    SpeciesHists m_e_nocuts{"Electron_NoCuts"};
    SpeciesHists m_pip{"PiPlus"};
    SpeciesHists m_pim{"PiMinus"};
    SpeciesHists m_pip_nocuts{"PiPlus_NoCuts"};
    SpeciesHists m_pim_nocuts{"PiMinus_NoCuts"};

    SpeciesHists m_pip_q_hi{"PiPlus_QHi"};
    SpeciesHists m_pip_q_mid{"PiPlus_QMid"};
    SpeciesHists m_pip_q_lo{"PiPlus_QLo"};
    SpeciesHists m_pim_q_hi{"PiMinus_QHi"};
    SpeciesHists m_pim_q_mid{"PiMinus_QMid"};
    SpeciesHists m_pim_q_lo{"PiMinus_QLo"};

    SpeciesHists m_pip_truth{"PiPlus_Truth"};
    SpeciesHists m_pim_truth{"PiMinus_Truth"};

    ResidualHists m_pip_res;
    ResidualHists m_pim_res;
    ResidualHists m_pip_res_truth;
    ResidualHists m_pim_res_truth;

    PidCompositionHists m_pip_pid;
    PidCompositionHists m_pim_pid;

    TrackQualityHists m_pip_trkq;
    TrackQualityHists m_pim_trkq;

    static constexpr std::array<const char*, 5> PION_CUT_STEP_LABELS = {
        "noPionCuts", "plusDCFiducial", "plusChi2PID", "plusMomentum", "plusVertexZ"
    };

    std::array<SpeciesHists, 5> m_pip_cut_steps{
        SpeciesHists{"PiPlus_noPionCuts"},
        SpeciesHists{"PiPlus_plusDCFiducial"},
        SpeciesHists{"PiPlus_plusChi2PID"},
        SpeciesHists{"PiPlus_plusMomentum"},
        SpeciesHists{"PiPlus_plusVertexZ"},
    };
    std::array<SpeciesHists, 5> m_pim_cut_steps{
        SpeciesHists{"PiMinus_noPionCuts"},
        SpeciesHists{"PiMinus_plusDCFiducial"},
        SpeciesHists{"PiMinus_plusChi2PID"},
        SpeciesHists{"PiMinus_plusMomentum"},
        SpeciesHists{"PiMinus_plusVertexZ"},
    };
};

}  // namespace CuSnContamination

REFL_AUTO(type(CuSnContamination::CuSnContaminationAlg), field(m_hist))
