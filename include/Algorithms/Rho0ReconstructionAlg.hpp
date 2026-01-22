#pragma once

#include <Math/Vector4D.h>
#include <TF1.h>
#include <TFile.h>
#include <TMath.h>
#include <TParameter.h>
#include <TTree.h>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>
#include <vector>
#include "CLAS/Constants.hpp"
#include "CLAS/PIDs.hpp"
#include "CLAS/Particle.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Decisions.hpp"
#include "Framework/Event.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Property.hpp"
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/OutputFileService.hpp"
#include "Physics/DISAnalysis.hpp"

namespace Algorithms {

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
class Rho0ReconstructionAlg : public Framework::SimpleAlgorithm<Rho0ReconstructionAlg> {
   public:
    static constexpr double RHO0_MASS = CLAS::Constants::RHO0MASS;  // GeV/c² (~770 MeV)
    static constexpr double RHO0_WIDTH = 0.150;                      // GeV (~150 MeV natural width)
    static constexpr double HBARC = 0.197327;                        // GeV·fm (for coherence length)

    explicit Rho0ReconstructionAlg(
        std::string name = "Rho0ReconstructionAlg",
        double min_pion_momentum = 0.3,    // GeV/c
        double rho0_mass_window = 0.15     // GeV (±150 MeV around 770 MeV due to broad width)
    )
        : SimpleAlgorithm(std::move(name))
        , m_min_pion_momentum{"min_pion_momentum", min_pion_momentum, "Minimum pion momentum [GeV/c]"}
        , m_rho0_mass_window{"rho0_mass_window", rho0_mass_window, "Rho0 mass window [GeV]"}
        , m_cutflow("Rho0 Reconstruction") {
        declare_property(m_min_pion_momentum);
        declare_property(m_rho0_mass_window);
        declare_property(m_pion_chi2pid_max);
        declare_property(m_zh_min);
        declare_property(m_lc_max);
        declare_property(m_t_min);
        declare_property(m_t_max);
        declare_property(m_delta_vz_max);
        declare_property(m_enable_ntuple);
        declare_property(m_enable_fitting);
    }

    auto configure() -> void override {
        m_hist_svc = &require_service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<Framework::NTupleService>("NTupleService");
        m_output_svc = &require_service<Framework::OutputFileService>("OutputFileService");
    }

    auto on_initialize() -> Framework::StatusCode override {
        using Log = Framework::Log;

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Pion selection", "Using pre-selected pions from PionSelectionAlg");
        Log::config(name(), "Rho0 mass window", fmt::format("±{} GeV (nominal: {} GeV)", m_rho0_mass_window.get(), RHO0_MASS));
        Log::config(name(), "Min pion momentum", fmt::format("{} GeV/c", m_min_pion_momentum.get()));
        Log::config(name(), "Max |chi2pid|", m_pion_chi2pid_max.get());
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
        Log::blank();

        book_histograms();

        if (m_ntuple_svc && m_ntuple_svc->is_valid() && m_enable_ntuple.get()) {
            setup_ntuple();
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

        return Framework::StatusCode::SUCCESS();
    }

    auto process(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Track run number from RUN::config bank
        auto& run_config = event.get_bank("RUN::config");
        if (run_config.rows() > 0) {
            int run = run_config.get<int>("run", 0);
            m_runs.insert(run);
        }

        // Check for good electron decision from upstream algorithm
        if (!event.passes_decision_typed(Framework::Decisions::HasGoodDISElectron)) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Has Good DIS Electron");

        // Get DIS kinematics from upstream (using typed retrieval)
        auto Q2_opt = event.retrieve_typed(Framework::Keys::DIS_Q2);
        auto nu_opt = event.retrieve_typed(Framework::Keys::DIS_nu);
        if (!Q2_opt || !nu_opt) return Framework::StatusCode::FILTERED();
        double Q2 = *Q2_opt;
        double nu = *nu_opt;

        // Get beam and electron for t calculation
        auto electrons_opt = event.retrieve_typed(Framework::Keys::SelectedElectronsCapital);
        if (!electrons_opt || electrons_opt->empty()) return Framework::StatusCode::FILTERED();
        const auto& electron = electrons_opt->front();

        // Get pre-selected pions from PionSelectionAlg
        auto selected_piplus_opt = event.retrieve_typed(Framework::Keys::SelectedPiPlus);
        auto selected_piminus_opt = event.retrieve_typed(Framework::Keys::SelectedPiMinus);

        // Use pre-selected pions from PionSelectionAlg
        std::vector<CLAS::Particle> pions_plus;
        std::vector<CLAS::Particle> pions_minus;

        if (selected_piplus_opt) {
            pions_plus = *selected_piplus_opt;
        }
        if (selected_piminus_opt) {
            pions_minus = *selected_piminus_opt;
        }

        if (pions_plus.empty()) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Has >= 1 Pi+");

        if (pions_minus.empty()) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Has >= 1 Pi-");

        // Fill all pi+pi- invariant mass combinations
        for (const auto& pip : pions_plus) {
            for (const auto& pim : pions_minus) {
                double mass = calculate_invariant_mass(pip, pim);
                if (h_pipim_mass_all) h_pipim_mass_all->Fill(mass);
            }
        }

        // Find rho0 candidates
        auto candidates_opt = find_all_rho0_candidates(pions_plus, pions_minus);
        if (!candidates_opt.has_value()) return Framework::StatusCode::FILTERED();

        auto rho0_candidates = candidates_opt.value();
        m_cutflow.pass("Rho0 Found");

        // Calculate beam 4-momentum (default 10.53 GeV along z)
        Physics::BeamConfig beam;
        ROOT::Math::PxPyPzEVector beam_4vec(beam.px, beam.py, beam.pz, beam.energy);
        ROOT::Math::PxPyPzEVector electron_4vec = electron.PxPyPzEVector();
        ROOT::Math::PxPyPzEVector virtual_photon = beam_4vec - electron_4vec;

        // Process candidates with kinematic cuts
        std::vector<std::tuple<CLAS::Particle, CLAS::Particle, double>> passed_candidates;

        for (const auto& [pip, pim, rho0_mass] : rho0_candidates) {
            auto p_rho0 = pip.PxPyPzEVector() + pim.PxPyPzEVector();
            double E_rho0 = p_rho0.E();

            // Calculate kinematic variables
            double zh = E_rho0 / nu;
            double lc = calculate_coherence_length(nu, Q2);
            double t = calculate_t(virtual_photon, p_rho0);
            double minus_t = -t;

            // Calculate delta vz between electron and pions
            double e_vz = electron.vz();
            double delta_vz_pip = std::abs(e_vz - pip.vz());
            double delta_vz_pim = std::abs(e_vz - pim.vz());

            // Fill kinematic histograms before cuts
            if (h_zh) h_zh->Fill(zh);
            if (h_lc) h_lc->Fill(lc);
            if (h_t) h_t->Fill(minus_t);
            if (h_delta_vz_pim) h_delta_vz_pim->Fill(delta_vz_pim);
            if (h_delta_vz_pip) h_delta_vz_pip->Fill(delta_vz_pip);

            // Apply kinematic cuts
            bool passed_delta_vz_pim = delta_vz_pim < m_delta_vz_max.get();
            bool passed_delta_vz_pip = delta_vz_pip < m_delta_vz_max.get();
            bool passed_zh = zh > m_zh_min.get();
            bool passed_lc = lc < m_lc_max.get();
            bool passed_Q2 = (Q2 > 1.0);  // Basic Q² cut to avoid very low Q²
            bool passed_t = (minus_t > m_t_min.get()) && (minus_t < m_t_max.get());

            if (passed_delta_vz_pim && passed_delta_vz_pip && passed_zh && passed_lc && passed_t) {
                passed_candidates.emplace_back(pip, pim, rho0_mass);
            }

            // Fill lc-binned invariant mass histograms
            if (passed_delta_vz_pim && passed_delta_vz_pip && passed_zh && passed_Q2 && passed_t) {
                fill_lc_binned_histogram(lc, rho0_mass);
            }

            // TODO: Should be one by one to update cutflow correctly
            if (delta_vz_pim < m_delta_vz_max.get()) m_cutflow.pass("delta_vz_pim Cut");
            if (delta_vz_pip < m_delta_vz_max.get()) m_cutflow.pass("delta_vz_pip Cut");
            if (zh > m_zh_min.get()) m_cutflow.pass("zh > 0.9");
            if (lc < m_lc_max.get()) m_cutflow.pass("lc < 1 fm");
            if (minus_t > m_t_min.get() && minus_t < m_t_max.get()) m_cutflow.pass("0.1 < -t < 0.5");
        }

        // Update cutflow for kinematic cuts (at least one candidate passed)
        if (passed_candidates.empty()) return Framework::StatusCode::FILTERED();

        // Use local vectors to avoid holding the lock during calculations
        std::vector<double> local_pip_p, local_pip_theta, local_pip_phi;
        std::vector<double> local_pim_p, local_pim_theta, local_pim_phi;
        std::vector<double> local_rho0_masses, local_rho0_ps, local_rho0_thetas, local_rho0_phis;
        std::vector<double> local_zh, local_lc, local_t, local_delta_vz_pim, local_delta_vz_pip;
        double local_e_p{}, local_e_theta{}, local_e_phi{}, local_e_vz{};
        double local_Q2 = Q2;

        for (const auto& [pip, pim, rho0_mass] : passed_candidates) {
            auto p1 = pip.PxPyPzEVector();
            auto p2 = pim.PxPyPzEVector();
            auto p_rho0 = p1 + p2;

            double rho0_p = std::sqrt(p_rho0.Px() * p_rho0.Px() + p_rho0.Py() * p_rho0.Py() + p_rho0.Pz() * p_rho0.Pz());
            double rho0_theta = std::acos(p_rho0.Pz() / rho0_p) * 180.0 / M_PI;
            double rho0_phi = std::atan2(p_rho0.Py(), p_rho0.Px()) * 180.0 / M_PI;

            // Calculate kinematic variables
            double zh = p_rho0.E() / nu;
            double lc = calculate_coherence_length(nu, Q2);
            double t = calculate_t(virtual_photon, p_rho0);
            double minus_t = -t;

            // Calculate opening angle between pions
            auto p1_3 = pip.PxPyPzVector();
            auto p2_3 = pim.PxPyPzVector();
            double cos_angle = p1_3.Dot(p2_3) / (p1_3.R() * p2_3.R());
            double opening_angle = std::acos(cos_angle) * 180.0 / M_PI;

            // Fill histograms
            if (h_pipim_mass_selected) h_pipim_mass_selected->Fill(rho0_mass);
            if (h_rho0_momentum) h_rho0_momentum->Fill(rho0_p);
            if (h_rho0_theta) h_rho0_theta->Fill(rho0_theta);
            if (h_rho0_phi) h_rho0_phi->Fill(rho0_phi);
            if (h_opening_angle) h_opening_angle->Fill(opening_angle);
            if (h_pipim_mass_vs_momentum) h_pipim_mass_vs_momentum->Fill(rho0_p, rho0_mass);

            // Calculate delta vz for selected histogram
            double e_vz = electron.vz();
            double delta_vz_pim_val = std::abs(e_vz - pim.vz());
            double delta_vz_pip_val = std::abs(e_vz - pip.vz());

            // Fill kinematic histograms after cuts
            if (h_zh_selected) h_zh_selected->Fill(zh);
            if (h_lc_selected) h_lc_selected->Fill(lc);
            if (h_t_selected) h_t_selected->Fill(minus_t);
            if (h_delta_vz_pim_selected) h_delta_vz_pim_selected->Fill(delta_vz_pim_val);
            if (h_delta_vz_pip_selected) h_delta_vz_pip_selected->Fill(delta_vz_pip_val);

            // Fill Q²-binned invariant mass histograms
            fill_q2_binned_histogram(Q2, rho0_mass);

            // Fill local vectors for ntuple
            local_pip_p.push_back(pip.p());
            local_pip_theta.push_back(pip.theta());
            local_pip_phi.push_back(pip.phi());
            local_pim_p.push_back(pim.p());
            local_pim_theta.push_back(pim.theta());
            local_pim_phi.push_back(pim.phi());
            local_rho0_masses.push_back(rho0_mass);
            local_rho0_ps.push_back(rho0_p);
            local_rho0_thetas.push_back(rho0_theta);
            local_rho0_phis.push_back(rho0_phi);
            local_zh.push_back(zh);
            local_lc.push_back(lc);
            local_t.push_back(minus_t);
            local_delta_vz_pim.push_back(delta_vz_pim_val);
            local_delta_vz_pip.push_back(delta_vz_pip_val);
        }

        // Store rho0 candidates in event for downstream algorithms
        std::vector<CLAS::Particle> stored_candidates;
        for (const auto& [pip, pim, mass] : passed_candidates) {
            stored_candidates.push_back(pip);
            stored_candidates.push_back(pim);
        }
        event.store_particles("Rho0_PionPairs", stored_candidates);

        // Fill ntuple
        if (m_ntuple_svc && m_ntuple_svc->is_valid() && m_enable_ntuple.get() && !passed_candidates.empty()) {
            local_e_p = electron.p();
            local_e_theta = electron.theta();
            local_e_phi = electron.phi();
            local_e_vz = electron.vz();

            std::lock_guard<std::mutex> lock(m_ntuple_mutex);

            m_e_p = local_e_p;
            m_e_theta = local_e_theta;
            m_e_phi = local_e_phi;
            m_e_vz = local_e_vz;
            m_Q2 = local_Q2;

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

            m_ntuple_svc->get()->fill("rho0_events");

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
        }

        return Framework::StatusCode::SUCCESS();
    }
    

    auto on_finalize() -> Framework::StatusCode override {
        using Log = Framework::Log;

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

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Save run metadata to the output ROOT file
     */
    auto save_run_metadata() -> void {
        if (!m_output_svc || !m_output_svc->is_valid() || m_runs.empty()) return;

        auto* out_svc = m_output_svc->get();
        if (!out_svc->is_file_open()) {
            Framework::Log::error(name(), "Output file is not open for writing metadata");
            return;
        }

        // Acquire lock once and do all operations
        std::lock_guard<std::mutex> lock(out_svc->mutex());
        
        TFile* file = out_svc->get_file();
        if (!file) {
            Framework::Log::error(name(), "Failed to get file pointer");
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

        Framework::Log::info(name(), "Saved {} unique runs to metadata", m_runs.size());

        // Print runs to console
        fmt::print("\n");
        fmt::print("╔═══════════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║                      Run Metadata                                     ║\n");
        fmt::print("╠═══════════════════════════════════════════════════════════════════════╣\n");
        fmt::print("║ Total runs: {:58} ║\n", m_runs.size());
        fmt::print("║ Runs: ");
        int count = 0;
        for (int run : m_runs) {
            if (count > 0) fmt::print(", ");
            if (count > 0 && count % 10 == 0) fmt::print("\n║       ");
            fmt::print("{}", run);
            count++;
        }
        fmt::print("\n╚═══════════════════════════════════════════════════════════════════════╝\n");
    }

    // Public helper methods
    static auto calculate_invariant_mass(const CLAS::Particle& pip, const CLAS::Particle& pim) -> double {
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

    auto find_best_rho0(
        const std::vector<CLAS::Particle>& pions_plus,
        const std::vector<CLAS::Particle>& pions_minus
    ) const -> std::optional<std::tuple<CLAS::Particle, CLAS::Particle, double>> {
        if (pions_plus.empty() || pions_minus.empty()) return std::nullopt;

        std::optional<std::tuple<CLAS::Particle, CLAS::Particle, double>> best;
        double best_mass_diff = std::numeric_limits<double>::max();

        for (const auto& pip : pions_plus) {
            for (const auto& pim : pions_minus) {
                double mass = calculate_invariant_mass(pip, pim);
                double mass_diff = std::abs(mass - RHO0_MASS);
                if (mass_diff < m_rho0_mass_window.get() && mass_diff < best_mass_diff) {
                    best_mass_diff = mass_diff;
                    best = std::make_tuple(pip, pim, mass);
                }
            }
        }

        return best;
    }

    auto find_all_rho0_candidates(
        const std::vector<CLAS::Particle>& pions_plus,
        const std::vector<CLAS::Particle>& pions_minus
    ) const -> std::optional<std::vector<std::tuple<CLAS::Particle, CLAS::Particle, double>>> {
        if (pions_plus.empty() || pions_minus.empty()) return std::nullopt;

        std::vector<std::tuple<CLAS::Particle, CLAS::Particle, double>> candidates;
        for (const auto& pip : pions_plus) {
            for (const auto& pim : pions_minus) {
                double mass = calculate_invariant_mass(pip, pim);
                candidates.emplace_back(pip, pim, mass);
            }
        }

        return candidates.empty() ? std::nullopt : std::make_optional(candidates);
    }

    // std::vector<std::tuple<CLAS::Particle, CLAS::Particle, double>> candidates;
    // std::vector<bool> used_plus(pions_plus.size(), false);
    // std::vector<bool> used_minus(pions_minus.size(), false);
// 
        // while (true) {
        //     double best_mass_diff = std::numeric_limits<double>::max();
        //     size_t best_i = 0, best_j = 0;
        //     bool found_pair = false;
// 
        //     for (size_t i = 0; i < pions_plus.size(); ++i) {
        //         if (used_plus[i]) continue;
// 
        //         for (size_t j = 0; j < pions_minus.size(); ++j) {
        //             if (used_minus[j]) continue;
// 
        //             double mass = calculate_invariant_mass(pions_plus[i], pions_minus[j]);
        //             double mass_diff = std::abs(mass - RHO0_MASS);
// 
        //             if (mass_diff < m_rho0_mass_window.get() && mass_diff < best_mass_diff) {
        //                 best_mass_diff = mass_diff;
        //                 best_i = i;
        //                 best_j = j;
        //                 found_pair = true;
        //             }
        //         }
        //     }
// 
        //     if (!found_pair) break;
// 
        //     double mass = calculate_invariant_mass(pions_plus[best_i], pions_minus[best_j]);
        //     candidates.emplace_back(pions_plus[best_i], pions_minus[best_j], mass);
        //     used_plus[best_i] = true;
        //     used_minus[best_j] = true;
        // }
// 
        // return candidates.empty() ? std::nullopt : std::make_optional(candidates);

    auto min_pion_momentum() const -> double { return m_min_pion_momentum.get(); }
    auto rho0_mass_window() const -> double { return m_rho0_mass_window.get(); }

   private:
    // Properties
    Framework::Property<double> m_min_pion_momentum{"min_pion_momentum", 0.3, "Minimum pion momentum [GeV/c]"};
    Framework::Property<double> m_rho0_mass_window{"rho0_mass_window", 0.15, "Rho0 mass window [GeV]"};
    Framework::Property<double> m_pion_chi2pid_max{"pion_chi2pid_max", 10.0, "Maximum |chi2pid| for pions"};
    Framework::Property<double> m_zh_min{"zh_min", 0.9, "Minimum zh (energy fraction)"};
    Framework::Property<double> m_lc_max{"lc_max", 1.0, "Maximum coherence length [fm]"};
    Framework::Property<double> m_t_min{"t_min", 0.1, "Minimum -t [GeV²]"};
    Framework::Property<double> m_t_max{"t_max", 0.5, "Maximum -t [GeV²]"};
    Framework::Property<double> m_delta_vz_max{"delta_vz_max", 5.0, "Maximum |vz_e - vz_pion| [cm]"};
    Framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    Framework::Property<bool> m_enable_fitting{"enable_fitting", true, "Enable BW+pol3 fitting in finalize"};

    // Service handles
    Framework::ServiceHandle<Framework::HistogramService>* m_hist_svc = nullptr;
    Framework::ServiceHandle<Framework::NTupleService>* m_ntuple_svc = nullptr;
    Framework::ServiceHandle<Framework::OutputFileService>* m_output_svc = nullptr;

    // Cutflow
    Framework::Cutflow m_cutflow;

    // Run tracking for metadata
    std::set<int> m_runs;

    // Mutex for thread safety
    mutable std::mutex m_ntuple_mutex;

    // Histograms - Raw pions
    TH1D* h_piplus_momentum_raw = nullptr;
    TH1D* h_piplus_theta_raw = nullptr;
    TH1D* h_piminus_momentum_raw = nullptr;
    TH1D* h_piminus_theta_raw = nullptr;
    TH1D* h_piplus_chi2pid_raw = nullptr;
    TH1D* h_piminus_chi2pid_raw = nullptr;
    TH1D* h_n_piplus = nullptr;
    TH1D* h_n_piminus = nullptr;

    // Histograms - Selected pions
    TH1D* h_piplus_momentum = nullptr;
    TH1D* h_piplus_theta = nullptr;
    TH1D* h_piminus_momentum = nullptr;
    TH1D* h_piminus_theta = nullptr;
    TH1D* h_piplus_chi2pid = nullptr;
    TH1D* h_piminus_chi2pid = nullptr;

    // Histograms - Rho0 candidates
    TH1D* h_pipim_mass_all = nullptr;
    TH1D* h_pipim_mass_selected = nullptr;
    TH1D* h_rho0_momentum = nullptr;
    TH1D* h_rho0_theta = nullptr;
    TH1D* h_rho0_phi = nullptr;
    TH1D* h_opening_angle = nullptr;
    TH2D* h_pipim_mass_vs_momentum = nullptr;

    // Histograms - Kinematic variables (before cuts)
    TH1D* h_zh = nullptr;
    TH1D* h_lc = nullptr;
    TH1D* h_t = nullptr;
    TH1D* h_delta_vz_pim = nullptr;
    TH1D* h_delta_vz_pip = nullptr;

    // Histograms - Kinematic variables (after cuts)
    TH1D* h_zh_selected = nullptr;
    TH1D* h_lc_selected = nullptr;
    TH1D* h_t_selected = nullptr;
    TH1D* h_delta_vz_pim_selected = nullptr;
    TH1D* h_delta_vz_pip_selected = nullptr;

    // Q²-binned invariant mass histograms
    std::map<std::string, TH1D*> h_pipim_mass_q2_bins;

    // lc-binned invariant mass histograms
    std::map<std::string, TH1D*> h_pipim_mass_lc_bins;

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

    auto fill_q2_binned_histogram(double Q2, double mass) -> void {
        for (size_t i = 0; i < Q2_BINS.size(); ++i) {
            if (Q2 >= Q2_BINS[i].first && Q2 < Q2_BINS[i].second) {
                std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", Q2_BINS[i].first, Q2_BINS[i].second);
                auto it = h_pipim_mass_q2_bins.find(bin_name);
                if (it != h_pipim_mass_q2_bins.end() && it->second) it->second->Fill(mass);  
                break;
            }
        }
    }

    auto fill_lc_binned_histogram(double lc, double mass) -> void {
        for (size_t i = 0; i < LC_BINS.size(); ++i) {
            if (lc >= LC_BINS[i].first && lc < LC_BINS[i].second) {
                std::string bin_name = fmt::format("LC_{:.1f}_{:.1f}", LC_BINS[i].first, LC_BINS[i].second);
                auto it = h_pipim_mass_lc_bins.find(bin_name);
                if (it != h_pipim_mass_lc_bins.end() && it->second) it->second->Fill(mass);
                break;
            }
        }
    }

    auto book_histograms() -> void {
        if (!m_hist_svc || !m_hist_svc->is_valid()) return;
        auto* svc = m_hist_svc->get();

        // Raw pion histograms
        h_piplus_momentum_raw = svc->book_hist1D("/Rho0/piplus_momentum_raw", "Raw #pi^{+} Momentum;p [GeV/c];Events", 100, 0, 10);
        h_piplus_theta_raw = svc->book_hist1D("/Rho0/piplus_theta_raw", "Raw #pi^{+} Theta;#theta [deg];Events", 180, 0, 180);
        h_piminus_momentum_raw = svc->book_hist1D("/Rho0/piminus_momentum_raw", "Raw #pi^{-} Momentum;p [GeV/c];Events", 100, 0, 10);
        h_piminus_theta_raw = svc->book_hist1D("/Rho0/piminus_theta_raw", "Raw #pi^{-} Theta;#theta [deg];Events", 180, 0, 180);
        h_piplus_chi2pid_raw = svc->book_hist1D("/Rho0/piplus_chi2pid_raw", "Raw #pi^{+} chi2pid;chi2pid;Events", 100, -20, 20);
        h_piminus_chi2pid_raw = svc->book_hist1D("/Rho0/piminus_chi2pid_raw", "Raw #pi^{-} chi2pid;chi2pid;Events", 100, -20, 20);
        
        h_n_piplus = svc->book_hist1D("/Rho0/n_piplus", "Number of #pi^{+};N_{#pi^{+}};Events", 10, 0, 10);
        h_n_piminus = svc->book_hist1D("/Rho0/n_piminus", "Number of #pi^{-};N_{#pi^{-}};Events", 10, 0, 10);

        // Selected pion histograms
        h_piplus_momentum = svc->book_hist1D("/Rho0/piplus_momentum", "#pi^{+} Momentum;p [GeV/c];Events", 100, 0, 10);
        h_piplus_theta = svc->book_hist1D("/Rho0/piplus_theta", "#pi^{+} Theta;#theta [deg];Events", 180, 0, 180);
        h_piplus_chi2pid = svc->book_hist1D("/Rho0/piplus_chi2pid", "#pi^{+} chi2pid;chi2pid;Events", 100, -20, 20);

        h_piminus_momentum = svc->book_hist1D("/Rho0/piminus_momentum", "#pi^{-} Momentum;p [GeV/c];Events", 100, 0, 10);
        h_piminus_theta = svc->book_hist1D("/Rho0/piminus_theta", "#pi^{-} Theta;#theta [deg];Events", 180, 0, 180);
        h_piminus_chi2pid = svc->book_hist1D("/Rho0/piminus_chi2pid", "#pi^{-} chi2pid;chi2pid;Events", 100, -20, 20);

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
        h_delta_vz_pim = svc->book_hist1D("/Rho0/delta_vz_pim", "#Delta v_{z} (before cuts);|v_{z}^{e} - v_{z}^{#pi^{-}}| [cm];Events", 100, -50, 50);
        h_delta_vz_pip = svc->book_hist1D("/Rho0/delta_vz_pip", "#Delta v_{z} (before cuts);|v_{z}^{e} - v_{z}^{#pi^{+}}| [cm];Events", 100, -50, 50);

        // Kinematic variable histograms (after cuts)
        h_zh_selected = svc->book_hist1D("/Rho0/zh_selected", "z_{h} (after cuts);z_{h} = E_{#rho^{0}}/#nu;Events", 100, 0, 1.5);
        h_lc_selected = svc->book_hist1D("/Rho0/lc_selected", "Coherence Length (after cuts);l_{c} [fm];Events", 100, 0, 5);
        h_t_selected = svc->book_hist1D("/Rho0/t_selected", "-t (after cuts);-t [GeV^{2}];Events", 100, 0, 2);
        h_delta_vz_pim_selected = svc->book_hist1D("/Rho0/delta_vz_pim_selected", "#Delta v_{z} (after cuts);|v_{z}^{e} - v_{z}^{#pi^{-}}| [cm];Events", 100, -50, 50);
        h_delta_vz_pip_selected = svc->book_hist1D("/Rho0/delta_vz_pip_selected", "#Delta v_{z} (after cuts);|v_{z}^{e} - v_{z}^{#pi^{+}}| [cm];Events", 100, -50, 50);

        // Q²-binned invariant mass histograms
        for (const auto& [q2_min, q2_max] : Q2_BINS) {
            std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", q2_min, q2_max);
            std::string hist_path = fmt::format("/Rho0/Q2_bins/pipim_mass_{}", bin_name);
            std::string title = fmt::format("#pi^{{+}}#pi^{{-}} Mass ({:.1f} < Q^{{2}} < {:.1f} GeV^{{2}});M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", q2_min, q2_max);
            h_pipim_mass_q2_bins[bin_name] = svc->book_hist1D(hist_path, title, 200, 0.0, 1.2);
        }

        // lc-binned invariant mass histograms
        for (const auto& [lc_min, lc_max] : LC_BINS) {
            std::string bin_name = fmt::format("LC_{:.1f}_{:.1f}", lc_min, lc_max);
            std::string hist_path = fmt::format("/Rho0/LC_bins/pipim_mass_{}", bin_name);
            std::string title = fmt::format("#pi^{{+}}#pi^{{-}} Mass ({:.1f} < l_{{c}} < {:.1f} fm);M_{{#pi^{{+}}#pi^{{-}}}} [GeV/c^{{2}}];Events", lc_min, lc_max);
            h_pipim_mass_lc_bins[bin_name] = svc->book_hist1D(hist_path, title, 200, 0.0, 1.2);
        } 
    }

    auto setup_ntuple() -> void {
        if (!m_ntuple_svc || !m_ntuple_svc->is_valid()) return;
        auto* svc = m_ntuple_svc->get();

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

        Framework::Log::info(name(), "N-tuple branches registered for rho0_events");
    }

    /**
     * @brief Fit invariant mass histograms with Breit-Wigner + pol3
     */
    auto fit_invariant_mass_histograms() -> void {
        Framework::Log::info(name(), "Fitting invariant mass histograms with BW + pol3...");

        std::map<std::string, FitResult> fit_results;
        double total_rho0 = 0.0;
        double total_rho0_err_sq = 0.0;

        // Fit the main selected mass histogram
        if (h_pipim_mass_selected && h_pipim_mass_selected->GetEntries() > 100) {
            auto result = fit_histogram(h_pipim_mass_selected, "All Q² (selected)");
            if (result.valid) {
                fit_results["All"] = result;
            }
        }

        // Fit Q²-binned histograms
        for (const auto& [bin_name, hist] : h_pipim_mass_q2_bins) {
            if (hist && hist->GetEntries() > 50) {
                auto result = fit_histogram(hist, bin_name);
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
            if (hist && hist->GetEntries() > 50) {
                auto result = fit_histogram(hist, bin_name);
                if (result.valid) {
                    fit_results[bin_name] = result;
                    Framework::Log::info(name(), "LC Bin {}: N(rho0) = {:.1f} ± {:.1f}, Mass = {:.4f} ± {:.4f} GeV, Chi2/NDF = {:.2f}",
                        bin_name, result.n_rho0, result.n_rho0_err,
                        result.mass, result.mass_err, result.chi2_ndf);
                    total_rho0 += result.n_rho0;
                    total_rho0_err_sq += result.n_rho0_err * result.n_rho0_err;
                }
                else {
                    Framework::Log::info(name(), "LC Bin {}: Fit failed", bin_name);
                }
            }
        }

        // Save fit results to ROOT file
        save_lc_fit_results_to_file(fit_results, total_rho0, std::sqrt(total_rho0_err_sq));


        // Print summary table
        if (!fit_results.empty()) {
            fmt::print("\n");
            fmt::print("╔═══════════════════════════════════════════════════════════════════════╗\n");
            fmt::print("║                      ρ⁰ Yield Summary                                 ║\n");
            fmt::print("╠═══════════════════════════════════════════════════════════════════════╣\n");
            fmt::print("║ {:20} │ {:>12} │ {:>14} │ {:>10} ║\n", "Q² Bin", "N(ρ⁰)", "Mass [GeV]", "χ²/NDF");
            fmt::print("╟──────────────────────┼──────────────┼────────────────┼────────────╢\n");

            for (const auto& [bin_name, result] : fit_results) {
                if (bin_name != "All") {
                    fmt::print("║ {:20} │ {:>6.0f} ± {:<4.0f} │ {:.4f} ± {:.4f} │ {:>10.2f} ║\n",
                              bin_name, result.n_rho0, result.n_rho0_err,
                              result.mass, result.mass_err, result.chi2_ndf);
                }
            }

            fmt::print("╟──────────────────────┼──────────────┼────────────────┼────────────╢\n");
            fmt::print("║ {:20} │ {:>6.0f} ± {:<4.0f} │                │            ║\n",
                      "Total (Q² bins)", total_rho0, std::sqrt(total_rho0_err_sq));

            if (fit_results.count("All")) {
                const auto& all_result = fit_results["All"];
                fmt::print("║ {:20} │ {:>6.0f} ± {:<4.0f} │ {:.4f} ± {:.4f} │ {:>10.2f} ║\n",
                          "All (integrated)", all_result.n_rho0, all_result.n_rho0_err,
                          all_result.mass, all_result.mass_err, all_result.chi2_ndf);
            }
            fmt::print("╚═══════════════════════════════════════════════════════════════════════╝\n");
        }
    }

    /**
     * @brief Save fit results to ROOT file as a TTree
     */
    auto save_Q2_fit_results_to_file(const std::map<std::string, FitResult>& results, double total_n_rho0, double total_n_rho0_err) -> void {
        if (!m_output_svc || !m_output_svc->is_valid()) return;

        auto* out_svc = m_output_svc->get();
        if (!out_svc->is_file_open()) {
            Framework::Log::error(name(), "Output file is not open for writing fit results");
            return;
        }

        // Acquire lock once and do all operations
        std::lock_guard<std::mutex> lock(out_svc->mutex());
        
        TFile* file = out_svc->get_file();
        if (!file) {
            Framework::Log::error(name(), "Failed to get file pointer");
            return;
        }

        // Navigate to Rho0 directory
        file->cd();
        if (!file->GetDirectory("Rho0")) {
            file->mkdir("Rho0");
        }
        file->cd("Rho0");

        // Create a TTree to store fit results (after cd'ing to the correct directory)
        auto* tree = new TTree("rho0_fit_results", "Rho0 Fit Results");

        // Variables for the tree
        char q2_bin[64];
        double q2_min, q2_max;
        double n_rho0, n_rho0_err;
        double mass, mass_err;
        double chi2_ndf;

        tree->Branch("q2_bin", q2_bin, "q2_bin/C");
        tree->Branch("q2_min", &q2_min, "q2_min/D");
        tree->Branch("q2_max", &q2_max, "q2_max/D");
        tree->Branch("n_rho0", &n_rho0, "n_rho0/D");
        tree->Branch("n_rho0_err", &n_rho0_err, "n_rho0_err/D");
        tree->Branch("mass", &mass, "mass/D");
        tree->Branch("mass_err", &mass_err, "mass_err/D");
        tree->Branch("chi2_ndf", &chi2_ndf, "chi2_ndf/D");

        // Fill tree with Q² bin results
        for (size_t i = 0; i < Q2_BINS.size(); ++i) {
            std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", Q2_BINS[i].first, Q2_BINS[i].second);
            auto it = results.find(bin_name);

            strncpy(q2_bin, bin_name.c_str(), sizeof(q2_bin) - 1);
            q2_bin[sizeof(q2_bin) - 1] = '\0';
            q2_min = Q2_BINS[i].first;
            q2_max = Q2_BINS[i].second;

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
        strncpy(q2_bin, "Total", sizeof(q2_bin) - 1);
        q2_min = Q2_BINS.front().first;
        q2_max = Q2_BINS.back().second;
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
        Framework::Log::info(name(), "Fit results saved to TTree 'Rho0/rho0_fit_results' with {} entries", tree->GetEntries());

        // Save fit functions separately for easy access in Python
        file->cd();
        if (!file->GetDirectory("Rho0/Fits")) {
            file->mkdir("Rho0/Fits");
        }
        file->cd("Rho0/Fits");

        // Save fit function for each Q² bin
        for (size_t i = 0; i < Q2_BINS.size(); ++i) {
            std::string bin_name = fmt::format("Q2_{:.1f}_{:.1f}", Q2_BINS[i].first, Q2_BINS[i].second);
            auto it = results.find(bin_name);
            
            if (it != results.end() && it->second.valid && it->second.fit_function) {
                std::string func_name = fmt::format("fit_{}", bin_name);
                it->second.fit_function->SetName(func_name.c_str());
                it->second.fit_function->Write("", TObject::kOverwrite);
                Framework::Log::info(name(), "  Saved fit function '{}'", func_name);
            }
        }

        // Save fit function for integrated (All) result
        if (results.count("All") && results.at("All").valid && results.at("All").fit_function) {
            auto& all_func = results.at("All").fit_function;
            all_func->SetName("fit_All");
            all_func->Write("", TObject::kOverwrite);
            Framework::Log::info(name(), "  Saved fit function 'fit_All'");
        }

        Framework::Log::info(name(), "All fit functions saved to 'Rho0/Fits' directory");
    }

    auto save_lc_fit_results_to_file(const std::map<std::string, FitResult>& results, double total_n_rho0, double total_n_rho0_err) -> void {
        if (!m_output_svc || !m_output_svc->is_valid()) return;

        auto* out_svc = m_output_svc->get();
        if (!out_svc->is_file_open()) {
            Framework::Log::error(name(), "Output file is not open for writing fit results");
            return;
        }

        // Acquire lock once and do all operations
        std::lock_guard<std::mutex> lock(out_svc->mutex());
        
        TFile* file = out_svc->get_file();
        if (!file) {
            Framework::Log::error(name(), "Failed to get file pointer");
            return;
        }

        // Navigate to Rho0 directory
        file->cd();
        if (!file->GetDirectory("Rho0")) {
            file->mkdir("Rho0");
        }
        file->cd("Rho0");

        // Create a TTree to store fit results (after cd'ing to the correct directory)
        auto* tree = new TTree("rho0_lc_fit_results", "Rho0 lc Fit Results");

        // Variables for the tree
        char lc_bin[64];
        double lc_min, lc_max;
        double n_rho0, n_rho0_err;
        double mass, mass_err;
        double chi2_ndf;

        tree->Branch("lc_bin", lc_bin, "lc_bin/C");
        tree->Branch("lc_min", &lc_min, "lc_min/D");
        tree->Branch("lc_max", &lc_max, "lc_max/D");
        tree->Branch("n_rho0", &n_rho0, "n_rho0/D");
        tree->Branch("n_rho0_err", &n_rho0_err, "n_rho0_err/D");
        tree->Branch("mass", &mass, "mass/D");
        tree->Branch("mass_err", &mass_err, "mass_err/D");
        tree->Branch("chi2_ndf", &chi2_ndf, "chi2_ndf/D");

        // Fill tree with lc bin results
        for (size_t i = 0; i < LC_BINS.size(); ++i) {
            std::string bin_name = fmt::format("LC_{:.1f}_{:.1f}", LC_BINS[i].first, LC_BINS[i].second);
            auto it = results.find(bin_name);
            strncpy(lc_bin, bin_name.c_str(), sizeof(lc_bin) - 1);
            lc_bin[sizeof(lc_bin) - 1] = '\0';
            lc_min = LC_BINS[i].first;
            lc_max = LC_BINS[i].second;
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
        strncpy(lc_bin, "Total", sizeof(lc_bin) - 1);
        lc_min = LC_BINS.front().first;
        lc_max = LC_BINS.back().second;
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
        Framework::Log::info(name(), "Fit results saved to TTree 'Rho0/rho0_lc_fit_results' with {} entries", tree->GetEntries());
        // Save fit functions separately for easy access in Python
        file->cd();
        if (!file->GetDirectory("Rho0/Fits_LC")) {
            file->mkdir("Rho0/Fits_LC");
        }
        file->cd("Rho0/Fits_LC");
        // Save fit function for each lc bin
        for (size_t i = 0; i < LC_BINS.size(); ++i) {
            std::string bin_name = fmt::format("LC_{:.1f}_{:.1f}", LC_BINS[i].first, LC_BINS[i].second);
            auto it = results.find(bin_name);
            if (it != results.end() && it->second.valid && it->second.fit_function) {
                std::string func_name = fmt::format("fit_{}", bin_name);
                it->second.fit_function->SetName(func_name.c_str());
                it->second.fit_function->Write("", TObject::kOverwrite);
                Framework::Log::info(name(), "  Saved fit function '{}'", func_name);
            }
        }
        // Save fit function for integrated (All) result
        if (results.count("All") && results.at("All").valid && results.at("All").fit_function) {
            auto& all_func = results.at("All").fit_function;
            all_func->SetName("fit_All_LC");
            all_func->Write("", TObject::kOverwrite);
            Framework::Log::info(name(), "  Saved fit function 'fit_All_LC'");
        }
        Framework::Log::info(name(), "All fit functions saved to 'Rho0/Fits_LC' directory");
    }   

    /**
     * @brief Fit a single histogram with BW + pol3
     * @return FitResult containing mass and signal yield
     */
    auto fit_histogram(TH1D* hist, const std::string& label) -> FitResult {
        FitResult result;
        if (!hist || hist->GetEntries() < 50) return result;

        double fit_min = 0.3;
        double fit_max = 1.0;
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
        fit_func->SetParLimits(0, fit_min, fit_max);   // Mass between 0.3-1.0 GeV
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
            Framework::Log::warning(name(), "Fit failed for {}", label);
            return result;
        }

        // Extract fit parameters
        result.mass = fit_func->GetParameter(0);
        result.mass_err = fit_func->GetParError(0);
        double amplitude = fit_func->GetParameter(1);
        double amplitude_err = fit_func->GetParError(1);
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

        // Print results
        fmt::print("\n");
        fmt::print("┌───────────────────────────────────────────────────────────────┐\n");
        fmt::print("│ Fit Results: {:48} │\n", label);
        fmt::print("├───────────────────────────────────────────────────────────────┤\n");
        fmt::print("│ Mass       = {:.4f} ± {:.4f} GeV/c²                           │\n", result.mass, result.mass_err);
        fmt::print("│ N(ρ⁰)      = {:.0f} ± {:.0f}                                    │\n", result.n_rho0, result.n_rho0_err);
        fmt::print("│ χ²/NDF     = {:.1f} / {} = {:.2f}                               │\n", chi2, ndf, result.chi2_ndf);
        fmt::print("└───────────────────────────────────────────────────────────────┘\n");

        // Store fit function in both the histogram and the result
        result.fit_function = std::shared_ptr<TF1>(static_cast<TF1*>(fit_func->Clone()));
        hist->GetListOfFunctions()->Add(fit_func.release());

        return result;
    }
};

}  // namespace Algorithms
