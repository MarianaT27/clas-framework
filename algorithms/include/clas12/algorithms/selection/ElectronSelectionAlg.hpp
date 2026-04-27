#pragma once

/**
 * @file ElectronSelectionAlg.hpp
 * @brief Algorithm for electron selection and kinematics analysis
 *
 * This algorithm selects good electrons from CLAS12 data using:
 * - Particle identification cuts (chi2pid)
 * - Kinematic cuts (momentum, vertex z)
 * - Sampling fraction cuts (calorimeter response)
 * - Fiducial cuts (PCAL lv/lw and DC region edge thresholds — applied
 *   inline against the algorithm's `pcal_*_min` / `dc_r*_edge_min`
 *   properties; tunable via the [ElectronSelectionAlg] TOML section).
 *
 * Uses modern framework patterns:
 * - ContextCutflow for combined cut definition and tracking
 * - NTupleWriter for struct-based N-tuple management
 *
 * @author CLAS12 Analysis Framework
 */

#include <optional>
#include <vector>

#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Histogram.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>
#include "clas12/bankdefs/generated_readers.hpp"

namespace clas12::algorithms {

using clas12::framework::Log;

/**
 * @brief Algorithm for electron selection and kinematics
 *
 * This algorithm implements a complete electron selection pipeline for
 * CLAS12 forward detector data. It combines:
 *
 * **Particle ID Cuts:**
 * - Chi2PID within configurable range (default: [-5, 5])
 *
 * **Kinematic Cuts:**
 * - Minimum momentum (default: 0.8 GeV)
 * - Vertex z within target-specific range
 *
 * **Calorimeter Cuts:**
 * - Sampling fraction within n-sigma band (default: 3.5 sigma)
 *
 * **Fiducial Cuts (inline):**
 * - PCAL lv/lw coordinates (default: > 9 cm; tunable via `pcal_lv_min` / `pcal_lw_min`)
 * - DC edge distances for regions 1, 2, 3 (defaults: 1.68 / 2.0 / 8.75 cm)
 *
 * The algorithm outputs:
 * - Histograms of electron kinematics (before/after cuts)
 * - Optional N-tuple with per-electron data
 * - Cutflow summary showing cut efficiencies
 *
 * @see ElectronCutsService for PID and kinematic cut configuration
 */
class ElectronSelectionAlg : public clas12::framework::Algorithm {
    using StatusCode = clas12::framework::StatusCode;

   public:
    // ========================================================================
    // Construction
    // ========================================================================

    /**
     * @brief Construct electron selection algorithm
     *
     * @param name Algorithm name (default: "ElectronSelectionAlg")
     * @param target Target material (affects vertex z cuts)
     * @param polarity Torus polarity (affects sampling fraction)
     */
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", clas12::Target target = clas12::Target::LD2, clas12::Polarity polarity = clas12::Polarity::Outbending) : Algorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Electron Selection") {
        required_services(m_hist, m_ntuple_svc, m_electron_cuts);
        declare_properties(m_enable_ntuple, m_enable_chi2pid_cut, m_enable_momentum_cut, m_enable_vz_cut, m_enable_sf_cut, m_enable_pcal_fiducial_cut, m_enable_dc_edge_cut, m_pcal_lv_min, m_pcal_lw_min, m_dc_r1_edge_min, m_dc_r2_edge_min, m_dc_r3_edge_min);
        declare_banks("REC::Particle", "REC::Calorimeter", "REC::Traj");
    }

    /**
     * @brief Initialize algorithm after services are injected
     */
    auto on_initialize() -> StatusCode override {

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Target", clas12::to_string(m_target));
        Log::info(name(), "Polarity", clas12::to_string(m_polarity));
        Log::info(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");

        m_electron_cuts->set_target(m_target);
        m_electron_cuts->set_polarity(m_polarity);

        // Book histograms
        book_histograms();

        // Setup N-Tuple
        m_ntuple.init(m_ntuple_svc.get(), "electron_kinematics");

        // Setup cutflow
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

        // Log configuration summary
        auto vz_range = framework::ElectronCutsService::get_vz_range(clas12::RunGroup::RG_D, m_target);
        Log::info(name(), "Vz range", fmt::format("[{:.2f}, {:.2f}] cm", vz_range.min, vz_range.max));
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Event Processing
    // ========================================================================

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> StatusCode override {
        // Select trigger electron
        if (event.electrons().empty()) return StatusCode::FILTERED();
        std::optional<clas12::Particle> trigger_e = event.find_trigger_electron();
        if (!trigger_e.has_value()) return StatusCode::FILTERED();
        clas12::Particle electron = trigger_e.value();
        m_cutflow.pass("Trigger Electron");

        CalorimeterBank calorimeter = electron.calo();
        DCTrajBank traj = electron.traj();
        double sampling_fraction = (calorimeter.pcal_energy + calorimeter.inner_energy + calorimeter.outer_energy) / electron.p();

        // Fill histograms before cuts
        h_before.fill(electron, calorimeter, sampling_fraction);

        if (m_enable_chi2pid_cut.get() && !m_electron_cuts->pass_chi2pid_cut(electron.chi2pid())) return StatusCode::FILTERED();
        m_cutflow.pass("Chi2 PID");

        if (m_enable_momentum_cut.get() && !m_electron_cuts->pass_momentum_cut(electron.p())) return StatusCode::FILTERED();
        m_cutflow.pass("Momentum > 0.8 GeV");

        if (m_enable_vz_cut.get() && !m_electron_cuts->pass_vz_cut(electron.vz())) return StatusCode::FILTERED();
        m_cutflow.pass("Vertex Z");

        if (m_enable_sf_cut.get() && !m_electron_cuts->pass_sampling_fraction_cut(sampling_fraction, electron.p(), calorimeter.pcal_sector)) return StatusCode::FILTERED();
        m_cutflow.pass("Sampling Fraction");

        if (m_enable_pcal_fiducial_cut.get() && !m_electron_cuts->pass_pcal_fiducial_cut(calorimeter.pcal_lv, calorimeter.pcal_lw)) return StatusCode::FILTERED();
        m_cutflow.pass("PCAL Fiducial");

        if (m_enable_dc_edge_cut.get() && !m_electron_cuts->pass_dc_edge_cut(traj.region1_edge, traj.region2_edge, traj.region3_edge)) return StatusCode::FILTERED();
        m_cutflow.pass("DC Edge");

        // Fill histograms after cuts
        h_after.fill(electron, calorimeter, sampling_fraction);

        // Fill N-tuple if enabled
        if (m_enable_ntuple.get()) {
            m_ntuple.data().e_p = electron.p();
            m_ntuple.data().e_theta = electron.theta();
            m_ntuple.data().e_phi = electron.phi();
            m_ntuple.data().e_vz = electron.vz();
            m_ntuple.data().e_chi2pid = electron.chi2pid();
            m_ntuple.data().e_sector = calorimeter.pcal_sector;
            m_ntuple.fill();
        }

        event.store_typed(clas12::framework::Keys::TriggerElectron, electron);
        event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, true);

        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Finalization
    // ========================================================================

    /**
     * @brief Finalization - print summaries
     */
    auto on_finalize() -> StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        // N-tuple statistics
        if (m_enable_ntuple.get() && m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("electron_kinematics");
            Log::stat(name(), "N-tuple entries", entries);
        }

        // Print cutflow
        Log::blank();
        m_cutflow.print();

        Log::success(name(), "Finalization complete");
        return StatusCode::SUCCESS();
    }

   private:
    // ========================================================================
    // Configuration
    // ========================================================================

    clas12::Target m_target;
    clas12::Polarity m_polarity;

    // Properties
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable chi2 PID cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
    clas12::framework::Property<bool> m_enable_sf_cut{"enable_sf_cut", true, "Enable sampling fraction cut"};
    clas12::framework::Property<bool> m_enable_pcal_fiducial_cut{"enable_pcal_fiducial_cut", true, "Enable PCAL fiducial cut"};
    clas12::framework::Property<bool> m_enable_dc_edge_cut{"enable_dc_edge_cut", true, "Enable DC edge cut"};

    // Fiducial-cut thresholds.
    clas12::framework::Property<double> m_pcal_lv_min{"pcal_lv_min", 9.0, "PCAL lv minimum [cm]"};
    clas12::framework::Property<double> m_pcal_lw_min{"pcal_lw_min", 9.0, "PCAL lw minimum [cm]"};
    clas12::framework::Property<double> m_dc_r1_edge_min{"dc_r1_edge_min", 1.68, "DC R1 edge minimum [cm]"};
    clas12::framework::Property<double> m_dc_r2_edge_min{"dc_r2_edge_min", 2.0, "DC R2 edge minimum [cm]"};
    clas12::framework::Property<double> m_dc_r3_edge_min{"dc_r3_edge_min", 8.75, "DC R3 edge minimum [cm]"};

    // ========================================================================
    // Services (cached in on_initialize via svc<T>())
    // ========================================================================
    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;
    clas12::framework::ServiceRef<clas12::framework::NTupleService> m_ntuple_svc;
    clas12::framework::ServiceRef<clas12::framework::ElectronCutsService> m_electron_cuts;

    // ========================================================================
    // Cutflow
    // ========================================================================
    clas12::framework::Cutflow m_cutflow;

    // ========================================================================
    // Histograms
    // ========================================================================
    struct StageHistograms {
        // Kinematics
        clas12::framework::Hist1D* p = nullptr;
        clas12::framework::Hist1D* theta = nullptr;
        clas12::framework::Hist1D* phi = nullptr;
        clas12::framework::Hist1D* pt = nullptr;
        clas12::framework::Hist1D* sector = nullptr;
        // Vertex
        clas12::framework::Hist1D* vz = nullptr;
        clas12::framework::Hist1D* vx = nullptr;
        clas12::framework::Hist1D* vy = nullptr;
        // PID
        clas12::framework::Hist1D* chi2pid = nullptr;
        // PCAL fiducial
        clas12::framework::Hist1D* pcal_lv = nullptr;
        clas12::framework::Hist1D* pcal_lw = nullptr;
        // 2D correlations
        clas12::framework::Hist2D* sf_vs_p = nullptr;
        clas12::framework::Hist2D* chi2pid_vs_p = nullptr;
        clas12::framework::Hist2D* vz_vs_phi = nullptr;
        clas12::framework::Hist2D* vz_vs_theta = nullptr;
        clas12::framework::Hist2D* theta_vs_phi = nullptr;
        clas12::framework::Hist2D* p_vs_theta = nullptr;

        auto book(clas12::framework::HistogramService* svc, const std::string& dir) -> void {
            m_group = std::make_unique<clas12::framework::HistogramGroup>(svc, dir);
            auto* g = m_group.get();

            p = &g->add_hist1D("p", "Electron Momentum;p [GeV/c];Events", 100, 0.0, 12.0);
            theta = &g->add_hist1D("theta", "Electron #theta;#theta [deg];Events", 90, 0.0, 45.0);
            phi = &g->add_hist1D("phi", "Electron #phi;#phi [deg];Events", 360, -180.0, 180.0);
            pt = &g->add_hist1D("pt", "Electron p_{T};p_{T} [GeV/c];Events", 100, 0.0, 4.0);
            sector = &g->add_hist1D("sector", "Electron Sector;Sector;Events", 7, 0.5, 7.5);
            vz = &g->add_hist1D("vz", "Vertex Z;v_{z} [cm];Events", 200, -40.0, 40.0);
            vx = &g->add_hist1D("vx", "Vertex X;v_{x} [cm];Events", 100, -2.0, 2.0);
            vy = &g->add_hist1D("vy", "Vertex Y;v_{y} [cm];Events", 100, -2.0, 2.0);
            chi2pid = &g->add_hist1D("chi2pid", "Electron #chi^{2}_{PID};#chi^{2}_{PID};Events", 200, -10.0, 10.0);
            pcal_lv = &g->add_hist1D("pcal_lv", "PCAL Local V;PCAL_{lv} [cm];Events", 100, 0.0, 450.0);
            pcal_lw = &g->add_hist1D("pcal_lw", "PCAL Local W;PCAL_{lw} [cm];Events", 100, 0.0, 450.0);

            sf_vs_p = &g->add_hist2D("sf_vs_p", "SF vs p;p [GeV/c];SF", 100, 0.0, 12.0, 100, 0.0, 0.5);
            chi2pid_vs_p = &g->add_hist2D("chi2pid_vs_p", "#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}", 100, 0.0, 12.0, 200, -10.0, 10.0);
            vz_vs_phi = &g->add_hist2D("vz_vs_phi", "v_{z} vs #phi;#phi [deg];v_{z} [cm]", 180, -180.0, 180.0, 100, -40.0, 40.0);
            vz_vs_theta = &g->add_hist2D("vz_vs_theta", "v_{z} vs #theta;#theta [deg];v_{z} [cm]", 90, 0.0, 45.0, 100, -40.0, 40.0);
            theta_vs_phi = &g->add_hist2D("theta_vs_phi", "#theta vs #phi;#phi [deg];#theta [deg]", 180, -180.0, 180.0, 90, 0.0, 45.0);
            p_vs_theta = &g->add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]", 90, 0.0, 45.0, 100, 0.0, 12.0);
        }

        auto fill(const clas12::Particle& e, const clas12::CalorimeterBank& calo, double SF) const -> void {
            p->fill(e.p());
            theta->fill(e.theta());
            phi->fill(e.phi());
            pt->fill(e.pt());
            sector->fill(calo.pcal_sector);
            vz->fill(e.vz());
            vx->fill(e.vx());
            vy->fill(e.vy());
            chi2pid->fill(e.chi2pid());

            pcal_lv->fill(calo.pcal_lv);
            pcal_lw->fill(calo.pcal_lw);

            sf_vs_p->fill(e.p(), SF);
            chi2pid_vs_p->fill(e.p(), e.chi2pid());
            vz_vs_phi->fill(e.phi(), e.vz());
            vz_vs_theta->fill(e.theta(), e.vz());
            theta_vs_phi->fill(e.phi(), e.theta());
            p_vs_theta->fill(e.theta(), e.p());
        }

       private:
        // Owns the underlying Hist1D/Hist2D handles bound above.
        std::unique_ptr<clas12::framework::HistogramGroup> m_group;
    };

    StageHistograms h_before;
    StageHistograms h_after;

    // ========================================================================
    // Helper Methods
    // ========================================================================

    /**
     * @brief Book all histograms
     */
    void book_histograms() {
        h_before.book(m_hist.get(), "/Electron/Kinematics/before_cuts");
        h_after.book(m_hist.get(), "/Electron/Kinematics/after_cuts");
    }

    /**
     * @brief N-Tuple data structure for electron kinematics
     *
     * Branches are listed in `ntuple_fields()`; NTupleWriter wires each one
     * to the matching member at init() time.
     */
    struct ElectronNTupleData {
        double e_p = 0.0;        ///< Electron momentum [GeV/c]
        double e_theta = 0.0;    ///< Electron polar angle [rad]
        double e_phi = 0.0;      ///< Electron azimuthal angle [rad]
        double e_vz = 0.0;       ///< Electron vertex z [cm]
        double e_chi2pid = 0.0;  ///< Electron chi2pid
        int e_sector = 0;        ///< PCAL sector (1-6)

        static constexpr auto ntuple_fields() {
            using T = ElectronNTupleData;
            return std::make_tuple(clas12::framework::field("e_p", &T::e_p), clas12::framework::field("e_theta", &T::e_theta), clas12::framework::field("e_phi", &T::e_phi), clas12::framework::field("e_vz", &T::e_vz), clas12::framework::field("e_chi2pid", &T::e_chi2pid), clas12::framework::field("e_sector", &T::e_sector));
        }
    };

    clas12::framework::NTupleWriter<ElectronNTupleData> m_ntuple;
};

}  // namespace clas12::algorithms
