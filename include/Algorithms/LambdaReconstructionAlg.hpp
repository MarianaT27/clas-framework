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
 * @brief Reconstructed Lambda candidate with full kinematics
 */
struct LambdaCandidate {
    CLAS::Particle proton;
    CLAS::Particle pion;
    double mass = 0.0;
    double momentum = 0.0;
    double theta = 0.0;
    double phi = 0.0;
    double vertex_z = 0.0;
    double delta_vz = 0.0;        // Vertex z difference between proton and pion
    double coplanarity = 0.0;     // Coplanarity angle with electron [deg]
    double opening_angle = 0.0;   // Opening angle between proton and pion [deg]
    double decay_length = 0.0;    // Approximate decay length [cm]
    double missing_mass = 0.0;    // Missing mass M(e,e'Λ)X
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
        double mass_window = 0.5  // GeV (±500 MeV around 1.116 GeV)
    ) : Algorithm(std::move(name)), m_mass_window(mass_window), m_cutflow("Lambda Reconstruction") {}

    void set_mass_window(double window) { m_mass_window = window; }
    void set_min_proton_momentum(double p) { m_min_proton_p = p; }
    void set_min_pion_momentum(double p) { m_min_pion_p = p; }
    void set_max_delta_vz(double dz) { m_max_delta_vz = dz; }
    void set_coplanarity_cut(double max_copla) { m_max_coplanarity = max_copla; }
    void set_beam_energy(double E) { m_beam_energy = E; }
    
    /**
     * @brief Check if particle is in forward detector
     */
    static bool is_forward_detector(const CLAS::Particle& particle) {
        int abs_status = std::abs(particle.status());
        return 2000 <= abs_status && abs_status < 4000;
    }
    
    /**
     * @brief Calculate coplanarity angle between Lambda and electron
     * Returns the azimuthal angle difference (should be 0 or 180 for coplanar events)
     * This measures if the Lambda is in the electron scattering plane
     */
    static double calculate_coplanarity(const CLAS::Particle& electron, double lambda_phi) {
        double phi_diff = std::abs(electron.phi() - lambda_phi) * CLAS::Constants::RAD_TO_DEG;
        if (phi_diff > 180.0) phi_diff = 360.0 - phi_diff;
        // Return the minimum angle to either 0° (same side) or 180° (opposite side)
        return std::min(phi_diff, 180.0 - phi_diff);
    }
    
    /**
     * @brief Calculate opening angle between two particles
     */
    static double calculate_opening_angle(const CLAS::Particle& p1, const CLAS::Particle& p2) {
        double dot = p1.px() * p2.px() + p1.py() * p2.py() + p1.pz() * p2.pz();
        double mag1 = p1.p();
        double mag2 = p2.p();
        if (mag1 > 0 && mag2 > 0) {
            double cos_angle = dot / (mag1 * mag2);
            cos_angle = std::clamp(cos_angle, -1.0, 1.0);
            return std::acos(cos_angle) * CLAS::Constants::RAD_TO_DEG;
        }
        return 0.0;
    }

    auto initialize() -> Framework::StatusCode override {
        auto log_sc = initialize_handle(m_log_svc);
        if (log_sc.is_failure()) Log::error(name(), "WARNING: LoggingService not available");

        Log::info(name(), "Initializing Lambda reconstruction algorithm");
        Log::info(name(), "  Mass window: {} GeV", m_mass_window);
        Log::info(name(), "  Min proton momentum: {} GeV", m_min_proton_p);
        Log::info(name(), "  Min pion momentum: {} GeV", m_min_pion_p);
        Log::info(name(), "  Max vertex dz: {} cm", m_max_delta_vz);
        Log::info(name(), "  Max coplanarity: {} deg", m_max_coplanarity);
        Log::info(name(), "  Beam energy: {} GeV", m_beam_energy);

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
        m_cutflow.add_cut("Has Good DIS Electron");
        m_cutflow.add_cut("Has Proton");
        m_cutflow.add_cut("Has PiMinus");
        m_cutflow.add_cut("Proton in FD");
        m_cutflow.add_cut("Pion in FD");
        m_cutflow.add_cut("Proton Momentum");
        m_cutflow.add_cut("Pion Momentum");
        m_cutflow.add_cut("Vertex dz");
        m_cutflow.add_cut("Lambda Mass Window");
        m_cutflow.add_cut("Coplanarity");

        m_cutflow.set_logger(m_log_svc.get());
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Check for good electron from upstream
        if (!event.passes_decision("HasGoodDISElectron")) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Good DIS Electron");

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

        // Get electron for coplanarity calculation (from DIS selection)
        auto* electrons = event.get_particles<CLAS::Particle>("SelectedDISElectron");
        const CLAS::Particle* electron = (electrons && !electrons->empty()) ? &electrons->front() : nullptr;

        // Find Lambda candidates
        std::vector<LambdaCandidate> candidates;
        std::vector<LambdaCandidate> candidates_after_copla;
        bool passed_proton_fd = false;
        bool passed_pion_fd = false;
        bool passed_proton_p = false;
        bool passed_pion_p = false;
        bool passed_vertex_dz = false;
        bool passed_mass = false;
        bool passed_coplanarity = false;

        for (const auto& proton : protons) {
            for (const auto& pion : pions) {
                // Calculate invariant mass for ALL combinations (no cuts) for debugging
                double mass_raw = calculate_invariant_mass(proton, pion);
                if (h_ppim_mass_nocuts) h_ppim_mass_nocuts->Fill(mass_raw);
            }
        }

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

                // Vertex z difference cut
                double delta_vz = std::abs(proton.vz() - pion.vz());
                if (h_delta_vz_precut) h_delta_vz_precut->Fill(delta_vz);
                if (delta_vz > m_max_delta_vz) continue;
                passed_vertex_dz = true;

                // Calculate invariant mass
                double mass = calculate_invariant_mass(proton, pion);

                // Fill all combinations histogram (after FD and momentum cuts)
                if (h_ppim_mass_all) h_ppim_mass_all->Fill(mass);

                // Check mass window
                if (std::abs(mass - LAMBDA_MASS) > m_mass_window) continue;
                passed_mass = true;

                // Build candidate with full kinematics
                LambdaCandidate candidate;
                candidate.proton = proton;
                candidate.pion = pion;
                candidate.mass = mass;
                candidate.delta_vz = delta_vz;
                
                // Calculate Lambda 4-momentum
                double px = proton.px() + pion.px();
                double py = proton.py() + pion.py();
                double pz = proton.pz() + pion.pz();
                candidate.momentum = std::sqrt(px*px + py*py + pz*pz);
                candidate.theta = std::acos(pz / candidate.momentum);
                candidate.phi = std::atan2(py, px);
                candidate.vertex_z = (proton.vz() + pion.vz()) / 2.0;
                
                // Opening angle between proton and pion
                candidate.opening_angle = calculate_opening_angle(proton, pion);
                
                // Coplanarity with electron
                if (electron) {
                    candidate.coplanarity = calculate_coplanarity(*electron, candidate.phi);
                    
                    // Calculate missing mass M(e,e'Λ)X
                    double E_beam = m_beam_energy;
                    double E_e = electron->E();
                    double E_lambda = std::sqrt(candidate.momentum * candidate.momentum + LAMBDA_MASS * LAMBDA_MASS);
                    double px_miss = -electron->px() - px;
                    double py_miss = -electron->py() - py;
                    double pz_miss = E_beam - electron->pz() - pz;
                    double E_miss = E_beam + CLAS::Constants::PROTONMASS - E_e - E_lambda;
                    double mm2 = E_miss * E_miss - px_miss * px_miss - py_miss * py_miss - pz_miss * pz_miss;
                    candidate.missing_mass = (mm2 > 0) ? std::sqrt(mm2) : -std::sqrt(-mm2);
                }

                // Fill pre-coplanarity histograms
                if (h_coplanarity_precut) h_coplanarity_precut->Fill(candidate.coplanarity);
                if (h_opening_angle) h_opening_angle->Fill(candidate.opening_angle);
                
                candidates.push_back(candidate);
                
                // Apply coplanarity cut
                if (m_max_coplanarity > 0 && candidate.coplanarity > m_max_coplanarity) continue;
                passed_coplanarity = true;
                candidates_after_copla.push_back(candidate);
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

        if (!passed_vertex_dz) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Vertex dz");

        if (!passed_mass) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Lambda Mass Window");

        if (m_max_coplanarity > 0 && !passed_coplanarity) return Framework::StatusCode::SUCCESS();
        if (m_max_coplanarity > 0) m_cutflow.pass("Coplanarity");

        // Use candidates after coplanarity cut if enabled, otherwise all mass-window candidates
        auto& final_candidates = (m_max_coplanarity > 0) ? candidates_after_copla : candidates;
        if (final_candidates.empty()) return Framework::StatusCode::SUCCESS();

        // Store results
        event.set_decision("HasLambda", true);
        
        // Find best candidate (closest to Lambda mass)
        auto best_it = std::min_element(final_candidates.begin(), final_candidates.end(),
            [](const LambdaCandidate& a, const LambdaCandidate& b) {
                return std::abs(a.mass - LAMBDA_MASS) < std::abs(b.mass - LAMBDA_MASS);
            });

        // Store best Lambda
        event.store("lambda_mass", best_it->mass);
        event.store("lambda_p", best_it->momentum);
        event.store("lambda_theta", best_it->theta);
        event.store("lambda_coplanarity", best_it->coplanarity);
        event.store("lambda_missing_mass", best_it->missing_mass);

        // Fill histograms for all candidates
        for (const auto& cand : final_candidates) {
            if (h_ppim_mass_selected) h_ppim_mass_selected->Fill(cand.mass);
            if (h_lambda_momentum) h_lambda_momentum->Fill(cand.momentum);
            if (h_lambda_theta) h_lambda_theta->Fill(cand.theta * CLAS::Constants::RAD_TO_DEG);
            if (h_proton_momentum) h_proton_momentum->Fill(cand.proton.p());
            if (h_pion_momentum) h_pion_momentum->Fill(cand.pion.p());
            if (h_coplanarity) h_coplanarity->Fill(cand.coplanarity);
            if (h_delta_vz) h_delta_vz->Fill(cand.delta_vz);
            if (h_missing_mass) h_missing_mass->Fill(cand.missing_mass);
        }

        // Fill N-tuple
        if (m_ntuple_svc) {
            std::lock_guard<std::mutex> lock(m_ntuple_mutex);
            
            // Get electron info
            if (electron) {
                m_e_p = electron->p();
                m_e_theta = electron->theta();
                m_e_phi = electron->phi();
            }

            // Store all Lambda candidates
            m_lambda_masses.clear();
            m_lambda_ps.clear();
            m_lambda_thetas.clear();
            m_proton_ps.clear();
            m_pion_ps.clear();
            m_coplanarities.clear();
            m_missing_masses.clear();
            m_delta_vzs.clear();

            for (const auto& cand : final_candidates) {
                m_lambda_masses.push_back(cand.mass);
                m_lambda_ps.push_back(cand.momentum);
                m_lambda_thetas.push_back(cand.theta);
                m_proton_ps.push_back(cand.proton.p());
                m_pion_ps.push_back(cand.pion.p());
                m_coplanarities.push_back(cand.coplanarity);
                m_missing_masses.push_back(cand.missing_mass);
                m_delta_vzs.push_back(cand.delta_vz);
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
    double m_min_proton_p = 0.3;      // GeV
    double m_min_pion_p = 0.3;        // GeV
    double m_max_delta_vz = 5.0;      // cm - vertex z difference cut
    double m_max_coplanarity = 0.0;   // deg - 0 means disabled
    double m_beam_energy = 10.6;      // GeV
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
    TH1D* h_ppim_mass_nocuts = nullptr;
    TH1D* h_ppim_mass_all = nullptr;
    TH1D* h_ppim_mass_selected = nullptr;
    TH1D* h_lambda_momentum = nullptr;
    TH1D* h_lambda_theta = nullptr;
    TH1D* h_coplanarity = nullptr;
    TH1D* h_coplanarity_precut = nullptr;
    TH1D* h_delta_vz = nullptr;
    TH1D* h_delta_vz_precut = nullptr;
    TH1D* h_opening_angle = nullptr;
    TH1D* h_missing_mass = nullptr;

    // N-tuple variables
    double m_e_p = 0, m_e_theta = 0, m_e_phi = 0;
    std::vector<double> m_lambda_masses;
    std::vector<double> m_lambda_ps;
    std::vector<double> m_lambda_thetas;
    std::vector<double> m_proton_ps;
    std::vector<double> m_pion_ps;
    std::vector<double> m_coplanarities;
    std::vector<double> m_missing_masses;
    std::vector<double> m_delta_vzs;

    void book_histograms() {
        h_n_protons = m_hist_svc->book_hist1D("/Lambda/n_protons", "Number of protons;N_{p};Events", 10, 0, 10);
        h_n_pions = m_hist_svc->book_hist1D("/Lambda/n_pions", "Number of #pi^{-};N_{#pi^{-}};Events", 10, 0, 10);
        h_proton_momentum = m_hist_svc->book_hist1D("/Lambda/proton_p", "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_momentum = m_hist_svc->book_hist1D("/Lambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_ppim_mass_nocuts = m_hist_svc->book_hist1D("/Lambda/ppim_mass_nocuts", "p#pi^{-} invariant mass (no cuts);M(p#pi^{-}) [GeV];Events", 300, 1.0, 2.5);
        h_ppim_mass_all = m_hist_svc->book_hist1D("/Lambda/ppim_mass_all", "p#pi^{-} invariant mass (FD+p cuts);M(p#pi^{-}) [GeV];Events", 300, 1.0, 2.5);
        h_ppim_mass_selected = m_hist_svc->book_hist1D("/Lambda/ppim_mass_selected", "p#pi^{-} invariant mass (selected);M(p#pi^{-}) [GeV];Events", 200, 1.05, 1.25);
        h_lambda_momentum = m_hist_svc->book_hist1D("/Lambda/lambda_p", "#Lambda momentum;p [GeV];Events", 100, 0, 6);
        h_lambda_theta = m_hist_svc->book_hist1D("/Lambda/lambda_theta", "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
        h_coplanarity = m_hist_svc->book_hist1D("/Lambda/coplanarity", "Coplanarity;#Delta#phi [deg];Events", 90, 0, 90);
        h_coplanarity_precut = m_hist_svc->book_hist1D("/Lambda/coplanarity_precut", "Coplanarity (pre-cut);#Delta#phi [deg];Events", 180, 0, 180);
        h_delta_vz = m_hist_svc->book_hist1D("/Lambda/delta_vz", "Vertex #Deltaz;#Deltaz [cm];Events", 100, 0, 5);
        h_delta_vz_precut = m_hist_svc->book_hist1D("/Lambda/delta_vz_precut", "Vertex #Deltaz (pre-cut);#Deltaz [cm];Events", 100, 0, 20);
        h_opening_angle = m_hist_svc->book_hist1D("/Lambda/opening_angle", "Opening angle p-#pi^{-};#theta_{open} [deg];Events", 90, 0, 90);
        h_missing_mass = m_hist_svc->book_hist1D("/Lambda/missing_mass", "Missing mass M(e,e'#Lambda)X;M_{X} [GeV];Events", 200, -1, 3);
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
        m_ntuple_svc->register_branch("lambda_analysis", "coplanarity", &m_coplanarities);
        m_ntuple_svc->register_branch("lambda_analysis", "missing_mass", &m_missing_masses);
        m_ntuple_svc->register_branch("lambda_analysis", "delta_vz", &m_delta_vzs);
        Log::info(name(), "N-tuple branches registered for Lambda analysis");
    }
};

}  // namespace Algorithms