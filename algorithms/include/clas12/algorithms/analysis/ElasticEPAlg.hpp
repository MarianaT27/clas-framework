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
    Deuteron,
    Helium4
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
        case NuclearTarget::Helium4:
            return clas12::Constants::HELIUM4MASS;
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
        case NuclearTarget::Helium4:
            return "helium4";
        default:
            return "proton";
    }
}

/**
 * @brief Elastic electron scattering kinematics (electron-only)
 */
struct ElasticKinematics {
    double Q2 = 0.0;       // Four-momentum transfer squared [GeV^2]
    double W = 0.0;        // Invariant mass [GeV]
    double nu = 0.0;       // Energy transfer [GeV]
    double theta_e = 0.0;  // Electron scattering angle [rad]
    double phi_e = 0.0;    // Electron azimuthal angle [rad]
    double p_e = 0.0;      // Electron momentum [GeV]
};

/**
 * @brief Algorithm for elastic electron selection based on W cut
 *
 * Selects elastic scattering events using electron kinematics only:
 * - Calculates W (invariant mass) from electron momentum and angle
 * - Applies W cut to select elastic events where W ~ target mass
 *
 * The W formula: W^2 = M^2 + 2*M*nu - Q^2
 * where M is target mass, nu = E_beam - E_electron, Q^2 = 4*E*E'*sin^2(theta/2)
 *
 * Supports proton, neutron, deuteron, and helium-4 targets.
 * For elastic scattering, W should peak at the target mass.
 *
 * Note: This algorithm does NOT require recoil particle detection.
 */
class ElasticEPAlg : public clas12::framework::SimpleAlgorithm<ElasticEPAlg> {
   public:
    explicit ElasticEPAlg(std::string name = "ElasticEPAlg", NuclearTarget target = NuclearTarget::Proton)
        : SimpleAlgorithm(std::move(name)), m_target(target), m_target_mass(get_target_mass(target)), m_cutflow("Elastic e-" + get_target_name(target) + " Selection (W-only)") {
        declare_property(m_beam_energy);
        declare_property(m_W_min);
        declare_property(m_W_max);
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
        Log::info(name(), "Initializing elastic e-{} algorithm (W-only selection)", get_target_name(m_target));
        Log::info(name(), "  Beam energy: {} GeV", m_beam_energy.get());
        Log::info(name(), "  Target mass: {} GeV", m_target_mass);
        Log::info(name(), "  W cut: [{}, {}] GeV", m_W_min.get(), m_W_max.get());

        book_histograms();
        if (m_ntuple_svc) setup_ntuple();

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("W Cut");

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Debug: Check what's in REC::Particle bank
        hipo::bank& REC_Particle = event.hipo_event().get("REC::Particle");
        if (m_debug_count < 5 && REC_Particle.rows() > 0) {
            Log::info(name(), "DEBUG: REC::Particle has {} rows", REC_Particle.rows());
            for (int i = 0; i < REC_Particle.rows() && i < 10; i++) {
                int pid = REC_Particle.get<int>("pid", i);
                Log::info(name(), "  Row {}: pid={}", i, pid);
            }
            m_debug_count++;
        }

        // Get electrons directly from REC::Particle bank
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        const auto& electron = electrons.front();
        m_cutflow.pass("Has Electron");

        // Calculate kinematics from electron only
        ElasticKinematics kin = calculate_kinematics(electron);

        // Fill pre-cut histograms
        if (h_W_precut) h_W_precut->fill(kin.W);
        if (h_Q2_precut) h_Q2_precut->fill(kin.Q2);

        // Apply W cut only
        if (kin.W < m_W_min.get() || kin.W > m_W_max.get()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("W Cut");

        // Store results
        event.set_decision("HasElasticElectron", true);
        event.store_particles<clas12::Particle>("ElasticElectron", {electron});
        event.store("elastic_Q2", kin.Q2);
        event.store("elastic_W", kin.W);
        event.store("elastic_nu", kin.nu);

        // Fill histograms
        fill_histograms(kin);

        // Fill N-tuple
        if (m_ntuple_svc) {
            fill_ntuple(electron, kin);
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing elastic e-{} algorithm", get_target_name(m_target));
        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get()->get_entries("elastic_electron");
            Log::info(name(), "N-tuple has {} entries", entries);
        }
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    NuclearTarget m_target;
    double m_target_mass;
    clas12::framework::Cutflow m_cutflow;

    // Configuration properties
    clas12::framework::Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
    clas12::framework::Property<double> m_W_min{"W_min", 0.8, "Minimum W [GeV]"};
    clas12::framework::Property<double> m_W_max{"W_max", 1.05, "Maximum W [GeV]"};

    // Services (pointers set by configure())
    clas12::framework::ServiceHandle<clas12::framework::LoggingService>* m_log_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;

    // Histograms (atomic for thread safety)
    clas12::framework::AtomicHistogram1D* h_Q2 = nullptr;
    clas12::framework::AtomicHistogram1D* h_Q2_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_W = nullptr;
    clas12::framework::AtomicHistogram1D* h_W_precut = nullptr;
    clas12::framework::AtomicHistogram1D* h_nu = nullptr;
    clas12::framework::AtomicHistogram2D* h_p_e_vs_theta_e = nullptr;

    // N-tuple variables
    double m_nt_e_p = 0, m_nt_e_theta = 0, m_nt_e_phi = 0;
    double m_nt_Q2 = 0, m_nt_W = 0, m_nt_nu = 0;

    // Debug counter
    int m_debug_count = 0;

    auto calculate_kinematics(const clas12::Particle& electron) -> ElasticKinematics {
        ElasticKinematics kin;

        const double M = m_target_mass;
        const double Eb = m_beam_energy.get();

        kin.p_e = electron.p();
        kin.theta_e = electron.theta();
        kin.phi_e = electron.phi();

        // Q^2 from electron kinematics
        kin.Q2 = 4.0 * Eb * kin.p_e * std::pow(std::sin(kin.theta_e / 2.0), 2);

        // Energy transfer
        kin.nu = Eb - kin.p_e;

        // W (invariant mass) from electron kinematics
        // W^2 = M^2 + 2*M*nu - Q^2
        double W2 = M * M + 2.0 * M * kin.nu - kin.Q2;
        kin.W = (W2 > 0) ? std::sqrt(W2) : 0.0;

        return kin;
    }

    void book_histograms() {
        std::string prefix = "/Elastic_" + get_target_name(m_target);
        h_Q2 = m_hist_svc->get()->book_atomic_hist1D(prefix + "/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_Q2_precut = m_hist_svc->get()->book_atomic_hist1D(prefix + "/Q2_precut", "Q^{2} (pre-cut);Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_W = m_hist_svc->get()->book_atomic_hist1D(prefix + "/W", "W;W [GeV];Events", 100, m_target_mass - 0.3, m_target_mass + 0.5);
        h_W_precut = m_hist_svc->get()->book_atomic_hist1D(prefix + "/W_precut", "W (pre-cut);W [GeV];Events", 200, 0, m_target_mass + 2.0);
        h_nu = m_hist_svc->get()->book_atomic_hist1D(prefix + "/nu", "#nu;#nu [GeV];Events", 100, 0, 11);
        h_p_e_vs_theta_e = m_hist_svc->get()->book_atomic_hist2D(prefix + "/p_e_vs_theta_e", "p_{e} vs #theta_{e};#theta_{e} [deg];p_{e} [GeV]", 90, 0, 45, 100, 0, 11);
    }

    void fill_histograms(const ElasticKinematics& kin) {
        if (h_Q2) h_Q2->fill(kin.Q2);
        if (h_W) h_W->fill(kin.W);
        if (h_nu) h_nu->fill(kin.nu);
        if (h_p_e_vs_theta_e) h_p_e_vs_theta_e->fill(kin.theta_e * clas12::Constants::RAD_TO_DEG, kin.p_e);
    }

    void setup_ntuple() {
        m_ntuple_svc->get()->register_branch("elastic_electron", "e_p", &m_nt_e_p);
        m_ntuple_svc->get()->register_branch("elastic_electron", "e_theta", &m_nt_e_theta);
        m_ntuple_svc->get()->register_branch("elastic_electron", "e_phi", &m_nt_e_phi);
        m_ntuple_svc->get()->register_branch("elastic_electron", "Q2", &m_nt_Q2);
        m_ntuple_svc->get()->register_branch("elastic_electron", "W", &m_nt_W);
        m_ntuple_svc->get()->register_branch("elastic_electron", "nu", &m_nt_nu);
        Log::info(name(), "N-tuple branches registered for elastic electron");
    }

    void fill_ntuple(const clas12::Particle& electron, const ElasticKinematics& kin) {
        m_nt_e_p = electron.p();
        m_nt_e_theta = electron.theta();
        m_nt_e_phi = electron.phi();
        m_nt_Q2 = kin.Q2;
        m_nt_W = kin.W;
        m_nt_nu = kin.nu;
        m_ntuple_svc->get()->fill("elastic_electron");
    }
};

}  // namespace clas12::algorithms
