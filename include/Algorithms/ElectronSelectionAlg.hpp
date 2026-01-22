#pragma once

#include <optional>
#include <vector>
#include "CLAS/Banks.hpp"
#include "CLAS/Detectors.hpp"
#include "CLAS/Particle.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/CutBuilder.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Decisions.hpp"
#include "Framework/Event.hpp"
#include "Framework/HistogramTemplates.hpp"
#include "Framework/Property.hpp"
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/ElectronCutsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"

namespace Algorithms {

using Framework::Log;

/**
 * @brief Algorithm for electron selection and kinematics
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - CutBuilder for fluent cut definitions
 * - Reduced boilerplate through CRTP
 */
class ElectronSelectionAlg : public Framework::SimpleAlgorithm<ElectronSelectionAlg> {
   public:
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg",
                                   CLAS::Target target = CLAS::Target::LD2,
                                   CLAS::Polarity polarity = CLAS::Polarity::Outbending)
        : SimpleAlgorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Electron Selection") {
        // Declare properties
        declare_property(m_enable_ntuple);
    }

    /**
     * @brief Configure required services (called automatically during initialize)
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
        Log::info(name(), "Initializing algorithm");

        // Configure ElectronCutsService
        if (m_electron_cuts_svc && m_electron_cuts_svc->is_valid()) {
            m_electron_cuts_svc->get()->set_target(m_target);
            m_electron_cuts_svc->get()->set_polarity(m_polarity);
            if (m_sf_svc && m_sf_svc->is_valid()) {
                m_electron_cuts_svc->get()->set_sf_service(m_sf_svc->get());
            }
        }

        book_histograms();
        if (m_ntuple_svc && m_ntuple_svc->is_valid() && m_enable_ntuple.get()) {
            setup_ntuple();
        }

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        auto vz_range = m_electron_cuts_svc->get()->get_vz_range(CLAS::RunGroup::RG_D, m_target);
        Log::info(name(), "Using Vz range: [{:.2f}, {:.2f}] for {}", vz_range.min, vz_range.max, CLAS::to_string(m_target));

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons from event
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);

        auto& hipo_event = event.hipo_event();
        auto [REC_Calorimeter, REC_Traj, REC_Track] = hipo_event.banks("REC::Calorimeter", "REC::Traj", "REC::Track");

        if (electrons.empty()) {
            event.set_decision_typed(Framework::Decisions::HasGoodElectron, false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Select trigger electron
        const std::optional<CLAS::Particle>& possible_electron = Physics::ParticleUtils::find_trigger_electron(electrons, {CLAS::FD, CLAS::BD});
        if (!possible_electron.has_value()) {
            event.set_decision_typed(Framework::Decisions::HasGoodElectron, false);
            return Framework::StatusCode::FILTERED();
        }
        const CLAS::Particle& electron = possible_electron.value();

        // Get detector info for electron
        CLAS::CalorimeterBank calorimeter = CLAS::read_Calorimeter_bank(*REC_Calorimeter, electron.index());
        CLAS::DCTrajBank traj = CLAS::read_Traj_bank(*REC_Traj, electron.index());
        CLAS::TrackBank trk_r1 = CLAS::read_Track_bank(*REC_Track, traj.region1.index);
        CLAS::TrackBank trk_r2 = CLAS::read_Track_bank(*REC_Track, traj.region2.index);
        CLAS::TrackBank trk_r3 = CLAS::read_Track_bank(*REC_Track, traj.region3.index);

        // Calculate derived quantities
        const double p = electron.p();
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / p;
        const int sector = calorimeter.pcal.sector;

        kinematics_groups["before_cuts"]->hist1d("p").fill(p);
        kinematics_groups["before_cuts"]->hist1d("theta").fill(electron.theta());
        kinematics_groups["before_cuts"]->hist1d("phi").fill(electron.phi());
        kinematics_groups["before_cuts"]->hist1d("sector").fill(static_cast<double>(sector));
        kinematics_groups["before_cuts"]->hist1d("vz").fill(electron.vz());
        kinematics_groups["before_cuts"]->hist1d("vx").fill(electron.vx());
        kinematics_groups["before_cuts"]->hist1d("vy").fill(electron.vy());
        kinematics_groups["before_cuts"]->hist1d("chi2pid").fill(electron.chi2pid());
        kinematics_groups["before_cuts"]->hist1d("beta").fill(electron.beta());
        kinematics_groups["before_cuts"]->hist1d("pcal_E").fill(calorimeter.pcal.energy);
        kinematics_groups["before_cuts"]->hist1d("ecin_E").fill(calorimeter.inner.energy);
        kinematics_groups["before_cuts"]->hist1d("ecout_E").fill(calorimeter.outer.energy);
        kinematics_groups["before_cuts"]->hist1d("ecal_E").fill(E_tot);
        kinematics_groups["before_cuts"]->hist1d("sampling_fraction").fill(SF);
        kinematics_groups["before_cuts"]->hist1d("dc_chi2_R1").fill(trk_r1.chi2 / trk_r1.ndf);
        kinematics_groups["before_cuts"]->hist1d("dc_chi2_R2").fill(trk_r2.chi2 / trk_r2.ndf);
        kinematics_groups["before_cuts"]->hist1d("dc_chi2_R3").fill(trk_r3.chi2 / trk_r3.ndf);
        kinematics_groups["before_cuts"]->hist1d("pcal_lu").fill(calorimeter.pcal.lu);
        kinematics_groups["before_cuts"]->hist1d("pcal_lv").fill(calorimeter.pcal.lv);
        kinematics_groups["before_cuts"]->hist1d("pcal_lw").fill(calorimeter.pcal.lw);
        kinematics_groups["before_cuts"]->hist2d("sf_vs_p").fill(p, SF);
        kinematics_groups["before_cuts"]->hist2d("chi2pid_vs_p").fill(p, electron.chi2pid());
        kinematics_groups["before_cuts"]->hist2d("vz_vs_phi").fill(electron.phi(), electron.vz());
        kinematics_groups["before_cuts"]->hist2d("vz_vs_theta").fill(electron.theta(), electron.vz());
        kinematics_groups["before_cuts"]->hist2d("theta_vs_phi").fill(electron.phi(), electron.theta());
        kinematics_groups["before_cuts"]->hist2d("p_vs_theta").fill(electron.theta(), p);
        kinematics_groups["before_cuts"]->hist2d("pcal_lv_vs_lu").fill(calorimeter.pcal.lu, calorimeter.pcal.lv);
        kinematics_groups["before_cuts"]->hist2d("ecin_vs_pcal").fill(calorimeter.pcal.energy, calorimeter.inner.energy);
        

        auto* cuts_svc = m_electron_cuts_svc->get();

        bool passed = Framework::CutBuilder(m_cutflow)
            .require("Chi2 PID", cuts_svc->pass_chi2pid_cut(electron.chi2pid()))
            .require("Momentum > 0.8 GeV", cuts_svc->pass_momentum_cut(p))
            .require("Vertex Z", cuts_svc->pass_vz_cut(electron.vz()))
            .require("Sampling Fraction", cuts_svc->pass_sampling_fraction_cut(SF, p, sector))
            .require("PCAL Fiducial", cuts_svc->pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw))
            .require("DC Chi2/NDF", cuts_svc->pass_dc_edge_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge))
            .evaluate();

        if (!passed) {
            event.set_decision_typed(Framework::Decisions::HasGoodElectron, false);
            return Framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision_typed(Framework::Decisions::HasGoodElectron, true);
        event.store_typed(Framework::Keys::SelectedElectronsCapital, std::vector<CLAS::Particle>{electron});

        // Fill histograms after selection using templates
        kinematics_groups["after_cuts"]->hist1d("p").fill(p);
        kinematics_groups["after_cuts"]->hist1d("theta").fill(electron.theta());
        kinematics_groups["after_cuts"]->hist1d("phi").fill(electron.phi());
        kinematics_groups["after_cuts"]->hist1d("sector").fill(static_cast<double>(sector));
        kinematics_groups["after_cuts"]->hist1d("vz").fill(electron.vz());
        kinematics_groups["after_cuts"]->hist1d("vx").fill(electron.vx());
        kinematics_groups["after_cuts"]->hist1d("vy").fill(electron.vy());
        kinematics_groups["after_cuts"]->hist1d("chi2pid").fill(electron.chi2pid());
        kinematics_groups["after_cuts"]->hist1d("beta").fill(electron.beta());
        kinematics_groups["after_cuts"]->hist1d("pcal_E").fill(calorimeter.pcal.energy);
        kinematics_groups["after_cuts"]->hist1d("ecin_E").fill(calorimeter.inner.energy);
        kinematics_groups["after_cuts"]->hist1d("ecout_E").fill(calorimeter.outer.energy);
        kinematics_groups["after_cuts"]->hist1d("ecal_E").fill(E_tot);
        kinematics_groups["after_cuts"]->hist1d("sampling_fraction").fill(SF);
        kinematics_groups["after_cuts"]->hist1d("dc_chi2_R1").fill(trk_r1.chi2 / trk_r1.ndf);
        kinematics_groups["after_cuts"]->hist1d("dc_chi2_R2").fill(trk_r2.chi2 / trk_r2.ndf);
        kinematics_groups["after_cuts"]->hist1d("dc_chi2_R3").fill(trk_r3.chi2 / trk_r3.ndf);
        kinematics_groups["after_cuts"]->hist1d("pcal_lu").fill(calorimeter.pcal.lu);
        kinematics_groups["after_cuts"]->hist1d("pcal_lv").fill(calorimeter.pcal.lv);
        kinematics_groups["after_cuts"]->hist1d("pcal_lw").fill(calorimeter.pcal.lw);
        kinematics_groups["after_cuts"]->hist2d("sf_vs_p").fill(p, SF);
        kinematics_groups["after_cuts"]->hist2d("chi2pid_vs_p").fill(p, electron.chi2pid());
        kinematics_groups["after_cuts"]->hist2d("vz_vs_phi").fill(electron.phi(), electron.vz());
        kinematics_groups["after_cuts"]->hist2d("vz_vs_theta").fill(electron.theta(), electron.vz());
        kinematics_groups["after_cuts"]->hist2d("theta_vs_phi").fill(electron.phi(), electron.theta());
        kinematics_groups["after_cuts"]->hist2d("p_vs_theta").fill(electron.theta(), p);
        kinematics_groups["after_cuts"]->hist2d("pcal_lv_vs_lu").fill(calorimeter.pcal.lu, calorimeter.pcal.lv);
        kinematics_groups["after_cuts"]->hist2d("ecin_vs_pcal").fill(calorimeter.pcal.energy, calorimeter.inner.energy);

        // Fill N-tuple if service is available
        if (m_ntuple_svc && m_ntuple_svc->is_valid() && m_enable_ntuple.get()) {
            m_e_p = p;
            m_e_theta = electron.theta();
            m_e_phi = electron.phi();
            m_e_vz = electron.vz();
            m_e_chi2pid = electron.chi2pid();
            m_ntuple_svc->get()->fill("electron_kinematics");
        }

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> Framework::StatusCode override {
        Log::info(name(), "Finalizing algorithm");
        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            auto entries = m_ntuple_svc->get()->get_entries("electron_kinematics");
            Log::info(name(), "N-tuple has {} entries", entries);
        }

        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

   private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    Framework::Cutflow m_cutflow;

    // Properties
    Framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service)
    Framework::ServiceHandle<Framework::HistogramService>* m_hist_svc = nullptr;
    Framework::ServiceHandle<Framework::NTupleService>* m_ntuple_svc = nullptr;
    Framework::ServiceHandle<Framework::SamplingFractionService>* m_sf_svc = nullptr;
    Framework::ServiceHandle<Framework::ElectronCutsService>* m_electron_cuts_svc = nullptr;

    // Histogram groups using templates
    std::map<std::string, std::unique_ptr<Framework::HistogramGroup>> kinematics_groups;

    // N-tuple variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_e_chi2pid = 0.0;

    auto book_histograms() -> void {
        if (!m_hist_svc || !m_hist_svc->is_valid()) return;
        // Use histogram templates for standard electron histograms
        kinematics_groups = Framework::HistogramTemplates::ElectronHistograms::create_with_stages(
                m_hist_svc->get(), "/Electron/Kinematics", {"before_cuts", "after_cuts"});

    }

    auto setup_ntuple() -> void {
        if (!m_ntuple_svc || !m_ntuple_svc->is_valid()) return;
        auto* svc = m_ntuple_svc->get();
        svc->register_branch("electron_kinematics", "e_p", &m_e_p);
        svc->register_branch("electron_kinematics", "e_theta", &m_e_theta);
        svc->register_branch("electron_kinematics", "e_phi", &m_e_phi);
        svc->register_branch("electron_kinematics", "e_vz", &m_e_vz);
        svc->register_branch("electron_kinematics", "e_chi2pid", &m_e_chi2pid);
        Log::info(name(), "N-tuple branches registered for electron kinematics");
    }
};

}  // namespace Algorithms
