#pragma once

#include <Math/Vector4D.h>
#include <cmath>
#include <mutex>
#include <optional>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/physics/ParticleUtils.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

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
    clas12::Particle proton;
    clas12::Particle pion;
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
class LambdaReconstructionAlg : public clas12::framework::SimpleAlgorithm {
   public:
    explicit LambdaReconstructionAlg(std::string name = "LambdaReconstructionAlg")
        : SimpleAlgorithm(std::move(name)), m_cutflow("Lambda Reconstruction") {
        declare_property(m_mass_window);
        declare_property(m_min_proton_p);
        declare_property(m_min_pion_p);
        declare_property(m_max_delta_vz);
        declare_property(m_max_chi2pid);
        declare_property(m_max_coplanarity);
        declare_property(m_beam_energy);
        declare_property(m_min_displaced_vz);
        declare_property(m_max_displaced_vz);
        declare_property(m_enable_proton_fd_cut);
        declare_property(m_enable_pion_fd_cut);
        declare_property(m_enable_proton_momentum_cut);
        declare_property(m_enable_pion_momentum_cut);
        declare_property(m_enable_vertex_dz_cut);
        declare_property(m_enable_mass_window_cut);
        declare_property(m_enable_displaced_vertex_cut);
        declare_property(m_enable_coplanarity_cut);
    }


    /**
     * @brief Check if particle is in forward detector
     */
    static bool is_forward_detector(const clas12::Particle& particle) {
        int abs_status = std::abs(particle.status());
        return 2000 <= abs_status && abs_status < 4000;
    }

    /**
     * @brief Calculate coplanarity angle between Lambda and electron
     * Returns the azimuthal angle difference (should be 0 or 180 for coplanar events)
     * This measures if the Lambda is in the electron scattering plane
     */
    static double calculate_coplanarity(const clas12::Particle& electron, double lambda_phi) {
        double phi_diff = std::abs(electron.phi() - lambda_phi) * clas12::Constants::RAD_TO_DEG;
        if (phi_diff > 180.0) phi_diff = 360.0 - phi_diff;
        // Return the minimum angle to either 0° (same side) or 180° (opposite side)
        return std::min(phi_diff, 180.0 - phi_diff);
    }

    /**
     * @brief Calculate opening angle between two particles
     */
    static double calculate_opening_angle(const clas12::Particle& p1, const clas12::Particle& p2) {
        double dot = p1.px() * p2.px() + p1.py() * p2.py() + p1.pz() * p2.pz();
        double mag1 = p1.p();
        double mag2 = p2.p();
        if (mag1 > 0 && mag2 > 0) {
            double cos_angle = dot / (mag1 * mag2);
            cos_angle = std::clamp(cos_angle, -1.0, 1.0);
            return std::acos(cos_angle) * clas12::Constants::RAD_TO_DEG;
        }
        return 0.0;
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_log_svc = svc<clas12::framework::LoggingService>();
        m_hist_svc = svc<clas12::framework::HistogramService>();
        m_ntuple_svc = svc<clas12::framework::NTupleService>();

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Mass window", fmt::format("{} GeV", m_mass_window.get()));
        Log::config(name(), "Chi2pid max", m_max_chi2pid.get());
        Log::config(name(), "Min proton p", fmt::format("{} GeV", m_min_proton_p.get()));
        Log::config(name(), "Min pion p", fmt::format("{} GeV", m_min_pion_p.get()));
        Log::config(name(), "Max vertex dz", fmt::format("{} cm", m_max_delta_vz.get()));
        Log::config(name(), "Max coplanarity", fmt::format("{} deg", m_max_coplanarity.get()));
        Log::config(name(), "Beam energy", fmt::format("{} GeV", m_beam_energy.get()));
        Log::config(name(), "Displaced vz range", fmt::format("[{}, {}] cm", m_min_displaced_vz.get(), m_max_displaced_vz.get()));

        if (!m_hist_svc) {
            Log::error(name(), "Could not get HistogramService");
            return clas12::framework::StatusCode::FAILURE();
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get ALL protons and pions from event BEFORE any electron cuts
        // This is for debugging - to see if we have Lambda signal at all
        const auto& all_protons = event.particles(clas12::PIDs::PROTON);
        const auto& all_pions = event.particles(clas12::PIDs::PIMINUS);

        // Fill raw mass distribution before ANY cuts (including electron)
        for (const auto& proton : all_protons) {
            for (const auto& pion : all_pions) {
                double mass_raw = calculate_invariant_mass(proton, pion);
                if (h_ppim_mass_nocuts) h_ppim_mass_nocuts->fill(mass_raw);
            }
        }

        // Check for good DIS electron from upstream
        auto electron_opt = clas12::framework::EventHelpers::get_dis_electron(event);
        if (!electron_opt) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Has Good DIS Electron");

        // Get protons and pions from event
        const auto& protons = event.particles(clas12::PIDs::PROTON);
        const auto& pions = event.particles(clas12::PIDs::PIMINUS);

        if (protons.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Proton");

        if (pions.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has PiMinus");

        // Fill pre-selection histograms
        if (h_n_protons) h_n_protons->fill(protons.size());
        if (h_n_pions) h_n_pions->fill(pions.size());

        // Get electron for coplanarity calculation (already retrieved above)
        const clas12::Particle* electron = &(*electron_opt);

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
            if (m_enable_proton_fd_cut.get() && !is_forward_detector(proton)) continue;
            passed_proton_fd = true;

            if (m_enable_proton_momentum_cut.get() && proton.p() < m_min_proton_p.get()) continue;
            passed_proton_p = true;

            // Chi2pid cut for proton (good PID quality)
            if (std::abs(proton.chi2pid()) > m_max_chi2pid.get()) continue;

            for (const auto& pion : pions) {
                // Require pion in forward detector
                if (m_enable_pion_fd_cut.get() && !is_forward_detector(pion)) continue;
                passed_pion_fd = true;

                if (m_enable_pion_momentum_cut.get() && pion.p() < m_min_pion_p.get()) continue;
                passed_pion_p = true;

                // Chi2pid cut for pion (good PID quality)
                if (std::abs(pion.chi2pid()) > m_max_chi2pid.get()) continue;

                // Vertex z difference cut
                double delta_vz = std::abs(proton.vz() - pion.vz());
                if (h_delta_vz_precut) h_delta_vz_precut->fill(delta_vz);
                if (m_enable_vertex_dz_cut.get() && delta_vz > m_max_delta_vz.get()) continue;
                passed_vertex_dz = true;

                // Calculate invariant mass
                double mass = calculate_invariant_mass(proton, pion);

                // Fill all combinations histogram (after FD and momentum cuts)
                if (h_ppim_mass_all) h_ppim_mass_all->fill(mass);

                // Check mass window
                if (m_enable_mass_window_cut.get() && std::abs(mass - LAMBDA_MASS) > m_mass_window.get()) continue;
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
                    double E_beam = m_beam_energy.get();
                    double E_e = electron->E();
                    double E_lambda = std::sqrt(candidate.momentum * candidate.momentum + LAMBDA_MASS * LAMBDA_MASS);
                    double px_miss = -electron->px() - px;
                    double py_miss = -electron->py() - py;
                    double pz_miss = E_beam - electron->pz() - pz;
                    double E_miss = E_beam + clas12::Constants::PROTONMASS - E_e - E_lambda;
                    double mm2 = E_miss * E_miss - px_miss * px_miss - py_miss * py_miss - pz_miss * pz_miss;
                    candidate.missing_mass = (mm2 > 0) ? std::sqrt(mm2) : -std::sqrt(-mm2);
                }

                // Fill pre-coplanarity histograms
                if (h_coplanarity_precut) h_coplanarity_precut->fill(candidate.coplanarity);
                if (h_opening_angle) h_opening_angle->fill(candidate.opening_angle);

                candidates.push_back(candidate);

                // Apply displaced vertex cut (Lambda decays downstream)
                // Require decay vertex to be downstream of primary vertex
                if (m_enable_displaced_vertex_cut.get() && electron && m_min_displaced_vz.get() > -999) {
                    if (candidate.displaced_vz < m_min_displaced_vz.get()) continue;
                    if (m_max_displaced_vz.get() > 0 && candidate.displaced_vz > m_max_displaced_vz.get()) continue;
                }
                passed_displaced_vz = true;
                candidates_after_displaced.push_back(candidate);

                // Apply coplanarity cut
                if (m_enable_coplanarity_cut.get() && m_max_coplanarity.get() > 0 && candidate.coplanarity > m_max_coplanarity.get()) continue;
                passed_coplanarity = true;
                candidates_after_copla.push_back(candidate);
            }
        }

        if (!passed_proton_fd) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Proton in FD");

        if (!passed_pion_fd) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Pion in FD");

        if (!passed_proton_p) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Proton Momentum");

        if (!passed_pion_p) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Pion Momentum");

        if (!passed_vertex_dz) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Vertex dz");

        if (!passed_mass) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Lambda Mass Window");

        if (m_enable_displaced_vertex_cut.get() && m_min_displaced_vz.get() > -999 && !passed_displaced_vz) return clas12::framework::StatusCode::FILTERED();
        if (!m_enable_displaced_vertex_cut.get() || m_min_displaced_vz.get() > -999) m_cutflow.pass("Displaced Vertex");

        if (m_enable_coplanarity_cut.get() && m_max_coplanarity.get() > 0 && !passed_coplanarity) return clas12::framework::StatusCode::FILTERED();
        if (!m_enable_coplanarity_cut.get() || m_max_coplanarity.get() > 0) m_cutflow.pass("Coplanarity");

        // Use candidates after all cuts
        // Priority: coplanarity > displaced > mass window
        std::vector<LambdaCandidate>* final_candidates_ptr = &candidates;
        if (m_enable_displaced_vertex_cut.get() && m_min_displaced_vz.get() > -999) final_candidates_ptr = &candidates_after_displaced;
        if (m_enable_coplanarity_cut.get() && m_max_coplanarity.get() > 0) final_candidates_ptr = &candidates_after_copla;
        auto& final_candidates = *final_candidates_ptr;
        if (final_candidates.empty()) return clas12::framework::StatusCode::FILTERED();

        // Store results
        event.set_decision_typed(clas12::framework::Decisions::HasLambda, true);

        // Find best candidate (closest to Lambda mass)
        auto best_it = std::min_element(final_candidates.begin(), final_candidates.end(),
                                        [](const LambdaCandidate& a, const LambdaCandidate& b) {
                                            return std::abs(a.mass - LAMBDA_MASS) < std::abs(b.mass - LAMBDA_MASS);
                                        });

        // Store best Lambda using typed keys
        event.store_typed(clas12::framework::Keys::LambdaMass, best_it->mass);
        event.store_typed(clas12::framework::Keys::LambdaMomentum, best_it->momentum);
        event.store_typed(clas12::framework::Keys::LambdaTheta, best_it->theta);
        event.store_typed(clas12::framework::Keys::LambdaCoplanarity, best_it->coplanarity);
        event.store_typed(clas12::framework::Keys::LambdaMissingMass, best_it->missing_mass);

        // Fill histograms for all candidates
        for (const auto& cand : final_candidates) {
            if (h_ppim_mass_selected) h_ppim_mass_selected->fill(cand.mass);
            if (h_lambda_momentum) h_lambda_momentum->fill(cand.momentum);
            if (h_lambda_theta) h_lambda_theta->fill(cand.theta * clas12::Constants::RAD_TO_DEG);
            if (h_proton_momentum) h_proton_momentum->fill(cand.proton.p());
            if (h_pion_momentum) h_pion_momentum->fill(cand.pion.p());
            if (h_coplanarity) h_coplanarity->fill(cand.coplanarity);
            if (h_delta_vz) h_delta_vz->fill(cand.delta_vz);
            if (h_missing_mass) h_missing_mass->fill(cand.missing_mass);
            if (h_displaced_vz) h_displaced_vz->fill(cand.displaced_vz);
            if (h_decay_length) h_decay_length->fill(cand.decay_length);
        }

        // Fill N-tuple
        if (m_ntuple.is_initialized()) {
            std::lock_guard<std::mutex> lock(m_ntuple_mutex);
            auto& nt = m_ntuple.data();
            nt.clear_vectors();

            if (electron) {
                nt.e_p = electron->p();
                nt.e_theta = electron->theta();
                nt.e_phi = electron->phi();
            }

            for (const auto& cand : final_candidates) {
                nt.lambda_mass.push_back(cand.mass);
                nt.lambda_p.push_back(cand.momentum);
                nt.lambda_theta.push_back(cand.theta);
                nt.proton_p.push_back(cand.proton.p());
                nt.pion_p.push_back(cand.pion.p());
                nt.coplanarity.push_back(cand.coplanarity);
                nt.missing_mass.push_back(cand.missing_mass);
                nt.delta_vz.push_back(cand.delta_vz);
            }

            m_ntuple.fill();
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        if (m_ntuple.is_initialized()) {
            Log::stat(name(), "N-tuple entries", m_ntuple.service()->get_entries(m_ntuple.tree_name()));
        }

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Calculate invariant mass of proton + pion system using ROOT
     */
    static auto calculate_invariant_mass(const clas12::Particle& proton, const clas12::Particle& pion) -> double {
        // Use ROOT's PxPyPzMVector for proper Lorentz vector arithmetic
        ROOT::Math::PxPyPzMVector p4_proton(proton.px(), proton.py(), proton.pz(), clas12::Constants::PROTONMASS);
        ROOT::Math::PxPyPzMVector p4_pion(pion.px(), pion.py(), pion.pz(), clas12::Constants::PIMASS);

        // Add the 4-vectors and get invariant mass
        auto p4_total = p4_proton + p4_pion;
        return p4_total.M();
    }

   private:
    // Configuration properties
    clas12::framework::Property<double> m_mass_window{"mass_window", 0.5, "Mass window around Lambda mass [GeV]"};
    clas12::framework::Property<double> m_min_proton_p{"min_proton_p", 0.3, "Minimum proton momentum [GeV]"};
    clas12::framework::Property<double> m_min_pion_p{"min_pion_p", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_max_delta_vz{"max_delta_vz", 2.0, "Maximum vertex z difference [cm]"};
    clas12::framework::Property<double> m_max_chi2pid{"max_chi2pid", 3.0, "Maximum chi2pid for particle ID quality"};
    clas12::framework::Property<double> m_max_coplanarity{"max_coplanarity", 0.0, "Maximum coplanarity [deg], 0=disabled"};
    clas12::framework::Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
    clas12::framework::Property<double> m_min_displaced_vz{"min_displaced_vz", -1000.0, "Minimum displaced vertex z [cm], -1000=disabled"};
    clas12::framework::Property<double> m_max_displaced_vz{"max_displaced_vz", 0.0, "Maximum displaced vertex z [cm], 0=no limit"};

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_proton_fd_cut{"enable_proton_fd_cut", true, "Enable proton forward detector cut"};
    clas12::framework::Property<bool> m_enable_pion_fd_cut{"enable_pion_fd_cut", true, "Enable pion forward detector cut"};
    clas12::framework::Property<bool> m_enable_proton_momentum_cut{"enable_proton_momentum_cut", true, "Enable proton momentum cut"};
    clas12::framework::Property<bool> m_enable_pion_momentum_cut{"enable_pion_momentum_cut", true, "Enable pion momentum cut"};
    clas12::framework::Property<bool> m_enable_vertex_dz_cut{"enable_vertex_dz_cut", true, "Enable vertex dz cut"};
    clas12::framework::Property<bool> m_enable_mass_window_cut{"enable_mass_window_cut", true, "Enable Lambda mass window cut"};
    clas12::framework::Property<bool> m_enable_displaced_vertex_cut{"enable_displaced_vertex_cut", true, "Enable displaced vertex cut"};
    clas12::framework::Property<bool> m_enable_coplanarity_cut{"enable_coplanarity_cut", true, "Enable coplanarity cut"};

    clas12::framework::Cutflow m_cutflow;

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::LoggingService* m_log_svc = nullptr;
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::NTupleService* m_ntuple_svc = nullptr;

    mutable std::mutex m_ntuple_mutex;

    // Lock-free atomic histograms
    clas12::framework::AtomicHistogram1D* h_n_protons = nullptr;
    clas12::framework::AtomicHistogram1D* h_n_pions = nullptr;
    clas12::framework::AtomicHistogram1D* h_proton_momentum = nullptr;
    clas12::framework::AtomicHistogram1D* h_pion_momentum = nullptr;
    clas12::framework::AtomicHistogram1D* h_ppim_mass_nocuts = nullptr;
    clas12::framework::AtomicHistogram1D* h_ppim_mass_all = nullptr;
    clas12::framework::AtomicHistogram1D* h_ppim_mass_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_lambda_momentum = nullptr;
    clas12::framework::AtomicHistogram1D* h_lambda_theta = nullptr;
    clas12::framework::AtomicHistogram1D* h_coplanarity = nullptr;
    clas12::framework::AtomicHistogram1D* h_coplanarity_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_vz = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_vz_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_opening_angle = nullptr;
    clas12::framework::AtomicHistogram1D* h_missing_mass = nullptr;
    clas12::framework::AtomicHistogram1D* h_displaced_vz = nullptr;
    clas12::framework::AtomicHistogram1D* h_displaced_vz_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_decay_length = nullptr;

    // N-tuple data struct
    struct LambdaNTupleData {
        double e_p = 0, e_theta = 0, e_phi = 0;
        std::vector<double> lambda_mass, lambda_p, lambda_theta;
        std::vector<double> proton_p, pion_p, coplanarity, missing_mass, delta_vz;

        template <typename Svc>
        auto register_branches(Svc* svc, const std::string& tree) -> void {
            svc->register_branch(tree, "e_p", &e_p);
            svc->register_branch(tree, "e_theta", &e_theta);
            svc->register_branch(tree, "e_phi", &e_phi);
            svc->register_branch(tree, "lambda_mass", &lambda_mass);
            svc->register_branch(tree, "lambda_p", &lambda_p);
            svc->register_branch(tree, "lambda_theta", &lambda_theta);
            svc->register_branch(tree, "proton_p", &proton_p);
            svc->register_branch(tree, "pion_p", &pion_p);
            svc->register_branch(tree, "coplanarity", &coplanarity);
            svc->register_branch(tree, "missing_mass", &missing_mass);
            svc->register_branch(tree, "delta_vz", &delta_vz);
        }

        auto clear_vectors() -> void {
            lambda_mass.clear(); lambda_p.clear(); lambda_theta.clear();
            proton_p.clear(); pion_p.clear(); coplanarity.clear();
            missing_mass.clear(); delta_vz.clear();
        }
    };
    clas12::framework::NTupleWriter<LambdaNTupleData> m_ntuple;

    void book_histograms() {
        h_n_protons = m_hist_svc->book_atomic_hist1D("/Lambda/n_protons", "Number of protons;N_{p};Events", 10, 0, 10);
        h_n_pions = m_hist_svc->book_atomic_hist1D("/Lambda/n_pions", "Number of #pi^{-};N_{#pi^{-}};Events", 10, 0, 10);
        h_proton_momentum = m_hist_svc->book_atomic_hist1D("/Lambda/proton_p", "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_momentum = m_hist_svc->book_atomic_hist1D("/Lambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_ppim_mass_nocuts = m_hist_svc->book_atomic_hist1D("/Lambda/ppim_mass_nocuts", "p#pi^{-} invariant mass (no cuts);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_ppim_mass_all = m_hist_svc->book_atomic_hist1D("/Lambda/ppim_mass_all", "p#pi^{-} invariant mass (FD+p cuts);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_ppim_mass_selected = m_hist_svc->book_atomic_hist1D("/Lambda/ppim_mass_selected", "p#pi^{-} invariant mass (selected);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_lambda_momentum = m_hist_svc->book_atomic_hist1D("/Lambda/lambda_p", "#Lambda momentum;p [GeV];Events", 100, 0, 6);
        h_lambda_theta = m_hist_svc->book_atomic_hist1D("/Lambda/lambda_theta", "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
        h_coplanarity = m_hist_svc->book_atomic_hist1D("/Lambda/coplanarity", "Coplanarity;#Delta#phi [deg];Events", 90, 0, 90);
        h_coplanarity_precut = m_hist_svc->book_atomic_hist1D("/Lambda/coplanarity_precut", "Coplanarity (pre-cut);#Delta#phi [deg];Events", 180, 0, 180);
        h_delta_vz = m_hist_svc->book_atomic_hist1D("/Lambda/delta_vz", "Vertex #Deltaz;#Deltaz [cm];Events", 100, 0, 5);
        h_delta_vz_precut = m_hist_svc->book_atomic_hist1D("/Lambda/delta_vz_precut", "Vertex #Deltaz (pre-cut);#Deltaz [cm];Events", 100, 0, 20);
        h_opening_angle = m_hist_svc->book_atomic_hist1D("/Lambda/opening_angle", "Opening angle p-#pi^{-};#theta_{open} [deg];Events", 90, 0, 90);
        h_missing_mass = m_hist_svc->book_atomic_hist1D("/Lambda/missing_mass", "Missing mass M(e,e'#Lambda)X;M_{X} [GeV];Events", 200, -1, 3);
        h_displaced_vz = m_hist_svc->book_atomic_hist1D("/Lambda/displaced_vz", "Displaced vertex z (decay - primary);#Deltaz [cm];Events", 200, -20, 40);
        h_displaced_vz_precut = m_hist_svc->book_atomic_hist1D("/Lambda/displaced_vz_precut", "Displaced vertex z (pre-cut);#Deltaz [cm];Events", 200, -20, 40);
        h_decay_length = m_hist_svc->book_atomic_hist1D("/Lambda/decay_length", "Decay length;L [cm];Events", 200, -20, 60);
    }

    void setup_ntuple() {
        if (m_ntuple_svc) {
            m_ntuple.init(m_ntuple_svc, "lambda_analysis");
            Log::info(name(), "N-tuple branches registered for Lambda analysis");
        }
    }
};

}  // namespace clas12::algorithms