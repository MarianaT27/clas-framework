#pragma once

#include <optional>
#include <vector>
#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/HistogramFiller.hpp>
#include <clas12/framework/core/HistogramTemplates.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

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
    const clas12::Particle& electron;
    const clas12::CalorimeterBank& calo;
    const clas12::DCTrajBank& traj;
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
class ElectronSelectionAlg : public clas12::framework::SimpleAlgorithm<ElectronSelectionAlg> {
   public:
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", clas12::Target target = clas12::Target::LD2, clas12::Polarity polarity = clas12::Polarity::Outbending)
        : SimpleAlgorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Electron Selection") {
        declare_property(m_enable_ntuple);
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<clas12::framework::NTupleService>("NTupleService");
        m_sf_svc = &require_service<clas12::framework::SamplingFractionService>("SamplingFractionService");
        m_electron_cuts_svc = &require_service<clas12::framework::ElectronCutsService>("ElectronCutsService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Target", clas12::to_string(m_target));
        Log::config(name(), "Polarity", clas12::to_string(m_polarity));
        Log::config(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");

        // Configure ElectronCutsService
        clas12::framework::safe(m_electron_cuts_svc).with([this](auto* svc) {svc->set_target(m_target); svc->set_polarity(m_polarity);
            clas12::framework::safe(m_sf_svc).with([svc](auto* sf) { svc->set_sf_service(sf); });
        });

        // Book histograms using templates
        clas12::framework::safe(m_hist_svc).with([this](auto* svc) {
            kinematics_groups = clas12::framework::HistogramTemplates::ElectronHistograms::create_with_stages(
                svc, "/Electron/Kinematics", {"before_cuts", "after_cuts"});
        });

        // Setup N-Tuple using NTupleWriter
        if (m_enable_ntuple.get()) {
            clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) { m_ntuple.init(svc, "electron_kinematics"); });
        }

        // Define cutflow with predicates (single definition, auto-tracking)
        clas12::framework::ElectronCutsService* cuts_svc = m_electron_cuts_svc->get();
        m_cutflow.define_always("All Events")
            .define("Has Electron", [](const ElectronCutContext&) { return true; })  // Already filtered before context
            .define("Chi2 PID", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_chi2pid_cut(ctx.electron.chi2pid()); })
            .define("Momentum > 0.8 GeV", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_momentum_cut(ctx.electron.p()); })
            .define("Vertex Z", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_vz_cut(ctx.electron.vz()); })
            .define("Sampling Fraction", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_sampling_fraction_cut(ctx.SF, ctx.electron.p(), ctx.sector); })
            .define("PCAL Fiducial", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_pcal_fiducial_cut(ctx.calo.pcal.lv, ctx.calo.pcal.lw); })
            .define("DC Chi2/NDF", [cuts_svc](const ElectronCutContext& ctx) { return cuts_svc->pass_dc_edge_cut(ctx.traj.region1.edge, ctx.traj.region2.edge, ctx.traj.region3.edge); });

        auto vz_range = cuts_svc->get_vz_range(clas12::RunGroup::RG_D, m_target);
        Log::config(name(), "Vz range", fmt::format("[{:.2f}, {:.2f}] cm", vz_range.min, vz_range.max));
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Get electrons from event
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);

        auto& hipo_event = event.hipo_event();
        auto [REC_Calorimeter, REC_Traj, REC_Track] = hipo_event.banks("REC::Calorimeter", "REC::Traj", "REC::Track");

        if (electrons.empty()) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }

        // Select trigger electron
        const std::optional<clas12::Particle>& possible_electron = clas12::physics::ParticleUtils::find_trigger_electron(electrons, {clas12::FD, clas12::BD});
        if (!possible_electron.has_value()) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        const clas12::Particle& electron = possible_electron.value();

        // Get detector info
        clas12::CalorimeterBank calorimeter = clas12::read_Calorimeter_bank(*REC_Calorimeter, electron.index());
        clas12::DCTrajBank traj = clas12::read_Traj_bank(*REC_Traj, electron.index());
        clas12::TrackBank trk_r1 = clas12::read_Track_bank(*REC_Track, traj.region1.index);
        clas12::TrackBank trk_r2 = clas12::read_Track_bank(*REC_Track, traj.region2.index);
        clas12::TrackBank trk_r3 = clas12::read_Track_bank(*REC_Track, traj.region3.index);

        // Calculate derived quantities
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / electron.p();
        const int sector = calorimeter.pcal.sector;

        // Fill before cuts - ONE LINE instead of 28!
        clas12::framework::HistogramFiller::fill_electron(kinematics_groups["before_cuts"].get(), electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

        // Create context for cut evaluation
        ElectronCutContext ctx{electron, calorimeter, traj, SF, sector};

        // Evaluate all cuts
        if (!m_cutflow.evaluate(ctx)) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts
        event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, true);
        event.store_typed(clas12::framework::Keys::SelectedElectronsCapital, std::vector<clas12::Particle>{electron});

        // Fill after cuts
        clas12::framework::HistogramFiller::fill_electron(kinematics_groups["after_cuts"].get(), electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

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

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) {
            auto entries = svc->get_entries("electron_kinematics");
            Log::stat(name(), "N-tuple entries", entries);
        });

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    clas12::Polarity m_polarity;

    // Cutflow with automatic tracking
    clas12::framework::ContextCutflow<ElectronCutContext> m_cutflow;

    // Properties
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::SamplingFractionService>* m_sf_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ElectronCutsService>* m_electron_cuts_svc = nullptr;

    // Histogram groups
    std::map<std::string, std::unique_ptr<clas12::framework::HistogramGroup>> kinematics_groups;

    // N-tuple writer (replaces 5 separate member variables + setup function)
    clas12::framework::NTupleWriter<ElectronNTupleData> m_ntuple;
};

}  // namespace clas12::algorithms
