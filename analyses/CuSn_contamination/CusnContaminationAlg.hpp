#pragma once

/**
 * @file CusnContaminationAlg.hpp
 * @brief Algorithm for Cu/Sn contamination analysis
 */

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include "Math/GenVector/PxPyPzE4D.h"
#include "Math/Vector3Dfwd.h"

namespace CuSnContamination {

using framework::Log;

// Momentum binning shared by every species' vz-vs-theta breakdown.
static constexpr int N_P_BINS = 20;
static constexpr std::array<double, N_P_BINS + 1> p_bin_edges = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0};

/// Configuration for CuSnContaminationAlg.
struct CuSnContaminationConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.

    double example_cut = 1.0;
    bool enable_histograms = true;

    static constexpr auto config_fields() {
        using T = CuSnContaminationConfig;
        return std::make_tuple(
            framework::field("example_cut", &T::example_cut),
            framework::field("enable_histograms", &T::enable_histograms));
    }
};

/**
 * @brief Per-species set of vertex/kinematics histograms.
 *
 * Booked once per particle species (electron, pi+, pi-). The MC histograms
 * live under `/MC/<species>/`, the reconstructed ones under `/Data/<species>/`.
 */
struct SpeciesHists {
    explicit SpeciesHists(std::string species)
        : m_species(std::move(species)),
          h_p{"/MC/" + m_species + "/p", "p;p;Events", 0.0, 10.0, 100},
          h_phi{"/MC/" + m_species + "/phi", "#phi;#phi;Events", -180.0, 180.0, 100},
          h_theta{"/MC/" + m_species + "/theta", "#theta;#theta;Events", 0.0, 90.0, 100},
          h_vz{"/MC/" + m_species + "/vz", "vz;vz;Events", -13.0, 0.0, 100},
          h_vz_Cu{"/MC/" + m_species + "/vz_Cu", "vz_Cu;vz_Cu;Events", -13.0, 0.0, 100},
          h_vz_Sn{"/MC/" + m_species + "/vz_Sn", "vz_Sn;vz_Sn;Events", -13.0, 0.0, 100},
          h_vz_CuSn{"/MC/" + m_species + "/vz_CuSn", "vz_CuSn;vz_CuSn;Events", -13.0, 0.0, 100},
          h_rec_vz_Cu{"/Data/" + m_species + "/vz_Cu", "vz_Cu;vz_Cu;Events", -13.0, 0.0, 100},
          h_rec_vz_Sn{"/Data/" + m_species + "/vz_Sn", "vz_Sn;vz_Sn;Events", -13.0, 0.0, 100},
          h_rec_vz_CuSn{"/Data/" + m_species + "/vz_CuSn", "vz_CuSn;vz_CuSn;Events", -13.0, 0.0, 100},
          h_vz_vs_theta{"/MC/" + m_species + "/vz_vs_theta", "vz vs #theta;#theta [deg];vz [cm]", 0.0, 40.0, 100, -13.0, 0.0, 100},
          h_vz_vs_p{"/MC/" + m_species + "/vz_vs_p", "vz vs p;p [GeV/c];vz [cm]", 0.0, 10.0, 100, -13.0, 0.0, 100} {}

    // Declared first: the histogram initializers below reference it.
    std::string m_species;

    // MC truth histograms.
    framework::Hist1<> h_p;
    framework::Hist1<> h_phi;
    framework::Hist1<> h_theta;
    framework::Hist1<> h_vz;
    // MC-truth-target gated: reconstructed vz for events whose MC vertex is in the region.
    framework::Hist1<> h_vz_Cu;
    framework::Hist1<> h_vz_Sn;
    framework::Hist1<> h_vz_CuSn;
    // Reconstructed-vz gated: reconstructed vz for tracks whose own vz is in the region.
    framework::Hist1<> h_rec_vz_Cu;
    framework::Hist1<> h_rec_vz_Sn;
    framework::Hist1<> h_rec_vz_CuSn;
    framework::Hist2<> h_vz_vs_theta;
    framework::Hist2<> h_vz_vs_p;
    // Variable-sized arrays of histograms need indirection: heap-allocate so
    // each slot has a stable address for register_hist.
    std::array<std::unique_ptr<framework::H2<>>, N_P_BINS> h_vz_vs_theta_pbin{};

    /// Register the per-momentum-bin vz-vs-theta histograms with the service.
    auto book_pbins(framework::HistogramService& hist) -> void {
        for (int i = 0; i < N_P_BINS; ++i) {
            auto plo = std::to_string(static_cast<int>(p_bin_edges[i]));
            auto phi = std::to_string(static_cast<int>(p_bin_edges[i + 1]));
            auto title = "vz vs #theta (" + plo + "<p<" + phi + " GeV/c);#theta [deg];vz [cm]";
            h_vz_vs_theta_pbin[i] = std::make_unique<framework::H2<>>(
                mh::regular(0.0, 40.0, 100), mh::regular(-13.0, 0.0, 100)
            );
            hist.register_hist("/MC/" + m_species + "/vz_vs_theta_p" + plo + "_" + phi, title, *h_vz_vs_theta_pbin[i]);
        }
    }

    /// Fill the MC-truth histograms from one generated particle.
    auto fill_mc(double px, double py, double pz, double vz, double mass) -> void {
        double E = std::sqrt(px * px + py * py + pz * pz + mass * mass);
        ROOT::Math::PxPyPzE4D p4(px, py, pz, E);
        double theta_deg = p4.Theta() * 180.0 / M_PI;
        double mom = p4.P();
        h_p.fill(mom);
        h_phi.fill(p4.Phi() * 180.0 / M_PI);
        h_theta.fill(theta_deg);
        h_vz.fill(vz);
        h_vz_vs_theta.fill(theta_deg, vz);
        h_vz_vs_p.fill(mom, vz);
        for (int i = 0; i < N_P_BINS; ++i) {
            if (mom >= p_bin_edges[i] && mom < p_bin_edges[i + 1]) {
                h_vz_vs_theta_pbin[i]->fill(theta_deg, vz);
                break;
            }
        }
    }

    /// Fill the reconstructed vz histograms for one reconstructed track.
    /// `mc_vz` is the event's MC vertex z (only used when `has_mc`).
    auto fill_rec(double rec_vz, double mc_vz, bool has_mc) -> void {
        // RG-D vz ranges: Cu [-10.6, -6.5], Sn [-5.5, 5.0], CuSn [-10.6, 5.0]
        if (-10.6 < rec_vz && rec_vz < -6.5) h_rec_vz_Cu.fill(rec_vz);
        if (-5.5 < rec_vz && rec_vz < 5.0) h_rec_vz_Sn.fill(rec_vz);
        h_rec_vz_CuSn.fill(rec_vz);
        if (has_mc) {
            if (-10.6 < mc_vz && mc_vz < -6.5) h_vz_Cu.fill(rec_vz);
            if (-5.5 < mc_vz && mc_vz < 5.0) h_vz_Sn.fill(rec_vz);
            h_vz_CuSn.fill(rec_vz);
        }
    }
};

/**
 * @brief Algorithm for Cu/Sn contamination analysis
 */
class CuSnContaminationAlg : public framework::Algorithm {
   public:
    using Config = CuSnContaminationConfig;
    using StatusCode = framework::StatusCode;

    explicit CuSnContaminationAlg(std::string name = "CuSnContaminationAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        required_services(m_hist);
        declare_optional_bank("MC::Particle");
        declare_config(m_config);
        declare_species_histograms(m_e);
        declare_species_histograms(m_pip);
        declare_species_histograms(m_pim);
        declare_species_histograms(m_e_nocuts);
        declare_species_histograms(m_pip_nocuts);
        declare_species_histograms(m_pim_nocuts);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        if (m_config.enable_histograms) {
            m_e.book_pbins(*m_hist);
            m_pip.book_pbins(*m_hist);
            m_pim.book_pbins(*m_hist);
            m_e_nocuts.book_pbins(*m_hist);
            m_pip_nocuts.book_pbins(*m_hist);
            m_pim_nocuts.book_pbins(*m_hist);
        }
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
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

        auto electron_opt = framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();

        if (m_config.enable_histograms) {
            m_e.fill_rec(electron_opt->vz(), mc_vz, has_mc);

            auto electron = event.find_trigger_electron();
            if (electron) m_e_nocuts.fill_rec(electron->vz(), mc_vz, has_mc);

            auto pi_plus = event.pions_plus();
            for (auto p : pi_plus) { 
                // Check that pion is indeed a pion track and not a misidentified other particle
                int mc_index = get_mc_particle_index(event, p.index());
                fmt::print("pindex {} matched to mc_index {}", p.index(), mc_index);
                if (mc_index >= 0) {
                    int pid = event.get_bank("MC::Particle").getInt("pid", mc_index);
                    if (pid == 211) m_pip_nocuts.fill_rec(p.vz(), mc_vz, has_mc);
                } 
            }
            auto pi_minus = event.pions_minus();
            for (auto p : pi_minus) { 
                int mc_index = get_mc_particle_index(event, p.index());
                if (mc_index >= 0) {
                    int pid = event.get_bank("MC::Particle").getInt("pid", mc_index);
                    if (pid == -211) m_pim_nocuts.fill_rec(p.vz(), mc_vz, has_mc);
                }
            }

            if (auto pip = event.retrieve_typed(framework::Keys::SelectedPiPlus)) {
                for (const auto& pion : *pip) m_pip.fill_rec(pion.vz(), mc_vz, has_mc);
            }
            if (auto pim = event.retrieve_typed(framework::Keys::SelectedPiMinus)) {
                for (const auto& pion : *pim) m_pim.fill_rec(pion.vz(), mc_vz, has_mc);
            }
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;

    framework::ServiceRef<framework::HistogramService> m_hist;

    SpeciesHists m_e{"Electron"};
    SpeciesHists m_e_nocuts{"Electron_NoCuts"};
    SpeciesHists m_pip{"PiPlus"};
    SpeciesHists m_pim{"PiMinus"};
    SpeciesHists m_pip_nocuts{"PiPlus_NoCuts"};
    SpeciesHists m_pim_nocuts{"PiMinus_NoCuts"};

    /// Queue every self-registering wrapper of one species for booking.
    auto declare_species_histograms(SpeciesHists& s) -> void {
        declare_histograms(s.h_p, s.h_phi, s.h_theta, s.h_vz, s.h_vz_Cu, s.h_vz_Sn,
                           s.h_vz_CuSn, s.h_rec_vz_Cu, s.h_rec_vz_Sn, s.h_rec_vz_CuSn,
                           s.h_vz_vs_theta, s.h_vz_vs_p);
    }
};

}  // namespace CuSnContamination
