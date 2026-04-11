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
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include "bank.h"

namespace clas12::algorithms {

using clas12::framework::Log;

class PhotonSelectionAlg : public clas12::framework::SimpleAlgorithm {
   public:
    explicit PhotonSelectionAlg(std::string name = "PhotonSelection",
                                 bool use_ai = true)
        : SimpleAlgorithm(std::move(name))
        , m_use_ai{"use_ai", use_ai, "Use AI-based photon selection"}
        , m_cutflow("Photon Selection") {
        declare_property(m_min_energy);
        declare_property(m_min_beta);
        declare_property(m_max_beta);
        declare_property(m_use_ai);
        declare_bank("RUN::config");
        declare_bank("REC::Particle");
        declare_bank("REC::Calorimeter");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hist = svc<clas12::framework::HistogramService>();
        m_photon_cuts = svc<clas12::framework::PhotonCutsService>();

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Use AI selection", m_use_ai.get() ? "yes" : "no");
        Log::config(name(), "Min photon energy", fmt::format("{} GeV", m_min_energy.get()));
        Log::config(name(), "Beta range", fmt::format("[{}, {}]", m_min_beta.get(), m_max_beta.get()));

        m_cutflow.add_cut("All Photons");
        m_cutflow.add_cut("AI / Energy Cut");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("Beta Cut");

        book_histograms();

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Require upstream DIS electron (typed decision set by DISSelectionAlg)
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodDISElectron)) return clas12::framework::StatusCode::FILTERED();

        const auto& RUN_config = event.get_bank("RUN::config");
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");

        const auto& all_photons = event.particles(clas12::PIDs::PHOTON);

        // Fill raw photon histograms
        if (h_n_photons_raw) h_n_photons_raw->fill(static_cast<double>(all_photons.size()));
        for (const auto& photon : all_photons) {
            m_cutflow.pass("All Photons");
            if (h_energy_raw) h_energy_raw->fill(photon.E());
            if (h_theta_raw) h_theta_raw->fill(photon.theta());
            if (h_phi_raw) h_phi_raw->fill(photon.phi());
        }

        // Step 1: AI-based or energy-threshold selection
        std::vector<clas12::Particle> candidates;
        if (m_use_ai.get()) {
            candidates = m_photon_cuts->select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter);
        } else {
            for (const auto& photon : all_photons) {
                if (photon.E() >= m_min_energy.get()) candidates.push_back(photon);
            }
        }
        for (size_t i = 0; i < candidates.size(); ++i) m_cutflow.pass("AI / Energy Cut");

        // Step 2: PCAL fiducial + beta cuts
        std::vector<clas12::Particle> selected;
        for (const auto& photon : candidates) {
            clas12::CalorimeterBank calo = clas12::read_Calorimeter_bank(event, photon.index());
            if (!clas12::framework::PhotonCutsService::pass_pcal_fiducial_cut(calo.pcal.lv, calo.pcal.lw)) continue;
            m_cutflow.pass("PCAL Fiducial");

            if (photon.beta() <= m_min_beta.get() || photon.beta() >= m_max_beta.get()) continue;
            m_cutflow.pass("Beta Cut");

            selected.push_back(photon);
        }

        // Fill selected photon histograms
        if (h_n_photons_selected) h_n_photons_selected->fill(static_cast<double>(selected.size()));
        for (const auto& photon : selected) {
            if (h_energy_selected) h_energy_selected->fill(photon.E());
            if (h_theta_selected) h_theta_selected->fill(photon.theta());
            if (h_phi_selected) h_phi_selected->fill(photon.phi());
        }

        // Store in event store
        event.store_typed(clas12::framework::Keys::SelectedPhotons_Pi0, selected);
        event.set_decision_typed(clas12::framework::Decisions::HasGoodPhoton, !selected.empty());

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));
        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    // Properties
    clas12::framework::Property<double> m_min_energy{"min_photon_energy", 0.2, "Minimum photon energy [GeV]"};
    clas12::framework::Property<double> m_min_beta{"min_beta", 0.9, "Minimum photon beta"};
    clas12::framework::Property<double> m_max_beta{"max_beta", 1.1, "Maximum photon beta"};
    clas12::framework::Property<bool> m_use_ai{"use_ai", true, "Use AI-based photon selection"};

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::HistogramService* m_hist = nullptr;
    clas12::framework::PhotonCutsService* m_photon_cuts = nullptr;

    // Cutflow
    clas12::framework::Cutflow m_cutflow;

    // Histograms
    clas12::framework::AtomicHistogram1D* h_n_photons_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_energy_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_theta_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_phi_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_n_photons_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_energy_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_theta_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_phi_selected = nullptr;

    auto book_histograms() -> void {
        if (m_hist) { auto* svc = m_hist;
            const std::string dir = "/PhotonSelection";

            // Raw photon distributions
            h_n_photons_raw = svc->book_atomic_hist1D(dir + "/n_photons_raw", "Number of Photons (raw);N_{#gamma};Events", 20, 0, 20);
            h_energy_raw = svc->book_atomic_hist1D(dir + "/energy_raw", "Photon Energy (raw);E [GeV];Photons", 100, 0, 5);
            h_theta_raw = svc->book_atomic_hist1D(dir + "/theta_raw", "Photon #theta (raw);#theta [deg];Photons", 180, 0, 40);
            h_phi_raw = svc->book_phi_hist(dir + "/phi_raw", "Photon #phi (raw)");

            // Selected photon distributions
            h_n_photons_selected = svc->book_atomic_hist1D(dir + "/n_photons_selected", "Number of Photons (selected);N_{#gamma};Events", 20, 0, 20);
            h_energy_selected = svc->book_atomic_hist1D(dir + "/energy_selected", "Photon Energy (selected);E [GeV];Photons", 100, 0, 5);
            h_theta_selected = svc->book_atomic_hist1D(dir + "/theta_selected", "Photon #theta (selected);#theta [deg];Photons", 180, 0, 40);
            h_phi_selected = svc->book_phi_hist(dir + "/phi_selected", "Photon #phi (selected)");
        }
    }
};

}  // namespace clas12::algorithms
