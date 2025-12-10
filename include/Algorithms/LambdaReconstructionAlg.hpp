#pragma once

#include <cmath>
#include <mutex>
#include <optional>
#include <tuple>
#include <vector>
#include "CLAS/Constants.hpp"
#include "CLAS/Detectors.hpp"
#include "CLAS/PIDs.hpp"
#include "CLAS/Particle.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Physics/ParticleUtils.hpp"

namespace Algorithms {

using Framework::Log;

/**
 * @brief Lambda baryon mass constant
 */
constexpr double LAMBDA_MASS = 1.115683;  // GeV/c²

/**
 * @brief Reconstructed Lambda candidate
 */
struct LambdaCandidate {
    CLAS::Particle proton;
    CLAS::Particle pion;
    double mass = 0.0;
    double momentum = 0.0;
    double theta = 0.0;
    double phi = 0.0;
    double vertex_z = 0.0;
};

/**
 * @brief Algorithm for Λ → p + π⁻ reconstruction
 *
 * This algorithm reconstructs Λ candidates by:
 * 1. Selecting protons and π⁻ from the event
 * 2. Forming all possible p + π⁻ combinations
 * 3. Calculating invariant mass for each pair
 * 4. Selecting candidates within a mass window around the Λ mass
 */
class LambdaReconstructionAlg : public Framework::Algorithm {
public:
    explicit LambdaReconstructionAlg(
        std::string name = "LambdaReconstructionAlg",
        double mass_window = 0.02  // GeV (±20 MeV around 1.116 GeV)
    ) : Algorithm(std::move(name)), m_mass_window(mass_window), m_cutflow("Lambda Reconstruction") {}

    void set_mass_window(double window) { m_mass_window = window; }
    void set_min_proton_momentum(double p) { m_min_proton_p = p; }
    void set_min_pion_momentum(double p) { m_min_pion_p = p; }
    
    /**
     * @brief Check if particle is in forward detector
     */
    static bool is_forward_detector(const CLAS::Particle& particle) {
        int abs_status = std::abs(particle.status());
        return abs_status >= CLAS::FD && abs_status < CLAS::CD;
    }

    auto initialize() -> Framework::StatusCode override {
        auto log_sc = initialize_handle(m_log_svc);
        if (log_sc.is_failure()) Log::error(name(), "WARNING: LoggingService not available");

        Log::info(name(), "Initializing Lambda reconstruction algorithm");
        Log::info(name(), "  Mass window: {} GeV", m_mass_window);
        Log::info(name(), "  Min proton momentum: {} GeV", m_min_proton_p);
        Log::info(name(), "  Min pion momentum: {} GeV", m_min_pion_p);

        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            Log::error(name(), "Could not get HistogramService: {}", hist_sc.message());
            return hist_sc;
        }

        auto ntuple_sc = initialize_handle(m_ntuple_svc);
        if (ntuple_sc.is_failure()) {
            Log::warning(name(), "Could not get NTupleService - N-tuple output disabled");
        }

        book_histograms();
        if (m_ntuple_svc) setup_ntuple();

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Has Proton");
        m_cutflow.add_cut("Has PiMinus");
        m_cutflow.add_cut("Proton in FD");
        m_cutflow.add_cut("Pion in FD");
        m_cutflow.add_cut("Proton Momentum");
        m_cutflow.add_cut("Pion Momentum");
        m_cutflow.add_cut("Lambda Mass Window");

        m_cutflow.set_logger(m_log_svc.get());
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Check for good electron from upstream
        if (!event.passes_decision("HasGoodElectron")) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Electron");

        // Get protons and pions from event
        const auto& protons = event.particles(CLAS::PIDs::PROTON);
        const auto& pions = event.particles(CLAS::PIDs::PIMINUS);

        if (protons.empty()) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Proton");

        if (pions.empty()) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has PiMinus");

        // Fill pre-selection histograms
        if (h_n_protons) h_n_protons->Fill(protons.size());
        if (h_n_pions) h_n_pions->Fill(pions.size());

        // Find Lambda candidates
        std::vector<LambdaCandidate> candidates;
        bool passed_proton_fd = false;
        bool passed_pion_fd = false;
        bool passed_proton_p = false;
        bool passed_pion_p = false;

        for (const auto& proton : protons) {
            // Require proton in forward detector
            if (!is_forward_detector(proton)) continue;
            passed_proton_fd = true;
            
            if (proton.p() < m_min_proton_p) continue;
            passed_proton_p = true;

            for (const auto& pion : pions) {
                // Require pion in forward detector
                if (!is_forward_detector(pion)) continue;
                passed_pion_fd = true;
                
                if (pion.p() < m_min_pion_p) continue;
                passed_pion_p = true;

                // Calculate invariant mass
                double mass = calculate_invariant_mass(proton, pion);

                // Fill all combinations histogram
                if (h_ppim_mass_all) h_ppim_mass_all->Fill(mass);

                // Check mass window
                if (std::abs(mass - LAMBDA_MASS) < m_mass_window) {
                    LambdaCandidate candidate;
                    candidate.proton = proton;
                    candidate.pion = pion;
                    candidate.mass = mass;
                    
                    // Calculate Lambda 4-momentum
                    double px = proton.px() + pion.px();
                    double py = proton.py() + pion.py();
                    double pz = proton.pz() + pion.pz();
                    candidate.momentum = std::sqrt(px*px + py*py + pz*pz);
                    candidate.theta = std::acos(pz / candidate.momentum);
                    candidate.phi = std::atan2(py, px);
                    candidate.vertex_z = (proton.vz() + pion.vz()) / 2.0;

                    candidates.push_back(candidate);
                }
            }
        }

        if (!passed_proton_fd) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Proton in FD");

        if (!passed_pion_fd) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Pion in FD");

        if (!passed_proton_p) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Proton Momentum");

        if (!passed_pion_p) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Pion Momentum");

        if (candidates.empty()) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Lambda Mass Window");

        // Store results
        event.set_decision("HasLambda", true);
        
        // Find best candidate (closest to Lambda mass)
        auto best_it = std::min_element(candidates.begin(), candidates.end(),
            [](const LambdaCandidate& a, const LambdaCandidate& b) {
                return std::abs(a.mass - LAMBDA_MASS) < std::abs(b.mass - LAMBDA_MASS);
            });

        // Store best Lambda
        event.store("lambda_mass", best_it->mass);
        event.store("lambda_p", best_it->momentum);
        event.store("lambda_theta", best_it->theta);

        // Fill histograms for all candidates
        for (const auto& cand : candidates) {
            if (h_ppim_mass_selected) h_ppim_mass_selected->Fill(cand.mass);
            if (h_lambda_momentum) h_lambda_momentum->Fill(cand.momentum);
            if (h_lambda_theta) h_lambda_theta->Fill(cand.theta * CLAS::Constants::RAD_TO_DEG);
            if (h_proton_momentum) h_proton_momentum->Fill(cand.proton.p());
            if (h_pion_momentum) h_pion_momentum->Fill(cand.pion.p());
        }

        // Fill N-tuple
        if (m_ntuple_svc) {
            std::lock_guard<std::mutex> lock(m_ntuple_mutex);
            
            // Get electron info
            auto* electrons = event.get_particles<CLAS::Particle>("SelectedElectrons");
            if (electrons && !electrons->empty()) {
                const auto& e = electrons->front();
                m_e_p = e.p();
                m_e_theta = e.theta();
                m_e_phi = e.phi();
            }

            // Store all Lambda candidates
            m_lambda_masses.clear();
            m_lambda_ps.clear();
            m_lambda_thetas.clear();
            m_proton_ps.clear();
            m_pion_ps.clear();

            for (const auto& cand : candidates) {
                m_lambda_masses.push_back(cand.mass);
                m_lambda_ps.push_back(cand.momentum);
                m_lambda_thetas.push_back(cand.theta);
                m_proton_ps.push_back(cand.proton.p());
                m_pion_ps.push_back(cand.pion.p());
            }

            m_ntuple_svc->fill("lambda_analysis");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        Log::info(name(), "Finalizing Lambda reconstruction algorithm");
        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("lambda_analysis");
            Log::info(name(), "N-tuple has {} entries", entries);
        }
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Calculate invariant mass of proton + pion system
     */
    static auto calculate_invariant_mass(const CLAS::Particle& proton, const CLAS::Particle& pion) -> double {
        double E_p = std::sqrt(proton.p() * proton.p() + CLAS::Constants::PROTONMASS * CLAS::Constants::PROTONMASS);
        double E_pi = std::sqrt(pion.p() * pion.p() + CLAS::Constants::PIMASS * CLAS::Constants::PIMASS);
        
        double E_tot = E_p + E_pi;
        double px_tot = proton.px() + pion.px();
        double py_tot = proton.py() + pion.py();
        double pz_tot = proton.pz() + pion.pz();
        
        double mass2 = E_tot * E_tot - px_tot * px_tot - py_tot * py_tot - pz_tot * pz_tot;
        return (mass2 > 0) ? std::sqrt(mass2) : 0.0;
    }

private:
    double m_mass_window;
    double m_min_proton_p = 0.0;  // GeV
    double m_min_pion_p = 0.0;    // GeV
    Framework::Cutflow m_cutflow;

    // Services
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};

    mutable std::mutex m_ntuple_mutex;

    // Histograms
    TH1D* h_n_protons = nullptr;
    TH1D* h_n_pions = nullptr;
    TH1D* h_proton_momentum = nullptr;
    TH1D* h_pion_momentum = nullptr;
    TH1D* h_ppim_mass_all = nullptr;
    TH1D* h_ppim_mass_selected = nullptr;
    TH1D* h_lambda_momentum = nullptr;
    TH1D* h_lambda_theta = nullptr;

    // N-tuple variables
    double m_e_p = 0, m_e_theta = 0, m_e_phi = 0;
    std::vector<double> m_lambda_masses;
    std::vector<double> m_lambda_ps;
    std::vector<double> m_lambda_thetas;
    std::vector<double> m_proton_ps;
    std::vector<double> m_pion_ps;

    void book_histograms() {
        h_n_protons = m_hist_svc->book_hist1D("/Lambda/n_protons", "Number of protons;N_{p};Events", 10, 0, 10);
        h_n_pions = m_hist_svc->book_hist1D("/Lambda/n_pions", "Number of #pi^{-};N_{#pi^{-}};Events", 10, 0, 10);
        h_proton_momentum = m_hist_svc->book_hist1D("/Lambda/proton_p", "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_momentum = m_hist_svc->book_hist1D("/Lambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_ppim_mass_all = m_hist_svc->book_hist1D("/Lambda/ppim_mass_all", "p#pi^{-} invariant mass (all);M(p#pi^{-}) [GeV];Events", 200, 1.0, 1.4);
        h_ppim_mass_selected = m_hist_svc->book_hist1D("/Lambda/ppim_mass_selected", "p#pi^{-} invariant mass (selected);M(p#pi^{-}) [GeV];Events", 100, 1.08, 1.16);
        h_lambda_momentum = m_hist_svc->book_hist1D("/Lambda/lambda_p", "#Lambda momentum;p [GeV];Events", 100, 0, 6);
        h_lambda_theta = m_hist_svc->book_hist1D("/Lambda/lambda_theta", "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
    }

    void setup_ntuple() {
        m_ntuple_svc->register_branch("lambda_analysis", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("lambda_analysis", "e_theta", &m_e_theta);
        m_ntuple_svc->register_branch("lambda_analysis", "e_phi", &m_e_phi);
        m_ntuple_svc->register_branch("lambda_analysis", "lambda_mass", &m_lambda_masses);
        m_ntuple_svc->register_branch("lambda_analysis", "lambda_p", &m_lambda_ps);
        m_ntuple_svc->register_branch("lambda_analysis", "lambda_theta", &m_lambda_thetas);
        m_ntuple_svc->register_branch("lambda_analysis", "proton_p", &m_proton_ps);
        m_ntuple_svc->register_branch("lambda_analysis", "pion_p", &m_pion_ps);
        Log::info(name(), "N-tuple branches registered for Lambda analysis");
    }
};

}  // namespace Algorithms
