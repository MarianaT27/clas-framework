#pragma once

/**
 * @file PhotonSelectionAlg.hpp
 * @brief Photon selection algorithm with AI-based and traditional cuts
 *
 * Selects good photons from CLAS12 data using:
 * - AI/ML-based photon classification (Catboost GBT models via PhotonCutsService)
 * - PCAL fiducial cuts (lv, lw)
 * - Beta consistency cuts
 * - Minimum energy threshold
 *
 * Stores selected photons in the event store for downstream algorithms
 * (e.g., Pi0ReconstructionAlg).
 *
 * Outputs:
 * - Keys::SelectedPhotons_Pi0: selected photons after all cuts
 * - Decisions::HasGoodPhoton: true if at least one good photon found
 */

#include <cmath>
#include <string>
#include <vector>

#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/services/PhotonCutsService.hpp>
#include "bank.h"

namespace algorithms {

using framework::Log;

/// Configuration for PhotonSelectionAlg.
struct PhotonSelectionConfig {
    bool use_ai = true;

    double min_photon_energy = 0.2;
    double min_beta = 0.9;
    double max_beta = 1.1;

    static constexpr auto config_fields() {
        using T = PhotonSelectionConfig;
        return std::make_tuple(framework::field("use_ai", &T::use_ai), framework::field("min_photon_energy", &T::min_photon_energy), framework::field("min_beta", &T::min_beta), framework::field("max_beta", &T::max_beta));
    }
};

class PhotonSelectionAlg : public framework::ConfigurableAlgorithm<PhotonSelectionConfig> {
   public:
    using StatusCode = framework::StatusCode;

    explicit PhotonSelectionAlg(std::string name = "PhotonSelection", Config config = {})
        : ConfigurableAlgorithm(std::move(name), std::move(config)), m_cutflow("Photon Selection") {
        required_services(m_hist, m_photon_cuts);
        declare_banks("RUN::config", "REC::Particle", "REC::Calorimeter");
        declare_histograms(m_h_n_photons_raw, m_h_energy_raw, m_h_theta_raw, m_h_phi_raw, m_h_n_photons_selected,
             m_h_energy_selected, m_h_theta_selected, m_h_phi_selected);
    }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        log_info("Use AI selection: {}", m_config.use_ai ? "yes" : "no");
        log_info("Min photon energy: {} GeV", m_config.min_photon_energy);
        log_info("Beta range: [{}, {}]", m_config.min_beta, m_config.max_beta);

        m_cutflow.add_cut("All Photons");
        m_cutflow.add_cut("AI / Energy Cut");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("Beta Cut");

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();

        const auto& RUN_config = event.get_bank("RUN::config");
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");

        const auto& all_photons = event.particles(PIDs::PHOTON);

        m_h_n_photons_raw.fill(static_cast<double>(all_photons.size()));
        for (const auto& photon : all_photons) {
            m_cutflow.pass("All Photons");
            m_h_energy_raw.fill(photon.E());
            m_h_theta_raw.fill(photon.theta());
            m_h_phi_raw.fill(photon.phi());
        }

        std::vector<Particle> candidates;
        if (m_config.use_ai) {
            candidates = m_photon_cuts->select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter);
        } else {
            for (const auto& photon : all_photons) {
                if (photon.E() >= m_config.min_photon_energy) candidates.push_back(photon);
            }
        }
        for (size_t i = 0; i < candidates.size(); ++i)
            m_cutflow.pass("AI / Energy Cut");

        std::vector<Particle> selected;
        for (const auto& photon : candidates) {
            CalorimeterBank calo = read_Calorimeter_bank(event, photon.index());
            if (!framework::PhotonCutsService::pass_pcal_fiducial_cut(calo.pcal_lv, calo.pcal_lw)) continue;
            m_cutflow.pass("PCAL Fiducial");

            if (photon.beta() <= m_config.min_beta || photon.beta() >= m_config.max_beta) continue;
            m_cutflow.pass("Beta Cut");

            selected.push_back(photon);
        }

        m_h_n_photons_selected.fill(static_cast<double>(selected.size()));
        for (const auto& photon : selected) {
            m_h_energy_selected.fill(photon.E());
            m_h_theta_selected.fill(photon.theta());
            m_h_phi_selected.fill(photon.phi());
        }

        event.store_typed(framework::Keys::SelectedPhotons_Pi0, selected);
        event.set_decision_typed(framework::Decisions::HasGoodPhoton, !selected.empty());

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));
        Log::blank();
        m_cutflow.print();
        log_success("Finalization complete");
        return StatusCode::SUCCESS();
    }

   private:
    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::PhotonCutsService> m_photon_cuts;

    framework::Hist1<> m_h_n_photons_raw{"/PhotonSelection/n_photons_raw", "Number of Photons (raw);N_{#gamma};Events", 0, 20, 20};
    framework::Hist1<> m_h_energy_raw{"/PhotonSelection/energy_raw", "Photon Energy (raw);E [GeV];Photons", 0, 5, 100};
    framework::Hist1<> m_h_theta_raw{"/PhotonSelection/theta_raw", "Photon #theta (raw);#theta [deg];Photons", 0, 40, 180};
    framework::Hist1<> m_h_phi_raw{"/PhotonSelection/phi_raw", "Photon #phi (raw);#phi [deg];Counts", -180.0, 180.0, 360};
    framework::Hist1<> m_h_n_photons_selected{"/PhotonSelection/n_photons_selected", "Number of Photons (selected);N_{#gamma};Events", 0, 20, 20};
    framework::Hist1<> m_h_energy_selected{"/PhotonSelection/energy_selected", "Photon Energy (selected);E [GeV];Photons", 0, 5, 100};
    framework::Hist1<> m_h_theta_selected{"/PhotonSelection/theta_selected", "Photon #theta (selected);#theta [deg];Photons", 0, 40, 180};
    framework::Hist1<> m_h_phi_selected{"/PhotonSelection/phi_selected", "Photon #phi (selected);#phi [deg];Counts", -180.0, 180.0, 360};
};

}  // namespace algorithms
