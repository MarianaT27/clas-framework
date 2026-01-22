#pragma once

#include <Math/Vector4D.h>
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
#include "Framework/Decisions.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/ThreadSafeHistogram.hpp"
#include "Physics/ParticleUtils.hpp"

namespace Algorithms {

using Framework::Log;

/**
 * @brief Lambda baryon mass constant
 */
constexpr double LAMBDA_MASS = 1.115683;  // GeV/c²

/**
 * @brief Lambda decay constants
 * Lambda has cτ = 7.89 cm, so it travels a measurable distance before decaying
 */
constexpr double LAMBDA_CTAU = 7.89;  // cm

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
    double vertex_z = 0.0;       // Decay vertex z (average of proton and pion)
    double delta_vz = 0.0;       // Vertex z difference between proton and pion
    double coplanarity = 0.0;    // Coplanarity angle with electron [deg]
    double opening_angle = 0.0;  // Opening angle between proton and pion [deg]
    double decay_length = 0.0;   // Decay length from primary vertex [cm]
    double missing_mass = 0.0;   // Missing mass M(e,e'Λ)X
    double primary_vz = 0.0;     // Primary vertex z (from electron)
    double displaced_vz = 0.0;   // Displacement: decay_vz - primary_vz [cm]
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

    void set_min_displaced_vz(double dz) { m_min_displaced_vz = dz; }

    void set_max_displaced_vz(double dz) { m_max_displaced_vz = dz; }

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

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Mass window", fmt::format("{} GeV", m_mass_window));
        Log::config(name(), "Chi2pid max", m_max_chi2pid);
        Log::config(name(), "Min proton p", fmt::format("{} GeV", m_min_proton_p));
        Log::config(name(), "Min pion p", fmt::format("{} GeV", m_min_pion_p));
        Log::config(name(), "Max vertex dz", fmt::format("{} cm", m_max_delta_vz));
        Log::config(name(), "Max coplanarity", fmt::format("{} deg", m_max_coplanarity));
        Log::config(name(), "Beam energy", fmt::format("{} GeV", m_beam_energy));
        Log::config(name(), "Displaced vz range", fmt::format("[{}, {}] cm", m_min_displaced_vz, m_max_displaced_vz));

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
        m_cutflow.add_cut("Displaced Vertex");
        m_cutflow.add_cut("Coplanarity");

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get ALL protons and pions from event BEFORE any electron cuts
        // This is for debugging - to see if we have Lambda signal at all
        const auto& all_protons = event.particles(CLAS::PIDs::PROTON);
        const auto& all_pions = event.particles(CLAS::PIDs::PIMINUS);

        // Fill raw mass distribution before ANY cuts (including electron)
        for (const auto& proton : all_protons) {
            for (const auto& pion : all_pions) {
                double mass_raw = calculate_invariant_mass(proton, pion);
                if (h_ppim_mass_nocuts) h_ppim_mass_nocuts->fill(mass_raw);
            }
        }

        // Check for good electron from upstream
        if (!event.passes_decision_typed(Framework::Decisions::HasGoodDISElectron)) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Good DIS Electron");

        // Get protons and pions from event
        const auto& protons = event.particles(CLAS::PIDs::PROTON);
        const auto& pions = event.particles(CLAS::PIDs::PIMINUS);

        if (protons.empty()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Proton");

        if (pions.empty()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has PiMinus");

        // Fill pre-selection histograms
        if (h_n_protons) h_n_protons->fill(protons.size());
        if (h_n_pions) h_n_pions->fill(pions.size());

        // Get electron for coplanarity calculation (from DIS selection)
        auto electrons_opt = event.retrieve_typed(Framework::Keys::SelectedDISElectron);
        const CLAS::Particle* electron = (electrons_opt && !electrons_opt->empty()) ? &electrons_opt->front() : nullptr;

        // Find Lambda candidates
        std::vector<LambdaCandidate> candidates;
        std::vector<LambdaCandidate> candidates_after_displaced;
        std::vector<LambdaCandidate> candidates_after_copla;
        bool passed_proton_fd = false;
        bool passed_pion_fd = false;
        bool passed_proton_p = false;
        bool passed_pion_p = false;
        bool passed_vertex_dz = false;
        bool passed_mass = false;
        bool passed_displaced_vz = false;
        bool passed_coplanarity = false;

        for (const auto& proton : protons) {
            // Require proton in forward detector
            if (!is_forward_detector(proton)) continue;
            passed_proton_fd = true;

            if (proton.p() < m_min_proton_p) continue;
            passed_proton_p = true;

            // Chi2pid cut for proton (good PID quality)
            if (std::abs(proton.chi2pid()) > m_max_chi2pid) continue;

            for (const auto& pion : pions) {
                // Require pion in forward detector
                if (!is_forward_detector(pion)) continue;
                passed_pion_fd = true;

                if (pion.p() < m_min_pion_p) continue;
                passed_pion_p = true;

                // Chi2pid cut for pion (good PID quality)
                if (std::abs(pion.chi2pid()) > m_max_chi2pid) continue;

                // Vertex z difference cut
                double delta_vz = std::abs(proton.vz() - pion.vz());
                if (h_delta_vz_precut) h_delta_vz_precut->fill(delta_vz);
                if (delta_vz > m_max_delta_vz) continue;
                passed_vertex_dz = true;

                // Calculate invariant mass
                double mass = calculate_invariant_mass(proton, pion);

                // Fill all combinations histogram (after FD and momentum cuts)
                if (h_ppim_mass_all) h_ppim_mass_all->fill(mass);

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
                candidate.momentum = std::sqrt(px * px + py * py + pz * pz);
                candidate.theta = std::acos(pz / candidate.momentum);
                candidate.phi = std::atan2(py, px);
                candidate.vertex_z = (proton.vz() + pion.vz()) / 2.0;  // Decay vertex

                // Opening angle between proton and pion
                candidate.opening_angle = calculate_opening_angle(proton, pion);

                // Displaced vertex calculation (Lambda decays downstream of primary vertex)
                // Primary vertex is the electron vertex
                if (electron) {
                    candidate.primary_vz = electron->vz();
                    candidate.displaced_vz = candidate.vertex_z - candidate.primary_vz;

                    // Estimate decay length (approximate, using z displacement and Lambda angle)
                    if (candidate.theta > 0.01) {  // Avoid division by zero
                        candidate.decay_length = candidate.displaced_vz / std::cos(candidate.theta);
                    }
                }

                // Fill displaced vertex histogram before cut
                if (h_displaced_vz_precut && electron) h_displaced_vz_precut->fill(candidate.displaced_vz);

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
                if (h_coplanarity_precut) h_coplanarity_precut->fill(candidate.coplanarity);
                if (h_opening_angle) h_opening_angle->fill(candidate.opening_angle);

                candidates.push_back(candidate);

                // Apply displaced vertex cut (Lambda decays downstream)
                // Require decay vertex to be downstream of primary vertex
                if (electron && m_min_displaced_vz > -999) {
                    if (candidate.displaced_vz < m_min_displaced_vz) continue;
                    if (m_max_displaced_vz > 0 && candidate.displaced_vz > m_max_displaced_vz) continue;
                }
                passed_displaced_vz = true;
                candidates_after_displaced.push_back(candidate);

                // Apply coplanarity cut
                if (m_max_coplanarity > 0 && candidate.coplanarity > m_max_coplanarity) continue;
                passed_coplanarity = true;
                candidates_after_copla.push_back(candidate);
            }
        }

        if (!passed_proton_fd) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Proton in FD");

        if (!passed_pion_fd) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Pion in FD");

        if (!passed_proton_p) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Proton Momentum");

        if (!passed_pion_p) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Pion Momentum");

        if (!passed_vertex_dz) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Vertex dz");

        if (!passed_mass) return Framework::StatusCode::FILTERED();
        m_cutflow.pass("Lambda Mass Window");

        if (m_min_displaced_vz > -999 && !passed_displaced_vz) return Framework::StatusCode::FILTERED();
        if (m_min_displaced_vz > -999) m_cutflow.pass("Displaced Vertex");

        if (m_max_coplanarity > 0 && !passed_coplanarity) return Framework::StatusCode::FILTERED();
        if (m_max_coplanarity > 0) m_cutflow.pass("Coplanarity");

        // Use candidates after all cuts
        // Priority: coplanarity > displaced > mass window
        std::vector<LambdaCandidate>* final_candidates_ptr = &candidates;
        if (m_min_displaced_vz > -999) final_candidates_ptr = &candidates_after_displaced;
        if (m_max_coplanarity > 0) final_candidates_ptr = &candidates_after_copla;
        auto& final_candidates = *final_candidates_ptr;
        if (final_candidates.empty()) return Framework::StatusCode::FILTERED();

        // Store results
        event.set_decision_typed(Framework::Decisions::HasLambda, true);

        // Find best candidate (closest to Lambda mass)
        auto best_it = std::min_element(final_candidates.begin(), final_candidates.end(),
                                        [](const LambdaCandidate& a, const LambdaCandidate& b) {
                                            return std::abs(a.mass - LAMBDA_MASS) < std::abs(b.mass - LAMBDA_MASS);
                                        });

        // Store best Lambda using typed keys
        event.store_typed(Framework::Keys::LambdaMass, best_it->mass);
        event.store_typed(Framework::Keys::LambdaMomentum, best_it->momentum);
        event.store_typed(Framework::Keys::LambdaTheta, best_it->theta);
        event.store_typed(Framework::Keys::LambdaCoplanarity, best_it->coplanarity);
        event.store_typed(Framework::Keys::LambdaMissingMass, best_it->missing_mass);

        // Fill histograms for all candidates
        for (const auto& cand : final_candidates) {
            if (h_ppim_mass_selected) h_ppim_mass_selected->fill(cand.mass);
            if (h_lambda_momentum) h_lambda_momentum->fill(cand.momentum);
            if (h_lambda_theta) h_lambda_theta->fill(cand.theta * CLAS::Constants::RAD_TO_DEG);
            if (h_proton_momentum) h_proton_momentum->fill(cand.proton.p());
            if (h_pion_momentum) h_pion_momentum->fill(cand.pion.p());
            if (h_coplanarity) h_coplanarity->fill(cand.coplanarity);
            if (h_delta_vz) h_delta_vz->fill(cand.delta_vz);
            if (h_missing_mass) h_missing_mass->fill(cand.missing_mass);
            if (h_displaced_vz) h_displaced_vz->fill(cand.displaced_vz);
            if (h_decay_length) h_decay_length->fill(cand.decay_length);
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
        Log::subsection(fmt::format("{} Summary", name()));

        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("lambda_analysis");
            Log::stat(name(), "N-tuple entries", entries);
        }

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Calculate invariant mass of proton + pion system using ROOT
     */
    static auto calculate_invariant_mass(const CLAS::Particle& proton, const CLAS::Particle& pion) -> double {
        // Use ROOT's PxPyPzMVector for proper Lorentz vector arithmetic
        ROOT::Math::PxPyPzMVector p4_proton(proton.px(), proton.py(), proton.pz(), CLAS::Constants::PROTONMASS);
        ROOT::Math::PxPyPzMVector p4_pion(pion.px(), pion.py(), pion.pz(), CLAS::Constants::PIMASS);

        // Add the 4-vectors and get invariant mass
        auto p4_total = p4_proton + p4_pion;
        return p4_total.M();
    }

   private:
    double m_mass_window;
    double m_min_proton_p = 0.3;          // GeV
    double m_min_pion_p = 0.3;            // GeV
    double m_max_delta_vz = 2.0;          // cm - tighter vertex z difference cut
    double m_max_chi2pid = 3.0;           // chi2pid cut for particle ID quality
    double m_max_coplanarity = 0.0;       // deg - 0 means disabled
    double m_beam_energy = 10.6;          // GeV
    double m_min_displaced_vz = -1000.0;  // cm - minimum displaced vertex (decay_vz - primary_vz), -1000 means disabled
    double m_max_displaced_vz = 0.0;      // cm - maximum displaced vertex, 0 means no upper limit
    Framework::Cutflow m_cutflow;

    // Services
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};

    mutable std::mutex m_ntuple_mutex;

    // Thread-safe histograms
    Framework::ThreadSafeHist1D* h_n_protons = nullptr;
    Framework::ThreadSafeHist1D* h_n_pions = nullptr;
    Framework::ThreadSafeHist1D* h_proton_momentum = nullptr;
    Framework::ThreadSafeHist1D* h_pion_momentum = nullptr;
    Framework::ThreadSafeHist1D* h_ppim_mass_nocuts = nullptr;
    Framework::ThreadSafeHist1D* h_ppim_mass_all = nullptr;
    Framework::ThreadSafeHist1D* h_ppim_mass_selected = nullptr;
    Framework::ThreadSafeHist1D* h_lambda_momentum = nullptr;
    Framework::ThreadSafeHist1D* h_lambda_theta = nullptr;
    Framework::ThreadSafeHist1D* h_coplanarity = nullptr;
    Framework::ThreadSafeHist1D* h_coplanarity_precut = nullptr;
    Framework::ThreadSafeHist1D* h_delta_vz = nullptr;
    Framework::ThreadSafeHist1D* h_delta_vz_precut = nullptr;
    Framework::ThreadSafeHist1D* h_opening_angle = nullptr;
    Framework::ThreadSafeHist1D* h_missing_mass = nullptr;
    Framework::ThreadSafeHist1D* h_displaced_vz = nullptr;
    Framework::ThreadSafeHist1D* h_displaced_vz_precut = nullptr;
    Framework::ThreadSafeHist1D* h_decay_length = nullptr;

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
        h_n_protons = m_hist_svc->book_safe_hist1D("/Lambda/n_protons", "Number of protons;N_{p};Events", 10, 0, 10);
        h_n_pions = m_hist_svc->book_safe_hist1D("/Lambda/n_pions", "Number of #pi^{-};N_{#pi^{-}};Events", 10, 0, 10);
        h_proton_momentum = m_hist_svc->book_safe_hist1D("/Lambda/proton_p", "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_momentum = m_hist_svc->book_safe_hist1D("/Lambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_ppim_mass_nocuts = m_hist_svc->book_safe_hist1D("/Lambda/ppim_mass_nocuts", "p#pi^{-} invariant mass (no cuts);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_ppim_mass_all = m_hist_svc->book_safe_hist1D("/Lambda/ppim_mass_all", "p#pi^{-} invariant mass (FD+p cuts);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_ppim_mass_selected = m_hist_svc->book_safe_hist1D("/Lambda/ppim_mass_selected", "p#pi^{-} invariant mass (selected);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_lambda_momentum = m_hist_svc->book_safe_hist1D("/Lambda/lambda_p", "#Lambda momentum;p [GeV];Events", 100, 0, 6);
        h_lambda_theta = m_hist_svc->book_safe_hist1D("/Lambda/lambda_theta", "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
        h_coplanarity = m_hist_svc->book_safe_hist1D("/Lambda/coplanarity", "Coplanarity;#Delta#phi [deg];Events", 90, 0, 90);
        h_coplanarity_precut = m_hist_svc->book_safe_hist1D("/Lambda/coplanarity_precut", "Coplanarity (pre-cut);#Delta#phi [deg];Events", 180, 0, 180);
        h_delta_vz = m_hist_svc->book_safe_hist1D("/Lambda/delta_vz", "Vertex #Deltaz;#Deltaz [cm];Events", 100, 0, 5);
        h_delta_vz_precut = m_hist_svc->book_safe_hist1D("/Lambda/delta_vz_precut", "Vertex #Deltaz (pre-cut);#Deltaz [cm];Events", 100, 0, 20);
        h_opening_angle = m_hist_svc->book_safe_hist1D("/Lambda/opening_angle", "Opening angle p-#pi^{-};#theta_{open} [deg];Events", 90, 0, 90);
        h_missing_mass = m_hist_svc->book_safe_hist1D("/Lambda/missing_mass", "Missing mass M(e,e'#Lambda)X;M_{X} [GeV];Events", 200, -1, 3);
        h_displaced_vz = m_hist_svc->book_safe_hist1D("/Lambda/displaced_vz", "Displaced vertex z (decay - primary);#Deltaz [cm];Events", 200, -20, 40);
        h_displaced_vz_precut = m_hist_svc->book_safe_hist1D("/Lambda/displaced_vz_precut", "Displaced vertex z (pre-cut);#Deltaz [cm];Events", 200, -20, 40);
        h_decay_length = m_hist_svc->book_safe_hist1D("/Lambda/decay_length", "Decay length;L [cm];Events", 200, -20, 60);
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