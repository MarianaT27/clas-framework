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
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include "bank.h"

namespace algorithms {

using framework::Log;

/// Configuration for PhotonSelectionAlg.
struct PhotonSelectionConfig {
    bool use_ai = true;
};

class PhotonSelectionAlg : public framework::Algorithm {
   public:
    using Config = PhotonSelectionConfig;
    using StatusCode = framework::StatusCode;

    explicit PhotonSelectionAlg(std::string name = "PhotonSelection", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Photon Selection") {
        m_use_ai.set(m_config.use_ai);
        required_services(m_hist, m_photon_cuts);
        declare_banks("RUN::config", "REC::Particle", "REC::Calorimeter");
        declare_properties(m_min_energy, m_min_beta, m_max_beta, m_use_ai);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Use AI selection: {}", m_use_ai.get() ? "yes" : "no");
        Log::info(name(), "Min photon energy: {} GeV", m_min_energy.get());
        Log::info(name(), "Beta range: [{}, {}]", m_min_beta.get(), m_max_beta.get());

        m_cutflow.add_cut("All Photons");
        m_cutflow.add_cut("AI / Energy Cut");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("Beta Cut");

        book_histograms();

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();

        const auto& RUN_config = event.get_bank("RUN::config");
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");

        const auto& all_photons = event.particles(PIDs::PHOTON);

        h_n_photons_raw.fill(static_cast<double>(all_photons.size()));
        for (const auto& photon : all_photons) {
            m_cutflow.pass("All Photons");
            h_energy_raw.fill(photon.E());
            h_theta_raw.fill(photon.theta());
            h_phi_raw.fill(photon.phi());
        }

        std::vector<Particle> candidates;
        if (m_use_ai.get()) {
            candidates = m_photon_cuts->select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter);
        } else {
            for (const auto& photon : all_photons) {
                if (photon.E() >= m_min_energy.get()) candidates.push_back(photon);
            }
        }
        for (size_t i = 0; i < candidates.size(); ++i) m_cutflow.pass("AI / Energy Cut");

        std::vector<Particle> selected;
        for (const auto& photon : candidates) {
            CalorimeterBank calo = read_Calorimeter_bank(event, photon.index());
            if (!framework::PhotonCutsService::pass_pcal_fiducial_cut(calo.pcal_lv, calo.pcal_lw)) continue;
            m_cutflow.pass("PCAL Fiducial");

            if (photon.beta() <= m_min_beta.get() || photon.beta() >= m_max_beta.get()) continue;
            m_cutflow.pass("Beta Cut");

            selected.push_back(photon);
        }

        h_n_photons_selected.fill(static_cast<double>(selected.size()));
        for (const auto& photon : selected) {
            h_energy_selected.fill(photon.E());
            h_theta_selected.fill(photon.theta());
            h_phi_selected.fill(photon.phi());
        }

        event.store_typed(framework::Keys::SelectedPhotons_Pi0, selected);
        event.set_decision_typed(framework::Decisions::HasGoodPhoton, !selected.empty());

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));
        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow;

    framework::Property<double> m_min_energy{"min_photon_energy", 0.2, "Minimum photon energy [GeV]"};
    framework::Property<double> m_min_beta{"min_beta", 0.9, "Minimum photon beta"};
    framework::Property<double> m_max_beta{"max_beta", 1.1, "Maximum photon beta"};
    framework::Property<bool> m_use_ai{"use_ai", true, "Use AI-based photon selection"};

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::PhotonCutsService> m_photon_cuts;

    framework::H1<> h_n_photons_raw{mh::regular(0, 20, static_cast<std::size_t>(20))};
    framework::H1<> h_energy_raw{mh::regular(0, 5, static_cast<std::size_t>(100))};
    framework::H1<> h_theta_raw{mh::regular(0, 40, static_cast<std::size_t>(180))};
    framework::H1<> h_phi_raw{mh::regular(-180.0, 180.0, static_cast<std::size_t>(360))};
    framework::H1<> h_n_photons_selected{mh::regular(0, 20, static_cast<std::size_t>(20))};
    framework::H1<> h_energy_selected{mh::regular(0, 5, static_cast<std::size_t>(100))};
    framework::H1<> h_theta_selected{mh::regular(0, 40, static_cast<std::size_t>(180))};
    framework::H1<> h_phi_selected{mh::regular(-180.0, 180.0, static_cast<std::size_t>(360))};

    auto book_histograms() -> void {
        const std::string dir = "/PhotonSelection";

        m_hist->register_hist(dir + "/n_photons_raw", "Number of Photons (raw);N_{#gamma};Events", h_n_photons_raw);
        m_hist->register_hist(dir + "/energy_raw", "Photon Energy (raw);E [GeV];Photons", h_energy_raw);
        m_hist->register_hist(dir + "/theta_raw", "Photon #theta (raw);#theta [deg];Photons", h_theta_raw);
        m_hist->register_hist(dir + "/phi_raw", "Photon #phi (raw);#phi [deg];Counts", h_phi_raw);
        m_hist->register_hist(dir + "/n_photons_selected", "Number of Photons (selected);N_{#gamma};Events", h_n_photons_selected);
        m_hist->register_hist(dir + "/energy_selected", "Photon Energy (selected);E [GeV];Photons", h_energy_selected);
        m_hist->register_hist(dir + "/theta_selected", "Photon #theta (selected);#theta [deg];Photons", h_theta_selected);
        m_hist->register_hist(dir + "/phi_selected", "Photon #phi (selected);#phi [deg];Counts", h_phi_selected);
    }
};

}  // namespace algorithms
