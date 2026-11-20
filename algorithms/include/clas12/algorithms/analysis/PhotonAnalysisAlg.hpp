#pragma once

/**
 * @file PhotonAnalysisAlg.hpp
 * @brief Photon identification check: cutflow and the distributions of every photon cut
 *
 * Runs after ElectronSelectionAlg and DISSelectionAlg (events with a selected DIS electron,
 * as in the pi0 analysis). Every pid-22 particle goes through the photon cuts IN THIS ORDER,
 * one cutflow step each (photon_check/PROVENANCE.md gives the origin of every value):
 *
 *   1. pid 22
 *   2. E_gamma >= 0.2 GeV                    GBT pre-filter (PhotonCutsService::pass_pid_purity_filter)
 *   3. E_PCAL > 0                            GBT pre-filter
 *   4. 5 <= theta <= 35 deg                  GBT pre-filter
 *   5. GBT score > 0.78                      PhotonCutsService (RG-A inbending pass-1 model for RG-D runs)
 *   6. PCAL lv > 14 cm and lw > 14 cm        PhotonCutsService::pass_pcal_fiducial_cut
 *   7. 0.9 < beta < 1.1                      PhotonSelectionAlg
 *   8. theta(e, gamma) > 8 deg               Pi0AnalysisAlg::MIN_ELECTRON_PHOTON_ANGLE
 *
 * Each variable is histogrammed for the photons that reach its cut ("before") and for those that
 * pass it ("after"), so a figure shows exactly what that cut removes after all the previous ones.
 * The cutflow is printed at the end of the run and stored as the histogram /PhotonAnalysis/cutflow
 * (bin i = photons passing step i, i = 1..8) for the loss tables.
 * Step 6 also stores E_PCAL/E_gamma vs PCAL lv and vs lw, per sector, for the photons that reach the
 * fiducial cut (E_gamma = |p| as in step 2), to see where the PCAL edge degrades the shower.
 *
 * Every cut can be switched off and every threshold changed from the [PhotonAnalysis] section of
 * the TOML; the defaults are the values of the production photon selection above. Switching a cut
 * off makes its step pass every photon, so the later histograms show the sample without it.
 *
 * The GBT pre-filter and score come from PhotonCutsService::gbt_scores, i.e. the SAME code the
 * production selection uses. A photon with no score (no calorimeter row) fails step 5.
 */

#include <array>
#include <cmath>
#include <map>
#include <string>

#include <clas12/Constants.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Calorimeter.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/bankdefs/RUN_config.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SectorHistograms.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/services/PhotonCutsService.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for PhotonAnalysisAlg. Defaults = the production photon selection.
struct PhotonAnalysisConfig {
    Target target = Target::LD2;
    Polarity polarity = Polarity::Outbending;

    bool enable_energy_cut = true;
    bool enable_pcal_energy_cut = true;
    bool enable_theta_cut = true;
    bool enable_gbt_cut = true;
    bool enable_pcal_fiducial_cut = true;
    bool enable_beta_cut = true;
    bool enable_e_gamma_angle_cut = true;

    double min_energy = framework::PhotonCutsService::MIN_PHOTON_ENERGY;  // GeV, inclusive
    double theta_min = framework::PhotonCutsService::MIN_FD_THETA;        // deg, inclusive
    double theta_max = framework::PhotonCutsService::MAX_FD_THETA;        // deg, inclusive
    double gbt_threshold = framework::PhotonCutsService::AI_THRESHOLD;    // score > threshold
    double pcal_lv_min = framework::PhotonCutsService::MIN_PCAL_V;        // cm, strict
    double pcal_lw_min = framework::PhotonCutsService::MIN_PCAL_W;        // cm, strict
    double beta_min = 0.9;                                                // PhotonSelectionConfig, strict
    double beta_max = 1.1;                                                // PhotonSelectionConfig, strict
    double min_e_gamma_angle = 8.0;  // deg, strict (Pi0AnalysisAlg::MIN_ELECTRON_PHOTON_ANGLE)

    static constexpr auto config_fields() {
        using T = PhotonAnalysisConfig;
        return std::make_tuple(
            framework::field("target", &T::target),
            framework::field("polarity", &T::polarity),
            framework::field("enable_energy_cut", &T::enable_energy_cut),
            framework::field("enable_pcal_energy_cut", &T::enable_pcal_energy_cut),
            framework::field("enable_theta_cut", &T::enable_theta_cut),
            framework::field("enable_gbt_cut", &T::enable_gbt_cut),
            framework::field("enable_pcal_fiducial_cut", &T::enable_pcal_fiducial_cut),
            framework::field("enable_beta_cut", &T::enable_beta_cut),
            framework::field("enable_e_gamma_angle_cut", &T::enable_e_gamma_angle_cut),
            framework::field("min_energy", &T::min_energy),
            framework::field("theta_min", &T::theta_min),
            framework::field("theta_max", &T::theta_max),
            framework::field("gbt_threshold", &T::gbt_threshold),
            framework::field("pcal_lv_min", &T::pcal_lv_min),
            framework::field("pcal_lw_min", &T::pcal_lw_min),
            framework::field("beta_min", &T::beta_min),
            framework::field("beta_max", &T::beta_max),
            framework::field("min_e_gamma_angle", &T::min_e_gamma_angle));
    }
};

class PhotonAnalysisAlg : public framework::Algorithm {
   public:
    using Config = PhotonAnalysisConfig;
    using StatusCode = framework::StatusCode;

    /// Cutflow step names, in order (bin i of /PhotonAnalysis/cutflow = step i).
    static constexpr std::array<const char*, 8> kSteps{
        "pid 22", "E >= 0.2 GeV", "E_PCAL > 0", "5 <= theta <= 35 deg",
        "GBT score > 0.78", "PCAL lv, lw > 14 cm", "0.9 < beta < 1.1", "theta(e,gamma) > 8 deg"};

    explicit PhotonAnalysisAlg(std::string name = "PhotonAnalysis", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Photon Analysis") {
        required_services(m_hist, m_photon_cuts);
        declare_banks("RUN::config", "REC::Particle", "REC::Calorimeter");
        declare_config(m_config);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Target: {}, Polarity: {}", to_string(m_config.target), to_string(m_config.polarity));
        Log::info(name(), "E >= {} GeV [{}], E_PCAL > 0 [{}], {} <= theta <= {} deg [{}]",
                  m_config.min_energy, on(m_config.enable_energy_cut), on(m_config.enable_pcal_energy_cut),
                  m_config.theta_min, m_config.theta_max, on(m_config.enable_theta_cut));
        Log::info(name(), "GBT score > {} [{}] (run-range model, pass {})", m_config.gbt_threshold,
                  on(m_config.enable_gbt_cut), m_photon_cuts->pass());
        Log::info(name(), "PCAL lv > {} cm, lw > {} cm [{}]", m_config.pcal_lv_min, m_config.pcal_lw_min,
                  on(m_config.enable_pcal_fiducial_cut));
        Log::info(name(), "{} < beta < {} [{}]", m_config.beta_min, m_config.beta_max, on(m_config.enable_beta_cut));
        Log::info(name(), "theta(e, gamma) > {} deg [{}]", m_config.min_e_gamma_angle,
                  on(m_config.enable_e_gamma_angle_cut));

        for (const char* step : kSteps) m_cutflow.add_cut(step);
        book_histograms();
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();
        auto electron_opt = framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();
        const Particle& electron = *electron_opt;

        h.n_dis_events.fill(1.0);
        const auto& photons = event.photons();
        if (photons.empty()) return StatusCode::SUCCESS();

        // Same pre-filter, features and model as the production selection (row -> score).
        const auto scores = m_photon_cuts->gbt_scores(event.get_bank("RUN::config"), event.get_bank("REC::Particle"),
                                                      event.get_bank("REC::Calorimeter"));

        int n_selected = 0;
        for (const auto& photon : photons) {
            if (process_photon(photon, electron, scores)) ++n_selected;
        }
        h.n_photons_selected.fill(static_cast<double>(n_selected));
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::PhotonCutsService> m_photon_cuts;

    static auto on(bool enabled) -> const char* { return enabled ? "on" : "OFF"; }
    static auto finite_or_zero(double x) -> double { return std::isnan(x) ? 0.0 : x; }

    struct Histograms {
        framework::H1<> cutflow{mh::regular(0.5, 8.5, 8)};
        framework::H1<> n_dis_events{mh::regular(0.5, 1.5, 1)};
        framework::H1<> n_photons_selected{mh::regular(-0.5, 19.5, 20)};

        // 2-4: GBT pre-filter, all pid-22 (before) and after all three pre-filter cuts
        framework::H1<> E_before{mh::regular(0.0, 5.0, 250)};
        framework::H1<> E_after{mh::regular(0.0, 5.0, 250)};
        framework::H1<> Epcal_before{mh::regular(0.0, 1.0, 200)};
        framework::H1<> Epcal_after{mh::regular(0.0, 1.0, 200)};
        framework::H1<> theta_before{mh::regular(0.0, 45.0, 180)};
        framework::H1<> theta_after{mh::regular(0.0, 45.0, 180)};
        framework::H2<> theta_vs_E_before{mh::regular(0.0, 5.0, 100), mh::regular(0.0, 45.0, 90)};

        // 5: GBT score of the photons that pass the pre-filter
        framework::H1<> score{mh::regular(0.0, 1.0, 200)};
        framework::H2<> score_vs_E{mh::regular(0.0, 5.0, 100), mh::regular(0.0, 1.0, 100)};

        // 6: PCAL fiducial, per sector (photons passing 1-5) and after the cut
        framework::PerSectorHist1D<6> lv_before;
        framework::PerSectorHist1D<6> lw_before;
        framework::PerSectorHist1D<6> lv_after;
        framework::PerSectorHist1D<6> lw_after;
        // E_PCAL / E_gamma vs PCAL lv and lw, per sector, photons passing 1-5 (before the fiducial cut)
        framework::PerSectorHist2D<6> frac_vs_lv_before;
        framework::PerSectorHist2D<6> frac_vs_lw_before;
        framework::H2<> pcal_xy_before{mh::regular(-450.0, 450.0, 300), mh::regular(-450.0, 450.0, 300)};
        framework::H2<> pcal_xy_after{mh::regular(-450.0, 450.0, 300), mh::regular(-450.0, 450.0, 300)};

        // 7: beta (photons passing 1-6) and after the cut
        framework::H1<> beta_before{mh::regular(0.5, 1.5, 200)};
        framework::H1<> beta_after{mh::regular(0.5, 1.5, 200)};
        framework::H2<> beta_vs_E_before{mh::regular(0.0, 5.0, 100), mh::regular(0.5, 1.5, 100)};

        // 8: angle between the selected electron and the photon (photons passing 1-7) and after
        framework::H1<> e_gamma_angle_before{mh::regular(0.0, 60.0, 240)};
        framework::H1<> e_gamma_angle_after{mh::regular(0.0, 60.0, 240)};
        framework::H2<> e_gamma_angle_vs_E_before{mh::regular(0.0, 5.0, 100), mh::regular(0.0, 60.0, 120)};
    };
    Histograms h;

    /// Runs one photon through steps 1-8; true if it passes all the enabled cuts.
    auto process_photon(const Particle& photon, const Particle& electron, const std::map<int, double>& scores) -> bool {
        // 1. pid 22
        m_cutflow.pass(kSteps[0]);
        h.cutflow.fill(1.0);

        // Energy as the GBT pre-filter computes it (|p|, massless photon); a PCAL layer without a
        // hit is NaN in the bank and counts as zero energy, as in PhotonCutsService.
        const double E = photon.p();
        const double theta = photon.theta();
        const auto calo = photon.calo();
        const double E_pcal = finite_or_zero(calo.pcal_energy);
        h.E_before.fill(E);
        h.Epcal_before.fill(E_pcal);
        h.theta_before.fill(theta);
        h.theta_vs_E_before.fill(E, theta);

        // 2-4. GBT pre-filter (bounds as in PhotonCutsService::pass_pid_purity_filter)
        if (m_config.enable_energy_cut && !(E >= m_config.min_energy)) return false;
        m_cutflow.pass(kSteps[1]);
        h.cutflow.fill(2.0);
        if (m_config.enable_pcal_energy_cut && !(E_pcal > 0.0)) return false;
        m_cutflow.pass(kSteps[2]);
        h.cutflow.fill(3.0);
        if (m_config.enable_theta_cut && !(theta >= m_config.theta_min && theta <= m_config.theta_max)) return false;
        m_cutflow.pass(kSteps[3]);
        h.cutflow.fill(4.0);
        h.E_after.fill(E);
        h.Epcal_after.fill(E_pcal);
        h.theta_after.fill(theta);

        // 5. GBT score (absent = no calorimeter row or failed the service's own pre-filter)
        const auto it = scores.find(photon.index());
        const bool has_score = it != scores.end();
        if (has_score) {
            h.score.fill(it->second);
            h.score_vs_E.fill(E, it->second);
        }
        if (m_config.enable_gbt_cut && !(has_score && it->second > m_config.gbt_threshold)) return false;
        m_cutflow.pass(kSteps[4]);
        h.cutflow.fill(5.0);

        // 6. PCAL fiducial
        const int sector = calo.pcal_sector;
        const double lv = calo.pcal_lv;
        const double lw = calo.pcal_lw;
        h.lv_before.fill(sector, lv);
        h.lw_before.fill(sector, lw);
        h.pcal_xy_before.fill(calo.pcal_x, calo.pcal_y);
        if (E > 0.0) {
            h.frac_vs_lv_before.fill(sector, lv, E_pcal / E);
            h.frac_vs_lw_before.fill(sector, lw, E_pcal / E);
        }
        if (m_config.enable_pcal_fiducial_cut && !(lv > m_config.pcal_lv_min && lw > m_config.pcal_lw_min)) return false;
        m_cutflow.pass(kSteps[5]);
        h.cutflow.fill(6.0);
        h.lv_after.fill(sector, lv);
        h.lw_after.fill(sector, lw);
        h.pcal_xy_after.fill(calo.pcal_x, calo.pcal_y);

        // 7. beta
        const double beta = photon.beta();
        h.beta_before.fill(beta);
        h.beta_vs_E_before.fill(E, beta);
        if (m_config.enable_beta_cut && !(beta > m_config.beta_min && beta < m_config.beta_max)) return false;
        m_cutflow.pass(kSteps[6]);
        h.cutflow.fill(7.0);
        h.beta_after.fill(beta);

        // 8. angle to the selected electron (Particle::theta(other) is in radians)
        const double angle = photon.theta(electron) * 180.0 / M_PI;
        h.e_gamma_angle_before.fill(angle);
        h.e_gamma_angle_vs_E_before.fill(E, angle);
        if (m_config.enable_e_gamma_angle_cut && !(angle > m_config.min_e_gamma_angle)) return false;
        m_cutflow.pass(kSteps[7]);
        h.cutflow.fill(8.0);
        h.e_gamma_angle_after.fill(angle);
        return true;
    }

    auto book_histograms() -> void {
        auto* svc = m_hist.get();
        const std::string d = "/PhotonAnalysis";
        svc->register_hist(d + "/cutflow", "Photon cutflow;step;Photons", h.cutflow);
        svc->register_hist(d + "/n_dis_events", "Events with a selected DIS electron;;Events", h.n_dis_events);
        svc->register_hist(d + "/n_photons_selected", "Selected photons per DIS event;N_{#gamma};Events", h.n_photons_selected);

        svc->register_hist(d + "/E_before", "#gamma energy, pid 22;E_{#gamma} [GeV];Photons", h.E_before);
        svc->register_hist(d + "/E_after", "#gamma energy, after the GBT pre-filter;E_{#gamma} [GeV];Photons", h.E_after);
        svc->register_hist(d + "/Epcal_before", "PCAL energy, pid 22;E_{PCAL} [GeV];Photons", h.Epcal_before);
        svc->register_hist(d + "/Epcal_after", "PCAL energy, after the GBT pre-filter;E_{PCAL} [GeV];Photons", h.Epcal_after);
        svc->register_hist(d + "/theta_before", "#gamma #theta, pid 22;#theta [deg];Photons", h.theta_before);
        svc->register_hist(d + "/theta_after", "#gamma #theta, after the GBT pre-filter;#theta [deg];Photons", h.theta_after);
        svc->register_hist(d + "/theta_vs_E_before", "#theta vs E_{#gamma}, pid 22;E_{#gamma} [GeV];#theta [deg]", h.theta_vs_E_before);

        svc->register_hist(d + "/score", "GBT score, after the pre-filter;score;Photons", h.score);
        svc->register_hist(d + "/score_vs_E", "GBT score vs E_{#gamma};E_{#gamma} [GeV];score", h.score_vs_E);

        framework::SectorHistogramGroup<6> g(svc, d + "/PCAL");
        h.lv_before = g.add_hist1D("lv_before", "PCAL l_{v}, before the fiducial cut;l_{v} [cm];Photons", 200, 0, 100);
        h.lw_before = g.add_hist1D("lw_before", "PCAL l_{w}, before the fiducial cut;l_{w} [cm];Photons", 200, 0, 100);
        h.lv_after = g.add_hist1D("lv_after", "PCAL l_{v}, after the fiducial cut;l_{v} [cm];Photons", 200, 0, 100);
        h.lw_after = g.add_hist1D("lw_after", "PCAL l_{w}, after the fiducial cut;l_{w} [cm];Photons", 200, 0, 100);
        h.frac_vs_lv_before = g.add_hist2D("frac_vs_lv_before", "E_{PCAL}/E_{#gamma} vs PCAL l_{v}, before the fiducial cut;l_{v} [cm];E_{PCAL}/E_{#gamma}", 100, 0, 100, 200, 0, 1);
        h.frac_vs_lw_before = g.add_hist2D("frac_vs_lw_before", "E_{PCAL}/E_{#gamma} vs PCAL l_{w}, before the fiducial cut;l_{w} [cm];E_{PCAL}/E_{#gamma}", 100, 0, 100, 200, 0, 1);
        svc->register_hist(d + "/PCAL/xy_before", "PCAL hit position, before the fiducial cut;x [cm];y [cm]", h.pcal_xy_before);
        svc->register_hist(d + "/PCAL/xy_after", "PCAL hit position, after the fiducial cut;x [cm];y [cm]", h.pcal_xy_after);

        svc->register_hist(d + "/beta_before", "#gamma #beta, before the #beta cut;#beta;Photons", h.beta_before);
        svc->register_hist(d + "/beta_after", "#gamma #beta, after the #beta cut;#beta;Photons", h.beta_after);
        svc->register_hist(d + "/beta_vs_E_before", "#beta vs E_{#gamma}, before the #beta cut;E_{#gamma} [GeV];#beta", h.beta_vs_E_before);

        svc->register_hist(d + "/e_gamma_angle_before", "#theta_{e#gamma}, before the cut;#theta_{e#gamma} [deg];Photons", h.e_gamma_angle_before);
        svc->register_hist(d + "/e_gamma_angle_after", "#theta_{e#gamma}, after the cut;#theta_{e#gamma} [deg];Photons", h.e_gamma_angle_after);
        svc->register_hist(d + "/e_gamma_angle_vs_E_before", "#theta_{e#gamma} vs E_{#gamma}, before the cut;E_{#gamma} [GeV];#theta_{e#gamma} [deg]", h.e_gamma_angle_vs_E_before);
    }
};

}  // namespace algorithms
