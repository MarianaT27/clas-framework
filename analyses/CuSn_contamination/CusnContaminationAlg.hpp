#pragma once

/**
 * @file CusnContaminationAlg.hpp
 * @brief Algorithm for Cu/Sn contamination analysis
 */

#include <array>
#include <cmath>
#include <string>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include "Math/GenVector/PxPyPzE4D.h"
#include "Math/Vector3Dfwd.h"

namespace CuSnContamination {

using clas12::framework::Log;

/// Configuration for CuSnContaminationAlg.
struct CuSnContaminationConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.
};

/**
 * @brief Algorithm for Cu/Sn contamination analysis
 */
class CuSnContaminationAlg : public clas12::framework::Algorithm {
   public:
    using Config = CuSnContaminationConfig;
    using StatusCode = clas12::framework::StatusCode;

    explicit CuSnContaminationAlg(std::string name = "CuSnContaminationAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        required_services(m_hist);
        declare_optional_bank("MC::Particle");
        declare_properties(m_example_cut, m_enable_histograms);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        if (m_enable_histograms.get()) book_histograms();
        return StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> StatusCode override {
        double mc_vz = 0;
        const bool has_mc = event.has_bank("MC::Particle");
        if (has_mc) {
            const auto& MC_particles = event.get_bank("MC::Particle");
            ROOT::Math::PxPyPzE4D electron4VecP;
            ROOT::Math::XYZVector electron3VecV;
            for (int i = 0; i < MC_particles.rows(); ++i) {
                int pid = MC_particles.get<int32_t>("pid", i);
                if (pid == 11) {
                    double px = static_cast<double>(MC_particles.get<float>("px", i));
                    double py = static_cast<double>(MC_particles.get<float>("py", i));
                    double pz = static_cast<double>(MC_particles.get<float>("pz", i));
                    double vx = static_cast<double>(MC_particles.get<float>("vx", i));
                    double vy = static_cast<double>(MC_particles.get<float>("vy", i));
                    double vz = static_cast<double>(MC_particles.get<float>("vz", i));
                    double E = std::sqrt(px * px + py * py + pz * pz + 0.000511 * 0.000511);
                    electron4VecP = ROOT::Math::PxPyPzE4D(px, py, pz, E);
                    electron3VecV = ROOT::Math::XYZVector(vx, vy, vz);
                }
            }

            if (m_enable_histograms.get() && h_p && h_phi && h_theta && h_vz) {
                h_p->fill(electron4VecP.P());
                h_phi->fill(electron4VecP.Phi() * 180.0 / M_PI);
                h_theta->fill(electron4VecP.Theta() * 180.0 / M_PI);
                h_vz->fill(electron3VecV.Z());
                double mc_theta_deg = electron4VecP.Theta() * 180.0 / M_PI;
                double mc_p = electron4VecP.P();
                h_vz_vs_theta->fill(mc_theta_deg, electron3VecV.Z());
                h_vz_vs_p->fill(mc_p, electron3VecV.Z());
                for (int i = 0; i < N_P_BINS; ++i) {
                    if (mc_p >= p_bin_edges[i] && mc_p < p_bin_edges[i + 1]) {
                        h_vz_vs_theta_pbin[i]->fill(mc_theta_deg, electron3VecV.Z());
                        break;
                    }
                }
            }
            mc_vz = electron3VecV.Z();
        }

        auto electron_opt = clas12::framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();
        auto recon_electron = *electron_opt;
        double rec_vz = recon_electron.vz();

        // RG-D vz ranges: Cu [-10.6, -6.5], Sn [-5.5, 5.0], CuSn [-10.6, 5.0]
        if (-10.6 < rec_vz && rec_vz < -6.5) h_rec_vz_Cu->fill(rec_vz);
        if (-5.5 < rec_vz && rec_vz < 5.0) h_rec_vz_Sn->fill(rec_vz);
        h_rec_vz_CuSn->fill(rec_vz);

        if (has_mc) {
            if (-10.6 < mc_vz && mc_vz < -6.5) h_vz_Cu->fill(rec_vz);
            if (-5.5 < mc_vz && mc_vz < 5.0) h_vz_Sn->fill(rec_vz);
            h_vz_CuSn->fill(rec_vz);
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;

    clas12::framework::Property<double> m_example_cut{"example_cut", 1.0, "Example cut threshold value"};
    clas12::framework::Property<bool> m_enable_histograms{"enable_histograms", true, "Enable histogram filling"};

    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;

    clas12::framework::ThreadLocalHistogram1D* h_p = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_phi = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_theta = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vz = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vz_Cu = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vz_Sn = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vz_CuSn = nullptr;

    clas12::framework::ThreadLocalHistogram1D* h_rec_vz_Cu = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_rec_vz_Sn = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_rec_vz_CuSn = nullptr;

    static constexpr int N_P_BINS = 20;
    static constexpr std::array<double, N_P_BINS + 1> p_bin_edges = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0};

    clas12::framework::ThreadLocalHistogram2D* h_vz_vs_theta = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_vz_vs_p = nullptr;
    std::array<clas12::framework::ThreadLocalHistogram2D*, N_P_BINS> h_vz_vs_theta_pbin = {};

    auto book_histograms() -> void {
        h_p = m_hist->book_hist1D("/MC/Electron/p", "p;p;Events", 100, 0, 10);
        h_phi = m_hist->book_hist1D("/MC/Electron/phi", "#phi;#phi;Events", 100, -180, 180);
        h_theta = m_hist->book_hist1D("/MC/Electron/theta", "#theta;#theta;Events", 100, 0, 90);
        h_vz = m_hist->book_hist1D("/MC/Electron/vz", "vz;vz;Events", 100, -13, 0);
        h_vz_Cu = m_hist->book_hist1D("/MC/Electron/vz_Cu", "vz_Cu;vz_Cu;Events", 100, -13, 0);
        h_vz_Sn = m_hist->book_hist1D("/MC/Electron/vz_Sn", "vz_Sn;vz_Sn;Events", 100, -13, 0);
        h_vz_CuSn = m_hist->book_hist1D("/MC/Electron/vz_CuSn", "vz_CuSn;vz_CuSn;Events", 100, -13, 0);

        h_rec_vz_Cu = m_hist->book_hist1D("/Data/Electron/vz_Cu", "vz_Cu;vz_Cu;Events", 100, -13, 0);
        h_rec_vz_Sn = m_hist->book_hist1D("/Data/Electron/vz_Sn", "vz_Sn;vz_Sn;Events", 100, -13, 0);
        h_rec_vz_CuSn = m_hist->book_hist1D("/Data/Electron/vz_CuSn", "vz_CuSn;vz_CuSn;Events", 100, -13, 0);

        h_vz_vs_theta = m_hist->book_hist2D("/MC/Electron/vz_vs_theta", "vz vs #theta;#theta [deg];vz [cm]", 100, 0, 40, 100, -13, 0);
        h_vz_vs_p = m_hist->book_hist2D("/MC/Electron/vz_vs_p", "vz vs p;p [GeV/c];vz [cm]", 100, 0, 10, 100, -13, 0);

        for (int i = 0; i < N_P_BINS; ++i) {
            auto plo = std::to_string(static_cast<int>(p_bin_edges[i]));
            auto phi = std::to_string(static_cast<int>(p_bin_edges[i + 1]));
            auto title = "vz vs #theta (" + plo + "<p<" + phi + " GeV/c);#theta [deg];vz [cm]";
            h_vz_vs_theta_pbin[i] = m_hist->book_hist2D("/MC/Electron/vz_vs_theta_p" + plo + "_" + phi, title, 100, 0, 40, 100, -13, 0);
        }
    }
};

}  // namespace CuSnContamination
