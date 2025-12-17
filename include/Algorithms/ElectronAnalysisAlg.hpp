#pragma once

/**
 * @file ElectronAnalysisAlg.hpp
 * @brief Electron analysis algorithm following C++ Core Guidelines
 *
 * Key features:
 * - Constexpr cut names to avoid magic strings
 * - ServiceHandle for consistent service access
 * - Separated concerns: configuration, cuts, histograms
 * - RAII patterns throughout
 */

#include <array>
#include <string_view>

#include "CLAS/ElectronCuts.hpp"
#include "CLAS/Particle.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/CutSequence.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "Framework/StatusCode.hpp"
#include "Physics/Kinematics.hpp"
#include "TH1D.h"
#include "TH2D.h"

namespace Algorithms {

// ============================================================================
// Constants
// ============================================================================
namespace ElectronCutNames {
    inline constexpr std::string_view kAllEvents = "All Events";
    inline constexpr std::string_view kTriggerElectron = "Trigger Electron";
    inline constexpr std::string_view kChi2PID = "Chi2 PID";
    inline constexpr std::string_view kMomentum = "Momentum > 0.8 GeV";
    inline constexpr std::string_view kVertexZ = "Vertex Z";
    inline constexpr std::string_view kSamplingFraction = "Sampling Fraction";
    inline constexpr std::string_view kPCALFiducial = "PCAL Fiducial";
    inline constexpr std::string_view kDCChi2NDF = "DC Chi2/NDF";

    inline constexpr std::array kAllCuts = {
        kAllEvents, kTriggerElectron, kChi2PID, kMomentum,
        kVertexZ, kSamplingFraction, kPCALFiducial, kDCChi2NDF
    };
}  // namespace ElectronCutNames

// ============================================================================
// Configuration
// ============================================================================
struct ElectronAnalysisConfig {
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;
    bool enable_ntuple = true;
    bool enable_cut_histograms = true;

    // Trigger selection
    double trigger_p_min = 2000.0;
    double trigger_p_max = 4000.0;

    // Histogram binning
    struct Binning {
        int n_bins;
        double min;
        double max;
    };

    Binning momentum{120, 0, 12};
    Binning theta{180, 0, 180};
    Binning phi{180, -180, 180};
    Binning vz{200, -20, 20};
};

// ============================================================================
// Electron Candidate
// ============================================================================
struct ElectronCandidate {
    CLAS::Particle particle;
    CLAS::CalorimeterBank calorimeter{};
    CLAS::DCTrajBank trajectory{};
    CLAS::TrackBank track{};

    [[nodiscard]] auto total_energy() const -> double {
        return calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
    }

    [[nodiscard]] auto sampling_fraction() const -> double {
        return particle.p() > 0 ? total_energy() / particle.p() : 0.0;
    }

    [[nodiscard]] auto sector() const -> int { return calorimeter.pcal.sector; }

    [[nodiscard]] auto is_valid() const -> bool {
        return sector() >= 1 && sector() <= 6 && particle.p() > 0;
    }
};

// ============================================================================
// Algorithm Implementation
// ============================================================================
class ElectronAnalysisAlg : public Framework::Algorithm {
public:
    using Config = ElectronAnalysisConfig;

    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysis", Config config = Config{})
        : Algorithm(std::move(name)), m_config(std::move(config)) {}

    auto initialize() -> Framework::StatusCode override {
        // Initialize services using ServiceHandle
        if (Framework::StatusCode sc = initialize_services(); sc.is_failure()) return sc;
    
        book_histograms();

        if (m_ntuple_svc && m_config.enable_ntuple) setup_ntuple();

        Framework::Log::info(name(), "Initialized - Target: {}, Polarity: {}",
                             CLAS::to_string(m_config.target),
                             CLAS::to_string(m_config.polarity));

        return Framework::StatusCode::SUCCESS();
    }

    /**
    * @brief Execute the algorithm for each event
    * @param event The event to process
    * @return StatusCode indicating success or filtering, will set decision flag "HasGoodElectron" if passed
    */
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cuts.cutflow().pass(ElectronCutNames::kAllEvents);

        // Extract candidate
        auto candidate = extract_candidate(event);
        if (!candidate) return Framework::StatusCode::FILTERED();
        m_cuts.cutflow().pass(ElectronCutNames::kTriggerElectron);

        // Store for cut lambdas
        m_current_candidate = &(*candidate);

        // Apply cuts
        if (!apply_selection_cuts()) return Framework::StatusCode::FILTERED();

        // All cuts passed
        fill_final_histograms(*candidate);
        fill_ntuple(*candidate);
        event.set_decision("HasGoodElectron", true);

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        Framework::Log::info(name(), "Events passing all cuts: {}",
                             m_cuts.cutflow().final_count());
        m_cuts.print();
        return Framework::StatusCode::SUCCESS();
    }

    /// Access configuration
    [[nodiscard]] auto config() const -> const Config& { return m_config; }

private:
    Config m_config;

    // Services
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};
    Framework::ServiceHandle<Framework::ParticleSelectorService> m_particle_svc{"ParticleSelectorService"};
    Framework::ServiceHandle<Framework::SamplingFractionService> m_sf_svc{"SamplingFractionService"};

    // Cut sequence
    Framework::CutSequence m_cuts{"Electron Selection"};
    const ElectronCandidate* m_current_candidate = nullptr;

    // Histograms (non-owning, managed by HistogramService)
    struct Histograms {
        TH1D* momentum = nullptr;
        TH1D* theta = nullptr;
        TH1D* phi = nullptr;
        TH1D* vz = nullptr;
        TH1D* chi2pid = nullptr;
        TH2D* p_vs_theta = nullptr;
        TH2D* sf_vs_p = nullptr;
    } m_hist;

    // N-tuple variables
    struct NTupleVars {
        double p = 0, theta = 0, phi = 0, vz = 0, chi2pid = 0, sf = 0;
        int sector = 0;
    } m_nt;

    // ========================================================================
    // Initialization helpers
    // ========================================================================
    auto initialize_services() -> Framework::StatusCode {
        // LoggingService is optional
        initialize_handle(m_log_svc);

        // Required services
        if (auto sc = initialize_handle(m_hist_svc); sc.is_failure()) {
            Framework::Log::error(name(), "HistogramService required: {}", sc.message());
            return sc;
        }

        if (auto sc = initialize_handle(m_particle_svc); sc.is_failure()) {
            Framework::Log::error(name(), "ParticleSelectorService required: {}", sc.message());
            return sc;
        }

        if (auto sc = initialize_handle(m_sf_svc); sc.is_failure()) {
            Framework::Log::error(name(), "SamplingFractionService required: {}", sc.message());
            return sc;
        }

        // Optional services
        initialize_handle(m_ntuple_svc);

        return Framework::StatusCode::SUCCESS();
    }

    auto setup_cut_sequence() -> void {
        for (const auto& cut_name : ElectronCutNames::kAllCuts) {
            m_cuts.cutflow().add_cut(std::string(cut_name));
        }
        m_cuts.set_logger(m_log_svc.get());
    }

    // ========================================================================
    // Event processing
    // ========================================================================
    [[nodiscard]] auto extract_candidate(Framework::Event& event) -> std::optional<ElectronCandidate> {

        auto& hipo = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Calorimeter, REC_Traj, REC_Track] =
            hipo.banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Traj", "REC::Track");

        if (REC_Event->rows() == 0) return std::nullopt;

        // Extract electrons
        auto particles = m_particle_svc->extract_particles(*REC_Particle, {CLAS::PIDs::ELECTRON});
        const auto& electrons = m_particle_svc->get_electrons(particles);

        if (electrons.empty()) return std::nullopt;

        // Find trigger electron
        auto trigger = Physics::ParticleUtils::find_trigger_electron(
            electrons, {m_config.trigger_p_min, m_config.trigger_p_max});

        if (!trigger) return std::nullopt;

        // Build candidate with detector info
        ElectronCandidate candidate;
        candidate.particle = *trigger;
        candidate.calorimeter = CLAS::read_Calorimeter_bank(*REC_Calorimeter, trigger->index());
        candidate.trajectory = CLAS::read_Traj_bank(*REC_Traj, trigger->index());
        candidate.track = CLAS::read_Track_bank(*REC_Track, trigger->index());

        return candidate;
    }

    [[nodiscard]] auto apply_selection_cuts() -> bool {
        const auto& c = *m_current_candidate;
        const auto& e = c.particle;
        const auto& calo = c.calorimeter;
        const auto& traj = c.trajectory;

        // Chi2 PID
        if (!CLAS::ElectronCuts::pass_chi2pid_cut(e.chi2pid())) return false;
        m_cuts.cutflow().pass(std::string(ElectronCutNames::kChi2PID));

        // Momentum
        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(e.p())) return false;
        m_cuts.cutflow().pass(std::string(ElectronCutNames::kMomentum));

        // Vertex Z
        if (!CLAS::ElectronCuts::pass_electron_vz_cut(e.vz(), m_config.target)) return false;
        m_cuts.cutflow().pass(std::string(ElectronCutNames::kVertexZ));

        // Sampling fraction
        if (!m_sf_svc->passes_cut(c.sampling_fraction(), e.p(), c.sector(),
                                   m_config.target, m_config.polarity)) return false;
        m_cuts.cutflow().pass(std::string(ElectronCutNames::kSamplingFraction));

        // PCAL fiducial
        if (!CLAS::ElectronCuts::pass_pcal_fiducial_cut(calo.pcal.lv, calo.pcal.lw)) return false;
        m_cuts.cutflow().pass(std::string(ElectronCutNames::kPCALFiducial));

        // DC chi2/ndf
        if (!CLAS::ElectronCuts::pass_chi2ndf_cut(traj.region1.edge, traj.region2.edge,
                                                   traj.region3.edge)) return false;
        m_cuts.cutflow().pass(std::string(ElectronCutNames::kDCChi2NDF));

        return true;
    }

    // ========================================================================
    // Histogram management
    // ========================================================================
    auto book_histograms() -> void {
        const auto& cfg = m_config;

        m_hist.momentum = m_hist_svc->book_hist1D(
            "/Electron/momentum", "Electron p;p [GeV/c];Events",
            cfg.momentum.n_bins, cfg.momentum.min, cfg.momentum.max);

        m_hist.theta = m_hist_svc->book_hist1D(
            "/Electron/theta", "Electron #theta;#theta [deg];Events",
            cfg.theta.n_bins, cfg.theta.min, cfg.theta.max);

        m_hist.phi = m_hist_svc->book_hist1D(
            "/Electron/phi", "Electron #phi;#phi [deg];Events",
            cfg.phi.n_bins, cfg.phi.min, cfg.phi.max);

        m_hist.vz = m_hist_svc->book_hist1D(
            "/Electron/vz", "Electron v_{z};v_{z} [cm];Events",
            cfg.vz.n_bins, cfg.vz.min, cfg.vz.max);

        m_hist.chi2pid = m_hist_svc->book_hist1D(
            "/Electron/chi2pid", "#chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);

        m_hist.p_vs_theta = m_hist_svc->book_hist2D(
            "/Electron/p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
            cfg.theta.n_bins, cfg.theta.min, cfg.theta.max,
            cfg.momentum.n_bins, cfg.momentum.min, cfg.momentum.max);

        m_hist.sf_vs_p = m_hist_svc->book_hist2D(
            "/Electron/sf_vs_p", "SF vs p;p [GeV/c];E/p",
            cfg.momentum.n_bins, cfg.momentum.min, cfg.momentum.max, 100, 0, 0.5);

        setup_cut_sequence();
    }

    auto fill_final_histograms(const ElectronCandidate& c) -> void {
        const auto& e = c.particle;
        m_hist.momentum->Fill(e.p());
        m_hist.theta->Fill(e.theta());
        m_hist.phi->Fill(e.phi());
        m_hist.vz->Fill(e.vz());
        m_hist.chi2pid->Fill(e.chi2pid());
        m_hist.p_vs_theta->Fill(e.theta(), e.p());
        m_hist.sf_vs_p->Fill(e.p(), c.sampling_fraction());
    }

    // ========================================================================
    // N-tuple management
    // ========================================================================
    auto setup_ntuple() -> void {
        constexpr auto tree = "electron_analysis";
        m_ntuple_svc->register_branch(tree, "e_p", &m_nt.p);
        m_ntuple_svc->register_branch(tree, "e_theta", &m_nt.theta);
        m_ntuple_svc->register_branch(tree, "e_phi", &m_nt.phi);
        m_ntuple_svc->register_branch(tree, "e_vz", &m_nt.vz);
        m_ntuple_svc->register_branch(tree, "e_chi2pid", &m_nt.chi2pid);
        m_ntuple_svc->register_branch(tree, "sampling_fraction", &m_nt.sf);
        m_ntuple_svc->register_branch(tree, "sector", &m_nt.sector);
        Framework::Log::info(name(), "N-tuple branches registered");
    }

    auto fill_ntuple(const ElectronCandidate& c) -> void {
        if (!m_ntuple_svc || !m_config.enable_ntuple) return;

        const auto& e = c.particle;
        m_nt.p = e.p();
        m_nt.theta = e.theta();
        m_nt.phi = e.phi();
        m_nt.vz = e.vz();
        m_nt.chi2pid = e.chi2pid();
        m_nt.sf = c.sampling_fraction();
        m_nt.sector = c.sector();
        m_ntuple_svc->fill("electron_analysis");
    }
};

}  // namespace Algorithms
