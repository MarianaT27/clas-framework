#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/bankdefs/REC_Scintillator.hpp>
#include <clas12/bankdefs/REC_Calorimeter.hpp>
#include <clas12/bankdefs/REC_Traj.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/CandidateUtils.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/HistogramReflect.hpp>
#include <clas12/framework/core/ServiceReflect.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/services/VzCorrectorService.hpp>
#include "clas12/bankdefs/generated_readers.hpp"

namespace algorithms {

using namespace framework;

/// Configuration for PionSelectionAlg.
struct PionSelectionConfig {
    Target target = Target::LD2;
    Polarity polarity = Polarity::Outbending;
    std::vector<double> sigma_scan = {};  ///< Override `sigma_scan_values` if non-empty.

    double chi2pid_min = -10.0;
    double chi2pid_max = 10.0;
    double momentum_min = 0.3;
    double momentum_max = 10.0;
    bool enable_ntuple = true;
    bool enable_detector_region_cut = true;
    bool enable_fiducial_cuts = true;
    bool enable_chi2pid_cut = true;
    bool enable_momentum_cut = true;
    bool enable_vz_cut = true;
    bool enable_delta_t_cut = true;
    bool enable_central_pions = false;
    double delta_t_sigma = 3.0;
    std::vector<double> sigma_scan_values = {};

    // Keep an explicit config_fields() so the TOML-binding surface is preserved
    // exactly: `target`, `sigma_scan`, etc. stay deliberately unbound. (Dropping
    // it and relying on PFR would newly bind every aggregate field to TOML.)
    static constexpr auto config_fields() {
        using T = PionSelectionConfig;
        return std::make_tuple(
            field("polarity",                   &T::polarity),
            field("chi2pid_min",                &T::chi2pid_min),
            field("chi2pid_max",                &T::chi2pid_max),
            field("momentum_min",               &T::momentum_min),
            field("momentum_max",               &T::momentum_max),
            field("enable_ntuple",              &T::enable_ntuple),
            field("enable_detector_region_cut", &T::enable_detector_region_cut),
            field("enable_fiducial_cuts",       &T::enable_fiducial_cuts),
            field("enable_chi2pid_cut",         &T::enable_chi2pid_cut),
            field("enable_momentum_cut",        &T::enable_momentum_cut),
            field("enable_vz_cut",              &T::enable_vz_cut),
            field("enable_delta_t_cut",         &T::enable_delta_t_cut),
            field("enable_central_pions",       &T::enable_central_pions),
            field("delta_t_sigma",              &T::delta_t_sigma),
            field("sigma_scan_values",          &T::sigma_scan_values));
    }
};

static_assert(std::is_aggregate_v<PionSelectionConfig>, "PionSelectionConfig must stay a simple aggregate for PFR reflection");

/// Pion species tag — selects the per-species histogram label/directory at
/// compile time so one templated histogram struct serves both π⁺ and π⁻.
enum class PionSpecies { Plus, Minus };

/// Compile-time ROOT title label ("#pi^{+}" / "#pi^{-}") for a species, so one
/// templated histogram struct reproduces the per-species titles verbatim.
template <PionSpecies Sp>
inline constexpr std::string_view pion_label = (Sp == PionSpecies::Plus) ? std::string_view{"#pi^{+}"} : std::string_view{"#pi^{-}"};

/**
 * @brief N-Tuple data structure for pion kinematics
 */
struct PionNTupleData {
    double p = 0.0;
    double theta = 0.0;
    double phi = 0.0;
    double vz = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double chi2pid = 0.0;
    double beta = 0.0;
    double delta_t = 0.0;
    double tof_mass2 = 0.0;
    double tof_path = 0.0;
    int sector = 0;
    double beta_expected = 0.0;
    double beta_diff = 0.0;
    int is_forward = 0;
    double pcal_lv = 0.0;
    double pcal_lw = 0.0;

    template <typename T>
    auto from(const T& c) -> void {
        p = c.p; theta = c.theta; phi = c.phi;
        vz = c.vz; vx = c.vx; vy = c.vy;
        chi2pid = c.chi2pid; beta = c.beta; delta_t = c.delta_t;
        tof_mass2 = c.tof_mass2; tof_path = c.tof_path; sector = c.sector;
        beta_expected = c.beta_expected; beta_diff = c.beta_diff;
        is_forward = c.is_forward_detector ? 1 : 0;
        pcal_lv = c.pcal_lv; pcal_lw = c.pcal_lw;
    }

    static constexpr auto ntuple_fields() {
        using T = PionNTupleData;
        return std::make_tuple(field("p", &T::p), field("theta", &T::theta), field("phi", &T::phi), field("vz", &T::vz), field("vx", &T::vx), field("vy", &T::vy), field("chi2pid", &T::chi2pid), field("beta", &T::beta), field("delta_t", &T::delta_t), field("tof_mass2", &T::tof_mass2), field("tof_path", &T::tof_path), field("sector", &T::sector), field("beta_expected", &T::beta_expected), field("beta_diff", &T::beta_diff), field("is_forward", &T::is_forward), field("pcal_lv", &T::pcal_lv), field("pcal_lw", &T::pcal_lw));
    }
};

/**
 * @brief Reconstructed pion candidate with full kinematics and cut decisions
 */
struct PionCandidate {
    Particle pion;
    double p = 0.0;
    double theta = 0.0;
    double phi = 0.0;
    double vz = 0.0;
    double chi2pid = 0.0;
    double beta = 0.0;
    double delta_t = 0.0;
    double beta_expected = 0.0;
    int sector = 0;
    double tof_time = 0.0;
    double tof_path = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double tof_mass2 = 0.0;
    double beta_diff = 0.0;
    double pcal_lv = 0.0;
    double pcal_lw = 0.0;
    bool is_forward_detector = false;
    bool is_central_detector = false;

    bool passes_dc_fiducial = false;
    bool passes_chi2pid = false;
    bool passes_momentum = false;
    bool passes_vz = false;
    bool passes_delta_t = false;
    bool passes_all_cuts = false;
};

/**
 * @brief Pion (π⁺/π⁻) selection and analysis algorithm
 */
class PionSelectionAlg : public ConfigurableAlgorithm<PionSelectionConfig> {
   public:

    explicit PionSelectionAlg(std::string name = "PionSelection", Config config = {})
        : ConfigurableAlgorithm(std::move(name), std::move(config)), m_cutflow_piplus("π⁺ Selection"), m_cutflow_piminus("π⁻ Selection") {
        if (!m_config.sigma_scan.empty()) m_config.sigma_scan_values = m_config.sigma_scan;
        wire_services(*this);  // reflects ServiceRef members (refl-cpp)
        declare_banks_typed<bankdefs::REC_Event, bankdefs::REC_Particle, bankdefs::REC_Scintillator, bankdefs::REC_Calorimeter, bankdefs::REC_Traj>();
        declare_ntuple(m_ntuple_piplus, [this] { return m_config.enable_ntuple; });
        declare_ntuple(m_ntuple_piminus, [this] { return m_config.enable_ntuple; });
    }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        log_info("Target: {}", m_config.target == Target::LD2 ? "LD2" : "Other");
        log_info("Chi2PID range: [{}, {}], Momentum range: [{}, {}] GeV", m_config.chi2pid_min, m_config.chi2pid_max, m_config.momentum_min, m_config.momentum_max);
        log_info("Cut toggles: detector_region={}, fiducial={}, chi2pid={}, momentum={}, vz={}, delta_t={}", m_config.enable_detector_region_cut, m_config.enable_fiducial_cuts, m_config.enable_chi2pid_cut, m_config.enable_momentum_cut, m_config.enable_vz_cut, m_config.enable_delta_t_cut);
        log_info("Central pions: {}", m_config.enable_central_pions ? "enabled" : "disabled");
        log_info("Delta-t sigma: {}", m_config.delta_t_sigma);
        if (!m_config.sigma_scan_values.empty()) log_info("Sigma scan enabled with values: [{}]", fmt::join(m_config.sigma_scan_values, ", "));

        // PionSelectionConfig has no enable_histograms flag, so (as before)
        // histograms are always booked. register_fields() derives each ROOT
        // path from the member name under the directory prefix; the per-sector
        // arrays carry runtime names (_S1.._S6) and are booked by hand.
        register_fields(*m_hist, "/Pion/PiPlus/Forward",          h_piplus_fwd);
        register_fields(*m_hist, "/Pion/PiPlus/Central",          h_piplus_central);
        register_fields(*m_hist, "/Pion/PiPlus/Forward/NMinus1",  h_piplus_nm1);
        register_fields(*m_hist, "/Pion/PiMinus/Forward",         h_piminus_fwd);
        register_fields(*m_hist, "/Pion/PiMinus/Central",         h_piminus_central);
        register_fields(*m_hist, "/Pion/PiMinus/Forward/NMinus1", h_piminus_nm1);
        book_sector_histograms(h_piplus_sectors,  "/Pion/PiPlus/Forward",  "#pi^{+}");
        book_sector_histograms(h_piminus_sectors, "/Pion/PiMinus/Forward", "#pi^{-}");

        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();

        auto& rec_event = event.bank<bankdefs::REC_Event>();
        if (rec_event.rows() == 0) return StatusCode::SUCCESS();
        const double start_time = static_cast<double>(rec_event.startTime(0));

        auto piplus_candidates = build_pion_candidates(event.pions_plus(), start_time, true);
        for (auto& cand : piplus_candidates) cand.passes_all_cuts = evaluate_cutflow(cand, m_cutflow_piplus);
        fill_pre_cut_histograms(piplus_candidates, true);
        fill_nminus1_histograms(piplus_candidates, true);
        fill_post_cut_histograms(piplus_candidates, true);
        fill_ntuple(piplus_candidates, true);
        auto selected_piplus = collect_passing_particles(piplus_candidates);

        auto piminus_candidates = build_pion_candidates(event.pions_minus(), start_time, false);
        for (auto& cand : piminus_candidates) cand.passes_all_cuts = evaluate_cutflow(cand, m_cutflow_piminus);
        fill_pre_cut_histograms(piminus_candidates, false);
        fill_nminus1_histograms(piminus_candidates, false);
        fill_post_cut_histograms(piminus_candidates, false);
        fill_ntuple(piminus_candidates, false);
        auto selected_piminus = collect_passing_particles(piminus_candidates);

        event.store_typed(Keys::SelectedPiPlus, selected_piplus);
        event.store_typed(Keys::SelectedPiMinus, selected_piminus);
        event.set_decision_typed(Decisions::HasGoodPiMinus, !selected_piminus.empty());
        event.set_decision_typed(Decisions::HasGoodPiPlus, !selected_piplus.empty());

        if (!m_config.sigma_scan_values.empty()) {
            for (double scan_sigma : m_config.sigma_scan_values) {
                auto scan_piplus = collect_passing_with_sigma(piplus_candidates, true, scan_sigma);
                event.store_typed(TypedKey<std::vector<Particle>>(format_sigma_key("SelectedPiPlus", scan_sigma)), std::move(scan_piplus));
                auto scan_piminus = collect_passing_with_sigma(piminus_candidates, false, scan_sigma);
                event.store_typed(TypedKey<std::vector<Particle>>(format_sigma_key("SelectedPiMinus", scan_sigma)), std::move(scan_piminus));
            }
        }

        static constexpr std::array<std::pair<int, const char*>, 5> CUMULATIVE_STEPS = {{{0, "noPionCuts"}, {1, "plusDCFiducial"}, {2, "plusChi2PID"}, {3, "plusMomentum"}, {4, "plusVertexZ"}}};
        for (const auto& [step, label] : CUMULATIVE_STEPS) {
            event.store_typed(TypedKey<std::vector<Particle>>(fmt::format("PiPlus_{}", label)), collect_passing_cumulative(piplus_candidates, step));
            event.store_typed(TypedKey<std::vector<Particle>>(fmt::format("PiMinus_{}", label)), collect_passing_cumulative(piminus_candidates, step));
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        log_info("Finalizing pion selection algorithm");

        auto piplus_count = m_cutflow_piplus.get_count("Delta-t");
        auto piminus_count = m_cutflow_piminus.get_count("Delta-t");
        log_info("π⁺ passing all cuts: {}", piplus_count);
        log_info("π⁻ passing all cuts: {}", piminus_count);

        if (m_config.enable_ntuple) {
            log_info("N-tuple entries: π⁺={}, π⁻={}", m_ntuple_piplus.entries(), m_ntuple_piminus.entries());
        }

        m_cutflow_piplus.print();
        m_cutflow_piminus.print();

        Log::print_yield_summary_table("Pion Analysis Statistics", {"Particle", "Passing All Cuts"}, {{"π⁺", fmt::format("{}", piplus_count)}, {"π⁻", fmt::format("{}", piminus_count)}});

        Log::print_yield_summary_table("Detector Region (pre-cut)", {"Region", "π⁺", "π⁻"}, {{"Forward", fmt::format("{}", m_piplus_forward.load()), fmt::format("{}", m_piminus_forward.load())}, {"Central", fmt::format("{}", m_piplus_central.load()), fmt::format("{}", m_piminus_central.load())}, {"Other", fmt::format("{}", m_piplus_other.load()), fmt::format("{}", m_piminus_other.load())}});

        auto pip_fwd = m_piplus_forward.load();
        auto pim_fwd = m_piminus_forward.load();
        auto pip_nan = m_piplus_pcal_nan.load();
        auto pim_nan = m_piminus_pcal_nan.load();
        Log::print_yield_summary_table("PCAL lv/lw NaN (forward pions)", {"", "π⁺", "π⁻"}, {{"Forward total", fmt::format("{}", pip_fwd), fmt::format("{}", pim_fwd)}, {"PCAL NaN", fmt::format("{}", pip_nan), fmt::format("{}", pim_nan)}, {"NaN fraction", pip_fwd > 0 ? fmt::format("{:.1f}%", 100.0 * pip_nan / pip_fwd) : "N/A", pim_fwd > 0 ? fmt::format("{:.1f}%", 100.0 * pim_nan / pim_fwd) : "N/A"}});

        auto sum_entries = [](const auto& slot) -> std::size_t {
            if (!slot) return 0;
            std::size_t total = 0;
            for (std::size_t i = 0; i < slot->size(); ++i) total += slot->at(i).n;
            return total;
        };
        std::vector<std::vector<std::string>> sector_rows;
        for (int sector = 1; sector <= 6; ++sector) {
            int idx = sector - 1;
            std::size_t pip = sum_entries(h_piplus_sectors.momentum_sector[idx]);
            std::size_t pim = sum_entries(h_piminus_sectors.momentum_sector[idx]);
            sector_rows.push_back({fmt::format("{}", sector), fmt::format("{}", pip), fmt::format("{}", pim)});
        }
        Log::print_yield_summary_table("Per-Sector Counts (post-cut)", {"Sector", "π⁺", "π⁻"}, sector_rows);

        return StatusCode::SUCCESS();
    }

   private:
    FRAMEWORK_REFLECT_FRIEND;  // let refl-cpp reflect the private ServiceRef members

    Cutflow m_cutflow_piplus;
    Cutflow m_cutflow_piminus;

    std::atomic<size_t> m_piplus_pcal_nan{0};
    std::atomic<size_t> m_piminus_pcal_nan{0};
    std::atomic<size_t> m_piplus_forward{0};
    std::atomic<size_t> m_piplus_central{0};
    std::atomic<size_t> m_piplus_other{0};
    std::atomic<size_t> m_piminus_forward{0};
    std::atomic<size_t> m_piminus_central{0};
    std::atomic<size_t> m_piminus_other{0};

    ServiceRef<HistogramService>    m_hist;
    ServiceRef<VzCorrectorService>  m_vz_corrector;

    NTupleWriter<PionNTupleData> m_ntuple_piplus{"pion_plus"};
    NTupleWriter<PionNTupleData> m_ntuple_piminus{"pion_minus"};

    // Forward-detector histograms (prefix "/Pion/Pi{Plus,Minus}/Forward"). The
    // member name reproduces the original leaf; titles carry the species label.
    // Pure H*Auto aggregate so register_fields()/Boost.PFR walks it cleanly; the
    // runtime-named per-sector histograms live in a separate, non-reflected
    // struct (ForwardSectorHistograms) booked by book_sector_histograms().
    template <PionSpecies Sp>
    struct ForwardHistograms {
        static constexpr std::string_view L = pion_label<Sp>;

        H1Auto momentum{std::string(L) + " Forward Momentum;Momentum [GeV/c];Counts", 0.0, 11.0, 100};
        H1Auto theta{std::string(L) + " Forward #theta;#theta [deg];Counts", 0.0, 180.0, 180};
        H1Auto phi{std::string(L) + " Forward #phi;#phi [deg];Counts", -180.0, 180.0, 360};
        H1Auto vz{std::string(L) + " Forward v_{z};v_{z} [cm];Events", -40, 40, 200};
        H1Auto chi2pid{std::string(L) + " Forward #chi^{2}_{PID};#chi^{2};Events", -10, 10, 200};
        H1Auto beta{std::string(L) + " Forward #beta;#beta;Events", 0.7, 1.05, 100};

        H1Auto momentum_precut{std::string(L) + " Forward Momentum (pre-cut);Momentum [GeV/c];Counts", 0.0, 11.0, 100};
        H1Auto theta_precut{std::string(L) + " Forward #theta (pre-cut);#theta [deg];Counts", 0.0, 180.0, 180};
        H1Auto phi_precut{std::string(L) + " Forward #phi (pre-cut);#phi [deg];Counts", -180.0, 180.0, 360};
        H1Auto vz_precut{std::string(L) + " Forward v_{z} (pre-cut);v_{z} [cm];Events", -40, 40, 200};
        H1Auto chi2pid_precut{std::string(L) + " Forward #chi^{2}_{PID} (pre-cut);#chi^{2};Events", -10, 10, 200};
        H1Auto delta_t_precut{std::string(L) + " Forward #Deltat (pre-cut);#Deltat [ns];Events", -2, 2, 200};

        H1Auto pcal_lv_precut{std::string(L) + " Forward PCAL lv (pre-cut);lv [cm];Events", 0, 450, 200};
        H1Auto pcal_lw_precut{std::string(L) + " Forward PCAL lw (pre-cut);lw [cm];Events", 0, 450, 200};
        H2Auto pcal_lw_vs_lv{std::string(L) + " Forward PCAL lw vs lv (pre-cut);lv [cm];lw [cm]", 0, 450, 200, 0, 450, 200};

        H2Auto beta_vs_p{std::string(L) + " Forward #beta vs p;p [GeV/c];#beta", 0, 10, 100, 0.7, 1.05, 100};
        H2Auto delta_t_vs_p{std::string(L) + " Forward #Deltat vs p;p [GeV/c];#Deltat [ns]", 0, 10, 100, -2, 2, 1000};
        H2Auto p_vs_theta{std::string(L) + " Forward p vs #theta", 0, 50, 200, 0, 10, 200};

        H2Auto chi2pid_vs_p{std::string(L) + " Forward #chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}", 0, 10, 100, -10, 10, 100};
        H2Auto vz_vs_theta{std::string(L) + " Forward v_{z} vs #theta;#theta [deg];v_{z} [cm]", 0, 45, 90, -40, 40, 100};
        H2Auto delta_t_vs_theta{std::string(L) + " Forward #Deltat vs #theta;#theta [deg];#Deltat [ns]", 0, 45, 90, -2, 2, 100};
        H2Auto delta_t_vs_p_precut{std::string(L) + " Forward #Deltat vs p (pre-cut);p [GeV/c];#Deltat [ns]", 0, 10, 100, -2, 2, 1000};

        H2Auto beta_diff_vs_p{std::string(L) + " Forward #Delta#beta vs p;p [GeV/c];#beta_{meas} - #beta_{exp}", 0, 10, 100, -0.1, 0.1, 200};
        H1Auto tof_mass2{std::string(L) + " Forward TOF m^{2};m^{2} [GeV^{2}];Events", -0.5, 1.5, 200};
        H1Auto tof_mass2_precut{std::string(L) + " Forward TOF m^{2} (pre-cut);m^{2} [GeV^{2}];Events", -0.5, 1.5, 200};
    };

    // Per-sector (S1..S6) forward histograms. Their ROOT names are built at
    // runtime, so they cannot be reflected; kept OUT of the register_fields()
    // structs and booked by hand in book_sector_histograms().
    struct ForwardSectorHistograms {
        std::array<std::unique_ptr<H1<>>, 6> momentum_sector = {};
        std::array<std::unique_ptr<H1<>>, 6> theta_sector = {};
        std::array<std::unique_ptr<H2<>>, 6> delta_t_vs_p_sector = {};
        std::array<std::unique_ptr<H2<>>, 6> delta_t_vs_p_precut_sector = {};
    };

    // Central-detector histograms (prefix "/Pion/Pi{Plus,Minus}/Central").
    template <PionSpecies Sp>
    struct CentralHistograms {
        static constexpr std::string_view L = pion_label<Sp>;

        H1Auto momentum{std::string(L) + " Central Momentum;Momentum [GeV/c];Counts", 0.0, 11.0, 100};
        H1Auto theta{std::string(L) + " Central #theta;#theta [deg];Counts", 0.0, 180.0, 180};
        H1Auto phi{std::string(L) + " Central #phi;#phi [deg];Counts", -180.0, 180.0, 360};
        H1Auto vz{std::string(L) + " Central v_{z};v_{z} [cm];Events", -40, 40, 200};
        H1Auto chi2pid{std::string(L) + " Central #chi^{2}_{PID};#chi^{2};Events", -10, 10, 200};
        H1Auto beta{std::string(L) + " Central #beta;#beta;Events", 0.0, 1.2, 100};
        H1Auto delta_t{std::string(L) + " Central #Deltat;#Deltat [ns];Events", -2, 2, 200};
        H2Auto beta_vs_p{std::string(L) + " Central #beta vs p;p [GeV/c];#beta", 0, 10, 100, 0.0, 1.2, 100};
        H2Auto delta_t_vs_p{std::string(L) + " Central #Deltat vs p;p [GeV/c];#Deltat [ns]", 0, 10, 100, -2, 2, 1000};
        H2Auto p_vs_theta{std::string(L) + " Central p vs #theta;#theta [deg];p [GeV/c]", 0, 140, 200, 0, 5, 200};
    };

    // N-1 histograms (prefix "/Pion/Pi{Plus,Minus}/Forward/NMinus1").
    template <PionSpecies Sp>
    struct NMinus1Histograms {
        static constexpr std::string_view L = pion_label<Sp>;

        H1Auto chi2pid{std::string(L) + " Forward #chi^{2}_{PID} (N-1);#chi^{2};Events", -10, 10, 200};
        H1Auto momentum{std::string(L) + " Forward Momentum (N-1);Momentum [GeV/c];Counts", 0.0, 11.0, 100};
        H1Auto vz{std::string(L) + " Forward v_{z} (N-1);v_{z} [cm];Events", -40, 40, 200};
        H2Auto delta_t_vs_p{std::string(L) + " Forward #Deltat vs p (N-1);p [GeV/c];#Deltat [ns]", 0, 10, 100, -2, 2, 200};
        H2Auto beta_vs_p{std::string(L) + " Forward #beta vs p (N-1);p [GeV/c];#beta", 0, 10, 100, 0.7, 1.05, 100};
    };

    ForwardHistograms<PionSpecies::Plus>   h_piplus_fwd;
    CentralHistograms<PionSpecies::Plus>   h_piplus_central;
    NMinus1Histograms<PionSpecies::Plus>   h_piplus_nm1;
    ForwardHistograms<PionSpecies::Minus>  h_piminus_fwd;
    CentralHistograms<PionSpecies::Minus>  h_piminus_central;
    NMinus1Histograms<PionSpecies::Minus>  h_piminus_nm1;
    ForwardSectorHistograms                h_piplus_sectors;
    ForwardSectorHistograms                h_piminus_sectors;

    /// Pass cand through the cutflow stages, incrementing counters and
    /// returning whether all enabled cuts pass. Expressed as a short-circuit
    /// chain of Cutflow::check (same names/order/predicates/flags as before:
    /// each stage counts only when reached and only when its cut passes).
    auto evaluate_cutflow(const PionCandidate& c, Cutflow& cf) const -> bool {
        cf.pass("All Pions");
        return cf.check("Detector Region", m_config.enable_detector_region_cut, [&] { return c.is_forward_detector || (c.is_central_detector && m_config.enable_central_pions); })
            && cf.check("DC Fiducial", m_config.enable_fiducial_cuts, [&] { return !c.is_forward_detector || c.passes_dc_fiducial; })
            && cf.check("Chi2 PID", m_config.enable_chi2pid_cut, [&] { return c.passes_chi2pid; })
            && cf.check("Momentum", m_config.enable_momentum_cut, [&] { return c.passes_momentum; })
            && cf.check("Vertex Z", m_config.enable_vz_cut, [&] { return c.passes_vz; })
            && cf.check("Delta-t", m_config.enable_delta_t_cut, [&] { return !c.is_forward_detector || c.passes_delta_t; });
    }

    /// Book the per-sector (S1..S6) forward histograms by hand: their ROOT names
    /// are built at runtime, so register_fields() cannot derive them.
    auto book_sector_histograms(ForwardSectorHistograms& h, const std::string& fwd, const std::string& label) -> void {
        auto* svc = m_hist.get();
        for (int sector = 1; sector <= 6; ++sector) {
            std::string sec_suffix = fmt::format("_S{}", sector);
            h.momentum_sector[sector - 1] = std::make_unique<H1<>>(mh::regular(0.0, 11.0, 100));
            svc->register_hist(fwd + "/momentum" + sec_suffix, std::string{fmt::format("{} Forward Momentum (Sector {})", label, sector)} + ";Momentum [GeV/c];Counts", *h.momentum_sector[sector - 1]);
            h.theta_sector[sector - 1] = std::make_unique<H1<>>(mh::regular(0.0, 180.0, 180));
            svc->register_hist(fwd + "/theta" + sec_suffix, std::string{fmt::format("{} Forward #theta (Sector {})", label, sector)} + ";#theta [deg];Counts", *h.theta_sector[sector - 1]);
            h.delta_t_vs_p_sector[sector - 1] = std::make_unique<H2<>>(mh::regular(0, 10, 100), mh::regular(-2, 2, 200));
            svc->register_hist(fwd + "/delta_t_vs_p" + sec_suffix, fmt::format("{} Forward #Deltat vs p (Sector {});p [GeV/c];#Deltat [ns]", label, sector), *h.delta_t_vs_p_sector[sector - 1]);
            h.delta_t_vs_p_precut_sector[sector - 1] = std::make_unique<H2<>>(mh::regular(0, 10, 100), mh::regular(-2, 2, 200));
            svc->register_hist(fwd + "/delta_t_vs_p_precut" + sec_suffix, fmt::format("{} Forward #Deltat vs p pre-cut (Sector {});p [GeV/c];#Deltat [ns]", label, sector), *h.delta_t_vs_p_precut_sector[sector - 1]);
        }
    }

    auto build_pion_candidates(const std::vector<Particle>& pions, double start_time, bool is_piplus) -> std::vector<PionCandidate> {
        std::vector<PionCandidate> candidates;
        if (pions.empty()) return candidates;

        for (const auto& pion : pions) {
            if (is_piplus && pion.pdg() != PIDs::PIPLUS) continue;
            if (!is_piplus && pion.pdg() != PIDs::PIMINUS) continue;

            PionCandidate cand;
            cand.pion = pion;
            cand.p = pion.p();
            cand.theta = pion.theta();
            cand.phi = pion.phi();
            cand.vz = pion.vz();
            cand.vx = pion.vx();
            cand.vy = pion.vy();
            cand.chi2pid = pion.chi2pid();
            cand.beta = pion.beta();

            cand.is_forward_detector = CandidateUtils::is_forward_detector(pion.status());
            cand.is_central_detector = CandidateUtils::is_central_detector(pion.status());
            if (is_piplus) {
                if (cand.is_forward_detector) m_piplus_forward++;
                else if (cand.is_central_detector) m_piplus_central++;
                else m_piplus_other++;
            } else {
                if (cand.is_forward_detector) m_piminus_forward++;
                else if (cand.is_central_detector) m_piminus_central++;
                else m_piminus_other++;
            }

            if (cand.is_forward_detector) {
                ScintillatorBank scintillator = pion.scintillator();
                cand.sector = scintillator.sector;
                cand.tof_time = scintillator.time;
                cand.tof_path = scintillator.path;
            }

            auto timing = CandidateUtils::compute_timing(cand.p, Constants::PIMASS, cand.beta, cand.tof_time, cand.tof_path, start_time);
            cand.beta_expected = timing.beta_expected;
            cand.delta_t = timing.delta_t;
            cand.tof_mass2 = timing.tof_mass2;
            cand.beta_diff = timing.beta_diff;

            auto calo = pion.calo();
            auto traj = pion.traj();

            cand.pcal_lv = calo.pcal_lv;
            cand.pcal_lw = calo.pcal_lw;
            if (cand.is_forward_detector && (std::isnan(cand.pcal_lv) || std::isnan(cand.pcal_lw))) {
                if (is_piplus) m_piplus_pcal_nan++;
                else m_piminus_pcal_nan++;
            }

            cand.passes_dc_fiducial = pass_dc_fiducial_cut(traj, is_piplus);
            cand.passes_chi2pid = pass_chi2pid_cut(cand.chi2pid);
            cand.passes_momentum = pass_momentum_cut(cand.p);
            cand.passes_vz = pass_vz_cut(cand, is_piplus);
            cand.passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t, is_piplus);

            candidates.push_back(std::move(cand));
        }

        return candidates;
    }

    // The π⁺ and π⁻ histogram structs are distinct template instantiations (so
    // each can carry its own species label as a compile-time title and stay a
    // PFR-walkable aggregate with no base class). The fill helpers are therefore
    // templated on the concrete struct types and dispatched by is_piplus.
    template <typename FwdH, typename CentralH>
    auto fill_pre_cut_histograms_impl(const std::vector<PionCandidate>& candidates, FwdH& fwd, CentralH& central, ForwardSectorHistograms& sec) -> void {
        for (const auto& cand : candidates) {
            if (cand.is_forward_detector) {
                fwd.momentum_precut.fill(cand.p);
                fwd.theta_precut.fill(cand.theta);
                fwd.phi_precut.fill(cand.phi);
                fwd.vz_precut.fill(cand.vz);
                fwd.chi2pid_precut.fill(cand.chi2pid);
                if (cand.delta_t != 0.0) fwd.delta_t_precut.fill(cand.delta_t);
                fwd.pcal_lv_precut.fill(cand.pcal_lv);
                fwd.pcal_lw_precut.fill(cand.pcal_lw);
                fwd.pcal_lw_vs_lv.fill(cand.pcal_lv, cand.pcal_lw);
                fwd.chi2pid_vs_p.fill(cand.p, cand.chi2pid);
                fwd.vz_vs_theta.fill(cand.theta, cand.vz);
                if (cand.delta_t != 0.0) fwd.delta_t_vs_theta.fill(cand.theta, cand.delta_t);
                if (cand.delta_t != 0.0) fwd.delta_t_vs_p_precut.fill(cand.p, cand.delta_t);
                if (cand.sector >= 1 && cand.sector <= 6) {
                    int idx = cand.sector - 1;
                    if (cand.delta_t != 0.0) sec.delta_t_vs_p_precut_sector[idx]->fill(cand.p, cand.delta_t);
                }

                fwd.beta_diff_vs_p.fill(cand.p, cand.beta_diff);
                fwd.tof_mass2_precut.fill(cand.tof_mass2);
            }

            if (cand.is_central_detector) {
                central.momentum.fill(cand.p);
                central.theta.fill(cand.theta);
                central.phi.fill(cand.phi);
                central.vz.fill(cand.vz);
                central.chi2pid.fill(cand.chi2pid);
                central.beta.fill(cand.beta);
                if (cand.delta_t != 0.0) central.delta_t.fill(cand.delta_t);
                central.beta_vs_p.fill(cand.p, cand.beta);
                if (cand.delta_t != 0.0) central.delta_t_vs_p.fill(cand.p, cand.delta_t);
                central.p_vs_theta.fill(cand.theta, cand.p);
            }
        }
    }

    auto fill_pre_cut_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        if (is_piplus) fill_pre_cut_histograms_impl(candidates, h_piplus_fwd, h_piplus_central, h_piplus_sectors);
        else           fill_pre_cut_histograms_impl(candidates, h_piminus_fwd, h_piminus_central, h_piminus_sectors);
    }

    template <typename FwdH>
    auto fill_post_cut_histograms_impl(const std::vector<PionCandidate>& candidates, FwdH& fwd, ForwardSectorHistograms& sec) -> void {
        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;
            fwd.momentum.fill(cand.p);
            fwd.theta.fill(cand.theta);
            fwd.phi.fill(cand.phi);
            fwd.vz.fill(cand.vz);
            fwd.chi2pid.fill(cand.chi2pid);
            fwd.beta.fill(cand.beta);
            fwd.beta_vs_p.fill(cand.p, cand.beta);
            if (cand.delta_t != 0.0) fwd.delta_t_vs_p.fill(cand.p, cand.delta_t);
            fwd.p_vs_theta.fill(cand.theta, cand.p);
            fwd.tof_mass2.fill(cand.tof_mass2);
            if (cand.sector >= 1 && cand.sector <= 6) {
                int idx = cand.sector - 1;
                sec.momentum_sector[idx]->fill(cand.p);
                sec.theta_sector[idx]->fill(cand.theta);
                if (cand.delta_t != 0.0) sec.delta_t_vs_p_sector[idx]->fill(cand.p, cand.delta_t);
            }
        }
    }

    auto fill_post_cut_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        if (is_piplus) fill_post_cut_histograms_impl(candidates, h_piplus_fwd, h_piplus_sectors);
        else           fill_post_cut_histograms_impl(candidates, h_piminus_fwd, h_piminus_sectors);
    }

    auto fill_ntuple(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& ntuple = is_piplus ? m_ntuple_piplus : m_ntuple_piminus;
        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;
            ntuple.data().from(cand);
            ntuple.fill();  // self-gates when the writer is disabled
        }
    }

    auto passes_all_except_pion(const PionCandidate& c, int skip) const -> bool {
        bool pass_det = (skip == 0) || !m_config.enable_detector_region_cut || c.is_forward_detector || (c.is_central_detector && m_config.enable_central_pions);
        bool pass_fid = (skip == 1) || !m_config.enable_fiducial_cuts || !c.is_forward_detector || c.passes_dc_fiducial;
        bool pass_chi2 = (skip == 2) || !m_config.enable_chi2pid_cut || c.passes_chi2pid;
        bool pass_mom = (skip == 3) || !m_config.enable_momentum_cut || c.passes_momentum;
        bool pass_vz = (skip == 4) || !m_config.enable_vz_cut || c.passes_vz;
        bool pass_dt = (skip == 5) || !m_config.enable_delta_t_cut || !c.is_forward_detector || c.passes_delta_t;
        return pass_det && pass_fid && pass_chi2 && pass_mom && pass_vz && pass_dt;
    }

    template <typename NM1H>
    auto fill_nminus1_histograms_impl(const std::vector<PionCandidate>& candidates, NM1H& nm1) -> void {
        for (const auto& cand : candidates) {
            if (!cand.is_forward_detector) continue;
            if (passes_all_except_pion(cand, 2)) nm1.chi2pid.fill(cand.chi2pid);
            if (passes_all_except_pion(cand, 3)) nm1.momentum.fill(cand.p);
            if (passes_all_except_pion(cand, 4)) nm1.vz.fill(cand.vz);
            if (passes_all_except_pion(cand, 5)) {
                if (cand.delta_t != 0.0) nm1.delta_t_vs_p.fill(cand.p, cand.delta_t);
                nm1.beta_vs_p.fill(cand.p, cand.beta);
            }
        }
    }

    auto fill_nminus1_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        if (is_piplus) fill_nminus1_histograms_impl(candidates, h_piplus_nm1);
        else           fill_nminus1_histograms_impl(candidates, h_piminus_nm1);
    }

    auto collect_passing_particles(const std::vector<PionCandidate>& candidates) -> std::vector<Particle> {
        std::vector<Particle> selected;
        for (const auto& cand : candidates) {
            if (cand.passes_all_cuts) selected.push_back(cand.pion);
        }
        return selected;
    }

    auto collect_passing_cumulative(const std::vector<PionCandidate>& candidates, int max_step) const -> std::vector<Particle> {
        std::vector<Particle> selected;
        for (const auto& cand : candidates) {
            if (!cand.is_forward_detector) continue;
            if (max_step >= 1 && m_config.enable_fiducial_cuts && !cand.passes_dc_fiducial) continue;
            if (max_step >= 2 && m_config.enable_chi2pid_cut && !cand.passes_chi2pid) continue;
            if (max_step >= 3 && m_config.enable_momentum_cut && !cand.passes_momentum) continue;
            if (max_step >= 4 && m_config.enable_vz_cut && !cand.passes_vz) continue;
            if (max_step >= 5 && m_config.enable_delta_t_cut && cand.is_forward_detector && !cand.passes_delta_t) continue;
            selected.push_back(cand.pion);
        }
        return selected;
    }

    static auto delta_t_mean(double p, double a, double b, double c) -> double {
        return a * p * p + b * p + c;
    }

    // σ(p) = sqrt(σ0² + (k/p)²) + c_σ·p — physically motivated timing-resolution model:
    // σ0 is the intrinsic detector resolution, (k/p) the multiple-scattering /
    // nuclear-interaction contribution that dominates at low momentum, and c_σ·p
    // a small linear high-p term.
    static auto delta_t_sigma(double p, double sigma0, double k, double c_sigma) -> double {
        const double inv_p = k / p;
        return std::sqrt(sigma0 * sigma0 + inv_p * inv_p) + c_sigma * p;
    }

    struct DeltaTCoefficients {
        std::array<double, 3> mean;   // (a_μ, b_μ, c_μ)
        std::array<double, 3> sigma;  // (σ0, k, c_σ)
    };

    auto get_piplus_delta_t_coefficients() const -> DeltaTCoefficients {
        switch (m_config.target) {
            case Target::LD2: return {{ 0.000944, -0.010445,  0.046100}, {0.076738, 0.054354, -0.000929}};
            case Target::CxC: return {{ 0.001103, -0.011081,  0.046219}, {0.075224, 0.054430, -0.000447}};
            case Target::Cu:  return {{ 0.001381, -0.013593,  0.049945}, {0.072581, 0.059173,  0.000113}};
            case Target::Sn:  return {{ 0.001150, -0.010677,  0.041478}, {0.076579, 0.050554, -0.000595}};
            default:          return {{ 0.000944, -0.010445,  0.046100}, {0.076738, 0.054354, -0.000929}};
        }
    }

    auto get_piminus_delta_t_coefficients() const -> DeltaTCoefficients {
        switch (m_config.target) {
            case Target::LD2: return {{-0.001640,  0.009816, -0.025958}, {0.080510, 0.118046, -0.001205}};
            case Target::CxC: return {{-0.001866,  0.010890, -0.026872}, {0.078231, 0.118102, -0.000447}};
            case Target::Cu:  return {{-0.001433,  0.008117, -0.022846}, {0.076203, 0.117692,  0.000187}};
            case Target::Sn:  return {{-0.001777,  0.010759, -0.029889}, {0.075510, 0.120755,  0.000220}};
            default:          return {{-0.001640,  0.009816, -0.025958}, {0.080510, 0.118046, -0.001205}};
        }
    }

    struct VzCutRange {
        double min;
        double max;
        [[nodiscard]] auto contains(double vz) const -> bool { return vz >= min && vz <= max; }
    };

    auto get_piplus_vz_range() const -> VzCutRange {
        switch (m_config.target) {
            case Target::LD2: return {-15.00, 5.00};
            case Target::CxC: return {-9.75, 5.00};
            case Target::Cu: return {-9.21, -5.80};
            case Target::Sn: return {-4.83, -0.34};
            default: return {-15.00, 5.00};
        }
    }

    auto get_piminus_vz_range() const -> VzCutRange {
        switch (m_config.target) {
            case Target::LD2: return {-15.00, 5.00};
            case Target::CxC: return {-11.69, 5.00};
            case Target::Cu: return {-11.57, -5.57};
            case Target::Sn: return {-6.52, -0.61};
            default: return {-15.00, 5.00};
        }
    }

    static auto pass_dc_fiducial_cut(const DCTrajBank& traj, bool is_piplus) -> bool {
        double r1_min = is_piplus ? 3.875 : 2.125;
        double r2_min = is_piplus ? 4.375 : 3.625;
        double r3_min = is_piplus ? 2.625 : 9.625;
        return traj.region1_edge > r1_min && traj.region2_edge > r2_min && traj.region3_edge > r3_min;
    }

    auto pass_chi2pid_cut(double chi2pid) -> bool {
        return m_config.chi2pid_min < chi2pid && chi2pid < m_config.chi2pid_max;
    }

    auto pass_momentum_cut(double p) -> bool {
        return p > m_config.momentum_min && p < m_config.momentum_max;
    }

    auto pass_vz_cut(const PionCandidate& cand, bool is_piplus) const -> bool {
        // Forward-detector tracks with a valid sector go through the
        // per-event Voigt correction (or the legacy LD2 window when the
        // corrector is configured for LD2).
        if (cand.is_forward_detector && cand.sector >= 1 && cand.sector <= 6 && m_vz_corrector) {
            return m_vz_corrector->pass_vz_cut(cand.vz, cand.p, cand.theta, cand.phi, cand.sector);
        }
        // Central or unknown detector region: fall back to the legacy
        // hard-coded ranges (they're already tuned per species).
        auto vz_range = is_piplus ? get_piplus_vz_range() : get_piminus_vz_range();
        return vz_range.contains(cand.vz);
    }

    auto pass_delta_t_cut(const double p, const double delta_t, bool is_piplus, double n_sigma) -> bool {
        auto coeffs = is_piplus ? get_piplus_delta_t_coefficients() : get_piminus_delta_t_coefficients();
        double mean  = delta_t_mean (p, coeffs.mean[0],  coeffs.mean[1],  coeffs.mean[2]);
        double sigma = delta_t_sigma(p, coeffs.sigma[0], coeffs.sigma[1], coeffs.sigma[2]);
        double cut_low  = mean - n_sigma * sigma;
        double cut_high = mean + n_sigma * sigma;
        return cut_low < delta_t && delta_t < cut_high;
    }

    auto pass_delta_t_cut(const double p, const double delta_t, bool is_piplus) -> bool {
        return pass_delta_t_cut(p, delta_t, is_piplus, m_config.delta_t_sigma);
    }

    static auto format_sigma_label(double sigma) -> std::string { return fmt::format("sigma_{:.1f}", sigma); }
    static auto format_sigma_key(const std::string& base, double sigma) -> std::string { return fmt::format("{}_{}", base, format_sigma_label(sigma)); }

    auto collect_passing_with_sigma(const std::vector<PionCandidate>& candidates, bool is_piplus, double n_sigma) -> std::vector<Particle> {
        std::vector<Particle> selected;
        for (const auto& cand : candidates) {
            bool scan_passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t, is_piplus, n_sigma);
            bool passes_detector_region = !m_config.enable_detector_region_cut || cand.is_forward_detector || (cand.is_central_detector && m_config.enable_central_pions);
            bool passes_fiducial = !m_config.enable_fiducial_cuts || !cand.is_forward_detector || cand.passes_dc_fiducial;
            bool passes_timing = !m_config.enable_delta_t_cut || (cand.is_forward_detector ? scan_passes_delta_t : true);
            bool passes_all = passes_detector_region && passes_fiducial && (!m_config.enable_chi2pid_cut || cand.passes_chi2pid) && (!m_config.enable_momentum_cut || cand.passes_momentum) && (!m_config.enable_vz_cut || cand.passes_vz) && passes_timing;
            if (passes_all) selected.push_back(cand.pion);
        }
        return selected;
    }
};

}  // namespace algorithms

REFL_AUTO(type(algorithms::PionSelectionAlg), field(m_hist), field(m_vz_corrector))
