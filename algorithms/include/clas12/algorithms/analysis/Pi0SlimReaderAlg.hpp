#pragma once

/**
 * @file Pi0SlimReaderAlg.hpp
 * @brief Adapter that unpacks the slim PI0 skim banks into the typed store.
 *
 * The slim pi0 skim (analyses/pi0_multiplicity_skim) reduces each passing event
 * to two custom banks — PI0::event (DIS kinematics + trigger-electron 4-vector +
 * helicity) and PI0::photons (selected photon 4-momenta, Catboost AI already
 * applied). This algorithm reads those banks back and re-populates exactly the
 * typed-store keys / decisions that the full electron→DIS→photon chain would
 * have set, so Pi0ReconstructionAlg and Pi0AnalysisAlg run downstream UNCHANGED.
 *
 * Reads (banks):  PI0::event (1 row), PI0::photons (N rows)
 * Writes (store): Keys::DIS_Q2/W/xB/y/nu, Keys::TriggerElectron,
 *                 Keys::SelectedPhotons_Pi0, Keys::Helicity
 * Decisions:      HasGoodDISElectron, HasGoodPhoton
 *
 * The full chain is the fallback: on a normal (non-slim) input the PI0 banks are
 * absent and pi0_multiplicity wires the full selection chain instead of this.
 */

#include <string>
#include <vector>

#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/bankdefs/PI0_event.hpp>
#include <clas12/bankdefs/PI0_photons.hpp>

namespace algorithms {

using namespace framework;

/**
 * @brief Unpack PI0::event / PI0::photons into the typed event store.
 *
 * Header-only; no configuration or histograms. Placed first in the slim
 * pi0_multiplicity pipeline, ahead of Pi0ReconstructionAlg.
 */
class Pi0SlimReaderAlg : public framework::Algorithm {
   public:
    explicit Pi0SlimReaderAlg(std::string name = "Pi0SlimReader")
        : Algorithm(std::move(name)) {
        declare_banks_typed<bankdefs::PI0_event, bankdefs::PI0_photons>();
    }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        log_info("Reading slim PI0::event + PI0::photons banks");
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        auto& evt = event.bank<bankdefs::PI0_event>();
        if (evt.rows() <= 0) return StatusCode::FILTERED();

        // --- DIS kinematics (stored as double, as the full chain does) -------
        event.store_typed(Keys::DIS_Q2, static_cast<double>(evt.Q2(0)));
        event.store_typed(Keys::DIS_W,  static_cast<double>(evt.W(0)));
        event.store_typed(Keys::DIS_xB, static_cast<double>(evt.xB(0)));
        event.store_typed(Keys::DIS_y,  static_cast<double>(evt.y(0)));
        event.store_typed(Keys::DIS_nu, static_cast<double>(evt.nu(0)));

        // --- Trigger electron 4-vector (used to build the gamma* frame) ------
        Particle electron(static_cast<int>(PIDs::ELECTRON), 0, 0, -1, Constants::ELECTRONMASS,
                          static_cast<double>(evt.ex(0)), static_cast<double>(evt.ey(0)),
                          static_cast<double>(evt.ez(0)), static_cast<double>(evt.eE(0)),
                          0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        event.store_typed(Keys::TriggerElectron, electron);

        // --- Helicity + original event id (provenance / ntuple parity) -------
        event.store_typed(Keys::Helicity, evt.helicity(0));
        event.set_event_number(static_cast<long>(evt.event(0)));

        // --- Selected photons (AI already applied at skim time) --------------
        auto& photons_bank = event.bank<bankdefs::PI0_photons>();
        const int n_photons = photons_bank.rows();
        std::vector<Particle> photons;
        photons.reserve(static_cast<std::size_t>(std::max(0, n_photons)));
        for (int i = 0; i < n_photons; ++i) {
            photons.emplace_back(static_cast<int>(PIDs::PHOTON), 0, i, 0, 0.0,
                                 static_cast<double>(photons_bank.px(i)),
                                 static_cast<double>(photons_bank.py(i)),
                                 static_cast<double>(photons_bank.pz(i)),
                                 static_cast<double>(photons_bank.E(i)),
                                 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        }
        event.store_typed(Keys::SelectedPhotons_Pi0, photons);

        // --- Decisions the downstream chain gates on -------------------------
        // The slim reader replaces BOTH ElectronSelectionAlg (HasGoodElectron)
        // and DISSelectionAlg (HasGoodDISElectron), so it must set both —
        // EventHelpers::get_selected_electron gates on HasGoodElectron.
        event.set_decision_typed(Decisions::HasGoodElectron, true);
        event.set_decision_typed(Decisions::HasGoodDISElectron, true);
        event.set_decision_typed(Decisions::HasGoodPhoton, !photons.empty());

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override { return StatusCode::SUCCESS(); }
};

}  // namespace algorithms
