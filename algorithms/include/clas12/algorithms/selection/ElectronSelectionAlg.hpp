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
 *
 * The Config and NTuple structs are plain aggregates whose fields are reflected
 * to TOML keys / RNTuple branches via Boost.PFR (no config_fields()/
 * ntuple_fields() tuples). Cut logic and histograms are written explicitly.
 */

#include <optional>
#include <type_traits>
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
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/HistogramReflect.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/ServiceReflect.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/bankdefs/REC_Calorimeter.hpp>
#include <clas12/bankdefs/REC_Traj.hpp>
#include "clas12/bankdefs/generated_readers.hpp"

namespace algorithms {

using framework::Log;
using StatusCode = framework::StatusCode;

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
};

static_assert(std::is_aggregate_v<ElectronSelectionConfig>, "ElectronSelectionConfig must stay a simple aggregate for PFR reflection");

/**
 * @brief Algorithm for electron selection and kinematics
 */
class ElectronSelectionAlg : public framework::ConfigurableAlgorithm<ElectronSelectionConfig> {
   public:

    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", Config config = {})
        : ConfigurableAlgorithm(std::move(name), config), m_cutflow("Electron Selection") {
        framework::wire_services(*this);  // reflects ServiceRef members (refl-cpp)
        declare_banks_typed<bankdefs::REC_Particle, bankdefs::REC_Calorimeter, bankdefs::REC_Traj>();
        declare_ntuple(m_ntuple, [this] { return m_config.enable_ntuple; });
    }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        log_info("Target: {}", m_config.target);
        log_info("Polarity: {}", m_config.polarity);
        log_info("NTuple output: {}", m_config.enable_ntuple ? "enabled" : "disabled");
        log_info("Histograms: {}", m_config.enable_histograms ? "enabled" : "disabled");

        m_electron_cuts->set_target(m_config.target);
        m_electron_cuts->set_polarity(m_config.polarity);

        // Histograms auto-register by member name (Boost.PFR) — no book() method.
        if (m_config.enable_histograms) {
            framework::register_fields(*m_hist, "/Electron/Kinematics/before_cuts", h_before);
            framework::register_fields(*m_hist, "/Electron/Kinematics/after_cuts",  h_after);
        }

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
        const double sampling_fraction = (calorimeter.pcal_energy + calorimeter.inner_energy + calorimeter.outer_energy) / electron.p();
        const double vz_corrected = m_electron_cuts->corrected_vz(electron.vz(), electron.p(), electron.theta(), electron.phi(), calorimeter.pcal_sector);

        if (m_config.enable_histograms) h_before.fill(electron, calorimeter, sampling_fraction, vz_corrected);

        const bool kept = 
           m_cutflow.check("Chi2 PID", m_config.enable_chi2pid_cut, [&] { return m_electron_cuts->pass_chi2pid_cut(electron.chi2pid()); })
        && m_cutflow.check("Momentum > 0.8 GeV", m_config.enable_momentum_cut, [&] { return m_electron_cuts->pass_momentum_cut(electron.p()); }) 
        && m_cutflow.check("Vertex Z", m_config.enable_vz_cut, [&] { return m_electron_cuts->pass_vz_cut(electron.vz(), electron.p(), electron.theta(), electron.phi(), calorimeter.pcal_sector); }) 
        && m_cutflow.check("Sampling Fraction", m_config.enable_sf_cut, [&] { return m_electron_cuts->pass_sampling_fraction_cut(sampling_fraction, electron.p(), calorimeter.pcal_sector); }) 
        && m_cutflow.check("PCAL Fiducial", m_config.enable_pcal_fiducial_cut, [&] { return m_electron_cuts->pass_pcal_fiducial_cut(calorimeter.pcal_lv, calorimeter.pcal_lw); }) 
        && m_cutflow.check("DC Edge", m_config.enable_dc_edge_cut, [&] { return m_electron_cuts->pass_dc_edge_cut(traj.region1_edge, traj.region2_edge, traj.region3_edge); });
        if (!kept) return StatusCode::FILTERED();

        if (m_config.enable_histograms) h_after.fill(electron, calorimeter, sampling_fraction, vz_corrected);

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

        if (m_config.enable_ntuple) {
            log_stat("N-tuple entries", m_ntuple.entries());
        }

        Log::blank();
        m_cutflow.print();

        log_success("Finalization complete");
        return StatusCode::SUCCESS();
    }

   private:
    FRAMEWORK_REFLECT_FRIEND;  // let refl-cpp reflect the private ServiceRef members

    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::ElectronCutsService> m_electron_cuts;

    struct StageHistograms {
        // Each member's NAME is its ROOT path (set by register_fields); the
        // title (axis labels) + binning live inline. No book() method.
        framework::H1Auto<> p{"Electron Momentum;p [GeV/c];Events", 0.0, 12.0, 100};
        framework::H1Auto<> theta{"Electron #theta;#theta [deg];Events", 0.0, 45.0, 90};
        framework::H1Auto<> phi{"Electron #phi;#phi [deg];Events", -180.0, 180.0, 360};
        framework::H1Auto<> pt{"Electron p_{T};p_{T} [GeV/c];Events", 0.0, 4.0, 100};
        framework::H1Auto<> sector{"Electron Sector;Sector;Events", 0.5, 7.5, 7};
        framework::H1Auto<> vz{"Vertex Z;v_{z} [cm];Events", -20.0, 5.0, 200};
        framework::H1Auto<> vz_corr{"Corrected Vertex Z;v_{z}^{corr} [cm];Events", -20.0, 5.0, 200};
        framework::H1Auto<> vx{"Vertex X;v_{x} [cm];Events", -2.0, 2.0, 100};
        framework::H1Auto<> vy{"Vertex Y;v_{y} [cm];Events", -2.0, 2.0, 100};
        framework::H1Auto<> chi2pid{"Electron #chi^{2}_{PID};#chi^{2}_{PID};Events", -10.0, 10.0, 200};
        framework::H1Auto<> pcal_lv{"PCAL Local V;PCAL_{lv} [cm];Events", 0.0, 450.0, 100};
        framework::H1Auto<> pcal_lw{"PCAL Local W;PCAL_{lw} [cm];Events", 0.0, 450.0, 100};
        framework::H2Auto<> sf_vs_p{"SF vs p;p [GeV/c];SF", 0.0, 12.0, 100, 0.0, 0.5, 100};
        framework::H2Auto<> chi2pid_vs_p{"#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}", 0.0, 12.0, 100, -10.0, 10.0, 200};
        framework::H2Auto<> vz_vs_phi{"v_{z} vs #phi;#phi [deg];v_{z} [cm]", -180.0, 180.0, 180, -20.0, 5.0, 100};
        framework::H2Auto<> vz_vs_theta{"v_{z} vs #theta;#theta [deg];v_{z} [cm]", 0.0, 45.0, 90, -20.0, 5.0, 100};
        framework::H2Auto<> vz_corr_vs_phi{"v_{z}^{corr} vs #phi;#phi [deg];v_{z}^{corr} [cm]", -180.0, 180.0, 180, -20.0, 5.0, 100};
        framework::H2Auto<> vz_corr_vs_theta{"v_{z}^{corr} vs #theta;#theta [deg];v_{z}^{corr} [cm]", 0.0, 45.0, 90, -20.0, 5.0, 100};
        framework::H2Auto<> theta_vs_phi{"#theta vs #phi;#phi [deg];#theta [deg]", -180.0, 180.0, 180, 0.0, 45.0, 90};
        framework::H2Auto<> p_vs_theta{"p vs #theta;#theta [deg];p [GeV/c]", 0.0, 45.0, 90, 0.0, 12.0, 100};

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

    // NTuple row — plain aggregate; branches reflected via Boost.PFR.
    struct ElectronNTupleData {
        double e_p = 0.0;
        double e_theta = 0.0;
        double e_phi = 0.0;
        double e_vz = 0.0;
        double e_chi2pid = 0.0;
        int e_sector = 0;
    };

    static_assert(std::is_aggregate_v<ElectronNTupleData> && std::is_standard_layout_v<ElectronNTupleData>, "ElectronNTupleData must be a standard-layout aggregate for PFR reflection");

    framework::NTupleWriter<ElectronNTupleData> m_ntuple{"electron_kinematics"};
};

}  // namespace algorithms

REFL_AUTO(type(algorithms::ElectronSelectionAlg), field(m_hist), field(m_electron_cuts))
