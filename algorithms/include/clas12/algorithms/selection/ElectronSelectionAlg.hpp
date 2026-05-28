#pragma once

/**
 * @file ElectronSelectionAlg.hpp
 * @brief Algorithm for electron selection and kinematics analysis
 *
 * Selects good electrons from CLAS12 data using:
 * - Particle identification cuts (chi2pid)
 * - Kinematic cuts (momentum, vertex z)
 * - Sampling fraction cuts (calorimeter response)
 * - Fiducial cuts (PCAL lv/lw and DC region edge thresholds)
 */

#include <optional>
#include <vector>

#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include "clas12/bankdefs/generated_readers.hpp"

namespace algorithms {

using framework::Log;

/// Configuration for ElectronSelectionAlg.
struct ElectronSelectionConfig {
    Target target = Target::LD2;
    Polarity polarity = Polarity::Outbending;

    bool enable_ntuple = true;
    bool enable_histograms = true;
    bool enable_chi2pid_cut = true;
    bool enable_momentum_cut = true;
    bool enable_vz_cut = true;
    bool enable_sf_cut = true;
    bool enable_pcal_fiducial_cut = true;
    bool enable_dc_edge_cut = true;
    double pcal_lv_min = 9.0;
    double pcal_lw_min = 9.0;
    double dc_r1_edge_min = 1.68;
    double dc_r2_edge_min = 2.0;
    double dc_r3_edge_min = 8.75;

    static constexpr auto config_fields() {
        using T = ElectronSelectionConfig;
        return std::make_tuple(
            framework::field("target",                   &T::target),
            framework::field("polarity",                 &T::polarity),
            framework::field("enable_ntuple",            &T::enable_ntuple),
            framework::field("enable_histograms",        &T::enable_histograms),
            framework::field("enable_chi2pid_cut",       &T::enable_chi2pid_cut),
            framework::field("enable_momentum_cut",      &T::enable_momentum_cut),
            framework::field("enable_vz_cut",            &T::enable_vz_cut),
            framework::field("enable_sf_cut",            &T::enable_sf_cut),
            framework::field("enable_pcal_fiducial_cut", &T::enable_pcal_fiducial_cut),
            framework::field("enable_dc_edge_cut",       &T::enable_dc_edge_cut),
            framework::field("pcal_lv_min",              &T::pcal_lv_min),
            framework::field("pcal_lw_min",              &T::pcal_lw_min),
            framework::field("dc_r1_edge_min",           &T::dc_r1_edge_min),
            framework::field("dc_r2_edge_min",           &T::dc_r2_edge_min),
            framework::field("dc_r3_edge_min",           &T::dc_r3_edge_min));
    }
};

/**
 * @brief Algorithm for electron selection and kinematics
 */
class ElectronSelectionAlg : public framework::Algorithm {
   public:
    using Config = ElectronSelectionConfig;
    using StatusCode = framework::StatusCode;

    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Electron Selection") {
        required_services(m_hist, m_ntuple_svc, m_electron_cuts);
        declare_banks("REC::Particle", "REC::Calorimeter", "REC::Traj");
        declare_config(m_config);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Target: {}", to_string(m_config.target));
        Log::info(name(), "Polarity: {}", to_string(m_config.polarity));
        Log::info(name(), "NTuple output: {}", m_config.enable_ntuple ? "enabled" : "disabled");
        Log::info(name(), "Histograms: {}", m_config.enable_histograms ? "enabled" : "disabled");

        m_electron_cuts->set_target(m_config.target);
        m_electron_cuts->set_polarity(m_config.polarity);

        if (m_config.enable_histograms) book_histograms();

        m_ntuple.init(m_ntuple_svc.get(), "electron_kinematics");

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC R1 Edge");
        m_cutflow.add_cut("DC R2 Edge");
        m_cutflow.add_cut("DC R3 Edge");

        auto vz_range = framework::ElectronCutsService::get_vz_range(RunGroup::RG_D, m_config.target);
        Log::info(name(), "Vz range: [{:.2f}, {:.2f}] cm", vz_range.min, vz_range.max);
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        std::optional<Particle> trigger_e = event.find_trigger_electron();
        if (!trigger_e.has_value()) return StatusCode::FILTERED();
        Particle electron = trigger_e.value();
        m_cutflow.pass("Has Electron");

        CalorimeterBank calorimeter = electron.calo();
        DCTrajBank traj = electron.traj();
        double sampling_fraction = (calorimeter.pcal_energy + calorimeter.inner_energy + calorimeter.outer_energy) / electron.p();
        const double vz_corrected = m_electron_cuts->corrected_vz(electron.vz(), electron.p(), electron.theta(),electron.phi(), calorimeter.pcal_sector);

        if (m_config.enable_histograms)
            h_before.fill(electron, calorimeter, sampling_fraction, vz_corrected);

        if (m_config.enable_chi2pid_cut && !m_electron_cuts->pass_chi2pid_cut(electron.chi2pid())) return StatusCode::FILTERED();
        m_cutflow.pass("Chi2 PID");

        if (m_config.enable_momentum_cut && !m_electron_cuts->pass_momentum_cut(electron.p())) return StatusCode::FILTERED();
        m_cutflow.pass("Momentum > 0.8 GeV");

        if (m_config.enable_vz_cut && !m_electron_cuts->pass_vz_cut(electron.vz(), electron.p(), electron.theta(), electron.phi(), calorimeter.pcal_sector)) return StatusCode::FILTERED();
        m_cutflow.pass("Vertex Z");

        if (m_config.enable_sf_cut && !m_electron_cuts->pass_sampling_fraction_cut(sampling_fraction, electron.p(), calorimeter.pcal_sector)) return StatusCode::FILTERED();
        m_cutflow.pass("Sampling Fraction");

        if (m_config.enable_pcal_fiducial_cut && !m_electron_cuts->pass_pcal_fiducial_cut(calorimeter.pcal_lv, calorimeter.pcal_lw)) return StatusCode::FILTERED();
        m_cutflow.pass("PCAL Fiducial");

        if (m_config.enable_dc_edge_cut && !m_electron_cuts->pass_dc_edge_cut(traj.region1_edge, traj.region2_edge, traj.region3_edge)) return StatusCode::FILTERED();
        m_cutflow.pass("DC R1 Edge");
        m_cutflow.pass("DC R2 Edge");
        m_cutflow.pass("DC R3 Edge");

        if (m_config.enable_histograms)
            h_after.fill(electron, calorimeter, sampling_fraction, vz_corrected);

        if (m_config.enable_ntuple) {
            m_ntuple.data().e_p = electron.p();
            m_ntuple.data().e_theta = electron.theta();
            m_ntuple.data().e_phi = electron.phi();
            m_ntuple.data().e_vz = electron.vz();
            m_ntuple.data().e_chi2pid = electron.chi2pid();
            m_ntuple.data().e_sector = calorimeter.pcal_sector;
            m_ntuple.fill();
        }

        event.store_typed(framework::Keys::TriggerElectron, electron);
        event.set_decision_typed(framework::Decisions::HasGoodElectron, true);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        if (m_config.enable_ntuple && m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("electron_kinematics");
            Log::stat(name(), "N-tuple entries", entries);
        }

        Log::blank();
        m_cutflow.print();

        Log::success(name(), "Finalization complete");
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;
    framework::ServiceRef<framework::ElectronCutsService> m_electron_cuts;

    struct StageHistograms {
        framework::H1<> p{mh::regular(0.0, 12.0, 100)};
        framework::H1<> theta{mh::regular(0.0, 45.0, 90)};
        framework::H1<> phi{mh::regular(-180.0, 180.0, 360)};
        framework::H1<> pt{mh::regular(0.0, 4.0, 100)};
        framework::H1<> sector{mh::regular(0.5, 7.5, 7)};
        framework::H1<> vz{mh::regular(-20.0, 5.0, 200)};
        framework::H1<> vz_corr{mh::regular(-20.0, 5.0, 200)};
        framework::H1<> vx{mh::regular(-2.0, 2.0, 100)};
        framework::H1<> vy{mh::regular(-2.0, 2.0, 100)};
        framework::H1<> chi2pid{mh::regular(-10.0, 10.0, 200)};
        framework::H1<> pcal_lv{mh::regular(0.0, 450.0, 100)};
        framework::H1<> pcal_lw{mh::regular(0.0, 450.0, 100)};
        framework::H2<> sf_vs_p{mh::regular(0.0, 12.0, 100), mh::regular(0.0, 0.5, 100)};
        framework::H2<> chi2pid_vs_p{mh::regular(0.0, 12.0, 100), mh::regular(-10.0, 10.0, 200)};
        framework::H2<> vz_vs_phi{mh::regular(-180.0, 180.0, 180), mh::regular(-20.0, 5.0, 100)};
        framework::H2<> vz_vs_theta{mh::regular(0.0, 45.0, 90), mh::regular(-20.0, 5.0, 100)};
        framework::H2<> vz_corr_vs_phi{mh::regular(-180.0, 180.0, 180), mh::regular(-20.0, 5.0, 100)};
        framework::H2<> vz_corr_vs_theta{mh::regular(0.0, 45.0, 90), mh::regular(-20.0, 5.0, 100)};
        framework::H2<> theta_vs_phi{mh::regular(-180.0, 180.0, 180), mh::regular(0.0, 45.0, 90)};
        framework::H2<> p_vs_theta{mh::regular(0.0, 45.0, 90), mh::regular(0.0, 12.0, 100)};

        auto book(framework::HistogramService* svc, const std::string& dir) -> void {
            svc->register_hist(dir + "/p", "Electron Momentum;p [GeV/c];Events", p);
            svc->register_hist(dir + "/theta", "Electron #theta;#theta [deg];Events", theta);
            svc->register_hist(dir + "/phi", "Electron #phi;#phi [deg];Events", phi);
            svc->register_hist(dir + "/pt", "Electron p_{T};p_{T} [GeV/c];Events", pt);
            svc->register_hist(dir + "/sector", "Electron Sector;Sector;Events", sector);
            svc->register_hist(dir + "/vz", "Vertex Z;v_{z} [cm];Events", vz);
            svc->register_hist(dir + "/vz_corr", "Corrected Vertex Z;v_{z}^{corr} [cm];Events", vz_corr);
            svc->register_hist(dir + "/vx", "Vertex X;v_{x} [cm];Events", vx);
            svc->register_hist(dir + "/vy", "Vertex Y;v_{y} [cm];Events", vy);
            svc->register_hist(dir + "/chi2pid", "Electron #chi^{2}_{PID};#chi^{2}_{PID};Events", chi2pid);
            svc->register_hist(dir + "/pcal_lv", "PCAL Local V;PCAL_{lv} [cm];Events", pcal_lv);
            svc->register_hist(dir + "/pcal_lw", "PCAL Local W;PCAL_{lw} [cm];Events", pcal_lw);
            svc->register_hist(dir + "/sf_vs_p", "SF vs p;p [GeV/c];SF", sf_vs_p);
            svc->register_hist(dir + "/chi2pid_vs_p", "#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}", chi2pid_vs_p);
            svc->register_hist(dir + "/vz_vs_phi", "v_{z} vs #phi;#phi [deg];v_{z} [cm]", vz_vs_phi);
            svc->register_hist(dir + "/vz_vs_theta", "v_{z} vs #theta;#theta [deg];v_{z} [cm]", vz_vs_theta);
            svc->register_hist(dir + "/vz_corr_vs_phi", "v_{z}^{corr} vs #phi;#phi [deg];v_{z}^{corr} [cm]", vz_corr_vs_phi);
            svc->register_hist(dir + "/vz_corr_vs_theta", "v_{z}^{corr} vs #theta;#theta [deg];v_{z}^{corr} [cm]", vz_corr_vs_theta);
            svc->register_hist(dir + "/theta_vs_phi", "#theta vs #phi;#phi [deg];#theta [deg]", theta_vs_phi);
            svc->register_hist(dir + "/p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]", p_vs_theta);
        }

        auto fill(const Particle& e, const CalorimeterBank& calo, double SF, double vz_corrected) -> void {
            p.fill(e.p());
            theta.fill(e.theta());
            phi.fill(e.phi());
            pt.fill(e.pt());
            sector.fill(calo.pcal_sector);
            vz.fill(e.vz());
            vz_corr.fill(vz_corrected);
            vx.fill(e.vx());
            vy.fill(e.vy());
            chi2pid.fill(e.chi2pid());
            pcal_lv.fill(calo.pcal_lv);
            pcal_lw.fill(calo.pcal_lw);
            sf_vs_p.fill(e.p(), SF);
            chi2pid_vs_p.fill(e.p(), e.chi2pid());
            vz_vs_phi.fill(e.phi(), e.vz());
            vz_vs_theta.fill(e.theta(), e.vz());
            vz_corr_vs_phi.fill(e.phi(), vz_corrected);
            vz_corr_vs_theta.fill(e.theta(), vz_corrected);
            theta_vs_phi.fill(e.phi(), e.theta());
            p_vs_theta.fill(e.theta(), e.p());
        }
    };

    StageHistograms h_before;
    StageHistograms h_after;

    auto book_histograms() -> void {
        h_before.book(m_hist.get(), "/Electron/Kinematics/before_cuts");
        h_after.book(m_hist.get(), "/Electron/Kinematics/after_cuts");
    }

    struct ElectronNTupleData {
        double e_p = 0.0;
        double e_theta = 0.0;
        double e_phi = 0.0;
        double e_vz = 0.0;
        double e_chi2pid = 0.0;
        int e_sector = 0;

        static constexpr auto ntuple_fields() {
            using T = ElectronNTupleData;
            return std::make_tuple(framework::field("e_p", &T::e_p), framework::field("e_theta", &T::e_theta), framework::field("e_phi", &T::e_phi), framework::field("e_vz", &T::e_vz), framework::field("e_chi2pid", &T::e_chi2pid), framework::field("e_sector", &T::e_sector));
        }
    };

    framework::NTupleWriter<ElectronNTupleData> m_ntuple;
};

}  // namespace algorithms
