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
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/physics/DISAnalysis.hpp>

// Optional DTF support - include if Eigen is available
#if __has_include(<Eigen/Dense>)
#define CLAS12_HAS_DTF 1
#include <clas12/framework/services/DecayTreeFitterService.hpp>
#else
#define CLAS12_HAS_DTF 0
#endif

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

    // DecayTreeFitter results
    bool dtf_converged = false;
    double dtf_mass = 0.0;          // Fitted mass [GeV/c²]
    double dtf_chi2 = -1.0;         // Fit chi-squared
    int dtf_ndf = 0;                // Degrees of freedom
    double dtf_probability = 0.0;   // Fit probability
    ROOT::Math::PxPyPzEVector dtf_pip_p4;  // Fitted pi+ 4-momentum
    ROOT::Math::PxPyPzEVector dtf_pim_p4;  // Fitted pi- 4-momentum
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
        std::string name = "Rho0ReconstructionAlg"
    )
        : SimpleAlgorithm(std::move(name))
        , m_cutflow("Rho0 Reconstruction") {
        declare_property(m_zh_min);
        declare_property(m_lc_max);
        declare_property(m_t_min);
        declare_property(m_t_max);
        declare_property(m_delta_vz_max);
        declare_property(m_enable_ntuple);
        declare_property(m_enable_fitting);
        declare_property(m_enable_dtf);
        declare_property(m_dtf_min_probability);
    }

    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<clas12::framework::NTupleService>("NTupleService");
        m_output_svc = &require_service<clas12::framework::OutputFileService>("OutputFileService");
#if CLAS12_HAS_DTF
        if (m_enable_dtf.get()) {
            m_dtf_svc = &require_service<clas12::framework::DecayTreeFitterService>("DecayTreeFitterService");
        }
#endif
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        using Log = clas12::framework::Log;

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Pion selection", "Using pre-selected pions from PionSelectionAlg");
        Log::blank();

        Log::subsection("Kinematic Cuts");
        Log::config(name(), "zh min", m_zh_min.get());
        Log::config(name(), "lc max", fmt::format("{} fm", m_lc_max.get()));
        Log::config(name(), "-t range", fmt::format("[{}, {}] GeV²", m_t_min.get(), m_t_max.get()));
        Log::config(name(), "Max |Δvz|", fmt::format("{} cm", m_delta_vz_max.get()));
        Log::blank();

        Log::subsection("Output Options");
        Log::config(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");
        Log::config(name(), "BW+pol3 fitting", m_enable_fitting.get() ? "enabled" : "disabled");
        Log::config(name(), "DecayTreeFitter", m_enable_dtf.get() ? "enabled" : "disabled");
        if (m_enable_dtf.get()) {
            Log::config(name(), "DTF min probability", m_dtf_min_probability.get());
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
        m_cutflow.add_cut("delta_vz_pim Cut");
        m_cutflow.add_cut("delta_vz_pip Cut");
        m_cutflow.add_cut("zh > 0.9");
        m_cutflow.add_cut("lc < 1 fm");
        m_cutflow.add_cut("0.1 < -t < 0.5");

        Log::stat(name(), "Cutflow stages", m_cutflow.num_cuts());
        Log::success(name(), "Initialization complete");

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Track run number from RUN::config bank
        auto& run_config = event.get_bank("RUN::config");
        if (run_config.rows() > 0) {
            int run = run_config.get<int>("run", 0);
            m_runs.insert(run);
        }

        // Check for good electron decision from upstream algorithm
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodDISElectron)) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Has Good DIS Electron");

        // Get DIS kinematics from upstream
        auto Q2_opt = event.retrieve_typed(clas12::framework::Keys::DIS_Q2);
        auto nu_opt = event.retrieve_typed(clas12::framework::Keys::DIS_nu);
        if (!Q2_opt || !nu_opt) return clas12::framework::StatusCode::FILTERED();
        double Q2 = *Q2_opt;
        double nu = *nu_opt;

        // Get electron
        auto electrons_opt = event.retrieve_typed(clas12::framework::Keys::SelectedElectronsCapital);
        if (!electrons_opt || electrons_opt->empty()) return clas12::framework::StatusCode::FILTERED();
        const auto& electron = electrons_opt->front();

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

        // Update cutflow counters
        update_cutflow(candidates);

        // Check if any candidates passed all cuts
        if (count_passing_candidates(candidates) == 0) return clas12::framework::StatusCode::FILTERED();

        // Fill post-cut histograms
        fill_post_cut_histograms(candidates, Q2);

        // Store candidates in event for downstream algorithms
        store_candidates_in_event(candidates, event);

        // Fill ntuple
        fill_ntuple(candidates, electron, Q2);

        return clas12::framework::StatusCode::SUCCESS();
    }
    

    auto on_finalize() -> clas12::framework::StatusCode override {
        using Log = clas12::framework::Log;

        Log::subsection(fmt::format("{} Summary", name()));

        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            auto entries = m_ntuple_svc->get()->get_entries("rho0_events");
            Log::stat(name(), "N-tuple entries", entries);
        }

        Log::stat(name(), "Unique runs processed", m_runs.size());
        Log::blank();

        // Perform BW + pol3 fitting if enabled
        if (m_enable_fitting.get()) {
            fit_invariant_mass_histograms();
        }

        // Save run metadata to ROOT file
        save_run_metadata();

        Log::blank();
        m_cutflow.print();

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

                // Apply cuts and cache results
                cand.passes_delta_vz_pip = std::abs(cand.delta_vz_pip) < m_delta_vz_max.get();
                cand.passes_delta_vz_pim = std::abs(cand.delta_vz_pim) < m_delta_vz_max.get();
                cand.passes_zh = cand.zh > m_zh_min.get();
                cand.passes_lc = cand.lc < m_lc_max.get();
                cand.passes_t = (cand.minus_t > m_t_min.get()) && (cand.minus_t < m_t_max.get());
                cand.passes_all_cuts = cand.passes_delta_vz_pip && cand.passes_delta_vz_pim &&
                                       cand.passes_zh && cand.passes_lc && cand.passes_t;

                // Perform DecayTreeFitter mass-constrained fit if enabled
#if CLAS12_HAS_DTF
                if (m_enable_dtf.get() && m_dtf_svc && m_dtf_svc->is_valid()) {
                    // Use primary vertex from electron for pointing constraint
                    ROOT::Math::XYZVector primary_vertex(electron.vx(), electron.vy(), e_vz);

                    // Perform mass-constrained fit to rho0 mass
                    auto dtf_result = m_dtf_svc->get()->fit_two_body(pip, pim, RHO0_MASS, primary_vertex);

                    cand.dtf_converged = dtf_result.converged;
                    cand.dtf_chi2 = dtf_result.chi2;
                    cand.dtf_ndf = dtf_result.ndf;
                    cand.dtf_probability = dtf_result.probability;

                    if (dtf_result.converged) {
                        // Store fitted 4-momenta
                        cand.dtf_pip_p4 = dtf_result.get_momentum(0);
                        cand.dtf_pim_p4 = dtf_result.get_momentum(1);
                        // Compute fitted mass (should be close to RHO0_MASS due to constraint)
                        cand.dtf_mass = (cand.dtf_pip_p4 + cand.dtf_pim_p4).M();
                    }
                }
#endif

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

            // Fill DTF histograms if fit converged
            if (cand.dtf_converged) {
                if (h_dtf_mass) h_dtf_mass->fill(cand.dtf_mass);
                if (h_dtf_chi2) h_dtf_chi2->fill(cand.dtf_chi2);
                if (h_dtf_probability) h_dtf_probability->fill(cand.dtf_probability);
                // Mass pull: (M_dtf - M_rho0) normalized by expected resolution (~10 MeV)
                double mass_pull = (cand.dtf_mass - RHO0_MASS) / 0.010;
                if (h_dtf_mass_pull) h_dtf_mass_pull->fill(mass_pull);
                if (h_dtf_mass_vs_unfitted) h_dtf_mass_vs_unfitted->fill(cand.mass, cand.dtf_mass);
            }

            fill_q2_binned_histogram(Q2, cand.mass);
        }
    }

    /**
     * @brief Fill lc-binned histograms for candidates passing vertex, zh, Q2, and t cuts
     */
    auto fill_lc_binned_histograms(const std::vector<Rho0Candidate>& candidates, double Q2) -> void {
        bool passes_Q2 = (Q2 > 1.0);
        for (const auto& cand : candidates) {
            if (cand.passes_delta_vz_pim && cand.passes_delta_vz_pip &&
                cand.passes_zh && passes_Q2 && cand.passes_t) {
                fill_lc_binned_histogram(cand.lc, cand.mass);
            }
        }
    }

    /**
     * @brief Update cutflow counters based on candidate cut results
     */
    auto update_cutflow(const std::vector<Rho0Candidate>& candidates) -> void {
        bool any_passed_delta_vz_pim = false;
        bool any_passed_delta_vz_pip = false;
        bool any_passed_zh = false;
        bool any_passed_lc = false;
        bool any_passed_t = false;

        for (const auto& cand : candidates) {
            any_passed_delta_vz_pim |= cand.passes_delta_vz_pim;
            any_passed_delta_vz_pip |= cand.passes_delta_vz_pip;
            any_passed_zh |= cand.passes_zh;
            any_passed_lc |= cand.passes_lc;
            any_passed_t |= cand.passes_t;
        }

        if (any_passed_delta_vz_pim) m_cutflow.pass("delta_vz_pim Cut");
        if (any_passed_delta_vz_pip) m_cutflow.pass("delta_vz_pip Cut");
        if (any_passed_zh) m_cutflow.pass("zh > 0.9");
        if (any_passed_lc) m_cutflow.pass("lc < 1 fm");
        if (any_passed_t) m_cutflow.pass("0.1 < -t < 0.5");
    }

    /**
     * @brief Fill ntuple with candidate data
     */
    auto fill_ntuple(const std::vector<Rho0Candidate>& candidates, const clas12::Particle& electron, double Q2) -> void {
        if (!m_ntuple_svc || !m_ntuple_svc->is_valid() || !m_enable_ntuple.get()) return;

        // Collect data from passing candidates
        std::vector<double> local_pip_p, local_pip_theta, local_pip_phi;
        std::vector<double> local_pim_p, local_pim_theta, local_pim_phi;
        std::vector<double> local_rho0_masses, local_rho0_ps, local_rho0_thetas, local_rho0_phis;
        std::vector<double> local_zh, local_lc, local_t, local_delta_vz_pim, local_delta_vz_pip;
        // DTF variables
        std::vector<int> local_dtf_converged;
        std::vector<double> local_dtf_mass, local_dtf_chi2, local_dtf_probability;

        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;

            local_pip_p.push_back(cand.pip.p());
            local_pip_theta.push_back(cand.pip.theta());
            local_pip_phi.push_back(cand.pip.phi());
            local_pim_p.push_back(cand.pim.p());
            local_pim_theta.push_back(cand.pim.theta());
            local_pim_phi.push_back(cand.pim.phi());
            local_rho0_masses.push_back(cand.mass);
            local_rho0_ps.push_back(cand.momentum);
            local_rho0_thetas.push_back(cand.theta);
            local_rho0_phis.push_back(cand.phi);
            local_zh.push_back(cand.zh);
            local_lc.push_back(cand.lc);
            local_t.push_back(cand.minus_t);
            local_delta_vz_pim.push_back(cand.delta_vz_pim);
            local_delta_vz_pip.push_back(cand.delta_vz_pip);

            // DTF variables
            local_dtf_converged.push_back(cand.dtf_converged ? 1 : 0);
            local_dtf_mass.push_back(cand.dtf_mass);
            local_dtf_chi2.push_back(cand.dtf_chi2);
            local_dtf_probability.push_back(cand.dtf_probability);
        }

        if (local_rho0_masses.empty()) return;

        std::lock_guard<std::mutex> lock(m_ntuple_mutex);

        m_e_p = electron.p();
        m_e_theta = electron.theta();
        m_e_phi = electron.phi();
        m_e_vz = electron.vz();
        m_Q2 = Q2;

        m_pip_p = std::move(local_pip_p);
        m_pip_theta = std::move(local_pip_theta);
        m_pip_phi = std::move(local_pip_phi);
        m_pim_p = std::move(local_pim_p);
        m_pim_theta = std::move(local_pim_theta);
        m_pim_phi = std::move(local_pim_phi);

        m_rho0_masses = std::move(local_rho0_masses);
        m_rho0_ps = std::move(local_rho0_ps);
        m_rho0_thetas = std::move(local_rho0_thetas);
        m_rho0_phis = std::move(local_rho0_phis);
        m_rho0_zh = std::move(local_zh);
        m_rho0_lc = std::move(local_lc);
        m_rho0_t = std::move(local_t);
        m_delta_vz_pim = std::move(local_delta_vz_pim);
        m_delta_vz_pip = std::move(local_delta_vz_pip);

        // DTF variables
        m_dtf_converged = std::move(local_dtf_converged);
        m_dtf_mass = std::move(local_dtf_mass);
        m_dtf_chi2 = std::move(local_dtf_chi2);
        m_dtf_probability = std::move(local_dtf_probability);

        m_ntuple_svc->get()->fill("rho0_events");

        // Clear vectors for next event
        m_pip_p.clear();
        m_pip_theta.clear();
        m_pip_phi.clear();
        m_pim_p.clear();
        m_pim_theta.clear();
        m_pim_phi.clear();
        m_rho0_masses.clear();
        m_rho0_ps.clear();
        m_rho0_thetas.clear();
        m_rho0_phis.clear();
        m_rho0_zh.clear();
        m_rho0_lc.clear();
        m_rho0_t.clear();
        m_delta_vz_pim.clear();
        m_delta_vz_pip.clear();

        // DTF vectors
        m_dtf_converged.clear();
        m_dtf_mass.clear();
        m_dtf_chi2.clear();
        m_dtf_probability.clear();
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
    // Properties
    clas12::framework::Property<double> m_zh_min{"zh_min", 0.9, "Minimum zh (energy fraction)"};
    clas12::framework::Property<double> m_lc_max{"lc_max", 1.0, "Maximum coherence length [fm]"};
    clas12::framework::Property<double> m_t_min{"t_min", 0.1, "Minimum -t [GeV²]"};
    clas12::framework::Property<double> m_t_max{"t_max", 0.5, "Maximum -t [GeV²]"};
    clas12::framework::Property<double> m_delta_vz_max{"delta_vz_max", 5.0, "Maximum |vz_e - vz_pion| [cm]"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    clas12::framework::Property<bool> m_enable_fitting{"enable_fitting", true, "Enable BW+pol3 fitting in finalize"};
    clas12::framework::Property<bool> m_enable_dtf{"enable_dtf", true, "Enable DecayTreeFitter for mass-constrained fits"};
    clas12::framework::Property<double> m_dtf_min_probability{"dtf_min_probability", 0.01, "Minimum DTF probability for good fit"};

    // Service handles
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::OutputFileService>* m_output_svc = nullptr;
#if CLAS12_HAS_DTF
    clas12::framework::ServiceHandle<clas12::framework::DecayTreeFitterService>* m_dtf_svc = nullptr;
#endif

    // Cutflow
    clas12::framework::Cutflow m_cutflow;

    // Run tracking for metadata
    std::set<int> m_runs;

    // Mutex for thread safety
    mutable std::mutex m_ntuple_mutex;

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

    // DTF histograms (only available when CLAS12_HAS_DTF is defined)
    clas12::framework::AtomicHistogram1D* h_dtf_mass = nullptr;           // DTF fitted mass
    clas12::framework::AtomicHistogram1D* h_dtf_chi2 = nullptr;           // DTF chi2
    clas12::framework::AtomicHistogram1D* h_dtf_probability = nullptr;    // DTF probability
    clas12::framework::AtomicHistogram1D* h_dtf_mass_pull = nullptr;      // (M_dtf - M_rho0) / sigma
    clas12::framework::AtomicHistogram2D* h_dtf_mass_vs_unfitted = nullptr; // DTF mass vs unfitted mass

    // Q² bin edges: 1.0-2.0, 2.0-2.5, 2.5-3.0, 3.0-4.5, 4.5-6 GeV²
    static constexpr std::array<std::pair<double, double>, 5> Q2_BINS = {{
        {1.0, 2.0},
        {2.0, 2.5},
        {2.5, 3.0},
        {3.0, 4.5},
        {4.5, 6.0}
    }};

    static constexpr std::array<std::pair<double, double>, 7> LC_BINS = {{
        {0.0, 0.2},
        {0.2, 0.4},
        {0.4, 0.6},
        {0.6, 0.8},
        {0.8, 1.0},
        {1.0, 1.5},
        {1.5, 2.0},
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
        bool valid = false;
        std::shared_ptr<TF1> fit_function;  // Store fit function for saving
    };

    // NTuple branch variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_Q2 = 0.0;
    std::vector<double> m_pip_p;
    std::vector<double> m_pip_theta;
    std::vector<double> m_pip_phi;
    std::vector<double> m_pim_p;
    std::vector<double> m_pim_theta;
    std::vector<double> m_pim_phi;
    std::vector<double> m_rho0_masses;
    std::vector<double> m_rho0_ps;
    std::vector<double> m_rho0_thetas;
    std::vector<double> m_rho0_phis;
    std::vector<double> m_rho0_zh;
    std::vector<double> m_rho0_lc;
    std::vector<double> m_rho0_t;
    std::vector<double> m_delta_vz_pim;
    std::vector<double> m_delta_vz_pip;

    // DTF ntuple variables
    std::vector<int> m_dtf_converged;        // 1 if converged, 0 otherwise
    std::vector<double> m_dtf_mass;          // Fitted mass
    std::vector<double> m_dtf_chi2;          // Fit chi2
    std::vector<double> m_dtf_probability;   // Fit probability

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
        h_delta_vz_pim = svc->book_hist1D("/Rho0/delta_vz_pim", "#Delta v_{z} (before cuts);v_{z}^{e} - v_{z}^{#pi^{-}} [cm];Events", 100, -50, 50);
        h_delta_vz_pip = svc->book_hist1D("/Rho0/delta_vz_pip", "#Delta v_{z} (before cuts);v_{z}^{e} - v_{z}^{#pi^{+}} [cm];Events", 100, -50, 50);

        // Kinematic variable histograms (after cuts)
        h_zh_selected = svc->book_hist1D("/Rho0/zh_selected", "z_{h} (after cuts);z_{h} = E_{#rho^{0}}/#nu;Events", 100, 0, 1.5);
        h_lc_selected = svc->book_hist1D("/Rho0/lc_selected", "Coherence Length (after cuts);l_{c} [fm];Events", 100, 0, 5);
        h_t_selected = svc->book_hist1D("/Rho0/t_selected", "-t (after cuts);-t [GeV^{2}];Events", 100, 0, 2);
        h_delta_vz_pim_selected = svc->book_hist1D("/Rho0/delta_vz_pim_selected", "#Delta v_{z} (after cuts);v_{z}^{e} - v_{z}^{#pi^{-}} [cm];Events", 100, -50, 50);
        h_delta_vz_pip_selected = svc->book_hist1D("/Rho0/delta_vz_pip_selected", "#Delta v_{z} (after cuts);v_{z}^{e} - v_{z}^{#pi^{+}} [cm];Events", 100, -50, 50);

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

            // DTF histograms (booked regardless of DTF availability for consistent output)
            h_dtf_mass = svc->book_hist1D("/Rho0/DTF/dtf_mass",
                "DTF Fitted #rho^{0} Mass;M_{DTF} [GeV/c^{2}];Events", 200, 0.4, 1.2);
            h_dtf_chi2 = svc->book_hist1D("/Rho0/DTF/dtf_chi2",
                "DTF #chi^{2};#chi^{2};Events", 100, 0, 50);
            h_dtf_probability = svc->book_hist1D("/Rho0/DTF/dtf_probability",
                "DTF Probability;P(#chi^{2});Events", 100, 0, 1);
            h_dtf_mass_pull = svc->book_hist1D("/Rho0/DTF/dtf_mass_pull",
                "DTF Mass Pull;(M_{DTF} - M_{#rho^{0}}) / #sigma;Events", 100, -5, 5);
            h_dtf_mass_vs_unfitted = svc->book_hist2D("/Rho0/DTF/dtf_mass_vs_unfitted",
                "DTF Mass vs Unfitted Mass;M_{unfitted} [GeV/c^{2}];M_{DTF} [GeV/c^{2}]",
                100, 0.4, 1.2, 100, 0.4, 1.2);
        });
    }

    auto setup_ntuple() -> void {
        clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) {

        // Electron kinematics
        svc->register_branch("rho0_events", "e_p", &m_e_p);
        svc->register_branch("rho0_events", "e_theta", &m_e_theta);
        svc->register_branch("rho0_events", "e_phi", &m_e_phi);
        svc->register_branch("rho0_events", "e_vz", &m_e_vz);
        svc->register_branch("rho0_events", "Q2", &m_Q2);

        // Pi+ kinematics
        svc->register_branch("rho0_events", "pip_p", &m_pip_p);
        svc->register_branch("rho0_events", "pip_theta", &m_pip_theta);
        svc->register_branch("rho0_events", "pip_phi", &m_pip_phi);

        // Pi- kinematics
        svc->register_branch("rho0_events", "pim_p", &m_pim_p);
        svc->register_branch("rho0_events", "pim_theta", &m_pim_theta);
        svc->register_branch("rho0_events", "pim_phi", &m_pim_phi);

        // Rho0 candidate properties
        svc->register_branch("rho0_events", "rho0_masses", &m_rho0_masses);
        svc->register_branch("rho0_events", "rho0_ps", &m_rho0_ps);
        svc->register_branch("rho0_events", "rho0_thetas", &m_rho0_thetas);
        svc->register_branch("rho0_events", "rho0_phis", &m_rho0_phis);

            // Kinematic variables
            svc->register_branch("rho0_events", "rho0_zh", &m_rho0_zh);
            svc->register_branch("rho0_events", "rho0_lc", &m_rho0_lc);
            svc->register_branch("rho0_events", "rho0_t", &m_rho0_t);
            svc->register_branch("rho0_events", "delta_vz_pim", &m_delta_vz_pim);
            svc->register_branch("rho0_events", "delta_vz_pip", &m_delta_vz_pip);

            // DTF variables
            svc->register_branch("rho0_events", "dtf_converged", &m_dtf_converged);
            svc->register_branch("rho0_events", "dtf_mass", &m_dtf_mass);
            svc->register_branch("rho0_events", "dtf_chi2", &m_dtf_chi2);
            svc->register_branch("rho0_events", "dtf_probability", &m_dtf_probability);

            clas12::framework::Log::info(name(), "N-tuple branches registered for rho0_events");
        });
    }

    /**
     * @brief Fit invariant mass histograms with Breit-Wigner + pol3
     */
    auto fit_invariant_mass_histograms() -> void {
        clas12::framework::Log::info(name(), "Fitting invariant mass histograms with BW + pol3...");

        std::map<std::string, FitResult> fit_results;
        double total_rho0 = 0.0;
        double total_rho0_err_sq = 0.0;

        // Fit the main selected mass histogram (convert atomic to ROOT for fitting)
        if (h_pipim_mass_selected && h_pipim_mass_selected->entries() > 100) {
            auto root_hist = h_pipim_mass_selected->to_root("pipim_mass_selected_fit", "Selected Mass");
            auto result = fit_histogram(root_hist.get(), "All Q² (selected)");
            if (result.valid) {
                fit_results["All"] = result;
            }
        }

        // Fit Q²-binned histograms
        for (const auto& [bin_name, hist] : h_pipim_mass_q2_bins) {
            if (hist && hist->entries() > 50) {
                auto root_hist = hist->to_root(bin_name + "_fit", bin_name);
                auto result = fit_histogram(root_hist.get(), bin_name);
                if (result.valid) {
                    fit_results[bin_name] = result;
                    total_rho0 += result.n_rho0;
                    total_rho0_err_sq += result.n_rho0_err * result.n_rho0_err;
                }
            }
        }

        // Save fit results to ROOT file
        save_Q2_fit_results_to_file(fit_results, total_rho0, std::sqrt(total_rho0_err_sq));

        // Fit lc-binned histograms
        for (const auto& [bin_name, hist] : h_pipim_mass_lc_bins) {
            if (hist && hist->entries() > 50) {
                auto root_hist = hist->to_root(bin_name + "_fit", bin_name);
                auto result = fit_histogram(root_hist.get(), bin_name);
                if (result.valid) {
                    fit_results[bin_name] = result;
                    clas12::framework::Log::info(name(), "LC Bin {}: N(rho0) = {:.1f} ± {:.1f}, Mass = {:.4f} ± {:.4f} GeV, Chi2/NDF = {:.2f}",
                        bin_name, result.n_rho0, result.n_rho0_err,
                        result.mass, result.mass_err, result.chi2_ndf);
                    total_rho0 += result.n_rho0;
                    total_rho0_err_sq += result.n_rho0_err * result.n_rho0_err;
                }
                else {
                    clas12::framework::Log::info(name(), "LC Bin {}: Fit failed", bin_name);
                }
            }
        }

        // Save fit results to ROOT file
        save_lc_fit_results_to_file(fit_results, total_rho0, std::sqrt(total_rho0_err_sq));


        // Print summary table using logger
        if (!fit_results.empty()) {
            std::vector<std::string> columns = {"LC Bin", "N(ρ⁰)", "Mass [GeV]", "χ²/NDF"};
            std::vector<std::vector<std::string>> rows;

            for (const auto& [bin_name, result] : fit_results) {
                if (bin_name != "All") {
                    rows.push_back({
                        bin_name,
                        fmt::format("{:.0f} ± {:.0f}", result.n_rho0, result.n_rho0_err),
                        fmt::format("{:.4f} ± {:.4f}", result.mass, result.mass_err),
                        fmt::format("{:.2f}", result.chi2_ndf)
                    });
                }
            }

            // Add total row
            rows.push_back({
                "Total (LC bins)",
                fmt::format("{:.0f} ± {:.0f}", total_rho0, std::sqrt(total_rho0_err_sq)),
                "",
                ""
            });

            // Add integrated fit result if available
            if (fit_results.count("All")) {
                const auto& all_result = fit_results["All"];
                rows.push_back({
                    "All (integrated)",
                    fmt::format("{:.0f} ± {:.0f}", all_result.n_rho0, all_result.n_rho0_err),
                    fmt::format("{:.4f} ± {:.4f}", all_result.mass, all_result.mass_err),
                    fmt::format("{:.2f}", all_result.chi2_ndf)
                });
            }

            clas12::framework::Log::print_yield_summary_table("ρ⁰ Yield Summary", columns, rows);
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

        // Navigate to Rho0 directory
        file->cd();
        if (!file->GetDirectory("Rho0")) {
            file->mkdir("Rho0");
        }
        file->cd("Rho0");

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
        clas12::framework::Log::info(name(), "Fit results saved to TTree 'Rho0/{}' with {} entries", config.tree_name, tree->GetEntries());

        // Save fit functions separately for easy access in Python
        std::string fits_path = fmt::format("Rho0/{}", config.fits_dir);
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
     * @brief Save Q² fit results to ROOT file
     */
    auto save_Q2_fit_results_to_file(const std::map<std::string, FitResult>& results, double total_n_rho0, double total_n_rho0_err) -> void {
        FitResultsConfig config{
            .bin_prefix = "Q2",
            .tree_name = "rho0_fit_results",
            .tree_title = "Rho0 Fit Results",
            .fits_dir = "Fits",
            .all_func_name = "fit_All"
        };
        save_fit_results_to_file(results, total_n_rho0, total_n_rho0_err, Q2_BINS, config);
    }

    /**
     * @brief Save lc fit results to ROOT file
     */
    auto save_lc_fit_results_to_file(const std::map<std::string, FitResult>& results, double total_n_rho0, double total_n_rho0_err) -> void {
        FitResultsConfig config{
            .bin_prefix = "LC",
            .tree_name = "rho0_lc_fit_results",
            .tree_title = "Rho0 lc Fit Results",
            .fits_dir = "Fits_LC",
            .all_func_name = "fit_All_LC"
        };
        save_fit_results_to_file(results, total_n_rho0, total_n_rho0_err, LC_BINS, config);
    }   

    /**
     * @brief Fit a single histogram with BW + pol3
     * @return FitResult containing mass and signal yield
     */
    auto fit_histogram(TH1D* hist, const std::string& label) -> FitResult {
        FitResult result;
        if (!hist || hist->GetEntries() < 50) return result;

        double fit_min = 0.3;
        double fit_max = 1.2;
        double bin_width = hist->GetBinWidth(1);

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
        fit_func->SetParLimits(0, fit_min, fit_max);   // Mass between 0.3-1.2 GeV
        fit_func->SetParLimits(1, 0, hist->GetMaximum() * 10);  // Amplitude positive

        // Set parameter names
        fit_func->SetParName(0, "M_{#rho}");
        fit_func->SetParName(1, "Amplitude");
        fit_func->SetParName(2, "p0");
        fit_func->SetParName(3, "p1");
        fit_func->SetParName(4, "p2");
        fit_func->SetParName(5, "p3");

        // Perform fit
        int fit_status = hist->Fit(fit_func.get(), "QRS");  // Q=quiet, R=use range, S=return status

        if (fit_status != 0) {
            clas12::framework::Log::warning(name(), "Fit failed for {}", label);
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

        // Integrate signal function and divide by bin width to get number of events
        // Integrate over mass +/- 150 MeV
        double sigma = 0.15;
        double signal_integral = signal_func.Integral(result.mass - sigma, result.mass + sigma);
        result.n_rho0 = signal_integral / bin_width;

        // Error propagation (simplified: dominated by amplitude error)
        // N_rho0 is proportional to amplitude, so relative error is same
        if (amplitude > 0) {
            result.n_rho0_err = std::sqrt(result.n_rho0);
        }

        result.valid = true;

        // Print results using logger table
        clas12::framework::Log::print_fit_results_table(
            fmt::format("Fit Results: {}", label),
            {
                {"Mass", fmt::format("{:.4f} ± {:.4f}", result.mass, result.mass_err), "GeV/c²"},
                {"N(ρ⁰)", fmt::format("{:.0f} ± {:.0f}", result.n_rho0, result.n_rho0_err), ""},
                {"χ²/NDF", fmt::format("{:.1f} / {} = {:.2f}", chi2, ndf, result.chi2_ndf), ""}
            }
        );

        // Store fit function in both the histogram and the result
        result.fit_function = std::shared_ptr<TF1>(static_cast<TF1*>(fit_func->Clone()));
        hist->GetListOfFunctions()->Add(fit_func.release());

        return result;
    }
};

}  // namespace clas12::algorithms
