#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/CandidateUtils.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for PionSelectionAlg.
struct PionSelectionConfig {
    Target target = Target::LD2;
    std::vector<double> sigma_scan = {};  ///< Override property `sigma_scan_values` if non-empty.
};

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
        return std::make_tuple(framework::field("p", &T::p), framework::field("theta", &T::theta), framework::field("phi", &T::phi), framework::field("vz", &T::vz), framework::field("vx", &T::vx), framework::field("vy", &T::vy), framework::field("chi2pid", &T::chi2pid), framework::field("beta", &T::beta), framework::field("delta_t", &T::delta_t), framework::field("tof_mass2", &T::tof_mass2), framework::field("tof_path", &T::tof_path), framework::field("sector", &T::sector), framework::field("beta_expected", &T::beta_expected), framework::field("beta_diff", &T::beta_diff), framework::field("is_forward", &T::is_forward), framework::field("pcal_lv", &T::pcal_lv), framework::field("pcal_lw", &T::pcal_lw));
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
class PionSelectionAlg : public framework::Algorithm {
   public:
    using Config = PionSelectionConfig;
    using StatusCode = framework::StatusCode;

    explicit PionSelectionAlg(std::string name = "PionSelection", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow_piplus("π⁺ Selection"), m_cutflow_piminus("π⁻ Selection") {
        if (!m_config.sigma_scan.empty()) m_sigma_scan_values.set(m_config.sigma_scan);
        required_services(m_hist, m_ntuple);
        declare_banks("REC::Event", "REC::Particle", "REC::Scintillator", "REC::Calorimeter", "REC::Traj");
        declare_properties(m_chi2pid_min, m_chi2pid_max, m_momentum_min, m_momentum_max, m_enable_ntuple, m_enable_detector_region_cut, m_enable_fiducial_cuts, m_enable_chi2pid_cut, m_enable_momentum_cut, m_enable_vz_cut, m_enable_delta_t_cut, m_enable_central_pions, m_delta_t_sigma, m_sigma_scan_values);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Target: {}", m_config.target == Target::LD2 ? "LD2" : "Other");
        Log::info(name(), "Chi2PID range: [{}, {}], Momentum range: [{}, {}] GeV", m_chi2pid_min.get(), m_chi2pid_max.get(), m_momentum_min.get(), m_momentum_max.get());
        Log::info(name(), "Cut toggles: detector_region={}, fiducial={}, chi2pid={}, momentum={}, vz={}, delta_t={}", m_enable_detector_region_cut.get(), m_enable_fiducial_cuts.get(), m_enable_chi2pid_cut.get(), m_enable_momentum_cut.get(), m_enable_vz_cut.get(), m_enable_delta_t_cut.get());
        Log::info(name(), "Central pions: {}", m_enable_central_pions.get() ? "enabled" : "disabled");
        Log::info(name(), "Delta-t sigma: {}", m_delta_t_sigma.get());
        if (!m_sigma_scan_values.get().empty()) Log::info(name(), "Sigma scan enabled with values: [{}]", fmt::join(m_sigma_scan_values.get(), ", "));

        for (auto* cf : {&m_cutflow_piplus, &m_cutflow_piminus}) {
            cf->add_cut("All Pions");
            cf->add_cut("Detector Region");
            cf->add_cut("DC Fiducial");
            cf->add_cut("Chi2 PID");
            cf->add_cut("Momentum");
            cf->add_cut("Vertex Z");
            cf->add_cut("Delta-t");
        }

        book_histograms();

        m_ntuple_piplus.init(m_ntuple.get(), "pion_plus");
        m_ntuple_piminus.init(m_ntuple.get(), "pion_minus");
        Log::info(name(), "N-tuple branches registered for π⁺ and π⁻");

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();

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

        event.store_typed(framework::Keys::SelectedPiPlus, selected_piplus);
        event.store_typed(framework::Keys::SelectedPiMinus, selected_piminus);
        event.set_decision_typed(framework::Decisions::HasGoodPiMinus, !selected_piminus.empty());
        event.set_decision_typed(framework::Decisions::HasGoodPiPlus, !selected_piplus.empty());

        if (!m_sigma_scan_values.get().empty()) {
            for (double scan_sigma : m_sigma_scan_values.get()) {
                auto scan_piplus = collect_passing_with_sigma(piplus_candidates, true, scan_sigma);
                event.store_typed(framework::TypedKey<std::vector<Particle>>(format_sigma_key("SelectedPiPlus", scan_sigma)), std::move(scan_piplus));
                auto scan_piminus = collect_passing_with_sigma(piminus_candidates, false, scan_sigma);
                event.store_typed(framework::TypedKey<std::vector<Particle>>(format_sigma_key("SelectedPiMinus", scan_sigma)), std::move(scan_piminus));
            }
        }

        static constexpr std::array<std::pair<int, const char*>, 5> CUMULATIVE_STEPS = {{{0, "noPionCuts"}, {1, "plusDCFiducial"}, {2, "plusChi2PID"}, {3, "plusMomentum"}, {4, "plusVertexZ"}}};
        for (const auto& [step, label] : CUMULATIVE_STEPS) {
            event.store_typed(framework::TypedKey<std::vector<Particle>>(fmt::format("PiPlus_{}", label)), collect_passing_cumulative(piplus_candidates, step));
            event.store_typed(framework::TypedKey<std::vector<Particle>>(fmt::format("PiMinus_{}", label)), collect_passing_cumulative(piminus_candidates, step));
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing pion selection algorithm");

        auto piplus_count = m_cutflow_piplus.get_count("Delta-t");
        auto piminus_count = m_cutflow_piminus.get_count("Delta-t");
        Log::info(name(), "π⁺ passing all cuts: {}", piplus_count);
        Log::info(name(), "π⁻ passing all cuts: {}", piminus_count);

        if (m_ntuple) {
            Log::info(name(), "N-tuple entries: π⁺={}, π⁻={}", m_ntuple->get_entries("pion_plus"), m_ntuple->get_entries("pion_minus"));
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
            std::size_t pip = sum_entries(h_piplus.momentum_sector[idx]);
            std::size_t pim = sum_entries(h_piminus.momentum_sector[idx]);
            sector_rows.push_back({fmt::format("{}", sector), fmt::format("{}", pip), fmt::format("{}", pim)});
        }
        Log::print_yield_summary_table("Per-Sector Counts (post-cut)", {"Sector", "π⁺", "π⁻"}, sector_rows);

        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow_piplus;
    framework::Cutflow m_cutflow_piminus;

    std::atomic<size_t> m_piplus_pcal_nan{0};
    std::atomic<size_t> m_piminus_pcal_nan{0};
    std::atomic<size_t> m_piplus_forward{0};
    std::atomic<size_t> m_piplus_central{0};
    std::atomic<size_t> m_piplus_other{0};
    std::atomic<size_t> m_piminus_forward{0};
    std::atomic<size_t> m_piminus_central{0};
    std::atomic<size_t> m_piminus_other{0};

    framework::Property<double> m_chi2pid_min{"chi2pid_min", -10.0, "Minimum chi2PID"};
    framework::Property<double> m_chi2pid_max{"chi2pid_max", 10.0, "Maximum chi2PID"};
    framework::Property<double> m_momentum_min{"momentum_min", 0.3, "Minimum pion momentum [GeV]"};
    framework::Property<double> m_momentum_max{"momentum_max", 10.0, "Maximum pion momentum [GeV]"};
    framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    framework::Property<bool> m_enable_detector_region_cut{"enable_detector_region_cut", true, "Enable detector region cut"};
    framework::Property<bool> m_enable_fiducial_cuts{"enable_fiducial_cuts", true, "Enable PCAL and DC fiducial cuts"};
    framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable chi2 PID cut"};
    framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
    framework::Property<bool> m_enable_delta_t_cut{"enable_delta_t_cut", true, "Enable delta-t cut"};
    framework::Property<bool> m_enable_central_pions{"enable_central_pions", false, "Enable central detector pion selection"};
    framework::Property<double> m_delta_t_sigma{"delta_t_sigma", 3.0, "Number of sigma for delta_t cut"};
    framework::Property<std::vector<double>> m_sigma_scan_values{"sigma_scan_values", {}, "Sigma values for delta_t scan (empty=disabled)"};

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple;

    framework::NTupleWriter<PionNTupleData> m_ntuple_piplus;
    framework::NTupleWriter<PionNTupleData> m_ntuple_piminus;

    struct PionHistograms {
        framework::H1<> momentum{mh::regular(0.0, 11.0, static_cast<std::size_t>(100))};
        framework::H1<> theta{mh::regular(0.0, 180.0, static_cast<std::size_t>(180))};
        framework::H1<> phi{mh::regular(-180.0, 180.0, static_cast<std::size_t>(360))};
        framework::H1<> vz{mh::regular(-40, 40, static_cast<std::size_t>(200))};
        framework::H1<> chi2pid{mh::regular(-10, 10, static_cast<std::size_t>(200))};
        framework::H1<> beta{mh::regular(0.7, 1.05, static_cast<std::size_t>(100))};

        framework::H1<> momentum_precut{mh::regular(0.0, 11.0, static_cast<std::size_t>(100))};
        framework::H1<> theta_precut{mh::regular(0.0, 180.0, static_cast<std::size_t>(180))};
        framework::H1<> phi_precut{mh::regular(-180.0, 180.0, static_cast<std::size_t>(360))};
        framework::H1<> vz_precut{mh::regular(-40, 40, static_cast<std::size_t>(200))};
        framework::H1<> chi2pid_precut{mh::regular(-10, 10, static_cast<std::size_t>(200))};
        framework::H1<> delta_t_precut{mh::regular(-2, 2, static_cast<std::size_t>(200))};

        framework::H2<> beta_vs_p{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(0.7, 1.05, static_cast<std::size_t>(100))};
        framework::H2<> delta_t_vs_p{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-2, 2, static_cast<std::size_t>(1000))};
        framework::H2<> p_vs_theta{mh::regular(0, 50, static_cast<std::size_t>(200)), mh::regular(0, 10, static_cast<std::size_t>(200))};

        framework::H1<> pcal_lv_precut{mh::regular(0, 450, static_cast<std::size_t>(200))};
        framework::H1<> pcal_lw_precut{mh::regular(0, 450, static_cast<std::size_t>(200))};
        framework::H2<> pcal_lw_vs_lv{mh::regular(0, 450, static_cast<std::size_t>(200)), mh::regular(0, 450, static_cast<std::size_t>(200))};

        framework::H2<> chi2pid_vs_p{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-10, 10, static_cast<std::size_t>(100))};
        framework::H2<> vz_vs_theta{mh::regular(0, 45, static_cast<std::size_t>(90)), mh::regular(-40, 40, static_cast<std::size_t>(100))};
        framework::H2<> delta_t_vs_theta{mh::regular(0, 45, static_cast<std::size_t>(90)), mh::regular(-2, 2, static_cast<std::size_t>(100))};
        framework::H2<> delta_t_vs_p_precut{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-2, 2, static_cast<std::size_t>(1000))};

        framework::H1<> momentum_central{mh::regular(0.0, 11.0, static_cast<std::size_t>(100))};
        framework::H1<> theta_central{mh::regular(0.0, 180.0, static_cast<std::size_t>(180))};
        framework::H1<> phi_central{mh::regular(-180.0, 180.0, static_cast<std::size_t>(360))};
        framework::H1<> vz_central{mh::regular(-40, 40, static_cast<std::size_t>(200))};
        framework::H1<> chi2pid_central{mh::regular(-10, 10, static_cast<std::size_t>(200))};
        framework::H1<> beta_central{mh::regular(0.0, 1.2, static_cast<std::size_t>(100))};
        framework::H1<> delta_t_central{mh::regular(-2, 2, static_cast<std::size_t>(200))};
        framework::H2<> beta_vs_p_central{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(0.0, 1.2, static_cast<std::size_t>(100))};
        framework::H2<> delta_t_vs_p_central{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-2, 2, static_cast<std::size_t>(1000))};
        framework::H2<> p_vs_theta_central{mh::regular(0, 140, static_cast<std::size_t>(200)), mh::regular(0, 5, static_cast<std::size_t>(200))};

        std::array<std::unique_ptr<framework::H1<>>, 6> momentum_sector = {};
        std::array<std::unique_ptr<framework::H1<>>, 6> theta_sector = {};
        std::array<std::unique_ptr<framework::H2<>>, 6> delta_t_vs_p_sector = {};
        std::array<std::unique_ptr<framework::H2<>>, 6> delta_t_vs_p_precut_sector = {};

        framework::H1<> chi2pid_nminus1{mh::regular(-10, 10, static_cast<std::size_t>(200))};
        framework::H1<> momentum_nminus1{mh::regular(0.0, 11.0, static_cast<std::size_t>(100))};
        framework::H1<> vz_nminus1{mh::regular(-40, 40, static_cast<std::size_t>(200))};
        framework::H2<> delta_t_vs_p_nminus1{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-2, 2, static_cast<std::size_t>(200))};
        framework::H2<> beta_vs_p_nminus1{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(0.7, 1.05, static_cast<std::size_t>(100))};

        framework::H2<> beta_diff_vs_p{mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-0.1, 0.1, static_cast<std::size_t>(200))};
        framework::H1<> tof_mass2{mh::regular(-0.5, 1.5, static_cast<std::size_t>(200))};
        framework::H1<> tof_mass2_precut{mh::regular(-0.5, 1.5, static_cast<std::size_t>(200))};
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    /// Pass cand through the cutflow stages, incrementing counters and
    /// returning whether all enabled cuts pass.
    auto evaluate_cutflow(const PionCandidate& c, framework::Cutflow& cf) const -> bool {
        cf.pass("All Pions");

        bool pass_det = !m_enable_detector_region_cut.get() || c.is_forward_detector || (c.is_central_detector && m_enable_central_pions.get());
        if (!pass_det) return false;
        cf.pass("Detector Region");

        bool pass_fid = !m_enable_fiducial_cuts.get() || !c.is_forward_detector || c.passes_dc_fiducial;
        if (!pass_fid) return false;
        cf.pass("DC Fiducial");

        if (m_enable_chi2pid_cut.get() && !c.passes_chi2pid) return false;
        cf.pass("Chi2 PID");

        if (m_enable_momentum_cut.get() && !c.passes_momentum) return false;
        cf.pass("Momentum");

        if (m_enable_vz_cut.get() && !c.passes_vz) return false;
        cf.pass("Vertex Z");

        bool pass_dt = !m_enable_delta_t_cut.get() || !c.is_forward_detector || c.passes_delta_t;
        if (!pass_dt) return false;
        cf.pass("Delta-t");

        return true;
    }

    auto book_histograms() -> void {
        auto* svc = m_hist.get();

        auto book_pion_histograms = [svc](PionHistograms& h, const std::string& dir, const std::string& label) {
            std::string fwd = dir + "/Forward";
            svc->register_hist(fwd + "/momentum", std::string{label + " Forward Momentum"} + ";Momentum [GeV/c];Counts", h.momentum);            svc->register_hist(fwd + "/theta", std::string{label + " Forward #theta"} + ";#theta [deg];Counts", h.theta);            svc->register_hist(fwd + "/phi", std::string{label + " Forward #phi"} + ";#phi [deg];Counts", h.phi);            svc->register_hist(fwd + "/vz", label + " Forward v_{z};v_{z} [cm];Events", h.vz);            svc->register_hist(fwd + "/chi2pid", label + " Forward #chi^{2}_{PID};#chi^{2};Events", h.chi2pid);            svc->register_hist(fwd + "/beta", label + " Forward #beta;#beta;Events", h.beta);
            svc->register_hist(fwd + "/momentum_precut", std::string{label + " Forward Momentum (pre-cut)"} + ";Momentum [GeV/c];Counts", h.momentum_precut);            svc->register_hist(fwd + "/theta_precut", std::string{label + " Forward #theta (pre-cut)"} + ";#theta [deg];Counts", h.theta_precut);            svc->register_hist(fwd + "/phi_precut", std::string{label + " Forward #phi (pre-cut)"} + ";#phi [deg];Counts", h.phi_precut);            svc->register_hist(fwd + "/vz_precut", label + " Forward v_{z} (pre-cut);v_{z} [cm];Events", h.vz_precut);            svc->register_hist(fwd + "/chi2pid_precut", label + " Forward #chi^{2}_{PID} (pre-cut);#chi^{2};Events", h.chi2pid_precut);            svc->register_hist(fwd + "/delta_t_precut", label + " Forward #Deltat (pre-cut);#Deltat [ns];Events", h.delta_t_precut);
            svc->register_hist(fwd + "/pcal_lv_precut", label + " Forward PCAL lv (pre-cut);lv [cm];Events", h.pcal_lv_precut);            svc->register_hist(fwd + "/pcal_lw_precut", label + " Forward PCAL lw (pre-cut);lw [cm];Events", h.pcal_lw_precut);            svc->register_hist(fwd + "/pcal_lw_vs_lv", label + " Forward PCAL lw vs lv (pre-cut);lv [cm];lw [cm]", h.pcal_lw_vs_lv);
            svc->register_hist(fwd + "/beta_vs_p", label + " Forward #beta vs p;p [GeV/c];#beta", h.beta_vs_p);            svc->register_hist(fwd + "/delta_t_vs_p", label + " Forward #Deltat vs p;p [GeV/c];#Deltat [ns]", h.delta_t_vs_p);            svc->register_hist(fwd + "/p_vs_theta", label + " Forward p vs #theta", h.p_vs_theta);
            svc->register_hist(fwd + "/chi2pid_vs_p", label + " Forward #chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}", h.chi2pid_vs_p);            svc->register_hist(fwd + "/vz_vs_theta", label + " Forward v_{z} vs #theta;#theta [deg];v_{z} [cm]", h.vz_vs_theta);            svc->register_hist(fwd + "/delta_t_vs_theta", label + " Forward #Deltat vs #theta;#theta [deg];#Deltat [ns]", h.delta_t_vs_theta);            svc->register_hist(fwd + "/delta_t_vs_p_precut", label + " Forward #Deltat vs p (pre-cut);p [GeV/c];#Deltat [ns]", h.delta_t_vs_p_precut);
            svc->register_hist(dir + "/Central/momentum", std::string{label + " Central Momentum"} + ";Momentum [GeV/c];Counts", h.momentum_central);            svc->register_hist(dir + "/Central/theta", std::string{label + " Central #theta"} + ";#theta [deg];Counts", h.theta_central);            svc->register_hist(dir + "/Central/phi", std::string{label + " Central #phi"} + ";#phi [deg];Counts", h.phi_central);            svc->register_hist(dir + "/Central/vz", label + " Central v_{z};v_{z} [cm];Events", h.vz_central);            svc->register_hist(dir + "/Central/chi2pid", label + " Central #chi^{2}_{PID};#chi^{2};Events", h.chi2pid_central);            svc->register_hist(dir + "/Central/beta", label + " Central #beta;#beta;Events", h.beta_central);            svc->register_hist(dir + "/Central/delta_t", label + " Central #Deltat;#Deltat [ns];Events", h.delta_t_central);            svc->register_hist(dir + "/Central/beta_vs_p", label + " Central #beta vs p;p [GeV/c];#beta", h.beta_vs_p_central);            svc->register_hist(dir + "/Central/delta_t_vs_p", label + " Central #Deltat vs p;p [GeV/c];#Deltat [ns]", h.delta_t_vs_p_central);            svc->register_hist(dir + "/Central/p_vs_theta", label + " Central p vs #theta;#theta [deg];p [GeV/c]", h.p_vs_theta_central);

            for (int sector = 1; sector <= 6; ++sector) {
                std::string sec_suffix = fmt::format("_S{}", sector);
                h.momentum_sector[sector - 1] = std::make_unique<framework::H1<>>(mh::regular(0.0, 11.0, static_cast<std::size_t>(100)));
                svc->register_hist(fwd + "/momentum" + sec_suffix, std::string{fmt::format("{} Forward Momentum (Sector {})", label, sector)} + ";Momentum [GeV/c];Counts", *h.momentum_sector[sector - 1]);
                h.theta_sector[sector - 1] = std::make_unique<framework::H1<>>(mh::regular(0.0, 180.0, static_cast<std::size_t>(180)));
                svc->register_hist(fwd + "/theta" + sec_suffix, std::string{fmt::format("{} Forward #theta (Sector {})", label, sector)} + ";#theta [deg];Counts", *h.theta_sector[sector - 1]);
                h.delta_t_vs_p_sector[sector - 1] = std::make_unique<framework::H2<>>(mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-2, 2, static_cast<std::size_t>(200)));
                svc->register_hist(fwd + "/delta_t_vs_p" + sec_suffix, fmt::format("{} Forward #Deltat vs p (Sector {});p [GeV/c];#Deltat [ns]", label, sector), *h.delta_t_vs_p_sector[sector - 1]);
                h.delta_t_vs_p_precut_sector[sector - 1] = std::make_unique<framework::H2<>>(mh::regular(0, 10, static_cast<std::size_t>(100)), mh::regular(-2, 2, static_cast<std::size_t>(200)));
                svc->register_hist(fwd + "/delta_t_vs_p_precut" + sec_suffix, fmt::format("{} Forward #Deltat vs p pre-cut (Sector {});p [GeV/c];#Deltat [ns]", label, sector), *h.delta_t_vs_p_precut_sector[sector - 1]);
            }

            std::string nm1 = fwd + "/NMinus1";            svc->register_hist(nm1 + "/chi2pid", label + " Forward #chi^{2}_{PID} (N-1);#chi^{2};Events", h.chi2pid_nminus1);            svc->register_hist(nm1 + "/momentum", std::string{label + " Forward Momentum (N-1)"} + ";Momentum [GeV/c];Counts", h.momentum_nminus1);            svc->register_hist(nm1 + "/vz", label + " Forward v_{z} (N-1);v_{z} [cm];Events", h.vz_nminus1);            svc->register_hist(nm1 + "/delta_t_vs_p", label + " Forward #Deltat vs p (N-1);p [GeV/c];#Deltat [ns]", h.delta_t_vs_p_nminus1);            svc->register_hist(nm1 + "/beta_vs_p", label + " Forward #beta vs p (N-1);p [GeV/c];#beta", h.beta_vs_p_nminus1);
            svc->register_hist(fwd + "/beta_diff_vs_p", label + " Forward #Delta#beta vs p;p [GeV/c];#beta_{meas} - #beta_{exp}", h.beta_diff_vs_p);            svc->register_hist(fwd + "/tof_mass2", label + " Forward TOF m^{2};m^{2} [GeV^{2}];Events", h.tof_mass2);            svc->register_hist(fwd + "/tof_mass2_precut", label + " Forward TOF m^{2} (pre-cut);m^{2} [GeV^{2}];Events", h.tof_mass2_precut);
        };

        book_pion_histograms(h_piplus, "/Pion/PiPlus", "#pi^{+}");
        book_pion_histograms(h_piminus, "/Pion/PiMinus", "#pi^{-}");
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

            cand.is_forward_detector = framework::CandidateUtils::is_forward_detector(pion.status());
            cand.is_central_detector = framework::CandidateUtils::is_central_detector(pion.status());
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

            auto timing = framework::CandidateUtils::compute_timing(cand.p, Constants::PIMASS, cand.beta, cand.tof_time, cand.tof_path, start_time);
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
            cand.passes_vz = pass_vz_cut(cand.vz, is_piplus);
            cand.passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t, is_piplus);

            candidates.push_back(std::move(cand));
        }

        return candidates;
    }

    auto fill_pre_cut_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& h = is_piplus ? h_piplus : h_piminus;

        for (const auto& cand : candidates) {
            if (cand.is_forward_detector) {
                h.momentum_precut.fill(cand.p);
                h.theta_precut.fill(cand.theta);
                h.phi_precut.fill(cand.phi);
                h.vz_precut.fill(cand.vz);
                h.chi2pid_precut.fill(cand.chi2pid);
                if (cand.delta_t != 0.0) h.delta_t_precut.fill(cand.delta_t);
                h.pcal_lv_precut.fill(cand.pcal_lv);
                h.pcal_lw_precut.fill(cand.pcal_lw);
                h.pcal_lw_vs_lv.fill(cand.pcal_lv, cand.pcal_lw);
                h.chi2pid_vs_p.fill(cand.p, cand.chi2pid);
                h.vz_vs_theta.fill(cand.theta, cand.vz);
                if (cand.delta_t != 0.0) h.delta_t_vs_theta.fill(cand.theta, cand.delta_t);
                if (cand.delta_t != 0.0) h.delta_t_vs_p_precut.fill(cand.p, cand.delta_t);
                if (cand.sector >= 1 && cand.sector <= 6) {
                    int idx = cand.sector - 1;
                    if (cand.delta_t != 0.0) h.delta_t_vs_p_precut_sector[idx]->fill(cand.p, cand.delta_t);
                }

                h.beta_diff_vs_p.fill(cand.p, cand.beta_diff);
                h.tof_mass2_precut.fill(cand.tof_mass2);
            }

            if (cand.is_central_detector) {
                h.momentum_central.fill(cand.p);
                h.theta_central.fill(cand.theta);
                h.phi_central.fill(cand.phi);
                h.vz_central.fill(cand.vz);
                h.chi2pid_central.fill(cand.chi2pid);
                h.beta_central.fill(cand.beta);
                if (cand.delta_t != 0.0) h.delta_t_central.fill(cand.delta_t);
                h.beta_vs_p_central.fill(cand.p, cand.beta);
                if (cand.delta_t != 0.0) h.delta_t_vs_p_central.fill(cand.p, cand.delta_t);
                h.p_vs_theta_central.fill(cand.theta, cand.p);
            }
        }
    }

    auto fill_post_cut_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& h = is_piplus ? h_piplus : h_piminus;
        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;
            h.momentum.fill(cand.p);
            h.theta.fill(cand.theta);
            h.phi.fill(cand.phi);
            h.vz.fill(cand.vz);
            h.chi2pid.fill(cand.chi2pid);
            h.beta.fill(cand.beta);
            h.beta_vs_p.fill(cand.p, cand.beta);
            if (cand.delta_t != 0.0) h.delta_t_vs_p.fill(cand.p, cand.delta_t);
            h.p_vs_theta.fill(cand.theta, cand.p);
            h.tof_mass2.fill(cand.tof_mass2);
            if (cand.sector >= 1 && cand.sector <= 6) {
                int idx = cand.sector - 1;
                h.momentum_sector[idx]->fill(cand.p);
                h.theta_sector[idx]->fill(cand.theta);
                if (cand.delta_t != 0.0) h.delta_t_vs_p_sector[idx]->fill(cand.p, cand.delta_t);
            }
        }
    }

    auto fill_ntuple(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& ntuple = is_piplus ? m_ntuple_piplus : m_ntuple_piminus;
        if (!ntuple) return;
        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;
            ntuple.data().from(cand);
            ntuple.fill();
        }
    }

    auto passes_all_except_pion(const PionCandidate& c, int skip) const -> bool {
        bool pass_det = (skip == 0) || !m_enable_detector_region_cut.get() || c.is_forward_detector || (c.is_central_detector && m_enable_central_pions.get());
        bool pass_fid = (skip == 1) || !m_enable_fiducial_cuts.get() || !c.is_forward_detector || c.passes_dc_fiducial;
        bool pass_chi2 = (skip == 2) || !m_enable_chi2pid_cut.get() || c.passes_chi2pid;
        bool pass_mom = (skip == 3) || !m_enable_momentum_cut.get() || c.passes_momentum;
        bool pass_vz = (skip == 4) || !m_enable_vz_cut.get() || c.passes_vz;
        bool pass_dt = (skip == 5) || !m_enable_delta_t_cut.get() || !c.is_forward_detector || c.passes_delta_t;
        return pass_det && pass_fid && pass_chi2 && pass_mom && pass_vz && pass_dt;
    }

    auto fill_nminus1_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& h = is_piplus ? h_piplus : h_piminus;
        for (const auto& cand : candidates) {
            if (!cand.is_forward_detector) continue;
            if (passes_all_except_pion(cand, 2)) h.chi2pid_nminus1.fill(cand.chi2pid);
            if (passes_all_except_pion(cand, 3)) h.momentum_nminus1.fill(cand.p);
            if (passes_all_except_pion(cand, 4)) h.vz_nminus1.fill(cand.vz);
            if (passes_all_except_pion(cand, 5)) {
                if (cand.delta_t != 0.0) h.delta_t_vs_p_nminus1.fill(cand.p, cand.delta_t);
                h.beta_vs_p_nminus1.fill(cand.p, cand.beta);
            }
        }
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
            if (max_step >= 1 && m_enable_fiducial_cuts.get() && !cand.passes_dc_fiducial) continue;
            if (max_step >= 2 && m_enable_chi2pid_cut.get() && !cand.passes_chi2pid) continue;
            if (max_step >= 3 && m_enable_momentum_cut.get() && !cand.passes_momentum) continue;
            if (max_step >= 4 && m_enable_vz_cut.get() && !cand.passes_vz) continue;
            if (max_step >= 5 && m_enable_delta_t_cut.get() && cand.is_forward_detector && !cand.passes_delta_t) continue;
            selected.push_back(cand.pion);
        }
        return selected;
    }

    static auto second_order_poly(double x, double a, double b, double c) -> double {
        return a * x * x + b * x + c;
    }

    struct DeltaTCoefficients {
        std::array<double, 3> mean;
        std::array<double, 3> sigma;
    };

    auto get_piplus_delta_t_coefficients() const -> DeltaTCoefficients {
        switch (m_config.target) {
            case Target::LD2: return {{0.000922, -0.010435, 0.046222}, {0.002655, -0.021940, 0.114758}};
            case Target::CxC: return {{-0.000063, -0.004623, 0.039045}, {0.001879, -0.017147, 0.108605}};
            case Target::Cu: return {{0.001801, -0.015591, 0.051889}, {0.003742, -0.027802, 0.120968}};
            case Target::Sn: return {{-0.000403, -0.001866, 0.031458}, {0.001510, -0.014369, 0.104379}};
            default: return {{0.000922, -0.010435, 0.046222}, {0.002655, -0.021940, 0.114758}};
        }
    }

    auto get_piminus_delta_t_coefficients() const -> DeltaTCoefficients {
        switch (m_config.target) {
            case Target::LD2: return {{-0.001635, 0.009566, -0.025434}, {0.012220, -0.089536, 0.229427}};
            case Target::CxC: return {{-0.001694, 0.009962, -0.025835}, {0.012632, -0.092243, 0.232097}};
            case Target::Cu: return {{-0.001349, 0.007585, -0.022235}, {0.013710, -0.095575, 0.233176}};
            case Target::Sn: return {{-0.001719, 0.010436, -0.029489}, {0.015162, -0.104777, 0.245153}};
            default: return {{-0.001635, 0.009566, -0.025434}, {0.012220, -0.089536, 0.229427}};
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
        return m_chi2pid_min.get() < chi2pid && chi2pid < m_chi2pid_max.get();
    }

    auto pass_momentum_cut(double p) -> bool {
        return p > m_momentum_min.get() && p < m_momentum_max.get();
    }

    auto pass_vz_cut(double vz, bool is_piplus) -> bool {
        auto vz_range = is_piplus ? get_piplus_vz_range() : get_piminus_vz_range();
        return vz_range.contains(vz);
    }

    auto pass_delta_t_cut(const double p, const double delta_t, bool is_piplus, double n_sigma) -> bool {
        auto coeffs = is_piplus ? get_piplus_delta_t_coefficients() : get_piminus_delta_t_coefficients();
        double mean = second_order_poly(p, coeffs.mean[0], coeffs.mean[1], coeffs.mean[2]);
        double sigma = second_order_poly(p, coeffs.sigma[0], coeffs.sigma[1], coeffs.sigma[2]);
        double cut_low = mean - n_sigma * sigma;
        double cut_high = mean + n_sigma * sigma;
        return cut_low < delta_t && delta_t < cut_high;
    }

    auto pass_delta_t_cut(const double p, const double delta_t, bool is_piplus) -> bool {
        return pass_delta_t_cut(p, delta_t, is_piplus, m_delta_t_sigma.get());
    }

    static auto format_sigma_label(double sigma) -> std::string { return fmt::format("sigma_{:.1f}", sigma); }
    static auto format_sigma_key(const std::string& base, double sigma) -> std::string { return fmt::format("{}_{}", base, format_sigma_label(sigma)); }

    auto collect_passing_with_sigma(const std::vector<PionCandidate>& candidates, bool is_piplus, double n_sigma) -> std::vector<Particle> {
        std::vector<Particle> selected;
        for (const auto& cand : candidates) {
            bool scan_passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t, is_piplus, n_sigma);
            bool passes_detector_region = !m_enable_detector_region_cut.get() || cand.is_forward_detector || (cand.is_central_detector && m_enable_central_pions.get());
            bool passes_fiducial = !m_enable_fiducial_cuts.get() || !cand.is_forward_detector || cand.passes_dc_fiducial;
            bool passes_timing = !m_enable_delta_t_cut.get() || (cand.is_forward_detector ? scan_passes_delta_t : true);
            bool passes_all = passes_detector_region && passes_fiducial && (!m_enable_chi2pid_cut.get() || cand.passes_chi2pid) && (!m_enable_momentum_cut.get() || cand.passes_momentum) && (!m_enable_vz_cut.get() || cand.passes_vz) && passes_timing;
            if (passes_all) selected.push_back(cand.pion);
        }
        return selected;
    }
};

}  // namespace algorithms
