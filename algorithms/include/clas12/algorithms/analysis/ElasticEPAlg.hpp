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
#include <clas12/framework/core/HistogramAliases.hpp>
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

    double W_min = 1.6;
    double W_max = 2.0;
    double Vz_min = -30.0;
    double Vz_max = 10.0;
    bool enable_vz_cut = true;
    bool enable_w_cut = true;

    static constexpr auto config_fields() {
        using T = ElasticEPConfig;
        return std::make_tuple(
            framework::field("beam_energy", &T::beam_energy),
            framework::field("W_min", &T::W_min),
            framework::field("W_max", &T::W_max),
            framework::field("Vz_min", &T::Vz_min),
            framework::field("Vz_max", &T::Vz_max),
            framework::field("enable_vz_cut", &T::enable_vz_cut),
            framework::field("enable_w_cut", &T::enable_w_cut));
    }
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
        required_services(m_hist, m_ntuple);
        declare_config(m_config);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing elastic e-{} algorithm (W-only selection)", get_target_name(m_config.target));
        Log::info(name(), "  Beam energy: {} GeV", m_config.beam_energy);
        Log::info(name(), "  Target mass: {} GeV", m_target_mass);
        Log::info(name(), "  W cut: [{}, {}] GeV", m_config.W_min, m_config.W_max);
        Log::info(name(), "  Vz cut: [{}, {}] cm", m_config.Vz_min, m_config.Vz_max);

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

        // find_trigger_electron() returns nullopt when none qualifies; the old
        // electrons().empty() guard was redundant and forced a full particle load.
        const auto possible_electron = event.find_trigger_electron({FD, BD});
        if (!possible_electron.has_value()) return StatusCode::FILTERED();
        const Particle& electron = possible_electron.value();

        if (electron.vz() == -3.0 || electron.p() <= 0.00001 || electron.theta() <= 0.00001) return StatusCode::FILTERED();

        ElasticKinematics kin = calculate_kinematics(electron);
        if (kin.W <= 0.00001 || kin.Q2 <= 0.000001) return StatusCode::FILTERED();
        m_cutflow.pass("Has Electron");

        double vz = electron.vz();
        h_Vz_precut.fill(vz);
        if (m_config.enable_vz_cut && (vz < m_config.Vz_min || vz > m_config.Vz_max)) return StatusCode::FILTERED();
        m_cutflow.pass("Vz Cut");

        h_W_precut.fill(kin.W);
        h_Q2_precut.fill(kin.Q2);
        if (m_config.enable_w_cut && (kin.W < m_config.W_min || kin.W > m_config.W_max)) return StatusCode::FILTERED();
        m_cutflow.pass("W Cut");

        event.set_decision_typed(framework::Decisions::HasElasticElectron, true);
        event.store_typed(framework::Keys::ElasticElectron, electron);
        event.store_typed(framework::Keys::ElasticQ2, kin.Q2);
        event.store_typed(framework::Keys::ElasticW, kin.W);
        event.store_typed(framework::Keys::ElasticNu, kin.nu);

        fill_histograms(kin);
        h_Vz.fill(vz);
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

framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple;

    framework::H1<> h_Q2{mh::regular(0, 10, 100)};
    framework::H1<> h_Q2_precut{mh::regular(0, 10, 100)};
    framework::H1<> h_W{mh::regular(m_target_mass - 0.3, m_target_mass + 0.5, 100)};
    framework::H1<> h_W_precut{mh::regular(0, m_target_mass + 2.0, 200)};
    framework::H1<> h_nu{mh::regular(0, 11, 100)};
    framework::H2<> h_p_e_vs_theta_e{mh::regular(0, 45, 90), mh::regular(0, 11, 100)};
    framework::H1<> h_Vz_precut{mh::regular(-50, 30, 200)};
    framework::H1<> h_Vz{mh::regular(-50, 30, 200)};

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
        const double Eb = m_config.beam_energy;
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
        m_hist->register_hist(prefix + "/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", h_Q2);
        m_hist->register_hist(prefix + "/Q2_precut", "Q^{2} (pre-cut);Q^{2} [GeV^{2}];Events", h_Q2_precut);
        m_hist->register_hist(prefix + "/W", "W;W [GeV];Events", h_W);
        m_hist->register_hist(prefix + "/W_precut", "W (pre-cut);W [GeV];Events", h_W_precut);
        m_hist->register_hist(prefix + "/nu", "#nu;#nu [GeV];Events", h_nu);
        m_hist->register_hist(prefix + "/p_e_vs_theta_e", "p_{e} vs #theta_{e};#theta_{e} [deg];p_{e} [GeV]", h_p_e_vs_theta_e);
        m_hist->register_hist(prefix + "/Vz_precut", "Vz (pre-cut);Vz [cm];Events", h_Vz_precut);
        m_hist->register_hist(prefix + "/Vz", "Vz;Vz [cm];Events", h_Vz);
    }

    auto fill_histograms(const ElasticKinematics& kin) -> void {
        h_Q2.fill(kin.Q2);
        h_W.fill(kin.W);
        h_nu.fill(kin.nu);
        h_p_e_vs_theta_e.fill(kin.theta_e * Constants::RAD_TO_DEG, kin.p_e);
    }
};

}  // namespace algorithms
