#pragma once

#include <Math/Vector4D.h>
#include <TF1.h>
#include <TFile.h>
#include <TMath.h>
#include <TParameter.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/CandidateUtils.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace clas12::algorithms {

// ============================================================================
// Fit functions for Breit-Wigner + polynomial background
// ============================================================================

/**
 * @brief Breit-Wigner function for rho0 resonance
 * par[0] = mass (M0)
 * par[1] = amplitude (normalization)
 */
inline Double_t BreitWigner(Double_t* x, Double_t* par) {
    constexpr double gamma_half = 0.150 / 2.0;  // rho0 width / 2
    return par[1] * (1.0 / (2.0 * TMath::Pi())) * gamma_half /
           ((x[0] - par[0]) * (x[0] - par[0]) + gamma_half * gamma_half);
}

/**
 * @brief 3rd order polynomial background
 * par[2] = p0, par[3] = p1, par[4] = p2, par[5] = p3
 */
inline Double_t Pol3Background(Double_t* x, Double_t* par) {
    return par[2] + par[3] * x[0] + par[4] * x[0] * x[0] + par[5] * x[0] * x[0] * x[0];
}

/**
 * @brief Combined Breit-Wigner + pol3 background
 * par[0] = BW mass, par[1] = BW amplitude
 * par[2-5] = polynomial coefficients
 */
inline Double_t BWPlusPol3(Double_t* x, Double_t* par) {
    return BreitWigner(x, par) + Pol3Background(x, par);
}

// ============================================================================
// Data structures for rho0 reconstruction
// ============================================================================

/**
 * @brief Detector region classification for a pion pair
 */
enum class DetectorRegion { ForwardForward, Mixed, CentralCentral, Unknown };

/**
 * @brief Holds all computed data for a single rho0 candidate
 *
 * This struct consolidates particle references, kinematic variables, and
 * cut decisions to avoid redundant calculations and simplify data flow.
 */
struct Rho0Candidate {
    // Particle references
    clas12::Particle pip;
    clas12::Particle pim;

    // Reconstructed rho0 properties
    double mass = 0.0;
    double momentum = 0.0;
    double theta = 0.0;  // degrees
    double phi = 0.0;    // degrees
    double opening_angle = 0.0;  // degrees

    // Kinematic variables
    double zh = 0.0;
    double lc = 0.0;
    double t = 0.0;       // Mandelstam t (negative)
    double minus_t = 0.0; // -t (positive)

    // Vertex differences
    double delta_vz_pip = 0.0;
    double delta_vz_pim = 0.0;

    // Cut decisions (cached to avoid recomputation)
    bool passes_delta_vz_pip = false;
    bool passes_delta_vz_pim = false;
    bool passes_zh = false;
    bool passes_lc = false;
    bool passes_t = false;
    bool passes_all_cuts = false;

    // Detector region classification
    DetectorRegion detector_region = DetectorRegion::Unknown;
};

/**
 * @brief Algorithm for rho0 reconstruction from charged pion pairs (pi+ pi-)
 *
 * The rho0 meson decays primarily to pi+ pi- with ~100% branching ratio.
 * This algorithm reconstructs rho0 candidates by combining opposite-sign pion pairs
 * and selecting those within a configurable mass window around the nominal rho0 mass (770 MeV).
 *
 * Pion selection:
 * - Forward detector (status 2000-4000)
 * - |chi2pid| < 3
 * - Momentum > threshold
 *
 * Applies kinematic cuts:
 * - zh > 0.9 (energy fraction)
 * - lc < 1 (coherence length in fm)
 * - 0.1 < -t < 0.5 GeV² (momentum transfer)
 *
 * Provides Q²-binned invariant mass histograms for:
 * - Q² = 1-2, 2-2.5, 2.5-3, 3.5-4.5, 4.5-6 GeV²
 *
 * Fits invariant mass with Breit-Wigner + pol3 background
 */
class Rho0ReconstructionAlg : public clas12::framework::SimpleAlgorithm<Rho0ReconstructionAlg> {
   public:
    static constexpr double RHO0_MASS = clas12::Constants::RHO0MASS;  // GeV/c² (~770 MeV)
    static constexpr double RHO0_WIDTH = 0.150;                      // GeV (~150 MeV natural width)
    static constexpr double HBARC = 0.197327;                        // GeV·fm (for coherence length)

    explicit Rho0ReconstructionAlg(
        std::string name = "Rho0ReconstructionAlg",
        clas12::Target target = clas12::Target::LD2,
        std::vector<double> sigma_scan = {}
    )
        : SimpleAlgorithm(std::move(name))
        , m_target(target)
        , m_cutflow("Rho0 Reconstruction")
        , m_candidate_cutflow("Rho0 Candidate Cuts") {
        declare_property(m_zh_min);
        declare_property(m_lc_max);
        declare_property(m_t_min);
        declare_property(m_t_max);
        declare_property(m_enable_delta_vz_pim_cut);
        declare_property(m_enable_delta_vz_pip_cut);
        declare_property(m_enable_zh_cut);
        declare_property(m_enable_lc_cut);
        declare_property(m_enable_t_cut);
        declare_property(m_enable_ntuple);
        declare_property(m_enable_fitting);
        declare_property(m_sigma_scan_values);
        if (!sigma_scan.empty()) {
            m_sigma_scan_values.set(std::move(sigma_scan));
        }
        declare_bank("RUN::config");
    }

    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<clas12::framework::NTupleService>("NTupleService");
        m_output_svc = &require_service<clas12::framework::OutputFileService>("OutputFileService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        using Log = clas12::framework::Log;

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Target", clas12::to_string(m_target));
        Log::config(name(), "Pion selection", "Using pre-selected pions from PionSelectionAlg");
        Log::blank();

        Log::subsection("Kinematic Cuts");
        Log::config(name(), "zh min", fmt::format("{} ({})", m_zh_min.get(), m_enable_zh_cut.get() ? "enabled" : "disabled"));
        Log::config(name(), "lc max", fmt::format("{} fm ({})", m_lc_max.get(), m_enable_lc_cut.get() ? "enabled" : "disabled"));
        Log::config(name(), "-t range", fmt::format("[{}, {}] GeV² ({})", m_t_min.get(), m_t_max.get(), m_enable_t_cut.get() ? "enabled" : "disabled"));
        auto pip_vz = get_piplus_delta_vz_range();
        auto pim_vz = get_piminus_delta_vz_range();
        Log::config(name(), "Δvz π⁺", fmt::format("[{:.2f}, {:.2f}] cm ({})", pip_vz.low, pip_vz.high, m_enable_delta_vz_pip_cut.get() ? "enabled" : "disabled"));
        Log::config(name(), "Δvz π⁻", fmt::format("[{:.2f}, {:.2f}] cm ({})", pim_vz.low, pim_vz.high, m_enable_delta_vz_pim_cut.get() ? "enabled" : "disabled"));
        Log::blank();

        Log::subsection("Output Options");
        Log::config(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");
        Log::config(name(), "BW+pol3 fitting", m_enable_fitting.get() ? "enabled" : "disabled");
        if (!m_sigma_scan_values.get().empty()) {
            Log::config(name(), "Sigma scan", fmt::format("[{}]", fmt::join(m_sigma_scan_values.get(), ", ")));
        }
        Log::blank();

        book_histograms();

        if (m_enable_ntuple.get()) {
            clas12::framework::safe(m_ntuple_svc).with([this](auto*) { setup_ntuple(); });
        }

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Good DIS Electron");
        m_cutflow.add_cut("Has >= 1 Pi+");
        m_cutflow.add_cut("Has >= 1 Pi-");
        m_cutflow.add_cut("Rho0 Found");

        // Candidate-level cuts (declarative predicates)
        m_candidate_cutflow
            .define("delta_vz_pim Cut", [this](const Rho0Candidate& c) {
                return !m_enable_delta_vz_pim_cut.get() || c.passes_delta_vz_pim;
            })
            .define("delta_vz_pip Cut", [this](const Rho0Candidate& c) {
                return !m_enable_delta_vz_pip_cut.get() || c.passes_delta_vz_pip;
            })
            .define("zh > 0.9", [this](const Rho0Candidate& c) {
                return !m_enable_zh_cut.get() || c.passes_zh;
            })
            .define("lc < 1 fm", [this](const Rho0Candidate& c) {
                return !m_enable_lc_cut.get() || c.passes_lc;
            })
            .define("0.1 < -t < 0.5", [this](const Rho0Candidate& c) {
                return !m_enable_t_cut.get() || c.passes_t;
            });

        Log::stat(name(), "Cutflow stages", m_cutflow.num_cuts() + m_candidate_cutflow.num_cuts());
        Log::success(name(), "Initialization complete");

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Track run number from RUN::config bank
        auto& run_config = event.get_bank("RUN::config");
        if (run_config.rows() > 0) {
            int run = run_config.get<int>("run", 0);
            std::lock_guard<std::mutex> lock(m_runs_mutex);
            m_runs.insert(run);
        }

        // Check for good DIS electron from upstream algorithm
        auto electron_ref = clas12::framework::EventHelpers::get_dis_electron(event);
        if (!electron_ref) return clas12::framework::StatusCode::FILTERED();
        const auto& electron = electron_ref->get();
        m_cutflow.pass("Has Good DIS Electron");

        // Get DIS kinematics from upstream
        auto dis = clas12::framework::EventHelpers::get_dis_kinematics(event);
        if (!dis) return clas12::framework::StatusCode::FILTERED();
        double Q2 = dis->Q2;
        double nu = dis->nu;

        // Get pre-selected pions from PionSelectionAlg
        auto pions_plus = event.retrieve_typed(clas12::framework::Keys::SelectedPiPlus).value_or(std::vector<clas12::Particle>{});
        auto pions_minus = event.retrieve_typed(clas12::framework::Keys::SelectedPiMinus).value_or(std::vector<clas12::Particle>{});

        if (pions_plus.empty()) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Has >= 1 Pi+");

        if (pions_minus.empty()) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Has >= 1 Pi-");

        // Calculate virtual photon 4-momentum
        clas12::physics::BeamConfig beam;
        ROOT::Math::PxPyPzEVector beam_4vec(beam.px, beam.py, beam.pz, beam.energy);
        ROOT::Math::PxPyPzEVector virtual_photon = beam_4vec - electron.PxPyPzEVector();

        // Build candidates with all kinematics computed once
        auto candidates = build_rho0_candidates(pions_plus, pions_minus, electron, virtual_photon, nu, Q2);
        if (candidates.empty()) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Rho0 Found");

        // Fill pre-cut histograms
        fill_pre_cut_histograms(candidates);

        // Fill lc-binned histograms (with specific cut combination)
        fill_lc_binned_histograms(candidates, Q2);

        // Update candidate-level cutflow counters
        for (const auto& cand : candidates) {
            (void)m_candidate_cutflow.evaluate(cand);
        }

        // Check if any candidates passed all cuts
        if (count_passing_candidates(candidates) == 0) return clas12::framework::StatusCode::FILTERED();

        // Fill post-cut histograms
        fill_post_cut_histograms(candidates, Q2);

        // Store candidates in event for downstream algorithms
        store_candidates_in_event(candidates, event);

        // Fill ntuple
        fill_ntuple(candidates, electron, Q2);

        // Process sigma scan variations
        for (double scan_sigma : m_sigma_scan_values.get()) {
            std::string label = format_sigma_label(scan_sigma);
            auto sh_it = m_sigma_scan_histograms.find(label);
            if (sh_it == m_sigma_scan_histograms.end()) continue;
            auto& sh = sh_it->second;

            auto* scan_piplus = event.get_particles<clas12::Particle>(
                format_sigma_key("SelectedPiPlus", scan_sigma));
            auto* scan_piminus = event.get_particles<clas12::Particle>(
                format_sigma_key("SelectedPiMinus", scan_sigma));

            if (!scan_piplus || !scan_piminus || scan_piplus->empty() || scan_piminus->empty()) continue;

            auto scan_candidates = build_rho0_candidates(
                *scan_piplus, *scan_piminus, electron, virtual_photon, nu, Q2);

            for (const auto& cand : scan_candidates) {
                if (sh.mass_all) sh.mass_all->fill(cand.mass);

                if (cand.passes_all_cuts) {
                    if (sh.mass_selected) sh.mass_selected->fill(cand.mass);

                    for (size_t i = 0; i < Q2_BINS.size(); ++i) {
                        if (Q2 >= Q2_BINS[i].first && Q2 < Q2_BINS[i].second) {
                            std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", Q2_BINS[i].first, Q2_BINS[i].second);
                            auto it = sh.mass_q2_bins.find(bin_name);
                            if (it != sh.mass_q2_bins.end() && it->second) it->second->fill(cand.mass);
                            break;
                        }
                    }
                }
            }
        }

        return clas12::framework::StatusCode::SUCCESS();
    }
    

    auto on_finalize() -> clas12::framework::StatusCode override {
        using Log = clas12::framework::Log;

        Log::subsection(fmt::format("{} Summary", name()));

        if (m_ntuple_writer.is_initialized()) {
            auto entries = m_ntuple_writer.service()->get_entries("rho0_events");
            Log::stat(name(), "N-tuple entries", entries);
        }

        Log::stat(name(), "Unique runs processed", m_runs.size());
        Log::blank();

        // Detector region breakdown
        Log::print_yield_summary_table(
            "ρ⁰ Detector Region (post-cut)",
            {"Region", "Count"},
            {
                {"Forward-Forward", fmt::format("{}", m_rho0_ff.load())},
                {"Mixed (FD+CD)",   fmt::format("{}", m_rho0_mixed.load())},
                {"Central-Central", fmt::format("{}", m_rho0_cc.load())}
            }
        );

        // Perform BW + pol3 fitting if enabled
        if (m_enable_fitting.get()) {
            fit_invariant_mass_histograms();
        }

        // Sigma scan analysis
        if (!m_sigma_scan_values.get().empty() && m_enable_fitting.get()) {
            fit_sigma_scan_histograms();
        }

        // Save run metadata to ROOT file
        save_run_metadata();

        Log::blank();
        m_cutflow.print();
        m_candidate_cutflow.print();

        Log::success(name(), "Finalization complete");

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Save run metadata to the output ROOT file
     */
    auto save_run_metadata() -> void {
        if (!m_output_svc || !m_output_svc->is_valid() || m_runs.empty()) return;

        auto* out_svc = m_output_svc->get();
        if (!out_svc->is_file_open()) {
            clas12::framework::Log::error(name(), "Output file is not open for writing metadata");
            return;
        }

        // Acquire lock once and do all operations
        std::lock_guard<std::mutex> lock(out_svc->mutex());
        
        TFile* file = out_svc->get_file();
        if (!file) {
            clas12::framework::Log::error(name(), "Failed to get file pointer");
            return;
        }

        // Navigate to Metadata directory
        file->cd();
        if (!file->GetDirectory("Metadata")) {
            file->mkdir("Metadata");
        }
        file->cd("Metadata");

        // Create a TTree to store run numbers
        auto* run_tree = new TTree("runs", "Run numbers used in analysis");
        int run_number;
        run_tree->Branch("run", &run_number, "run/I");

        for (int run : m_runs) {
            run_number = run;
            run_tree->Fill();
        }
        run_tree->Write("", TObject::kOverwrite);

        // Also save as a simple parameter for quick access
        TParameter<int> n_runs("n_runs", static_cast<int>(m_runs.size()));
        n_runs.Write();

        clas12::framework::Log::info(name(), "Saved {} unique runs to metadata", m_runs.size());

        // Print runs to console using logger table
        std::vector<int> runs_vec(m_runs.begin(), m_runs.end());
        clas12::framework::Log::print_run_metadata("Run Metadata", m_runs.size(), runs_vec);
    }

    // Public helper methods
    static auto calculate_invariant_mass(const clas12::Particle& pip, const clas12::Particle& pim) -> double {
        return (pip.PxPyPzEVector() + pim.PxPyPzEVector()).M();
    }

    /**
     * @brief Calculate coherence length in fm
     * lc = 2 * nu * hbar*c / (M_rho0² + Q²)
     */
    static auto calculate_coherence_length(double nu, double Q2) -> double {
        return 2.0 * nu * HBARC / (RHO0_MASS * RHO0_MASS + Q2);
    }

    /**
     * @brief Calculate Mandelstam t = (q - p_rho0)²
     */
    static auto calculate_t(const ROOT::Math::PxPyPzEVector& virtual_photon,
                            const ROOT::Math::PxPyPzEVector& p_rho0) -> double {
        auto delta = virtual_photon - p_rho0;
        return delta.M2();
    }

    /**
     * @brief Build fully-computed Rho0Candidate objects from pion pairs
     *
     * Computes all kinematic variables once and applies cuts, storing results
     * in the candidate struct to avoid redundant calculations downstream.
     */
    auto build_rho0_candidates(
        const std::vector<clas12::Particle>& pions_plus,
        const std::vector<clas12::Particle>& pions_minus,
        const clas12::Particle& electron,
        const ROOT::Math::PxPyPzEVector& virtual_photon,
        double nu,
        double Q2
    ) const -> std::vector<Rho0Candidate> {
        std::vector<Rho0Candidate> candidates;
        if (pions_plus.empty() || pions_minus.empty()) return candidates;

        double e_vz = electron.vz();

        for (const auto& pip : pions_plus) {
            for (const auto& pim : pions_minus) {
                Rho0Candidate cand;
                cand.pip = pip;
                cand.pim = pim;

                // Classify detector region
                bool pip_fwd = clas12::framework::CandidateUtils::is_forward_detector(pip.status());
                bool pim_fwd = clas12::framework::CandidateUtils::is_forward_detector(pim.status());
                bool pip_cd  = clas12::framework::CandidateUtils::is_central_detector(pip.status());
                bool pim_cd  = clas12::framework::CandidateUtils::is_central_detector(pim.status());

                if (pip_fwd && pim_fwd) cand.detector_region = DetectorRegion::ForwardForward;
                else if (pip_cd && pim_cd) cand.detector_region = DetectorRegion::CentralCentral;
                else cand.detector_region = DetectorRegion::Mixed;

                // Compute rho0 4-momentum
                auto p_rho0 = pip.PxPyPzEVector() + pim.PxPyPzEVector();
                cand.mass = p_rho0.M();
                cand.momentum = std::sqrt(p_rho0.Px() * p_rho0.Px() +
                                          p_rho0.Py() * p_rho0.Py() +
                                          p_rho0.Pz() * p_rho0.Pz());
                cand.theta = std::acos(p_rho0.Pz() / cand.momentum) * 180.0 / M_PI;
                cand.phi = std::atan2(p_rho0.Py(), p_rho0.Px()) * 180.0 / M_PI;

                // Compute opening angle
                auto p1_3 = pip.PxPyPzVector();
                auto p2_3 = pim.PxPyPzVector();
                double cos_angle = p1_3.Dot(p2_3) / (p1_3.R() * p2_3.R());
                cand.opening_angle = std::acos(cos_angle) * 180.0 / M_PI;

                // Compute kinematic variables
                cand.zh = p_rho0.E() / nu;
                cand.lc = calculate_coherence_length(nu, Q2);
                cand.t = calculate_t(virtual_photon, p_rho0);
                cand.minus_t = -cand.t;

                // Compute vertex differences
                cand.delta_vz_pip = e_vz - pip.vz();
                cand.delta_vz_pim = e_vz - pim.vz();

                // Apply cuts using target-specific and charge-specific ranges
                cand.passes_delta_vz_pip = get_piplus_delta_vz_range().contains(cand.delta_vz_pip);
                cand.passes_delta_vz_pim = get_piminus_delta_vz_range().contains(cand.delta_vz_pim);
                cand.passes_zh = cand.zh > m_zh_min.get();
                cand.passes_lc = cand.lc < m_lc_max.get();
                cand.passes_t = (cand.minus_t > m_t_min.get()) && (cand.minus_t < m_t_max.get());
                cand.passes_all_cuts =
                    (!m_enable_delta_vz_pip_cut.get() || cand.passes_delta_vz_pip) &&
                    (!m_enable_delta_vz_pim_cut.get() || cand.passes_delta_vz_pim) &&
                    (!m_enable_zh_cut.get() || cand.passes_zh) &&
                    (!m_enable_lc_cut.get() || cand.passes_lc) &&
                    (!m_enable_t_cut.get() || cand.passes_t);

                candidates.push_back(std::move(cand));
            }
        }

        return candidates;
    }

    /**
     * @brief Fill histograms for candidates before kinematic cuts
     */
    auto fill_pre_cut_histograms(const std::vector<Rho0Candidate>& candidates) -> void {
        for (const auto& cand : candidates) {
            if (h_pipim_mass_all) h_pipim_mass_all->fill(cand.mass);
            if (h_zh) h_zh->fill(cand.zh);
            if (h_lc) h_lc->fill(cand.lc);
            if (h_t) h_t->fill(cand.minus_t);
            if (h_delta_vz_pim) h_delta_vz_pim->fill(cand.delta_vz_pim);
            if (h_delta_vz_pip) h_delta_vz_pip->fill(cand.delta_vz_pip);

            // Region-specific pre-cut mass
            auto& rh = get_region_histograms(cand.detector_region);
            if (rh.mass_all) rh.mass_all->fill(cand.mass);
        }
    }

    /**
     * @brief Fill histograms for candidates that passed all cuts
     */
    auto fill_post_cut_histograms(const std::vector<Rho0Candidate>& candidates, double Q2) -> void {
        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;

            if (h_pipim_mass_selected) h_pipim_mass_selected->fill(cand.mass);
            if (h_rho0_momentum) h_rho0_momentum->fill(cand.momentum);
            if (h_rho0_theta) h_rho0_theta->fill(cand.theta);
            if (h_rho0_phi) h_rho0_phi->fill(cand.phi);
            if (h_opening_angle) h_opening_angle->fill(cand.opening_angle);
            if (h_pipim_mass_vs_momentum) h_pipim_mass_vs_momentum->fill(cand.momentum, cand.mass);

            if (h_zh_selected) h_zh_selected->fill(cand.zh);
            if (h_lc_selected) h_lc_selected->fill(cand.lc);
            if (h_t_selected) h_t_selected->fill(cand.minus_t);
            if (h_delta_vz_pim_selected) h_delta_vz_pim_selected->fill(cand.delta_vz_pim);
            if (h_delta_vz_pip_selected) h_delta_vz_pip_selected->fill(cand.delta_vz_pip);

            // Region-specific post-cut histograms
            auto& rh = get_region_histograms(cand.detector_region);
            if (rh.mass_selected) rh.mass_selected->fill(cand.mass);
            if (rh.momentum) rh.momentum->fill(cand.momentum);
            if (rh.theta) rh.theta->fill(cand.theta);
            if (rh.phi) rh.phi->fill(cand.phi);
            if (rh.opening_angle) rh.opening_angle->fill(cand.opening_angle);
            if (rh.mass_vs_momentum) rh.mass_vs_momentum->fill(cand.momentum, cand.mass);

            // Increment region counter
            switch (cand.detector_region) {
                case DetectorRegion::ForwardForward: m_rho0_ff++; break;
                case DetectorRegion::Mixed: m_rho0_mixed++; break;
                case DetectorRegion::CentralCentral: m_rho0_cc++; break;
                default: break;
            }

            fill_q2_binned_histogram(Q2, cand.mass);
            fill_region_q2_binned_histogram(rh, Q2, cand.mass);
        }
    }

    /**
     * @brief Fill lc-binned histograms for candidates passing vertex, zh, Q2, and t cuts
     */
    auto fill_lc_binned_histograms(const std::vector<Rho0Candidate>& candidates, double Q2) -> void {
        bool passes_Q2 = (Q2 > 1.0);
        for (const auto& cand : candidates) {
            if ((!m_enable_delta_vz_pim_cut.get() || cand.passes_delta_vz_pim) &&
                (!m_enable_delta_vz_pip_cut.get() || cand.passes_delta_vz_pip) &&
                (!m_enable_zh_cut.get() || cand.passes_zh) && passes_Q2 &&
                (!m_enable_t_cut.get() || cand.passes_t)) {
                fill_lc_binned_histogram(cand.lc, cand.mass);
            }
        }
    }

    /**
     * @brief Fill ntuple with candidate data
     */
    auto fill_ntuple(const std::vector<Rho0Candidate>& candidates, const clas12::Particle& electron, double Q2) -> void {
        if (!m_ntuple_writer.is_initialized() || !m_enable_ntuple.get()) return;

        std::lock_guard<std::mutex> lock(m_ntuple_mutex);
        auto& nt = m_ntuple_writer.data();
        nt.clear_vectors();

        nt.e_p = electron.p();
        nt.e_theta = electron.theta();
        nt.e_phi = electron.phi();
        nt.e_vz = electron.vz();
        nt.Q2 = Q2;

        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;

            nt.pip_p.push_back(cand.pip.p());
            nt.pip_theta.push_back(cand.pip.theta());
            nt.pip_phi.push_back(cand.pip.phi());
            nt.pim_p.push_back(cand.pim.p());
            nt.pim_theta.push_back(cand.pim.theta());
            nt.pim_phi.push_back(cand.pim.phi());
            nt.rho0_masses.push_back(cand.mass);
            nt.rho0_ps.push_back(cand.momentum);
            nt.rho0_thetas.push_back(cand.theta);
            nt.rho0_phis.push_back(cand.phi);
            nt.rho0_zh.push_back(cand.zh);
            nt.rho0_lc.push_back(cand.lc);
            nt.rho0_t.push_back(cand.minus_t);
            nt.delta_vz_pim.push_back(cand.delta_vz_pim);
            nt.delta_vz_pip.push_back(cand.delta_vz_pip);
        }

        if (nt.rho0_masses.empty()) return;
        m_ntuple_writer.fill();
    }
    /**
     * @brief Store passing candidates in event for downstream algorithms
     */
    auto store_candidates_in_event(const std::vector<Rho0Candidate>& candidates, clas12::framework::Event& event) -> void {
        std::vector<clas12::Particle> stored_particles;
        for (const auto& cand : candidates) {
            if (cand.passes_all_cuts) {
                stored_particles.push_back(cand.pip);
                stored_particles.push_back(cand.pim);
            }
        }
        event.store_particles("Rho0_PionPairs", stored_particles);
    }

    /**
     * @brief Count candidates that pass all cuts
     */
    [[nodiscard]] auto count_passing_candidates(const std::vector<Rho0Candidate>& candidates) const -> size_t {
        return std::count_if(candidates.begin(), candidates.end(),
                             [](const Rho0Candidate& c) { return c.passes_all_cuts; });
    }

   private:
    // Target
    clas12::Target m_target;

    // Properties
    clas12::framework::Property<double> m_zh_min{"zh_min", 0.9, "Minimum zh (energy fraction)"};
    clas12::framework::Property<double> m_lc_max{"lc_max", 1.0, "Maximum coherence length [fm]"};
    clas12::framework::Property<double> m_t_min{"t_min", 0.1, "Minimum -t [GeV²]"};
    clas12::framework::Property<double> m_t_max{"t_max", 0.5, "Maximum -t [GeV²]"};
    clas12::framework::Property<bool> m_enable_delta_vz_pim_cut{"enable_delta_vz_pim_cut", true, "Enable delta_vz pi- cut"};
    clas12::framework::Property<bool> m_enable_delta_vz_pip_cut{"enable_delta_vz_pip_cut", true, "Enable delta_vz pi+ cut"};
    clas12::framework::Property<bool> m_enable_zh_cut{"enable_zh_cut", true, "Enable zh cut"};
    clas12::framework::Property<bool> m_enable_lc_cut{"enable_lc_cut", true, "Enable coherence length cut"};
    clas12::framework::Property<bool> m_enable_t_cut{"enable_t_cut", true, "Enable -t cut"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    clas12::framework::Property<bool> m_enable_fitting{"enable_fitting", true, "Enable BW+pol3 fitting in finalize"};
    clas12::framework::Property<std::vector<double>> m_sigma_scan_values{"sigma_scan_values", {}, "Sigma values for delta_t scan (empty=disabled)"};

    /**
     * @brief Delta vz cut range structure
     */
    struct DeltaVzCutRange {
        double low;   // μ - 3σ
        double high;  // μ + 3σ

        [[nodiscard]] auto contains(double delta_vz) const -> bool {
            return delta_vz >= low && delta_vz <= high;
        }
    };

    /**
     * @brief Get delta_vz cut range for pi+ based on target
     * Values from: μ ± 3σ analysis
     */
    auto get_piplus_delta_vz_range() const -> DeltaVzCutRange {
        switch (m_target) {
            case clas12::Target::LD2:
                return {-4.0134, 2.3256};
            case clas12::Target::CxC:
                return {-4.0065, 2.3378};
            case clas12::Target::Cu:
                return {-3.6048, 2.1625};
            case clas12::Target::Sn:
                return {-3.9862, 2.3633};
            default:
                return {-4.0134, 2.3256};  // Default to LD2
        }
    }

    /**
     * @brief Get delta_vz cut range for pi- based on target
     * Values from: μ ± 3σ analysis
     */
    auto get_piminus_delta_vz_range() const -> DeltaVzCutRange {
        switch (m_target) {
            case clas12::Target::LD2:
                return {-3.8801, 4.2481};
            case clas12::Target::CxC:
                return {-3.6559, 3.9862};
            case clas12::Target::Cu:
                return {-3.5778, 3.9216};
            case clas12::Target::Sn:
                return {-3.5268, 3.8075};
            default:
                return {-3.8801, 4.2481};  // Default to LD2
        }
    }

    // Service handles
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::OutputFileService>* m_output_svc = nullptr;

    // Cutflow (event-level + candidate-level)
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::ContextCutflow<Rho0Candidate> m_candidate_cutflow;

    // Run tracking for metadata
    std::set<int> m_runs;
    mutable std::mutex m_runs_mutex;

    // Mutex for thread safety
    mutable std::mutex m_ntuple_mutex;

    // Region-specific histogram set (mass + kinematics)
    struct RegionHistograms {
        clas12::framework::AtomicHistogram1D* mass_all = nullptr;
        clas12::framework::AtomicHistogram1D* mass_selected = nullptr;
        clas12::framework::AtomicHistogram1D* momentum = nullptr;
        clas12::framework::AtomicHistogram1D* theta = nullptr;
        clas12::framework::AtomicHistogram1D* phi = nullptr;
        clas12::framework::AtomicHistogram1D* opening_angle = nullptr;
        clas12::framework::AtomicHistogram2D* mass_vs_momentum = nullptr;
        std::map<std::string, clas12::framework::AtomicHistogram1D*> mass_q2_bins;
    };

    RegionHistograms m_hist_ff;     // Forward-Forward
    RegionHistograms m_hist_mixed;  // Mixed (FD+CD)
    RegionHistograms m_hist_cc;     // Central-Central

    // Region counters (post-cut)
    std::atomic<size_t> m_rho0_ff{0};
    std::atomic<size_t> m_rho0_mixed{0};
    std::atomic<size_t> m_rho0_cc{0};

    // Sigma scan histogram set
    struct SigmaScanHistograms {
        clas12::framework::AtomicHistogram1D* mass_all = nullptr;
        clas12::framework::AtomicHistogram1D* mass_selected = nullptr;
        std::map<std::string, clas12::framework::AtomicHistogram1D*> mass_q2_bins;
    };
    std::map<std::string, SigmaScanHistograms> m_sigma_scan_histograms;

    auto get_region_histograms(DetectorRegion region) -> RegionHistograms& {
        switch (region) {
            case DetectorRegion::ForwardForward: return m_hist_ff;
            case DetectorRegion::Mixed: return m_hist_mixed;
            case DetectorRegion::CentralCentral: return m_hist_cc;
            default: return m_hist_ff;
        }
    }

    // Histograms - Rho0 candidates (atomic for thread-safety)
    clas12::framework::AtomicHistogram1D* h_pipim_mass_all = nullptr;
    clas12::framework::AtomicHistogram1D* h_pipim_mass_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_rho0_momentum = nullptr;
    clas12::framework::AtomicHistogram1D* h_rho0_theta = nullptr;
    clas12::framework::AtomicHistogram1D* h_rho0_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_opening_angle = nullptr;
    clas12::framework::AtomicHistogram2D* h_pipim_mass_vs_momentum = nullptr;

    // Histograms - Kinematic variables (before cuts)
    clas12::framework::AtomicHistogram1D* h_zh = nullptr;
    clas12::framework::AtomicHistogram1D* h_lc = nullptr;
    clas12::framework::AtomicHistogram1D* h_t = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_vz_pim = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_vz_pip = nullptr;

    // Histograms - Kinematic variables (after cuts)
    clas12::framework::AtomicHistogram1D* h_zh_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_lc_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_t_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_vz_pim_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_vz_pip_selected = nullptr;

    // Q²-binned invariant mass histograms
    std::map<std::string, clas12::framework::AtomicHistogram1D*> h_pipim_mass_q2_bins;

    // lc-binned invariant mass histograms
    std::map<std::string, clas12::framework::AtomicHistogram1D*> h_pipim_mass_lc_bins;

    // Q² bin edges: 1.0-2.0, 2.0-2.5, 2.5-3.0, 3.0-3.5, 3.5-4.5, 4.5-6 GeV²
    static constexpr std::array<std::pair<double, double>, 6> Q2_BINS = {{
        {1.0, 2.0},
        {2.0, 2.5},
        {2.5, 3.0},
        {3.0, 3.5},
        {3.5, 4.5},
        {4.5, 6.0}
    }};

    static constexpr std::array<std::pair<double, double>, 8> LC_BINS = {{
        {0.0, 0.2},
        {0.2, 0.4},
        {0.4, 0.6},
        {0.6, 0.8},
        {0.8, 1.0},
        {1.0, 1.5},
        {1.5, 2.0},
        {2.0, 3.0}
    }};

    /**
     * @brief Fit result structure
     */
    struct FitResult {
        double mass = 0.0;
        double mass_err = 0.0;
        double n_rho0 = 0.0;
        double n_rho0_err = 0.0;
        double chi2_ndf = 0.0;
        double fit_min = 0.0;           // Fit range minimum
        double fit_max = 0.0;           // Fit range maximum
        double integ_min = 0.0;         // Integration range minimum
        double integ_max = 0.0;         // Integration range maximum
        bool valid = false;
        std::shared_ptr<TF1> fit_function;  // Store fit function for saving
    };

    // NTuple data struct and writer
    struct Rho0NTupleData {
        double e_p = 0, e_theta = 0, e_phi = 0, e_vz = 0, Q2 = 0;
        std::vector<double> pip_p, pip_theta, pip_phi;
        std::vector<double> pim_p, pim_theta, pim_phi;
        std::vector<double> rho0_masses, rho0_ps, rho0_thetas, rho0_phis;
        std::vector<double> rho0_zh, rho0_lc, rho0_t, delta_vz_pim, delta_vz_pip;

        template <typename Svc>
        auto register_branches(Svc* svc, const std::string& tree) -> void {
            svc->register_branch(tree, "e_p", &e_p);
            svc->register_branch(tree, "e_theta", &e_theta);
            svc->register_branch(tree, "e_phi", &e_phi);
            svc->register_branch(tree, "e_vz", &e_vz);
            svc->register_branch(tree, "Q2", &Q2);
            svc->register_branch(tree, "pip_p", &pip_p);
            svc->register_branch(tree, "pip_theta", &pip_theta);
            svc->register_branch(tree, "pip_phi", &pip_phi);
            svc->register_branch(tree, "pim_p", &pim_p);
            svc->register_branch(tree, "pim_theta", &pim_theta);
            svc->register_branch(tree, "pim_phi", &pim_phi);
            svc->register_branch(tree, "rho0_masses", &rho0_masses);
            svc->register_branch(tree, "rho0_ps", &rho0_ps);
            svc->register_branch(tree, "rho0_thetas", &rho0_thetas);
            svc->register_branch(tree, "rho0_phis", &rho0_phis);
            svc->register_branch(tree, "rho0_zh", &rho0_zh);
            svc->register_branch(tree, "rho0_lc", &rho0_lc);
            svc->register_branch(tree, "rho0_t", &rho0_t);
            svc->register_branch(tree, "delta_vz_pim", &delta_vz_pim);
            svc->register_branch(tree, "delta_vz_pip", &delta_vz_pip);
        }

        auto clear_vectors() -> void {
            pip_p.clear(); pip_theta.clear(); pip_phi.clear();
            pim_p.clear(); pim_theta.clear(); pim_phi.clear();
            rho0_masses.clear(); rho0_ps.clear(); rho0_thetas.clear(); rho0_phis.clear();
            rho0_zh.clear(); rho0_lc.clear(); rho0_t.clear();
            delta_vz_pim.clear(); delta_vz_pip.clear();
        }
    };
    clas12::framework::NTupleWriter<Rho0NTupleData> m_ntuple_writer;

    auto fill_q2_binned_histogram(double Q2, double mass) -> void {
        for (size_t i = 0; i < Q2_BINS.size(); ++i) {
            if (Q2 >= Q2_BINS[i].first && Q2 < Q2_BINS[i].second) {
                std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", Q2_BINS[i].first, Q2_BINS[i].second);
                auto it = h_pipim_mass_q2_bins.find(bin_name);
                if (it != h_pipim_mass_q2_bins.end() && it->second) it->second->fill(mass);
                break;
            }
        }
    }

    auto fill_region_q2_binned_histogram(RegionHistograms& rh, double Q2, double mass) -> void {
        for (size_t i = 0; i < Q2_BINS.size(); ++i) {
            if (Q2 >= Q2_BINS[i].first && Q2 < Q2_BINS[i].second) {
                std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", Q2_BINS[i].first, Q2_BINS[i].second);
                auto it = rh.mass_q2_bins.find(bin_name);
                if (it != rh.mass_q2_bins.end() && it->second) it->second->fill(mass);
                break;
            }
        }
    }

    auto fill_lc_binned_histogram(double lc, double mass) -> void {
        for (size_t i = 0; i < LC_BINS.size(); ++i) {
            if (lc >= LC_BINS[i].first && lc < LC_BINS[i].second) {
                std::string bin_name = fmt::format("LC_{:.1f}_{:.1f}", LC_BINS[i].first, LC_BINS[i].second);
                auto it = h_pipim_mass_lc_bins.find(bin_name);
                if (it != h_pipim_mass_lc_bins.end() && it->second) it->second->fill(mass);
                break;
            }
        }
    }

    // ========================================================================
    // Sigma scan helpers
    // ========================================================================

    static auto format_sigma_label(double sigma) -> std::string {
        return fmt::format("sigma_{:.1f}", sigma);
    }

    static auto format_sigma_key(const std::string& base, double sigma) -> std::string {
        return fmt::format("{}_{}", base, format_sigma_label(sigma));
    }

    auto book_histograms() -> void {
        clas12::framework::safe(m_hist_svc).with([this](auto* svc) {

        // Rho0 candidate histograms
        h_pipim_mass_all = svc->book_hist1D("/Rho0/pipim_mass_all", "All #pi^{+}#pi^{-} Invariant Mass;M_{#pi^{+}#pi^{-}} [GeV/c^{2}];Events", 200, 0, 2.0);
        h_pipim_mass_selected = svc->book_hist1D("/Rho0/pipim_mass_selected", "Selected #rho^{0} Candidate Mass;M_{#pi^{+}#pi^{-}} [GeV/c^{2}];Events", 200, 0.4, 1.2);
        h_rho0_momentum = svc->book_hist1D("/Rho0/rho0_momentum", "#rho^{0} Momentum;p [GeV/c];Events", 100, 0, 10);
        h_rho0_theta = svc->book_hist1D("/Rho0/rho0_theta", "#rho^{0} Theta;#theta [deg];Events", 180, 0, 180);
        h_rho0_phi = svc->book_hist1D("/Rho0/rho0_phi", "#rho^{0} Phi;#phi [deg];Events", 360, -180, 180);
        h_opening_angle = svc->book_hist1D("/Rho0/opening_angle", "#pi^{+}#pi^{-} Opening Angle;#theta_{#pi^{+}#pi^{-}} [deg];Events", 180, 0, 180);

        h_pipim_mass_vs_momentum = svc->book_hist2D("/Rho0/pipim_mass_vs_momentum",
            "#pi^{+}#pi^{-} Mass vs Momentum;p [GeV/c];M_{#pi^{+}#pi^{-}} [GeV/c^{2}]",
            100, 0, 10, 100, 0.4, 1.2);

        // Kinematic variable histograms (before cuts)
        h_zh = svc->book_hist1D("/Rho0/zh", "z_{h} (before cuts);z_{h} = E_{#rho^{0}}/#nu;Events", 100, 0, 1.5);
        h_lc = svc->book_hist1D("/Rho0/lc", "Coherence Length (before cuts);l_{c} [fm];Events", 100, 0, 5);
        h_t = svc->book_hist1D("/Rho0/t", "-t (before cuts);-t [GeV^{2}];Events", 100, 0, 2);
        h_delta_vz_pim = svc->book_hist1D("/Rho0/delta_vz_pim", "#Delta v_{z} (before cuts);v_{z}^{e} - v_{z}^{#pi^{-}} [cm];Events", 100, -20, 20);
        h_delta_vz_pip = svc->book_hist1D("/Rho0/delta_vz_pip", "#Delta v_{z} (before cuts);v_{z}^{e} - v_{z}^{#pi^{+}} [cm];Events", 100, -20, 20);

        // Kinematic variable histograms (after cuts)
        h_zh_selected = svc->book_hist1D("/Rho0/zh_selected", "z_{h} (after cuts);z_{h} = E_{#rho^{0}}/#nu;Events", 100, 0, 1.5);
        h_lc_selected = svc->book_hist1D("/Rho0/lc_selected", "Coherence Length (after cuts);l_{c} [fm];Events", 100, 0, 5);
        h_t_selected = svc->book_hist1D("/Rho0/t_selected", "-t (after cuts);-t [GeV^{2}];Events", 100, 0, 2);
        h_delta_vz_pim_selected = svc->book_hist1D("/Rho0/delta_vz_pim_selected", "#Delta v_{z} (after cuts);v_{z}^{e} - v_{z}^{#pi^{-}} [cm];Events", 100, -20, 20);
        h_delta_vz_pip_selected = svc->book_hist1D("/Rho0/delta_vz_pip_selected", "#Delta v_{z} (after cuts);v_{z}^{e} - v_{z}^{#pi^{+}} [cm];Events", 100, -20, 20);

        // Q²-binned invariant mass histograms
        for (const auto& [q2_min, q2_max] : Q2_BINS) {
            std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", q2_min, q2_max);
            std::string hist_path = fmt::format("/Rho0/Q2_bins/pipim_mass_{}", bin_name);
            std::string title = fmt::format("#pi^{{+}}#pi^{{-}} Mass ({:.1f} < Q^{{2}} < {:.1f} GeV^{{2}});M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", q2_min, q2_max);
            h_pipim_mass_q2_bins[bin_name] = svc->book_hist1D(hist_path, title, 200, 0.0, 1.4);
        }

            // lc-binned invariant mass histograms
            for (const auto& [lc_min, lc_max] : LC_BINS) {
                std::string bin_name = fmt::format("LC_{:.1f}_{:.1f}", lc_min, lc_max);
                std::string hist_path = fmt::format("/Rho0/LC_bins/pipim_mass_{}", bin_name);
                std::string title = fmt::format("#pi^{{+}}#pi^{{-}} Mass ({:.1f} < l_{{c}} < {:.1f} fm);M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", lc_min, lc_max);
                h_pipim_mass_lc_bins[bin_name] = svc->book_hist1D(hist_path, title, 200, 0.0, 1.4);
            }

            // Region-specific histograms (Forward-Forward, Mixed, Central-Central)
            auto book_region = [svc](RegionHistograms& h, const std::string& dir, const std::string& label) {
                h.mass_all = svc->book_hist1D(dir + "/pipim_mass_all",
                    fmt::format("{} #pi^{{+}}#pi^{{-}} Invariant Mass;M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", label), 200, 0, 2.0);
                h.mass_selected = svc->book_hist1D(dir + "/pipim_mass_selected",
                    fmt::format("{} Selected #rho^{{0}} Mass;M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", label), 200, 0.4, 1.2);
                h.momentum = svc->book_hist1D(dir + "/rho0_momentum",
                    fmt::format("{} #rho^{{0}} Momentum;p [GeV/c];Events", label), 100, 0, 10);
                h.theta = svc->book_hist1D(dir + "/rho0_theta",
                    fmt::format("{} #rho^{{0}} Theta;#theta [deg];Events", label), 180, 0, 180);
                h.phi = svc->book_hist1D(dir + "/rho0_phi",
                    fmt::format("{} #rho^{{0}} Phi;#phi [deg];Events", label), 360, -180, 180);
                h.opening_angle = svc->book_hist1D(dir + "/opening_angle",
                    fmt::format("{} #pi^{{+}}#pi^{{-}} Opening Angle;#theta_{{#pi^{{+}}#pi^{{-}}}} [deg];Events", label), 180, 0, 180);
                h.mass_vs_momentum = svc->book_hist2D(dir + "/pipim_mass_vs_momentum",
                    fmt::format("{} #pi^{{+}}#pi^{{-}} Mass vs Momentum;p [GeV/c];M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}]", label),
                    100, 0, 10, 100, 0.4, 1.2);

                // Q²-binned invariant mass histograms per region
                for (const auto& [q2_min, q2_max] : Q2_BINS) {
                    std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", q2_min, q2_max);
                    std::string hist_path = fmt::format("{}/Q2_bins/pipim_mass_{}", dir, bin_name);
                    std::string title = fmt::format("{} #pi^{{+}}#pi^{{-}} Mass ({:.1f} < Q^{{2}} < {:.1f} GeV^{{2}});M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", label, q2_min, q2_max);
                    h.mass_q2_bins[bin_name] = svc->book_hist1D(hist_path, title, 200, 0.0, 1.4);
                }
            };
            book_region(m_hist_ff, "/Rho0/Forward", "FD-FD");
            book_region(m_hist_mixed, "/Rho0/Mixed", "Mixed");
            book_region(m_hist_cc, "/Rho0/Central", "CD-CD");

            // Book sigma scan histograms
            for (double sigma : m_sigma_scan_values.get()) {
                std::string label = format_sigma_label(sigma);
                std::string dir = fmt::format("/Rho0/SigmaScan/{}", label);

                SigmaScanHistograms sh;
                sh.mass_all = svc->book_hist1D(
                    dir + "/pipim_mass_all",
                    fmt::format("#pi^{{+}}#pi^{{-}} Mass ({:.1f}#sigma);M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", sigma),
                    200, 0, 2.0);
                sh.mass_selected = svc->book_hist1D(
                    dir + "/pipim_mass_selected",
                    fmt::format("Selected #rho^{{0}} Mass ({:.1f}#sigma);M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", sigma),
                    200, 0.4, 1.2);

                for (const auto& [q2_min, q2_max] : Q2_BINS) {
                    std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", q2_min, q2_max);
                    std::string hist_path = fmt::format("{}/Q2_bins/pipim_mass_{}", dir, bin_name);
                    std::string title = fmt::format("#pi^{{+}}#pi^{{-}} Mass ({:.1f}#sigma, {:.1f} < Q^{{2}} < {:.1f} GeV^{{2}});M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events",
                        sigma, q2_min, q2_max);
                    sh.mass_q2_bins[bin_name] = svc->book_hist1D(hist_path, title, 200, 0.0, 1.4);
                }

                m_sigma_scan_histograms[label] = sh;
            }
        });
    }
    auto setup_ntuple() -> void {
        clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) {
            m_ntuple_writer.init(svc, "rho0_events");
            clas12::framework::Log::info(name(), "N-tuple branches registered for rho0_events");
        });
    }
    /**
     * @brief Fit sigma scan histograms and print comparison table
     */
    auto fit_sigma_scan_histograms() -> void {
        using Log = clas12::framework::Log;

        Log::subsection("Delta-t Sigma Scan Results");

        struct SigmaScanResult {
            double sigma = 0.0;
            double n_rho0 = 0.0;
            double n_rho0_err = 0.0;
            double mass = 0.0;
            double mass_err = 0.0;
            double chi2_ndf = 0.0;
            double total_entries = 0.0;
            bool valid = false;
        };

        std::vector<SigmaScanResult> scan_results;

        for (double sigma : m_sigma_scan_values.get()) {
            std::string label = format_sigma_label(sigma);
            auto sh_it = m_sigma_scan_histograms.find(label);
            if (sh_it == m_sigma_scan_histograms.end()) continue;
            auto& sh = sh_it->second;

            SigmaScanResult sr;
            sr.sigma = sigma;
            sr.total_entries = sh.mass_selected ? static_cast<double>(sh.mass_selected->entries()) : 0;

            if (sh.mass_selected && sh.mass_selected->entries() > 100) {
                auto root_hist = sh.mass_selected->to_root(
                    fmt::format("sigma_scan_{}_fit", label),
                    fmt::format("Mass ({:.1f}#sigma)", sigma));

                auto result = fit_histogram(root_hist.get(),
                    fmt::format("Sigma scan ({:.1f}#sigma)", sigma),
                    0.3, 1.2, 0.6, 1.0);

                if (result.valid) {
                    sr.n_rho0 = result.n_rho0;
                    sr.n_rho0_err = result.n_rho0_err;
                    sr.mass = result.mass;
                    sr.mass_err = result.mass_err;
                    sr.chi2_ndf = result.chi2_ndf;
                    sr.valid = true;
                }
            }

            scan_results.push_back(sr);
        }

        // Print comparison table
        if (!scan_results.empty()) {
            std::vector<std::string> columns = {"Sigma", "Entries", "N(rho0)", "Mass [MeV]", "chi2/ndf"};
            std::vector<std::vector<std::string>> rows;

            double ref_n_rho0 = 0;
            for (const auto& sr : scan_results) {
                if (std::abs(sr.sigma - 3.0) < 0.01 && sr.valid) {
                    ref_n_rho0 = sr.n_rho0;
                    break;
                }
            }

            for (const auto& sr : scan_results) {
                rows.push_back({
                    fmt::format("{:.1f}", sr.sigma),
                    fmt::format("{:.0f}", sr.total_entries),
                    sr.valid ? fmt::format("{:.0f} +/- {:.0f}", sr.n_rho0, sr.n_rho0_err) : "N/A",
                    sr.valid ? fmt::format("{:.1f} +/- {:.1f}", sr.mass * 1000, sr.mass_err * 1000) : "N/A",
                    sr.valid ? fmt::format("{:.2f}", sr.chi2_ndf) : "N/A"
                });
            }

            Log::print_yield_summary_table(
                "Delta-t Sigma Scan: rho0 Yield Comparison",
                columns, rows);

            // Print relative change table
            if (ref_n_rho0 > 0) {
                std::vector<std::string> rel_columns = {"Sigma", "N(rho0)", "Relative to 3.0 sigma"};
                std::vector<std::vector<std::string>> rel_rows;

                for (const auto& sr : scan_results) {
                    if (!sr.valid) continue;
                    double rel = 100.0 * (sr.n_rho0 - ref_n_rho0) / ref_n_rho0;
                    rel_rows.push_back({
                        fmt::format("{:.1f}", sr.sigma),
                        fmt::format("{:.0f} +/- {:.0f}", sr.n_rho0, sr.n_rho0_err),
                        fmt::format("{:+.1f}%", rel)
                    });
                }

                Log::print_yield_summary_table(
                    "Delta-t Sigma Scan: Relative Yield Change",
                    rel_columns, rel_rows);
            }
        }

        // Save fit results to ROOT file
        save_sigma_scan_fit_results(scan_results);
    }

    /**
     * @brief Save sigma scan fit results to ROOT file as a TTree
     */
    template<typename ResultVec>
    auto save_sigma_scan_fit_results(const ResultVec& scan_results) -> void {
        if (!m_output_svc || !m_output_svc->is_valid()) return;
        auto* out_svc = m_output_svc->get();
        if (!out_svc->is_file_open()) return;

        std::lock_guard<std::mutex> lock(out_svc->mutex());
        TFile* file = out_svc->get_file();
        if (!file) return;

        file->cd();
        if (!file->GetDirectory("Rho0/SigmaScan")) {
            file->mkdir("Rho0/SigmaScan");
        }
        file->cd("Rho0/SigmaScan");

        auto* tree = new TTree("sigma_scan_results", "Delta-t Sigma Scan Results");
        double sigma_val, n_rho0, n_rho0_err, mass_val, mass_err, chi2_ndf, entries;
        tree->Branch("sigma", &sigma_val, "sigma/D");
        tree->Branch("entries", &entries, "entries/D");
        tree->Branch("n_rho0", &n_rho0, "n_rho0/D");
        tree->Branch("n_rho0_err", &n_rho0_err, "n_rho0_err/D");
        tree->Branch("mass", &mass_val, "mass/D");
        tree->Branch("mass_err", &mass_err, "mass_err/D");
        tree->Branch("chi2_ndf", &chi2_ndf, "chi2_ndf/D");

        for (const auto& sr : scan_results) {
            sigma_val = sr.sigma;
            entries = sr.total_entries;
            n_rho0 = sr.valid ? sr.n_rho0 : 0;
            n_rho0_err = sr.valid ? sr.n_rho0_err : 0;
            mass_val = sr.valid ? sr.mass : 0;
            mass_err = sr.valid ? sr.mass_err : 0;
            chi2_ndf = sr.valid ? sr.chi2_ndf : 0;
            tree->Fill();
        }

        tree->Write("", TObject::kOverwrite);
        clas12::framework::Log::info(name(), "Sigma scan results saved to TTree with {} entries", tree->GetEntries());
    }

    /**
     * @brief Fit invariant mass histograms with Breit-Wigner + pol3
     * Uses two fit ranges [0.3-1.2] and [0.3-1.4] for comparison.
     * Signal yield is integrated between [0.6-1.0] GeV.
     */
    auto fit_invariant_mass_histograms() -> void {
        clas12::framework::Log::info(name(), "Fitting invariant mass histograms with BW + pol3...");
        clas12::framework::Log::info(name(), "  Fit ranges: [0.3-1.2] and [0.3-1.4] GeV");
        clas12::framework::Log::info(name(), "  Integration range: [0.6-1.0] GeV");

        // Maps for narrow [0.3-1.2] and wide [0.3-1.4] fit results
        std::map<std::string, FitResult> fit_results_narrow;
        std::map<std::string, FitResult> fit_results_wide;
        double total_rho0_narrow = 0.0;
        double total_rho0_narrow_err_sq = 0.0;
        double total_rho0_wide = 0.0;
        double total_rho0_wide_err_sq = 0.0;

        // Fit the main selected mass histogram with both ranges
        if (h_pipim_mass_selected && h_pipim_mass_selected->entries() > 100) {
            auto root_hist = h_pipim_mass_selected->to_root("pipim_mass_selected_fit", "Selected Mass");
            auto [result_narrow, result_wide] = fit_histogram_compare_ranges(root_hist.get(), "All Q² (selected)");
            if (result_narrow.valid) fit_results_narrow["All"] = result_narrow;
            if (result_wide.valid) fit_results_wide["All"] = result_wide;
        }

        // Fit Q²-binned histograms with both ranges
        for (const auto& [bin_name, hist] : h_pipim_mass_q2_bins) {
            if (hist && hist->entries() > 50) {
                auto root_hist = hist->to_root(bin_name + "_fit", bin_name);
                auto [result_narrow, result_wide] = fit_histogram_compare_ranges(root_hist.get(), bin_name);
                if (result_narrow.valid) {
                    fit_results_narrow[bin_name] = result_narrow;
                    total_rho0_narrow += result_narrow.n_rho0;
                    total_rho0_narrow_err_sq += result_narrow.n_rho0_err * result_narrow.n_rho0_err;
                }
                if (result_wide.valid) {
                    fit_results_wide[bin_name] = result_wide;
                    total_rho0_wide += result_wide.n_rho0;
                    total_rho0_wide_err_sq += result_wide.n_rho0_err * result_wide.n_rho0_err;
                }
            }
        }

        // Save Q² fit results to ROOT file (using narrow range as primary)
        save_Q2_fit_results_to_file(fit_results_narrow, fit_results_wide,
                                    total_rho0_narrow, std::sqrt(total_rho0_narrow_err_sq),
                                    total_rho0_wide, std::sqrt(total_rho0_wide_err_sq));

        // Reset totals for LC bins
        total_rho0_narrow = 0.0;
        total_rho0_narrow_err_sq = 0.0;
        total_rho0_wide = 0.0;
        total_rho0_wide_err_sq = 0.0;

        // Fit lc-binned histograms with both ranges
        for (const auto& [bin_name, hist] : h_pipim_mass_lc_bins) {
            if (hist && hist->entries() > 50) {
                auto root_hist = hist->to_root(bin_name + "_fit", bin_name);
                auto [result_narrow, result_wide] = fit_histogram_compare_ranges(root_hist.get(), bin_name);
                if (result_narrow.valid) {
                    fit_results_narrow[bin_name] = result_narrow;
                    total_rho0_narrow += result_narrow.n_rho0;
                    total_rho0_narrow_err_sq += result_narrow.n_rho0_err * result_narrow.n_rho0_err;
                }
                if (result_wide.valid) {
                    fit_results_wide[bin_name] = result_wide;
                    total_rho0_wide += result_wide.n_rho0;
                    total_rho0_wide_err_sq += result_wide.n_rho0_err * result_wide.n_rho0_err;
                }
            }
        }

        // Save LC fit results to ROOT file
        save_lc_fit_results_to_file(fit_results_narrow, fit_results_wide,
                                    total_rho0_narrow, std::sqrt(total_rho0_narrow_err_sq),
                                    total_rho0_wide, std::sqrt(total_rho0_wide_err_sq));

        // Fit region-specific Q²-binned histograms
        fit_region_q2_histograms(m_hist_ff, "Forward-Forward", "Rho0/Forward");
        fit_region_q2_histograms(m_hist_mixed, "Mixed", "Rho0/Mixed");
        fit_region_q2_histograms(m_hist_cc, "Central-Central", "Rho0/Central");

        // Print summary table comparing both fit ranges
        if (!fit_results_narrow.empty() || !fit_results_wide.empty()) {
            std::vector<std::string> columns = {"Bin", "N(ρ⁰) [0.3-1.2]", "N(ρ⁰) [0.3-1.4]", "Diff (%)"};
            std::vector<std::vector<std::string>> rows;

            // Collect all bin names
            std::set<std::string> all_bins;
            for (const auto& [name, _] : fit_results_narrow) all_bins.insert(name);
            for (const auto& [name, _] : fit_results_wide) all_bins.insert(name);

            for (const auto& bin_name : all_bins) {
                if (bin_name == "All") continue;

                double n_narrow = 0, n_narrow_err = 0;
                double n_wide = 0, n_wide_err = 0;

                if (fit_results_narrow.count(bin_name)) {
                    n_narrow = fit_results_narrow[bin_name].n_rho0;
                    n_narrow_err = fit_results_narrow[bin_name].n_rho0_err;
                }
                if (fit_results_wide.count(bin_name)) {
                    n_wide = fit_results_wide[bin_name].n_rho0;
                    n_wide_err = fit_results_wide[bin_name].n_rho0_err;
                }

                double diff_pct = (n_narrow > 0) ? 100.0 * (n_wide - n_narrow) / n_narrow : 0.0;

                rows.push_back({
                    bin_name,
                    fmt::format("{:.0f} ± {:.0f}", n_narrow, n_narrow_err),
                    fmt::format("{:.0f} ± {:.0f}", n_wide, n_wide_err),
                    fmt::format("{:+.1f}%", diff_pct)
                });
            }

            // Add integrated result if available
            if (fit_results_narrow.count("All") || fit_results_wide.count("All")) {
                double n_narrow = fit_results_narrow.count("All") ? fit_results_narrow["All"].n_rho0 : 0;
                double n_narrow_err = fit_results_narrow.count("All") ? fit_results_narrow["All"].n_rho0_err : 0;
                double n_wide = fit_results_wide.count("All") ? fit_results_wide["All"].n_rho0 : 0;
                double n_wide_err = fit_results_wide.count("All") ? fit_results_wide["All"].n_rho0_err : 0;
                double diff_pct = (n_narrow > 0) ? 100.0 * (n_wide - n_narrow) / n_narrow : 0.0;

                rows.push_back({
                    "All (integrated)",
                    fmt::format("{:.0f} ± {:.0f}", n_narrow, n_narrow_err),
                    fmt::format("{:.0f} ± {:.0f}", n_wide, n_wide_err),
                    fmt::format("{:+.1f}%", diff_pct)
                });
            }

            clas12::framework::Log::print_yield_summary_table("ρ⁰ Yield Comparison (Integration: 0.6-1.0 GeV)", columns, rows);
        }
    }

    /**
     * @brief Configuration for saving fit results
     */
    struct FitResultsConfig {
        std::string bin_prefix;      // "Q2" or "LC"
        std::string tree_name;       // "rho0_fit_results" or "rho0_lc_fit_results"
        std::string tree_title;      // Tree description
        std::string fits_dir;        // "Fits" or "Fits_LC"
        std::string all_func_name;   // "fit_All" or "fit_All_LC"
        std::string root_dir = "Rho0";  // Root directory in ROOT file
    };

    /**
     * @brief Save fit results to ROOT file as a TTree (generic implementation)
     * @tparam BinArray Array type containing bin edges
     */
    template<typename BinArray>
    auto save_fit_results_to_file(
        const std::map<std::string, FitResult>& results,
        double total_n_rho0,
        double total_n_rho0_err,
        const BinArray& bins,
        const FitResultsConfig& config
    ) -> void {
        if (!m_output_svc || !m_output_svc->is_valid()) return;

        auto* out_svc = m_output_svc->get();
        if (!out_svc->is_file_open()) {
            clas12::framework::Log::error(name(), "Output file is not open for writing fit results");
            return;
        }

        // Acquire lock once and do all operations
        std::lock_guard<std::mutex> lock(out_svc->mutex());

        TFile* file = out_svc->get_file();
        if (!file) {
            clas12::framework::Log::error(name(), "Failed to get file pointer");
            return;
        }

        // Navigate to root directory
        file->cd();
        if (!file->GetDirectory(config.root_dir.c_str())) {
            file->mkdir(config.root_dir.c_str());
        }
        file->cd(config.root_dir.c_str());

        // Create a TTree to store fit results
        auto* tree = new TTree(config.tree_name.c_str(), config.tree_title.c_str());

        // Variables for the tree
        char bin_name_buf[64];
        double bin_min, bin_max;
        double n_rho0, n_rho0_err;
        double mass, mass_err;
        double chi2_ndf;

        std::string bin_branch = fmt::format("{}_bin", config.bin_prefix);
        std::string min_branch = fmt::format("{}_min", config.bin_prefix);
        std::string max_branch = fmt::format("{}_max", config.bin_prefix);

        tree->Branch(bin_branch.c_str(), bin_name_buf, fmt::format("{}/C", bin_branch).c_str());
        tree->Branch(min_branch.c_str(), &bin_min, fmt::format("{}/D", min_branch).c_str());
        tree->Branch(max_branch.c_str(), &bin_max, fmt::format("{}/D", max_branch).c_str());
        tree->Branch("n_rho0", &n_rho0, "n_rho0/D");
        tree->Branch("n_rho0_err", &n_rho0_err, "n_rho0_err/D");
        tree->Branch("mass", &mass, "mass/D");
        tree->Branch("mass_err", &mass_err, "mass_err/D");
        tree->Branch("chi2_ndf", &chi2_ndf, "chi2_ndf/D");

        // Fill tree with bin results
        for (size_t i = 0; i < bins.size(); ++i) {
            std::string bin_name = fmt::format("{}_{:.1f}_{:.1f}", config.bin_prefix, bins[i].first, bins[i].second);
            auto it = results.find(bin_name);

            strncpy(bin_name_buf, bin_name.c_str(), sizeof(bin_name_buf) - 1);
            bin_name_buf[sizeof(bin_name_buf) - 1] = '\0';
            bin_min = bins[i].first;
            bin_max = bins[i].second;

            if (it != results.end() && it->second.valid) {
                n_rho0 = it->second.n_rho0;
                n_rho0_err = it->second.n_rho0_err;
                mass = it->second.mass;
                mass_err = it->second.mass_err;
                chi2_ndf = it->second.chi2_ndf;
            } else {
                n_rho0 = 0;
                n_rho0_err = 0;
                mass = 0;
                mass_err = 0;
                chi2_ndf = 0;
            }
            tree->Fill();
        }

        // Add total row
        strncpy(bin_name_buf, "Total", sizeof(bin_name_buf) - 1);
        bin_min = bins.front().first;
        bin_max = bins.back().second;
        n_rho0 = total_n_rho0;
        n_rho0_err = total_n_rho0_err;
        if (results.count("All") && results.at("All").valid) {
            mass = results.at("All").mass;
            mass_err = results.at("All").mass_err;
            chi2_ndf = results.at("All").chi2_ndf;
        } else {
            mass = 0;
            mass_err = 0;
            chi2_ndf = 0;
        }
        tree->Fill();

        // Write tree to the file
        tree->Write("", TObject::kOverwrite);
        clas12::framework::Log::info(name(), "Fit results saved to TTree '{}/{}' with {} entries", config.root_dir, config.tree_name, tree->GetEntries());

        // Save fit functions separately for easy access in Python
        std::string fits_path = fmt::format("{}/{}", config.root_dir, config.fits_dir);
        file->cd();
        if (!file->GetDirectory(fits_path.c_str())) {
            file->mkdir(fits_path.c_str());
        }
        file->cd(fits_path.c_str());

        // Save fit function for each bin
        for (size_t i = 0; i < bins.size(); ++i) {
            std::string bin_name = fmt::format("{}_{:.1f}_{:.1f}", config.bin_prefix, bins[i].first, bins[i].second);
            auto it = results.find(bin_name);

            if (it != results.end() && it->second.valid && it->second.fit_function) {
                std::string func_name = fmt::format("fit_{}", bin_name);
                it->second.fit_function->SetName(func_name.c_str());
                it->second.fit_function->Write("", TObject::kOverwrite);
                clas12::framework::Log::info(name(), "  Saved fit function '{}'", func_name);
            }
        }

        // Save fit function for integrated (All) result
        if (results.count("All") && results.at("All").valid && results.at("All").fit_function) {
            auto& all_func = results.at("All").fit_function;
            all_func->SetName(config.all_func_name.c_str());
            all_func->Write("", TObject::kOverwrite);
            clas12::framework::Log::info(name(), "  Saved fit function '{}'", config.all_func_name);
        }

        clas12::framework::Log::info(name(), "All fit functions saved to '{}' directory", fits_path);
    }

    /**
     * @brief Save Q² fit results to ROOT file (both fit ranges)
     */
    auto save_Q2_fit_results_to_file(
        const std::map<std::string, FitResult>& results_narrow,
        const std::map<std::string, FitResult>& results_wide,
        double total_n_rho0_narrow, double total_n_rho0_narrow_err,
        double total_n_rho0_wide, double total_n_rho0_wide_err) -> void {

        FitResultsConfig config_narrow{
            .bin_prefix = "Q2",
            .tree_name = "rho0_fit_results_narrow",
            .tree_title = "Rho0 Fit Results (Fit Range 0.3-1.2)",
            .fits_dir = "Fits_narrow",
            .all_func_name = "fit_All_narrow"
        };
        save_fit_results_to_file(results_narrow, total_n_rho0_narrow, total_n_rho0_narrow_err, Q2_BINS, config_narrow);

        FitResultsConfig config_wide{
            .bin_prefix = "Q2",
            .tree_name = "rho0_fit_results_wide",
            .tree_title = "Rho0 Fit Results (Fit Range 0.3-1.4)",
            .fits_dir = "Fits_wide",
            .all_func_name = "fit_All_wide"
        };
        save_fit_results_to_file(results_wide, total_n_rho0_wide, total_n_rho0_wide_err, Q2_BINS, config_wide);
    }

    /**
     * @brief Save lc fit results to ROOT file (both fit ranges)
     */
    auto save_lc_fit_results_to_file(
        const std::map<std::string, FitResult>& results_narrow,
        const std::map<std::string, FitResult>& results_wide,
        double total_n_rho0_narrow, double total_n_rho0_narrow_err,
        double total_n_rho0_wide, double total_n_rho0_wide_err) -> void {

        FitResultsConfig config_narrow{
            .bin_prefix = "LC",
            .tree_name = "rho0_lc_fit_results_narrow",
            .tree_title = "Rho0 lc Fit Results (Fit Range 0.3-1.2)",
            .fits_dir = "Fits_LC_narrow",
            .all_func_name = "fit_All_LC_narrow"
        };
        save_fit_results_to_file(results_narrow, total_n_rho0_narrow, total_n_rho0_narrow_err, LC_BINS, config_narrow);

        FitResultsConfig config_wide{
            .bin_prefix = "LC",
            .tree_name = "rho0_lc_fit_results_wide",
            .tree_title = "Rho0 lc Fit Results (Fit Range 0.3-1.4)",
            .fits_dir = "Fits_LC_wide",
            .all_func_name = "fit_All_LC_wide"
        };
        save_fit_results_to_file(results_wide, total_n_rho0_wide, total_n_rho0_wide_err, LC_BINS, config_wide);
    }

    /**
     * @brief Fit region-specific Q²-binned histograms and save results
     */
    auto fit_region_q2_histograms(RegionHistograms& rh, const std::string& region_label, const std::string& root_dir) -> void {
        // Fit selected mass histogram for this region
        std::map<std::string, FitResult> results_narrow, results_wide;
        double total_narrow = 0.0, total_narrow_err_sq = 0.0;
        double total_wide = 0.0, total_wide_err_sq = 0.0;

        if (rh.mass_selected && rh.mass_selected->entries() > 100) {
            auto root_hist = rh.mass_selected->to_root("mass_selected_fit", region_label);
            auto [rn, rw] = fit_histogram_compare_ranges(root_hist.get(), region_label + " All Q²");
            if (rn.valid) results_narrow["All"] = rn;
            if (rw.valid) results_wide["All"] = rw;
        }

        // Fit Q²-binned histograms
        for (const auto& [bin_name, hist] : rh.mass_q2_bins) {
            if (hist && hist->entries() > 50) {
                auto root_hist = hist->to_root(bin_name + "_fit", bin_name);
                auto [rn, rw] = fit_histogram_compare_ranges(root_hist.get(), region_label + " " + bin_name);
                if (rn.valid) {
                    results_narrow[bin_name] = rn;
                    total_narrow += rn.n_rho0;
                    total_narrow_err_sq += rn.n_rho0_err * rn.n_rho0_err;
                }
                if (rw.valid) {
                    results_wide[bin_name] = rw;
                    total_wide += rw.n_rho0;
                    total_wide_err_sq += rw.n_rho0_err * rw.n_rho0_err;
                }
            }
        }

        if (results_narrow.empty() && results_wide.empty()) return;

        // Save results under the region directory
        FitResultsConfig config_narrow{
            .bin_prefix = "Q2",
            .tree_name = "rho0_fit_results_narrow",
            .tree_title = fmt::format("{} Rho0 Fit Results (Fit Range 0.3-1.2)", region_label),
            .fits_dir = "Fits_narrow",
            .all_func_name = "fit_All_narrow",
            .root_dir = root_dir
        };
        save_fit_results_to_file(results_narrow, total_narrow, std::sqrt(total_narrow_err_sq), Q2_BINS, config_narrow);

        FitResultsConfig config_wide{
            .bin_prefix = "Q2",
            .tree_name = "rho0_fit_results_wide",
            .tree_title = fmt::format("{} Rho0 Fit Results (Fit Range 0.3-1.4)", region_label),
            .fits_dir = "Fits_wide",
            .all_func_name = "fit_All_wide",
            .root_dir = root_dir
        };
        save_fit_results_to_file(results_wide, total_wide, std::sqrt(total_wide_err_sq), Q2_BINS, config_wide);

        clas12::framework::Log::info(name(), "{} region: fitted {} Q² bins", region_label, rh.mass_q2_bins.size());
    }

    /**
     * @brief Fit a single histogram with BW + pol3
     * @param hist Histogram to fit
     * @param label Label for logging
     * @param fit_min Fit range minimum [GeV]
     * @param fit_max Fit range maximum [GeV]
     * @param integ_min Integration range minimum [GeV] (default 0.6)
     * @param integ_max Integration range maximum [GeV] (default 1.0)
     * @return FitResult containing mass and signal yield
     */
    auto fit_histogram(TH1D* hist, const std::string& label,
                       double fit_min = 0.3, double fit_max = 1.2,
                       double integ_min = 0.6, double integ_max = 1.0) -> FitResult {
        FitResult result;
        if (!hist || hist->GetEntries() < 50) return result;

        double bin_width = hist->GetBinWidth(1);

        // Store fit/integration ranges in result
        result.fit_min = fit_min;
        result.fit_max = fit_max;
        result.integ_min = integ_min;
        result.integ_max = integ_max;

        // Create fit function: BW + pol3 (6 parameters)
        auto fit_func = std::make_unique<TF1>(
            fmt::format("fit_{}", hist->GetName()).c_str(),
            BWPlusPol3, fit_min, fit_max, 6
        );

        // Set initial parameters
        // par[0] = BW mass, par[1] = BW amplitude
        // par[2-5] = polynomial coefficients
        fit_func->SetParameter(0, RHO0_MASS);           // Mass ~ 0.77 GeV
        fit_func->SetParameter(1, hist->GetMaximum());  // Amplitude
        fit_func->SetParameter(2, 0);                   // p0
        fit_func->SetParameter(3, 0);                   // p1
        fit_func->SetParameter(4, 0);                   // p2
        fit_func->SetParameter(5, 0);                   // p3

        // Set parameter limits
        fit_func->SetParLimits(0, 0.6, 0.9);   // Mass between 0.6-0.9 GeV (around rho0 mass)
        fit_func->SetParLimits(1, 0, hist->GetMaximum() * 10);  // Amplitude positive

        // Set parameter names
        fit_func->SetParName(0, "M_{#rho}");
        fit_func->SetParName(1, "Amplitude");
        fit_func->SetParName(2, "p0");
        fit_func->SetParName(3, "p1");
        fit_func->SetParName(4, "p2");
        fit_func->SetParName(5, "p3");

        // Perform fit
        int fit_status = hist->Fit(fit_func.get(), "QRS0");  // Q=quiet, R=use range, S=return status, 0=don't draw

        if (fit_status != 0) {
            clas12::framework::Log::warning(name(), "Fit failed for {} (range [{:.1f}, {:.1f}])", label, fit_min, fit_max);
            return result;
        }

        // Extract fit parameters
        result.mass = fit_func->GetParameter(0);
        result.mass_err = fit_func->GetParError(0);
        double amplitude = fit_func->GetParameter(1);
        double chi2 = fit_func->GetChisquare();
        int ndf = fit_func->GetNDF();
        result.chi2_ndf = ndf > 0 ? chi2 / ndf : 0.0;

        // Create signal-only function to integrate
        TF1 signal_func("signal_func", BreitWigner, fit_min, fit_max, 2);
        signal_func.SetParameter(0, result.mass);
        signal_func.SetParameter(1, amplitude);

        // Integrate signal function between 0.6 and 1.0 GeV
        double signal_integral = signal_func.Integral(integ_min, integ_max);
        result.n_rho0 = signal_integral / bin_width;

        // Error propagation (simplified: dominated by amplitude error)
        if (amplitude > 0) {
            result.n_rho0_err = std::sqrt(result.n_rho0);
        }

        result.valid = true;

        // Store fit function in the result
        result.fit_function = std::shared_ptr<TF1>(static_cast<TF1*>(fit_func->Clone()));

        return result;
    }

    /**
     * @brief Fit histogram with two different fit ranges and compare yields
     * @param hist Histogram to fit
     * @param label Label for logging
     * @return Pair of FitResults: first=[0.3,1.2], second=[0.3,1.4]
     */
    auto fit_histogram_compare_ranges(TH1D* hist, const std::string& label)
        -> std::pair<FitResult, FitResult> {

        // Integration range is always 0.6 to 1.0 GeV
        constexpr double integ_min = 0.6;
        constexpr double integ_max = 1.0;

        // Fit with range [0.3, 1.2]
        auto result_narrow = fit_histogram(hist, label + " [0.3-1.2]", 0.3, 1.2, integ_min, integ_max);

        // Fit with range [0.3, 1.4]
        auto result_wide = fit_histogram(hist, label + " [0.3-1.4]", 0.3, 1.4, integ_min, integ_max);

        // Print comparison
        if (result_narrow.valid && result_wide.valid) {
            double yield_diff = result_wide.n_rho0 - result_narrow.n_rho0;
            double yield_diff_pct = (result_narrow.n_rho0 > 0) ?
                100.0 * yield_diff / result_narrow.n_rho0 : 0.0;

            clas12::framework::Log::print_fit_results_table(
                fmt::format("Fit Comparison: {}", label),
                {
                    {"Range [0.3-1.2]", fmt::format("N={:.0f}±{:.0f}, M={:.4f}, χ²/ndf={:.2f}",
                        result_narrow.n_rho0, result_narrow.n_rho0_err, result_narrow.mass, result_narrow.chi2_ndf), ""},
                    {"Range [0.3-1.4]", fmt::format("N={:.0f}±{:.0f}, M={:.4f}, χ²/ndf={:.2f}",
                        result_wide.n_rho0, result_wide.n_rho0_err, result_wide.mass, result_wide.chi2_ndf), ""},
                    {"Yield Diff", fmt::format("{:.0f} ({:+.1f}%)", yield_diff, yield_diff_pct), ""},
                    {"Integration", fmt::format("[{:.1f}, {:.1f}] GeV", integ_min, integ_max), ""}
                }
            );
        }

        return {result_narrow, result_wide};
    }
};

}  // namespace clas12::algorithms
