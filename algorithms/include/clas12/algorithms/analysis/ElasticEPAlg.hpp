#pragma once

#include <cmath>
#include <clas12/Constants.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/ParticleUtils.hpp>

namespace algorithms {

using framework::Log;

enum class NuclearTarget { Proton, Neutron, Deuteron, Helium4 };

inline auto get_target_mass(NuclearTarget target) -> double {
    switch (target) {
        case NuclearTarget::Proton: return Constants::PROTONMASS;
        case NuclearTarget::Neutron: return Constants::NEUTRONMASS;
        case NuclearTarget::Deuteron: return Constants::DEUTERIUMMASS;
        case NuclearTarget::Helium4: return Constants::HELIUM4MASS;
        default: return Constants::PROTONMASS;
    }
}

inline auto get_recoil_pid(NuclearTarget target) -> PIDs {
    switch (target) {
        case NuclearTarget::Proton: return PIDs::PROTON;
        case NuclearTarget::Neutron: return PIDs::NEUTRON;
        case NuclearTarget::Deuteron: return PIDs::DEUTERON;
        default: return PIDs::PROTON;
    }
}

inline auto get_target_name(NuclearTarget target) -> std::string {
    switch (target) {
        case NuclearTarget::Proton: return "proton";
        case NuclearTarget::Neutron: return "neutron";
        case NuclearTarget::Deuteron: return "deuteron";
        case NuclearTarget::Helium4: return "helium4";
        default: return "proton";
    }
}

struct ElasticKinematics {
    double Q2 = 0.0;
    double W = 0.0;
    double nu = 0.0;
    double theta_e = 0.0;
    double phi_e = 0.0;
    double p_e = 0.0;
};

/// Configuration for ElasticEPAlg.
struct ElasticEPConfig {
    NuclearTarget target = NuclearTarget::Proton;
    double beam_energy = 10.6;
};

/**
 * @brief Algorithm for elastic electron selection based on W cut
 */
class ElasticEPAlg : public framework::Algorithm {
   public:
    using Config = ElasticEPConfig;
    using StatusCode = framework::StatusCode;

    explicit ElasticEPAlg(std::string name = "ElasticEPAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_target_mass(get_target_mass(config.target)), m_cutflow("Elastic e-" + get_target_name(config.target) + " Selection (W-only)") {
        m_beam_energy.set(m_config.beam_energy);
        required_services(m_hist, m_ntuple);
        declare_properties(m_beam_energy, m_W_min, m_W_max, m_Vz_min, m_Vz_max, m_enable_vz_cut, m_enable_w_cut);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing elastic e-{} algorithm (W-only selection)", get_target_name(m_config.target));
        Log::info(name(), "  Beam energy: {} GeV", m_beam_energy.get());
        Log::info(name(), "  Target mass: {} GeV", m_target_mass);
        Log::info(name(), "  W cut: [{}, {}] GeV", m_W_min.get(), m_W_max.get());
        Log::info(name(), "  Vz cut: [{}, {}] cm", m_Vz_min.get(), m_Vz_max.get());

        book_histograms();
        m_ntuple_writer.init(m_ntuple.get(), "elastic_electron");
        Log::info(name(), "N-tuple branches registered for elastic electron");

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Vz Cut");
        m_cutflow.add_cut("W Cut");

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        if (event.electrons().empty()) return StatusCode::FILTERED();
        const auto possible_electron = event.find_trigger_electron({FD, BD});
        if (!possible_electron.has_value()) return StatusCode::FILTERED();
        const Particle& electron = possible_electron.value();

        if (electron.vz() == -3.0 || electron.p() <= 0.00001 || electron.theta() <= 0.00001) return StatusCode::FILTERED();

        ElasticKinematics kin = calculate_kinematics(electron);
        if (kin.W <= 0.00001 || kin.Q2 <= 0.000001) return StatusCode::FILTERED();
        m_cutflow.pass("Has Electron");

        double vz = electron.vz();
        h_Vz_precut->fill(vz);
        if (m_enable_vz_cut.get() && (vz < m_Vz_min.get() || vz > m_Vz_max.get())) return StatusCode::FILTERED();
        m_cutflow.pass("Vz Cut");

        h_W_precut->fill(kin.W);
        h_Q2_precut->fill(kin.Q2);
        if (m_enable_w_cut.get() && (kin.W < m_W_min.get() || kin.W > m_W_max.get())) return StatusCode::FILTERED();
        m_cutflow.pass("W Cut");

        event.set_decision_typed(framework::Decisions::HasElasticElectron, true);
        event.store_typed(framework::Keys::ElasticElectron, electron);
        event.store_typed(framework::Keys::ElasticQ2, kin.Q2);
        event.store_typed(framework::Keys::ElasticW, kin.W);
        event.store_typed(framework::Keys::ElasticNu, kin.nu);

        fill_histograms(kin);
        h_Vz->fill(vz);
        if (m_ntuple) {
            auto& d = m_ntuple_writer.data();
            d.e_p = electron.p();
            d.e_theta = electron.theta();
            d.e_phi = electron.phi();
            d.Q2 = kin.Q2;
            d.W = kin.W;
            d.nu = kin.nu;
            m_ntuple_writer.fill();
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing elastic e-{} algorithm", get_target_name(m_config.target));
        if (m_ntuple) Log::info(name(), "N-tuple has {} entries", m_ntuple->get_entries("elastic_electron"));
        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    double m_target_mass;
    framework::Cutflow m_cutflow;

    framework::Property<double> m_beam_energy{"beam_energy", 2.2, "Beam energy [GeV]"};
    framework::Property<double> m_W_min{"W_min", 1.6, "Minimum W [GeV]"};
    framework::Property<double> m_W_max{"W_max", 2.0, "Maximum W [GeV]"};
    framework::Property<double> m_Vz_min{"Vz_min", -30.0, "Minimum Vz [cm]"};
    framework::Property<double> m_Vz_max{"Vz_max", 10.0, "Maximum Vz [cm]"};
    framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
    framework::Property<bool> m_enable_w_cut{"enable_w_cut", true, "Enable W cut"};

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple;

    framework::ThreadLocalHistogram1D* h_Q2 = nullptr;
    framework::ThreadLocalHistogram1D* h_Q2_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_W = nullptr;
    framework::ThreadLocalHistogram1D* h_W_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_nu = nullptr;
    framework::ThreadLocalHistogram2D* h_p_e_vs_theta_e = nullptr;
    framework::ThreadLocalHistogram1D* h_Vz_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_Vz = nullptr;

    struct ElasticEPNTupleData {
        double e_p = 0.0;
        double e_theta = 0.0;
        double e_phi = 0.0;
        double Q2 = 0.0;
        double W = 0.0;
        double nu = 0.0;

        static constexpr auto ntuple_fields() {
            using T = ElasticEPNTupleData;
            return std::make_tuple(framework::field("e_p", &T::e_p), framework::field("e_theta", &T::e_theta), framework::field("e_phi", &T::e_phi), framework::field("Q2", &T::Q2), framework::field("W", &T::W), framework::field("nu", &T::nu));
        }
    };

    framework::NTupleWriter<ElasticEPNTupleData> m_ntuple_writer;

    auto calculate_kinematics(const Particle& electron) -> ElasticKinematics {
        ElasticKinematics kin;
        const double M = m_target_mass;
        const double Eb = m_beam_energy.get();
        kin.p_e = electron.p();
        kin.theta_e = electron.theta();
        kin.phi_e = electron.phi();
        kin.Q2 = 4.0 * Eb * kin.p_e * std::pow(std::sin(kin.theta_e / 2.0), 2);
        kin.nu = Eb - kin.p_e;
        double W2 = M * M + 2.0 * M * kin.nu - kin.Q2;
        kin.W = (W2 > 0) ? std::sqrt(W2) : 0.0;
        return kin;
    }

    auto book_histograms() -> void {
        std::string prefix = "/Elastic_" + get_target_name(m_config.target);
        h_Q2 = m_hist->book_hist1D(prefix + "/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_Q2_precut = m_hist->book_hist1D(prefix + "/Q2_precut", "Q^{2} (pre-cut);Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_W = m_hist->book_hist1D(prefix + "/W", "W;W [GeV];Events", 100, m_target_mass - 0.3, m_target_mass + 0.5);
        h_W_precut = m_hist->book_hist1D(prefix + "/W_precut", "W (pre-cut);W [GeV];Events", 200, 0, m_target_mass + 2.0);
        h_nu = m_hist->book_hist1D(prefix + "/nu", "#nu;#nu [GeV];Events", 100, 0, 11);
        h_p_e_vs_theta_e = m_hist->book_hist2D(prefix + "/p_e_vs_theta_e", "p_{e} vs #theta_{e};#theta_{e} [deg];p_{e} [GeV]", 90, 0, 45, 100, 0, 11);
        h_Vz_precut = m_hist->book_hist1D(prefix + "/Vz_precut", "Vz (pre-cut);Vz [cm];Events", 200, -50, 30);
        h_Vz = m_hist->book_hist1D(prefix + "/Vz", "Vz;Vz [cm];Events", 200, -50, 30);
    }

    auto fill_histograms(const ElasticKinematics& kin) -> void {
        h_Q2->fill(kin.Q2);
        h_W->fill(kin.W);
        h_nu->fill(kin.nu);
        h_p_e_vs_theta_e->fill(kin.theta_e * Constants::RAD_TO_DEG, kin.p_e);
    }
};

}  // namespace algorithms
