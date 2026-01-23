#pragma once

#include <optional>
#include <vector>
#include "CLAS/Banks.hpp"
#include "CLAS/Detectors.hpp"
#include "CLAS/Particle.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/AutoCutflow.hpp"
#include "Framework/Decisions.hpp"
#include "Framework/Event.hpp"
#include "Framework/HistogramFiller.hpp"
#include "Framework/HistogramTemplates.hpp"
#include "Framework/NTupleStruct.hpp"
#include "Framework/Property.hpp"
#include "Framework/ServiceHelpers.hpp"
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/Services/ElectronCutsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"

namespace Algorithms {

using Framework::Log;

/**
 * @brief N-Tuple data structure for electron kinematics
 *
 * Using NTUPLE_FIELDS macro for automatic branch registration.
 */
struct ElectronNTupleData {
    double e_p = 0.0;
    double e_theta = 0.0;
    double e_phi = 0.0;
    double e_vz = 0.0;
    double e_chi2pid = 0.0;

    NTUPLE_FIELDS(e_p, e_theta, e_phi, e_vz, e_chi2pid)
};

/**
 * @brief Context for electron cut evaluation
 *
 * Holds all per-event data needed for cut decisions, enabling
 * the ContextCutflow to evaluate cuts with full context access.
 */
struct ElectronCutContext {
    const CLAS::Particle& electron;
    const CLAS::CalorimeterBank& calo;
    const CLAS::DCTrajBank& traj;
    double SF;
    int sector;
};

/**
 * @brief Algorithm for electron selection and kinematics
 *
 * Modernized using:
 * - HistogramFiller for one-line histogram filling (reduces ~56 lines to ~4)
 * - ContextCutflow for combined cut definition and tracking
 * - NTupleWriter for struct-based N-tuple management
 * - SafeService for cleaner service access
 */
class ElectronSelectionAlg : public Framework::SimpleAlgorithm<ElectronSelectionAlg> {
   public:
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", CLAS::Target target = CLAS::Target::LD2, CLAS::Polarity polarity = CLAS::Polarity::Outbending)
        : SimpleAlgorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Electron Selection") {
        declare_property(m_enable_ntuple);
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_hist_svc = &require_service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<Framework::NTupleService>("NTupleService");
        m_sf_svc = &require_service<Framework::SamplingFractionService>("SamplingFractionService");
        m_electron_cuts_svc = &require_service<Framework::ElectronCutsService>("ElectronCutsService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> Framework::StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Target", CLAS::to_string(m_target));
        Log::config(name(), "Polarity", CLAS::to_string(m_polarity));
        Log::config(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");

        // Configure ElectronCutsService
        Framework::safe(m_electron_cuts_svc).with([this](auto* svc) {svc->set_target(m_target); svc->set_polarity(m_polarity);
            Framework::safe(m_sf_svc).with([svc](auto* sf) { svc->set_sf_service(sf); });
        });

        // Book histograms using templates
        Framework::safe(m_hist_svc).with([this](auto* svc) {
            kinematics_groups = Framework::HistogramTemplates::ElectronHistograms::create_with_stages(
                svc, "/Electron/Kinematics", {"before_cuts", "after_cuts"});
        });

        // Setup N-Tuple using NTupleWriter
        if (m_enable_ntuple.get()) {
            Framework::safe(m_ntuple_svc).with([this](auto* svc) { m_ntuple.init(svc, "electron_kinematics"); });
        }

        // Define cutflow with predicates (single definition, auto-tracking)
        Framework::ElectronCutsService* cuts_svc = m_electron_cuts_svc->get();
        m_cutflow.define_always("All Events")
            .define("Has Electron", [](const ElectronCutContext&) { return true; })  // Already filtered before context
            .define("Chi2 PID", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_chi2pid_cut(ctx.electron.chi2pid()); })
            .define("Momentum > 0.8 GeV", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_momentum_cut(ctx.electron.p()); })
            .define("Vertex Z", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_vz_cut(ctx.electron.vz()); })
            .define("Sampling Fraction", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_sampling_fraction_cut(ctx.SF, ctx.electron.p(), ctx.sector); })
            .define("PCAL Fiducial", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_pcal_fiducial_cut(ctx.calo.pcal.lv, ctx.calo.pcal.lw); })
            .define("DC Chi2/NDF", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_dc_edge_cut(ctx.traj.region1.edge, ctx.traj.region2.edge, ctx.traj.region3.edge); });

        auto vz_range = cuts_svc->get_vz_range(CLAS::RunGroup::RG_D, m_target);
        Log::config(name(), "Vz range", fmt::format("[{:.2f}, {:.2f}] cm", vz_range.min, vz_range.max));
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(Framework::Event& event) -> Framework::StatusCode override {
        // Get electrons from event
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);

        auto& hipo_event = event.hipo_event();
        auto [REC_Calorimeter, REC_Traj, REC_Track] = hipo_event.banks("REC::Calorimeter", "REC::Traj", "REC::Track");

        if (electrons.empty()) {
            event.set_decision_typed(Framework::Decisions::HasGoodElectron, false);
            return Framework::StatusCode::FILTERED();
        }

        // Select trigger electron
        const std::optional<CLAS::Particle>& possible_electron = Physics::ParticleUtils::find_trigger_electron(electrons, {CLAS::FD, CLAS::BD});
        if (!possible_electron.has_value()) {
            event.set_decision_typed(Framework::Decisions::HasGoodElectron, false);
            return Framework::StatusCode::FILTERED();
        }
        const CLAS::Particle& electron = possible_electron.value();

        // Get detector info
        CLAS::CalorimeterBank calorimeter = CLAS::read_Calorimeter_bank(*REC_Calorimeter, electron.index());
        CLAS::DCTrajBank traj = CLAS::read_Traj_bank(*REC_Traj, electron.index());
        CLAS::TrackBank trk_r1 = CLAS::read_Track_bank(*REC_Track, traj.region1.index);
        CLAS::TrackBank trk_r2 = CLAS::read_Track_bank(*REC_Track, traj.region2.index);
        CLAS::TrackBank trk_r3 = CLAS::read_Track_bank(*REC_Track, traj.region3.index);

        // Calculate derived quantities
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / electron.p();
        const int sector = calorimeter.pcal.sector;

        // Fill before cuts - ONE LINE instead of 28!
        Framework::HistogramFiller::fill_electron(kinematics_groups["before_cuts"].get(), electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

        // Create context for cut evaluation
        ElectronCutContext ctx{electron, calorimeter, traj, SF, sector};

        // Evaluate all cuts
        if (!m_cutflow.evaluate(ctx)) {
            event.set_decision_typed(Framework::Decisions::HasGoodElectron, false);
            return Framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts
        event.set_decision_typed(Framework::Decisions::HasGoodElectron, true);
        event.store_typed(Framework::Keys::SelectedElectronsCapital, std::vector<CLAS::Particle>{electron});

        // Fill after cuts
        Framework::HistogramFiller::fill_electron(kinematics_groups["after_cuts"].get(), electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

        // Fill N-tuple using NTupleWriter
        if (m_ntuple) {
            auto& nt = m_ntuple.data();
            nt.e_p = electron.p();
            nt.e_theta = electron.theta();
            nt.e_phi = electron.phi();
            nt.e_vz = electron.vz();
            nt.e_chi2pid = electron.chi2pid();
            m_ntuple.fill();
        }

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> Framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        Framework::safe(m_ntuple_svc).with([this](auto* svc) {
            auto entries = svc->get_entries("electron_kinematics");
            Log::stat(name(), "N-tuple entries", entries);
        });

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return Framework::StatusCode::SUCCESS();
    }

   private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;

    // Cutflow with automatic tracking
    Framework::ContextCutflow<ElectronCutContext> m_cutflow;

    // Properties
    Framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles
    Framework::ServiceHandle<Framework::HistogramService>* m_hist_svc = nullptr;
    Framework::ServiceHandle<Framework::NTupleService>* m_ntuple_svc = nullptr;
    Framework::ServiceHandle<Framework::SamplingFractionService>* m_sf_svc = nullptr;
    Framework::ServiceHandle<Framework::ElectronCutsService>* m_electron_cuts_svc = nullptr;

    // Histogram groups
    std::map<std::string, std::unique_ptr<Framework::HistogramGroup>> kinematics_groups;

    // N-tuple writer (replaces 5 separate member variables + setup function)
    Framework::NTupleWriter<ElectronNTupleData> m_ntuple;
};

}  // namespace Algorithms
