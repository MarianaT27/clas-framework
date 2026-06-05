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
 *
 * The Config struct is a plain aggregate whose fields are reflected to TOML
 * keys via Boost.PFR (no config_fields() tuple). Histograms live in a plain
 * aggregate and are registered by member name via register_fields; their fill()
 * is a no-op until registration, so process() fills unconditionally.
 */

#include <cmath>
#include <string>
#include <type_traits>
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
#include <clas12/framework/core/HistogramReflect.hpp>
#include <clas12/framework/core/ServiceReflect.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/services/PhotonCutsService.hpp>
#include <clas12/bankdefs/RUN_config.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/bankdefs/REC_Calorimeter.hpp>

namespace algorithms {

using namespace framework;

/// Configuration for PhotonSelectionAlg.
struct PhotonSelectionConfig {
    bool use_ai = true;

    double min_photon_energy = 0.2;
    double min_beta = 0.9;
    double max_beta = 1.1;
};

static_assert(std::is_aggregate_v<PhotonSelectionConfig>, "PhotonSelectionConfig must stay a simple aggregate for PFR reflection");

class PhotonSelectionAlg : public ConfigurableAlgorithm<PhotonSelectionConfig> {
   public:
    explicit PhotonSelectionAlg(std::string name = "PhotonSelection", Config config = {})
        : ConfigurableAlgorithm(std::move(name), config), m_cutflow("Photon Selection") {
        wire_services(*this);  // reflects ServiceRef members (refl-cpp)
        declare_banks_typed<bankdefs::RUN_config, bankdefs::REC_Particle, bankdefs::REC_Calorimeter>();
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

        register_fields(*m_hist, "/PhotonSelection", h);

        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();

        const auto& RUN_config = event.get_bank("RUN::config");
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");

        const auto& all_photons = event.particles(PIDs::PHOTON);

        h.n_photons_raw.fill(static_cast<double>(all_photons.size()));
        for (const auto& photon : all_photons) {
            m_cutflow.pass("All Photons");
            h.energy_raw.fill(photon.E());
            h.theta_raw.fill(photon.theta());
            h.phi_raw.fill(photon.phi());
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
            if (!PhotonCutsService::pass_pcal_fiducial_cut(calo.pcal_lv, calo.pcal_lw)) continue;
            m_cutflow.pass("PCAL Fiducial");

            if (photon.beta() <= m_config.min_beta || photon.beta() >= m_config.max_beta) continue;
            m_cutflow.pass("Beta Cut");

            selected.push_back(photon);
        }

        h.n_photons_selected.fill(static_cast<double>(selected.size()));
        for (const auto& photon : selected) {
            h.energy_selected.fill(photon.E());
            h.theta_selected.fill(photon.theta());
            h.phi_selected.fill(photon.phi());
        }

        event.store_typed(Keys::SelectedPhotons_Pi0, selected);
        event.set_decision_typed(Decisions::HasGoodPhoton, !selected.empty());

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
    FRAMEWORK_REFLECT_FRIEND;  // let refl-cpp reflect the private ServiceRef members

    Cutflow m_cutflow;

    ServiceRef<HistogramService> m_hist;
    ServiceRef<PhotonCutsService> m_photon_cuts;

    struct Histograms {
        H1Auto n_photons_raw{"Number of Photons (raw);N_{#gamma};Events", 0, 20, 20};
        H1Auto energy_raw{"Photon Energy (raw);E [GeV];Photons", 0, 5, 100};
        H1Auto theta_raw{"Photon #theta (raw);#theta [deg];Photons", 0, 40, 180};
        H1Auto phi_raw{"Photon #phi (raw);#phi [deg];Counts", -180.0, 180.0, 360};
        H1Auto n_photons_selected{"Number of Photons (selected);N_{#gamma};Events", 0, 20, 20};
        H1Auto energy_selected{"Photon Energy (selected);E [GeV];Photons", 0, 5, 100};
        H1Auto theta_selected{"Photon #theta (selected);#theta [deg];Photons", 0, 40, 180};
        H1Auto phi_selected{"Photon #phi (selected);#phi [deg];Counts", -180.0, 180.0, 360};
    };

    Histograms h;
};

}  // namespace algorithms

REFL_AUTO(type(algorithms::PhotonSelectionAlg), field(m_hist), field(m_photon_cuts))
