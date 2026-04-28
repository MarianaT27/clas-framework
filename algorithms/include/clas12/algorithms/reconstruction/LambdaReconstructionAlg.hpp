#pragma once

#include <Math/Vector4D.h>
#include <cmath>
#include <mutex>
#include <optional>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/ParticleUtils.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for LambdaReconstructionAlg.
struct LambdaReconstructionConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.
};

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
    Particle proton;
    Particle pion;
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
class LambdaReconstructionAlg : public framework::Algorithm {
   public:
    using Config = LambdaReconstructionConfig;
    using StatusCode = framework::StatusCode;

    explicit LambdaReconstructionAlg(std::string name = "LambdaReconstructionAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Lambda Reconstruction") {
        required_services(m_hist_svc, m_ntuple_svc);
        declare_properties(m_mass_window, m_min_proton_p, m_min_pion_p, m_max_delta_vz, m_max_chi2pid, m_max_coplanarity, m_beam_energy, m_min_displaced_vz, m_max_displaced_vz, m_enable_proton_fd_cut, m_enable_pion_fd_cut, m_enable_proton_momentum_cut, m_enable_pion_momentum_cut, m_enable_vertex_dz_cut, m_enable_mass_window_cut, m_enable_displaced_vertex_cut, m_enable_coplanarity_cut);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }


    /**
     * @brief Check if particle is in forward detector
     */
    static bool is_forward_detector(const Particle& particle) {
        int abs_status = std::abs(particle.status());
        return 2000 <= abs_status && abs_status < 4000;
    }

    /**
     * @brief Calculate coplanarity angle between Lambda and electron
     * Returns the azimuthal angle difference (should be 0 or 180 for coplanar events)
     * This measures if the Lambda is in the electron scattering plane
     */
    static double calculate_coplanarity(const Particle& electron, double lambda_phi) {
        double phi_diff = std::abs(electron.phi() - lambda_phi) * Constants::RAD_TO_DEG;
        if (phi_diff > 180.0) phi_diff = 360.0 - phi_diff;
        // Return the minimum angle to either 0° (same side) or 180° (opposite side)
        return std::min(phi_diff, 180.0 - phi_diff);
    }

    /**
     * @brief Calculate opening angle between two particles
     */
    static double calculate_opening_angle(const Particle& p1, const Particle& p2) {
        double dot = p1.px() * p2.px() + p1.py() * p2.py() + p1.pz() * p2.pz();
        double mag1 = p1.p();
        double mag2 = p2.p();
        if (mag1 > 0 && mag2 > 0) {
            double cos_angle = dot / (mag1 * mag2);
            cos_angle = std::clamp(cos_angle, -1.0, 1.0);
            return std::acos(cos_angle) * Constants::RAD_TO_DEG;
        }
        return 0.0;
    }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Mass window: {} GeV", m_mass_window.get());
        Log::info(name(), "Chi2pid max: {}", m_max_chi2pid.get());
        Log::info(name(), "Min proton p: {} GeV", m_min_proton_p.get());
        Log::info(name(), "Min pion p: {} GeV", m_min_pion_p.get());
        Log::info(name(), "Max vertex dz: {} cm", m_max_delta_vz.get());
        Log::info(name(), "Max coplanarity: {} deg", m_max_coplanarity.get());
        Log::info(name(), "Beam energy: {} GeV", m_beam_energy.get());
        Log::info(name(), "Displaced vz range: [{}, {}] cm", m_min_displaced_vz.get(), m_max_displaced_vz.get());

        book_histograms();
        m_ntuple.init(m_ntuple_svc.get(), "lambda_analysis");
        Log::info(name(), "N-tuple branches registered for Lambda analysis");

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

        return framework::StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get ALL protons and pions from event BEFORE any electron cuts
        // This is for debugging - to see if we have Lambda signal at all
        const auto& all_protons = event.particles(PIDs::PROTON);
        const auto& all_pions = event.particles(PIDs::PIMINUS);

        // Fill raw mass distribution before ANY cuts (including electron)
        for (const auto& proton : all_protons) {
            for (const auto& pion : all_pions) {
                double mass_raw = calculate_invariant_mass(proton, pion);
                h_ppim_mass_nocuts->fill(mass_raw);
            }
        }

        // Check for good DIS electron from upstream
        auto electron_opt = framework::EventHelpers::get_dis_electron(event);
        if (!electron_opt) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Has Good DIS Electron");

        // Get protons and pions from event
        const auto& protons = event.particles(PIDs::PROTON);
        const auto& pions = event.particles(PIDs::PIMINUS);

        if (protons.empty()) {
            return framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Proton");

        if (pions.empty()) {
            return framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has PiMinus");

        // Fill pre-selection histograms
        h_n_protons->fill(protons.size());
        h_n_pions->fill(pions.size());
        // Get electron for coplanarity calculation (already retrieved above)
        const Particle* electron = &(*electron_opt);

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
                h_delta_vz_precut->fill(delta_vz);
                if (m_enable_vertex_dz_cut.get() && delta_vz > m_max_delta_vz.get()) continue;
                passed_vertex_dz = true;

                // Calculate invariant mass
                double mass = calculate_invariant_mass(proton, pion);

                // Fill all combinations histogram (after FD and momentum cuts)
                h_ppim_mass_all->fill(mass);
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
                if (electron) h_displaced_vz_precut->fill(candidate.displaced_vz);

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
                    double E_miss = E_beam + Constants::PROTONMASS - E_e - E_lambda;
                    double mm2 = E_miss * E_miss - px_miss * px_miss - py_miss * py_miss - pz_miss * pz_miss;
                    candidate.missing_mass = (mm2 > 0) ? std::sqrt(mm2) : -std::sqrt(-mm2);
                }

                // Fill pre-coplanarity histograms
                h_coplanarity_precut->fill(candidate.coplanarity);
                h_opening_angle->fill(candidate.opening_angle);
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

        if (!passed_proton_fd) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Proton in FD");

        if (!passed_pion_fd) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Pion in FD");

        if (!passed_proton_p) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Proton Momentum");

        if (!passed_pion_p) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Pion Momentum");

        if (!passed_vertex_dz) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Vertex dz");

        if (!passed_mass) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Lambda Mass Window");

        if (m_enable_displaced_vertex_cut.get() && m_min_displaced_vz.get() > -999 && !passed_displaced_vz) return framework::StatusCode::FILTERED();
        if (!m_enable_displaced_vertex_cut.get() || m_min_displaced_vz.get() > -999) m_cutflow.pass("Displaced Vertex");

        if (m_enable_coplanarity_cut.get() && m_max_coplanarity.get() > 0 && !passed_coplanarity) return framework::StatusCode::FILTERED();
        if (!m_enable_coplanarity_cut.get() || m_max_coplanarity.get() > 0) m_cutflow.pass("Coplanarity");

        // Use candidates after all cuts
        // Priority: coplanarity > displaced > mass window
        std::vector<LambdaCandidate>* final_candidates_ptr = &candidates;
        if (m_enable_displaced_vertex_cut.get() && m_min_displaced_vz.get() > -999) final_candidates_ptr = &candidates_after_displaced;
        if (m_enable_coplanarity_cut.get() && m_max_coplanarity.get() > 0) final_candidates_ptr = &candidates_after_copla;
        auto& final_candidates = *final_candidates_ptr;
        if (final_candidates.empty()) return framework::StatusCode::FILTERED();

        // Store results
        event.set_decision_typed(framework::Decisions::HasLambda, true);

        // Find best candidate (closest to Lambda mass)
        auto best_it = std::min_element(final_candidates.begin(), final_candidates.end(),
                                        [](const LambdaCandidate& a, const LambdaCandidate& b) {
                                            return std::abs(a.mass - LAMBDA_MASS) < std::abs(b.mass - LAMBDA_MASS);
                                        });

        // Store best Lambda using typed keys
        event.store_typed(framework::Keys::LambdaMass, best_it->mass);
        event.store_typed(framework::Keys::LambdaMomentum, best_it->momentum);
        event.store_typed(framework::Keys::LambdaTheta, best_it->theta);
        event.store_typed(framework::Keys::LambdaCoplanarity, best_it->coplanarity);
        event.store_typed(framework::Keys::LambdaMissingMass, best_it->missing_mass);

        // Fill histograms for all candidates
        for (const auto& cand : final_candidates) {
            h_ppim_mass_selected->fill(cand.mass);
            h_lambda_momentum->fill(cand.momentum);
            h_lambda_theta->fill(cand.theta * Constants::RAD_TO_DEG);
            h_proton_momentum->fill(cand.proton.p());
            h_pion_momentum->fill(cand.pion.p());
            h_coplanarity->fill(cand.coplanarity);
            h_delta_vz->fill(cand.delta_vz);
            h_missing_mass->fill(cand.missing_mass);
            h_displaced_vz->fill(cand.displaced_vz);
            h_decay_length->fill(cand.decay_length);
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

        return framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        if (m_ntuple.is_initialized()) {
            Log::stat(name(), "N-tuple entries", m_ntuple.service()->get_entries(m_ntuple.tree_name()));
        }

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");
        return framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Calculate invariant mass of proton + pion system using ROOT
     */
    static auto calculate_invariant_mass(const Particle& proton, const Particle& pion) -> double {
        // Use ROOT's PxPyPzMVector for proper Lorentz vector arithmetic
        ROOT::Math::PxPyPzMVector p4_proton(proton.px(), proton.py(), proton.pz(), Constants::PROTONMASS);
        ROOT::Math::PxPyPzMVector p4_pion(pion.px(), pion.py(), pion.pz(), Constants::PIMASS);

        // Add the 4-vectors and get invariant mass
        auto p4_total = p4_proton + p4_pion;
        return p4_total.M();
    }

   private:
    // Configuration properties
    framework::Property<double> m_mass_window{"mass_window", 0.5, "Mass window around Lambda mass [GeV]"};
    framework::Property<double> m_min_proton_p{"min_proton_p", 0.3, "Minimum proton momentum [GeV]"};
    framework::Property<double> m_min_pion_p{"min_pion_p", 0.3, "Minimum pion momentum [GeV]"};
    framework::Property<double> m_max_delta_vz{"max_delta_vz", 2.0, "Maximum vertex z difference [cm]"};
    framework::Property<double> m_max_chi2pid{"max_chi2pid", 3.0, "Maximum chi2pid for particle ID quality"};
    framework::Property<double> m_max_coplanarity{"max_coplanarity", 0.0, "Maximum coplanarity [deg], 0=disabled"};
    framework::Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
    framework::Property<double> m_min_displaced_vz{"min_displaced_vz", -1000.0, "Minimum displaced vertex z [cm], -1000=disabled"};
    framework::Property<double> m_max_displaced_vz{"max_displaced_vz", 0.0, "Maximum displaced vertex z [cm], 0=no limit"};

    // Cut toggle properties
    framework::Property<bool> m_enable_proton_fd_cut{"enable_proton_fd_cut", true, "Enable proton forward detector cut"};
    framework::Property<bool> m_enable_pion_fd_cut{"enable_pion_fd_cut", true, "Enable pion forward detector cut"};
    framework::Property<bool> m_enable_proton_momentum_cut{"enable_proton_momentum_cut", true, "Enable proton momentum cut"};
    framework::Property<bool> m_enable_pion_momentum_cut{"enable_pion_momentum_cut", true, "Enable pion momentum cut"};
    framework::Property<bool> m_enable_vertex_dz_cut{"enable_vertex_dz_cut", true, "Enable vertex dz cut"};
    framework::Property<bool> m_enable_mass_window_cut{"enable_mass_window_cut", true, "Enable Lambda mass window cut"};
    framework::Property<bool> m_enable_displaced_vertex_cut{"enable_displaced_vertex_cut", true, "Enable displaced vertex cut"};
    framework::Property<bool> m_enable_coplanarity_cut{"enable_coplanarity_cut", true, "Enable coplanarity cut"};

    Config m_config;
    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist_svc;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;

    mutable std::mutex m_ntuple_mutex;

    // Lock-free atomic histograms
    framework::ThreadLocalHistogram1D* h_n_protons = nullptr;
    framework::ThreadLocalHistogram1D* h_n_pions = nullptr;
    framework::ThreadLocalHistogram1D* h_proton_momentum = nullptr;
    framework::ThreadLocalHistogram1D* h_pion_momentum = nullptr;
    framework::ThreadLocalHistogram1D* h_ppim_mass_nocuts = nullptr;
    framework::ThreadLocalHistogram1D* h_ppim_mass_all = nullptr;
    framework::ThreadLocalHistogram1D* h_ppim_mass_selected = nullptr;
    framework::ThreadLocalHistogram1D* h_lambda_momentum = nullptr;
    framework::ThreadLocalHistogram1D* h_lambda_theta = nullptr;
    framework::ThreadLocalHistogram1D* h_coplanarity = nullptr;
    framework::ThreadLocalHistogram1D* h_coplanarity_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_delta_vz = nullptr;
    framework::ThreadLocalHistogram1D* h_delta_vz_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_opening_angle = nullptr;
    framework::ThreadLocalHistogram1D* h_missing_mass = nullptr;
    framework::ThreadLocalHistogram1D* h_displaced_vz = nullptr;
    framework::ThreadLocalHistogram1D* h_displaced_vz_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_decay_length = nullptr;

    // N-tuple data struct
    struct LambdaNTupleData {
        double e_p = 0, e_theta = 0, e_phi = 0;
        std::vector<double> lambda_mass, lambda_p, lambda_theta;
        std::vector<double> proton_p, pion_p, coplanarity, missing_mass, delta_vz;

        static constexpr auto ntuple_fields() {
            using T = LambdaNTupleData;
            return std::make_tuple(
                framework::field("e_p", &T::e_p),
                framework::field("e_theta", &T::e_theta),
                framework::field("e_phi", &T::e_phi),
                framework::field("lambda_mass", &T::lambda_mass),
                framework::field("lambda_p", &T::lambda_p),
                framework::field("lambda_theta", &T::lambda_theta),
                framework::field("proton_p", &T::proton_p),
                framework::field("pion_p", &T::pion_p),
                framework::field("coplanarity", &T::coplanarity),
                framework::field("missing_mass", &T::missing_mass),
                framework::field("delta_vz", &T::delta_vz)
            );
        }

        auto clear_vectors() -> void {
            lambda_mass.clear(); lambda_p.clear(); lambda_theta.clear();
            proton_p.clear(); pion_p.clear(); coplanarity.clear();
            missing_mass.clear(); delta_vz.clear();
        }
    };
    framework::NTupleWriter<LambdaNTupleData> m_ntuple;

    void book_histograms() {
        h_n_protons = m_hist_svc.get()->book_hist1D("/Lambda/n_protons", "Number of protons;N_{p};Events", 10, 0, 10);
        h_n_pions = m_hist_svc.get()->book_hist1D("/Lambda/n_pions", "Number of #pi^{-};N_{#pi^{-}};Events", 10, 0, 10);
        h_proton_momentum = m_hist_svc.get()->book_hist1D("/Lambda/proton_p", "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_momentum = m_hist_svc.get()->book_hist1D("/Lambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_ppim_mass_nocuts = m_hist_svc.get()->book_hist1D("/Lambda/ppim_mass_nocuts", "p#pi^{-} invariant mass (no cuts);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_ppim_mass_all = m_hist_svc.get()->book_hist1D("/Lambda/ppim_mass_all", "p#pi^{-} invariant mass (FD+p cuts);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_ppim_mass_selected = m_hist_svc.get()->book_hist1D("/Lambda/ppim_mass_selected", "p#pi^{-} invariant mass (selected);M(p#pi^{-}) [GeV];Events", 400, 1.08, 1.8);
        h_lambda_momentum = m_hist_svc.get()->book_hist1D("/Lambda/lambda_p", "#Lambda momentum;p [GeV];Events", 100, 0, 6);
        h_lambda_theta = m_hist_svc.get()->book_hist1D("/Lambda/lambda_theta", "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
        h_coplanarity = m_hist_svc.get()->book_hist1D("/Lambda/coplanarity", "Coplanarity;#Delta#phi [deg];Events", 90, 0, 90);
        h_coplanarity_precut = m_hist_svc.get()->book_hist1D("/Lambda/coplanarity_precut", "Coplanarity (pre-cut);#Delta#phi [deg];Events", 180, 0, 180);
        h_delta_vz = m_hist_svc.get()->book_hist1D("/Lambda/delta_vz", "Vertex #Deltaz;#Deltaz [cm];Events", 100, 0, 5);
        h_delta_vz_precut = m_hist_svc.get()->book_hist1D("/Lambda/delta_vz_precut", "Vertex #Deltaz (pre-cut);#Deltaz [cm];Events", 100, 0, 20);
        h_opening_angle = m_hist_svc.get()->book_hist1D("/Lambda/opening_angle", "Opening angle p-#pi^{-};#theta_{open} [deg];Events", 90, 0, 90);
        h_missing_mass = m_hist_svc.get()->book_hist1D("/Lambda/missing_mass", "Missing mass M(e,e'#Lambda)X;M_{X} [GeV];Events", 200, -1, 3);
        h_displaced_vz = m_hist_svc.get()->book_hist1D("/Lambda/displaced_vz", "Displaced vertex z (decay - primary);#Deltaz [cm];Events", 200, -20, 40);
        h_displaced_vz_precut = m_hist_svc.get()->book_hist1D("/Lambda/displaced_vz_precut", "Displaced vertex z (pre-cut);#Deltaz [cm];Events", 200, -20, 40);
        h_decay_length = m_hist_svc.get()->book_hist1D("/Lambda/decay_length", "Decay length;L [cm];Events", 200, -20, 60);
    }

};

}  // namespace algorithms