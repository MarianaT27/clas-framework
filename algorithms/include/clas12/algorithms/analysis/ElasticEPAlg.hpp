#pragma once

#include <cmath>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/ParticleUtils.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

/**
 * @brief Nuclear target types for elastic scattering
 */
enum class NuclearTarget {
    Proton,
    Neutron,
    Deuteron
};

/**
 * @brief Get mass of nuclear target
 */
inline double get_target_mass(NuclearTarget target) {
    switch (target) {
        case NuclearTarget::Proton:
            return clas12::Constants::PROTONMASS;
        case NuclearTarget::Neutron:
            return clas12::Constants::NEUTRONMASS;
        case NuclearTarget::Deuteron:
            return clas12::Constants::DEUTERIUMMASS;
        default:
            return clas12::Constants::PROTONMASS;
    }
}

/**
 * @brief Get PDG ID for recoil particle
 */
inline clas12::PIDs get_recoil_pid(NuclearTarget target) {
    switch (target) {
        case NuclearTarget::Proton:
            return clas12::PIDs::PROTON;
        case NuclearTarget::Neutron:
            return clas12::PIDs::NEUTRON;
        case NuclearTarget::Deuteron:
            return clas12::PIDs::DEUTERON;
        default:
            return clas12::PIDs::PROTON;
    }
}

/**
 * @brief Get target name string
 */
inline std::string get_target_name(NuclearTarget target) {
    switch (target) {
        case NuclearTarget::Proton:
            return "proton";
        case NuclearTarget::Neutron:
            return "neutron";
        case NuclearTarget::Deuteron:
            return "deuteron";
        default:
            return "proton";
    }
}

/**
 * @brief Elastic electron-nucleon scattering kinematics
 */
struct ElasticKinematics {
    double Q2 = 0.0;                     // Four-momentum transfer squared [GeV^2]
    double W = 0.0;                      // Invariant mass [GeV]
    double theta_e = 0.0;                // Electron scattering angle [rad]
    double theta_recoil = 0.0;           // Recoil nucleon angle [rad]
    double phi_e = 0.0;                  // Electron azimuthal angle [rad]
    double phi_recoil = 0.0;             // Recoil nucleon azimuthal angle [rad]
    double delta_phi = 0.0;              // Coplanarity: |phi_e - phi_recoil| - 180 [deg]
    double p_e = 0.0;                    // Electron momentum [GeV]
    double p_recoil = 0.0;               // Recoil nucleon momentum [GeV]
    double p_e_expected = 0.0;           // Expected electron momentum from elastic [GeV]
    double p_recoil_expected = 0.0;      // Expected recoil momentum from elastic [GeV]
    double theta_recoil_expected = 0.0;  // Expected recoil angle from elastic [rad]
    double delta_p_e = 0.0;              // (p_e - p_e_expected) / p_e_expected
    double delta_p_recoil = 0.0;         // (p_recoil - p_recoil_expected) / p_recoil_expected
    double delta_theta_recoil = 0.0;     // theta_recoil - theta_recoil_expected [deg]
};

/**
 * @brief Algorithm for elastic electron-nucleon scattering selection
 *
 * Selects elastic eN -> eN events using:
 * - Coplanarity cut (electron and recoil back-to-back in phi)
 * - W cut (invariant mass near target mass)
 * - Momentum correlation cuts
 * - Angular correlation cuts
 *
 * Supports proton, neutron, and deuteron targets.
 */
class ElasticEPAlg : public clas12::framework::SimpleAlgorithm<ElasticEPAlg> {
   public:
    explicit ElasticEPAlg(std::string name = "ElasticEPAlg", NuclearTarget target = NuclearTarget::Proton)
        : SimpleAlgorithm(std::move(name)), m_target(target), m_target_mass(get_target_mass(target)), m_recoil_pid(get_recoil_pid(target)), m_cutflow("Elastic e" + get_target_name(target) + " Selection") {
        declare_property(m_beam_energy);
        declare_property(m_W_min);
        declare_property(m_W_max);
        declare_property(m_delta_phi_max);
        declare_property(m_delta_p_max);
        declare_property(m_delta_theta_max);
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_log_svc = &require_service<clas12::framework::LoggingService>("LoggingService");
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_ntuple_svc = &optional_service<clas12::framework::NTupleService>("NTupleService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Initializing elastic e-{} algorithm", get_target_name(m_target));
        Log::info(name(), "  Beam energy: {} GeV", m_beam_energy.get());
        Log::info(name(), "  Target mass: {} GeV", m_target_mass);
        Log::info(name(), "  Recoil PID: {} ({})", static_cast<int>(m_recoil_pid), clas12::pid_name(m_recoil_pid));

        book_histograms();
        if (m_ntuple_svc) setup_ntuple();

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Has Recoil");
        m_cutflow.add_cut("Coplanarity");
        m_cutflow.add_cut("W Cut");
        m_cutflow.add_cut("Momentum Correlation");
        m_cutflow.add_cut("Angle Correlation");

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Check for good electron from upstream
        if (!event.passes_decision("HasGoodElectron")) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Get selected electron
        auto* electrons = event.get_particles<clas12::Particle>("SelectedElectrons");
        if (!electrons || electrons->empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        const auto& electron = electrons->front();

        // Get recoil particles from event
        const auto& recoils = event.particles(m_recoil_pid);
        if (recoils.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Recoil");

        // Find best recoil candidate (highest momentum)
        const clas12::Particle* best_recoil = nullptr;
        ElasticKinematics best_kin;
        bool found_elastic = false;

        for (const auto& recoil : recoils) {
            ElasticKinematics kin = calculate_kinematics(electron, recoil);

            // Fill pre-cut histograms
            if (h_delta_phi_precut) h_delta_phi_precut->fill(kin.delta_phi);
            if (h_W_precut) h_W_precut->fill(kin.W);

            // Check coplanarity
            if (std::abs(kin.delta_phi) > m_delta_phi_max.get()) continue;

            // Check W cut
            if (kin.W < m_W_min.get() || kin.W > m_W_max.get()) continue;

            // Check momentum correlation
            if (std::abs(kin.delta_p_e) > m_delta_p_max.get()) continue;
            if (std::abs(kin.delta_p_recoil) > m_delta_p_max.get()) continue;

            // Check angle correlation
            if (std::abs(kin.delta_theta_recoil) > m_delta_theta_max.get()) continue;

            // Found a good elastic candidate
            if (!found_elastic || recoil.p() > best_recoil->p()) {
                best_recoil = &recoil;
                best_kin = kin;
                found_elastic = true;
            }
        }

        if (!found_elastic) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Pass cutflow for the best candidate
        m_cutflow.pass("Coplanarity");
        m_cutflow.pass("W Cut");
        m_cutflow.pass("Momentum Correlation");
        m_cutflow.pass("Angle Correlation");

        // Store results
        event.set_decision("HasElasticEN", true);
        event.store_particles<clas12::Particle>("ElasticElectron", {electron});
        event.store_particles<clas12::Particle>("ElasticRecoil", {*best_recoil});
        event.store("elastic_Q2", best_kin.Q2);
        event.store("elastic_W", best_kin.W);

        // Fill histograms
        fill_histograms(best_kin);

        // Fill N-tuple
        if (m_ntuple_svc) {
            fill_ntuple(electron, *best_recoil, best_kin);
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing elastic e-{} algorithm", get_target_name(m_target));
        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get()->get_entries("elastic_eN");
            Log::info(name(), "N-tuple has {} entries", entries);
        }
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    NuclearTarget m_target;
    double m_target_mass;
    clas12::PIDs m_recoil_pid;
    clas12::framework::Cutflow m_cutflow;

    // Configuration properties
    clas12::framework::Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
    clas12::framework::Property<double> m_W_min{"W_min", 0.8, "Minimum W [GeV]"};
    clas12::framework::Property<double> m_W_max{"W_max", 1.05, "Maximum W [GeV]"};
    clas12::framework::Property<double> m_delta_phi_max{"delta_phi_max", 5.0, "Maximum coplanarity [deg]"};
    clas12::framework::Property<double> m_delta_p_max{"delta_p_max", 0.1, "Maximum fractional momentum deviation"};
    clas12::framework::Property<double> m_delta_theta_max{"delta_theta_max", 2.0, "Maximum angle deviation [deg]"};

    // Services (pointers set by configure())
    clas12::framework::ServiceHandle<clas12::framework::LoggingService>* m_log_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;

    // Histograms (atomic for thread safety)
    clas12::framework::AtomicHistogram1D* h_Q2 = nullptr;
    clas12::framework::AtomicHistogram1D* h_W = nullptr;
    clas12::framework::AtomicHistogram1D* h_W_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_phi_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_p_e = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_p_recoil = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_theta_recoil = nullptr;
    clas12::framework::AtomicHistogram2D* h_p_e_vs_theta_e = nullptr;
    clas12::framework::AtomicHistogram2D* h_p_recoil_vs_theta_recoil = nullptr;
    clas12::framework::AtomicHistogram2D* h_p_e_measured_vs_expected = nullptr;
    clas12::framework::AtomicHistogram2D* h_theta_recoil_measured_vs_expected = nullptr;

    // N-tuple variables
    double m_nt_e_p = 0, m_nt_e_theta = 0, m_nt_e_phi = 0;
    double m_nt_recoil_p = 0, m_nt_recoil_theta = 0, m_nt_recoil_phi = 0;
    double m_nt_Q2 = 0, m_nt_W = 0;
    double m_nt_delta_phi = 0, m_nt_delta_p_e = 0, m_nt_delta_p_recoil = 0, m_nt_delta_theta_recoil = 0;

    auto calculate_kinematics(const clas12::Particle& electron, const clas12::Particle& recoil) -> ElasticKinematics {
        ElasticKinematics kin;

        const double M = m_target_mass;
        const double Eb = m_beam_energy.get();

        kin.p_e = electron.p();
        kin.theta_e = electron.theta();
        kin.phi_e = electron.phi();
        kin.p_recoil = recoil.p();
        kin.theta_recoil = recoil.theta();
        kin.phi_recoil = recoil.phi();

        // Q^2 from electron
        kin.Q2 = 4.0 * Eb * kin.p_e * std::pow(std::sin(kin.theta_e / 2.0), 2);

        // W (invariant mass)
        double nu = Eb - kin.p_e;
        double W2 = M * M + 2.0 * M * nu - kin.Q2;
        kin.W = (W2 > 0) ? std::sqrt(W2) : 0.0;

        // Coplanarity: phi difference should be 180 degrees for elastic
        double phi_diff = std::abs(kin.phi_e - kin.phi_recoil) * clas12::Constants::RAD_TO_DEG;
        if (phi_diff > 180.0) phi_diff = 360.0 - phi_diff;
        kin.delta_phi = phi_diff - 180.0;

        // Expected electron momentum for elastic scattering
        // E' = E / (1 + (2E/M) * sin^2(theta/2))
        double sin2_half = std::pow(std::sin(kin.theta_e / 2.0), 2);
        kin.p_e_expected = Eb / (1.0 + (2.0 * Eb / M) * sin2_half);

        // Expected recoil momentum and angle from elastic kinematics
        // tan(theta_recoil) = 1 / ((1 + E/M) * tan(theta_e/2))
        double tan_half_e = std::tan(kin.theta_e / 2.0);
        if (tan_half_e > 0) {
            kin.theta_recoil_expected = std::atan(1.0 / ((1.0 + Eb / M) * tan_half_e));
        }

        // Recoil momentum from energy conservation
        double E_recoil_expected = M + Eb - kin.p_e_expected;
        double p2 = E_recoil_expected * E_recoil_expected - M * M;
        kin.p_recoil_expected = (p2 > 0) ? std::sqrt(p2) : 0.0;

        // Relative differences
        if (kin.p_e_expected > 0) {
            kin.delta_p_e = (kin.p_e - kin.p_e_expected) / kin.p_e_expected;
        }
        if (kin.p_recoil_expected > 0) {
            kin.delta_p_recoil = (kin.p_recoil - kin.p_recoil_expected) / kin.p_recoil_expected;
        }
        kin.delta_theta_recoil = (kin.theta_recoil - kin.theta_recoil_expected) * clas12::Constants::RAD_TO_DEG;

        return kin;
    }

    void book_histograms() {
        std::string prefix = "/Elastic_" + get_target_name(m_target);
        h_Q2 = m_hist_svc->get()->book_atomic_hist1D(prefix + "/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_W = m_hist_svc->get()->book_atomic_hist1D(prefix + "/W", "W;W [GeV];Events", 100, m_target_mass - 0.2, m_target_mass + 0.3);
        h_W_precut = m_hist_svc->get()->book_atomic_hist1D(prefix + "/W_precut", "W (pre-cut);W [GeV];Events", 200, 0, 3);
        h_delta_phi = m_hist_svc->get()->book_atomic_hist1D(prefix + "/delta_phi", "#Delta#phi;#Delta#phi [deg];Events", 100, -10, 10);
        h_delta_phi_precut = m_hist_svc->get()->book_atomic_hist1D(prefix + "/delta_phi_precut", "#Delta#phi (pre-cut);#Delta#phi [deg];Events", 180, -180, 180);
        h_delta_p_e = m_hist_svc->get()->book_atomic_hist1D(prefix + "/delta_p_e", "#Deltap_{e}/p_{e};#Deltap_{e}/p_{e};Events", 100, -0.2, 0.2);
        h_delta_p_recoil = m_hist_svc->get()->book_atomic_hist1D(prefix + "/delta_p_recoil", "#Deltap_{N}/p_{N};#Deltap_{N}/p_{N};Events", 100, -0.2, 0.2);
        h_delta_theta_recoil = m_hist_svc->get()->book_atomic_hist1D(prefix + "/delta_theta_recoil", "#Delta#theta_{N};#Delta#theta_{N} [deg];Events", 100, -5, 5);
        h_p_e_vs_theta_e = m_hist_svc->get()->book_atomic_hist2D(prefix + "/p_e_vs_theta_e", "p_{e} vs #theta_{e};#theta_{e} [deg];p_{e} [GeV]", 90, 0, 45, 100, 0, 11);
        h_p_recoil_vs_theta_recoil = m_hist_svc->get()->book_atomic_hist2D(prefix + "/p_N_vs_theta_N", "p_{N} vs #theta_{N};#theta_{N} [deg];p_{N} [GeV]", 90, 0, 90, 100, 0, 5);
        h_p_e_measured_vs_expected = m_hist_svc->get()->book_atomic_hist2D(prefix + "/p_e_meas_vs_exp", "p_{e} measured vs expected;p_{e}^{exp} [GeV];p_{e}^{meas} [GeV]", 100, 0, 11, 100, 0, 11);
        h_theta_recoil_measured_vs_expected = m_hist_svc->get()->book_atomic_hist2D(prefix + "/theta_N_meas_vs_exp", "#theta_{N} measured vs expected;#theta_{N}^{exp} [deg];#theta_{N}^{meas} [deg]", 90, 0, 90, 90, 0, 90);
    }

    void fill_histograms(const ElasticKinematics& kin) {
        if (h_Q2) h_Q2->fill(kin.Q2);
        if (h_W) h_W->fill(kin.W);
        if (h_delta_phi) h_delta_phi->fill(kin.delta_phi);
        if (h_delta_p_e) h_delta_p_e->fill(kin.delta_p_e);
        if (h_delta_p_recoil) h_delta_p_recoil->fill(kin.delta_p_recoil);
        if (h_delta_theta_recoil) h_delta_theta_recoil->fill(kin.delta_theta_recoil);
        if (h_p_e_vs_theta_e) h_p_e_vs_theta_e->fill(kin.theta_e * clas12::Constants::RAD_TO_DEG, kin.p_e);
        if (h_p_recoil_vs_theta_recoil) h_p_recoil_vs_theta_recoil->fill(kin.theta_recoil * clas12::Constants::RAD_TO_DEG, kin.p_recoil);
        if (h_p_e_measured_vs_expected) h_p_e_measured_vs_expected->fill(kin.p_e_expected, kin.p_e);
        if (h_theta_recoil_measured_vs_expected) h_theta_recoil_measured_vs_expected->fill(kin.theta_recoil_expected * clas12::Constants::RAD_TO_DEG, kin.theta_recoil * clas12::Constants::RAD_TO_DEG);
    }

    void setup_ntuple() {
        m_ntuple_svc->get()->register_branch("elastic_eN", "e_p", &m_nt_e_p);
        m_ntuple_svc->get()->register_branch("elastic_eN", "e_theta", &m_nt_e_theta);
        m_ntuple_svc->get()->register_branch("elastic_eN", "e_phi", &m_nt_e_phi);
        m_ntuple_svc->get()->register_branch("elastic_eN", "recoil_p", &m_nt_recoil_p);
        m_ntuple_svc->get()->register_branch("elastic_eN", "recoil_theta", &m_nt_recoil_theta);
        m_ntuple_svc->get()->register_branch("elastic_eN", "recoil_phi", &m_nt_recoil_phi);
        m_ntuple_svc->get()->register_branch("elastic_eN", "Q2", &m_nt_Q2);
        m_ntuple_svc->get()->register_branch("elastic_eN", "W", &m_nt_W);
        m_ntuple_svc->get()->register_branch("elastic_eN", "delta_phi", &m_nt_delta_phi);
        m_ntuple_svc->get()->register_branch("elastic_eN", "delta_p_e", &m_nt_delta_p_e);
        m_ntuple_svc->get()->register_branch("elastic_eN", "delta_p_recoil", &m_nt_delta_p_recoil);
        m_ntuple_svc->get()->register_branch("elastic_eN", "delta_theta_recoil", &m_nt_delta_theta_recoil);
        Log::info(name(), "N-tuple branches registered for elastic e-{}", get_target_name(m_target));
    }

    void fill_ntuple(const clas12::Particle& electron, const clas12::Particle& recoil, const ElasticKinematics& kin) {
        m_nt_e_p = electron.p();
        m_nt_e_theta = electron.theta();
        m_nt_e_phi = electron.phi();
        m_nt_recoil_p = recoil.p();
        m_nt_recoil_theta = recoil.theta();
        m_nt_recoil_phi = recoil.phi();
        m_nt_Q2 = kin.Q2;
        m_nt_W = kin.W;
        m_nt_delta_phi = kin.delta_phi;
        m_nt_delta_p_e = kin.delta_p_e;
        m_nt_delta_p_recoil = kin.delta_p_recoil;
        m_nt_delta_theta_recoil = kin.delta_theta_recoil;
        m_ntuple_svc->get()->fill("elastic_eN");
    }
};

}  // namespace clas12::algorithms
