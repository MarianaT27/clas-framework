#pragma once

/**
 * @file Pi0MultiplicityRatioAlg.hpp
 * @brief Pi0 multiplicity ratio analysis algorithm
 *
 * Fills histograms binned in DIS kinematics (Q2, xB, z, pT2) for
 * computing the multiplicity ratio R_A = N_A^pi0 / N_D^pi0 offline
 * by dividing histograms from nuclear targets by deuterium.
 *
 * Runs downstream of ElectronSelectionAlg + DISSelectionAlg + Pi0ReconstructionAlg.
 */

#include <cmath>
#include <string>
#include <vector>

#include <Math/Vector4D.h>

#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/physics/CompositeParticle.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for Pi0MultiplicityRatioAlg.
struct Pi0MultiplicityRatioConfig {
    std::vector<double> Q2_edges = {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 7.0, 11.0};
    std::vector<double> xB_edges = {0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.7};
    std::vector<double> z_edges = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
    std::vector<double> pT2_edges = {0.0, 0.05, 0.1, 0.2, 0.3, 0.5, 0.7, 1.0, 1.5};
};

class Pi0MultiplicityRatioAlg : public framework::Algorithm {
   public:
    using Config = Pi0MultiplicityRatioConfig;
    using StatusCode = framework::StatusCode;

    explicit Pi0MultiplicityRatioAlg(std::string name = "Pi0MultiplicityRatio", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Pi0 Multiplicity") {
        required_services(m_hist);
        declare_properties(m_beam_energy);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Beam energy: {} GeV", m_beam_energy.get());
        Log::info(name(), "Q2 bins: {}", m_config.Q2_edges.size() - 1);
        Log::info(name(), "xB bins: {}", m_config.xB_edges.size() - 1);
        Log::info(name(), "z bins: {}", m_config.z_edges.size() - 1);
        Log::info(name(), "pT2 bins: {}", m_config.pT2_edges.size() - 1);
        Log::info(name(), "Total 4D bins: {}", total_4D_bins());

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has DIS Electron");
        m_cutflow.add_cut("Has Pi0");
        m_cutflow.add_cut("z in range");

        book_histograms();

        return framework::StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Retrieve DIS kinematics from event store
        if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return framework::StatusCode::FILTERED();
        m_cutflow.pass("Has DIS Electron");

        auto dis = framework::EventHelpers::get_dis_kinematics(event);
        if (!dis) return framework::StatusCode::FILTERED();

        const double Q2 = dis->Q2;
        const double xB = dis->xB;
        const double nu = dis->nu;

        // Fill DIS normalization histograms (one entry per DIS event)
        h_n_DIS_events->fill(0.5);
        h_n_DIS_vs_Q2->fill(Q2);
        h_n_DIS_vs_xB->fill(xB);
        // Fill per-Q2-bin normalization
        for (size_t i = 0; i < m_config.Q2_edges.size() - 1; ++i) {
            if (Q2 >= m_config.Q2_edges[i] && Q2 < m_config.Q2_edges[i + 1]) {
                h_n_DIS_Q2bin[i]->fill(0.5);
                break;
            }
        }

        // Fill per-xB-bin normalization
        for (size_t i = 0; i < m_config.xB_edges.size() - 1; ++i) {
            if (xB >= m_config.xB_edges[i] && xB < m_config.xB_edges[i + 1]) {
                h_n_DIS_xBbin[i]->fill(0.5);
                break;
            }
        }

        // Retrieve pi0 candidates
        if (!event.passes_decision_typed(framework::Decisions::HasPi0)) return framework::StatusCode::SUCCESS();
        auto pi0_opt = event.retrieve_typed(framework::Keys::Pi0Candidates);
        if (!pi0_opt || pi0_opt->empty()) return framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Pi0");

        // Get electron for virtual photon direction
        auto electron_opt = framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return framework::StatusCode::SUCCESS();

        // Virtual photon 4-vector: q = beam - scattered_electron
        ROOT::Math::PxPyPzEVector beam_4v(0.0, 0.0, m_beam_energy.get(), m_beam_energy.get());
        ROOT::Math::PxPyPzEVector electron_4v = electron_opt->PxPyPzEVector();
        ROOT::Math::PxPyPzEVector q_4v = beam_4v - electron_4v;

        // Virtual photon 3-direction (for pT calculation)
        auto q_3v = q_4v.Vect();
        double q_mag = std::sqrt(q_3v.Mag2());
        ROOT::Math::XYZVector q_hat = q_3v / q_mag;

        // Process each pi0 candidate
        for (const auto& pi0 : *pi0_opt) {
            double E_pi0 = pi0.energy();
            double z = E_pi0 / nu;

            // Pi0 3-momentum
            ROOT::Math::XYZVector pi0_3v(pi0.px(), pi0.py(), pi0.pz());

            // pT2: transverse momentum squared relative to virtual photon direction
            double p_parallel = pi0_3v.Dot(q_hat);
            ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
            double pT2 = p_perp.Mag2();

            // phi_h: azimuthal angle in virtual photon frame
            // Define coordinate system: z along q, y along q x beam (lepton plane normal)
            ROOT::Math::XYZVector beam_3v(0.0, 0.0, m_beam_energy.get());
            ROOT::Math::XYZVector y_hat = q_3v.Cross(beam_3v);
            double y_mag = std::sqrt(y_hat.Mag2());
            if (y_mag > 0) y_hat = y_hat / y_mag;
            ROOT::Math::XYZVector x_hat = y_hat.Cross(q_hat);

            double phi_h = std::atan2(p_perp.Dot(y_hat), p_perp.Dot(x_hat)) * 180.0 / M_PI;

            // Quality cut: z should be physical
            if (z <= 0.0 || z >= 1.0) continue;
            m_cutflow.pass("z in range");

            double pi0_mass = pi0.mass();

            // Fill 1D distributions
            h_z->fill(z);
            h_pT2->fill(pT2);
            h_phi_h->fill(phi_h);
            h_pi0_mass->fill(pi0_mass);
            // Fill 2D distributions
            h_z_vs_Q2->fill(Q2, z);
            h_z_vs_pT2->fill(pT2, z);
            h_pT2_vs_Q2->fill(Q2, pT2);
            // Find bin indices once for all binned fills
            int iQ2 = -1, ixB = -1, iz = -1, ipT2 = -1;
            for (size_t i = 0; i < m_config.Q2_edges.size() - 1; ++i) {
                if (Q2 >= m_config.Q2_edges[i] && Q2 < m_config.Q2_edges[i + 1]) { iQ2 = static_cast<int>(i); break; }
            }
            for (size_t i = 0; i < m_config.xB_edges.size() - 1; ++i) {
                if (xB >= m_config.xB_edges[i] && xB < m_config.xB_edges[i + 1]) { ixB = static_cast<int>(i); break; }
            }
            for (size_t i = 0; i < m_config.z_edges.size() - 1; ++i) {
                if (z >= m_config.z_edges[i] && z < m_config.z_edges[i + 1]) { iz = static_cast<int>(i); break; }
            }
            for (size_t i = 0; i < m_config.pT2_edges.size() - 1; ++i) {
                if (pT2 >= m_config.pT2_edges[i] && pT2 < m_config.pT2_edges[i + 1]) { ipT2 = static_cast<int>(i); break; }
            }

            // Fill multiplicity histograms per Q2 bin
            if (iQ2 >= 0) {
                h_z_Q2bin[iQ2]->fill(z);
                h_pT2_Q2bin[iQ2]->fill(pT2);
            }

            // Fill multiplicity histograms per xB bin
            if (ixB >= 0) {
                h_z_xBbin[ixB]->fill(z);
            }

            // Fill multiplicity histograms per z bin
            if (iz >= 0) {
                h_pT2_zbin[iz]->fill(pT2);
            }

            // Fill 4D flat-indexed histograms for pT broadening and azimuthal moments
            if (iQ2 >= 0 && ixB >= 0 && iz >= 0 && ipT2 >= 0) {
                double phi_h_rad = phi_h * M_PI / 180.0;
                double idx = static_cast<double>(flat_4D_index(iQ2, ixB, iz, ipT2)) + 0.5;
                h_counts_4D->fill(idx);
                h_sum_pT2_4D->fill(idx, pT2);
                h_sum_cosphi_4D->fill(idx, std::cos(phi_h_rad));
                h_sum_sinphi_4D->fill(idx, std::sin(phi_h_rad));
            }
        }

        return framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));
        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");

        return framework::StatusCode::SUCCESS();
    }

   private:
    Config m_config;

    framework::Property<double> m_beam_energy{"beam_energy", 10.53, "Beam energy [GeV]"};

    framework::ServiceRef<framework::HistogramService> m_hist;

    framework::Cutflow m_cutflow;

    // --- Histograms ---

    // DIS normalization
    framework::ThreadLocalHistogram1D* h_n_DIS_events = nullptr;
    framework::ThreadLocalHistogram1D* h_n_DIS_vs_Q2 = nullptr;
    framework::ThreadLocalHistogram1D* h_n_DIS_vs_xB = nullptr;
    std::vector<framework::ThreadLocalHistogram1D*> h_n_DIS_Q2bin;
    std::vector<framework::ThreadLocalHistogram1D*> h_n_DIS_xBbin;

    // 1D distributions
    framework::ThreadLocalHistogram1D* h_z = nullptr;
    framework::ThreadLocalHistogram1D* h_pT2 = nullptr;
    framework::ThreadLocalHistogram1D* h_phi_h = nullptr;
    framework::ThreadLocalHistogram1D* h_pi0_mass = nullptr;

    // 2D distributions
    framework::ThreadLocalHistogram2D* h_z_vs_Q2 = nullptr;
    framework::ThreadLocalHistogram2D* h_z_vs_pT2 = nullptr;
    framework::ThreadLocalHistogram2D* h_pT2_vs_Q2 = nullptr;

    // Multiplicity per kinematic bin
    std::vector<framework::ThreadLocalHistogram1D*> h_z_Q2bin;
    std::vector<framework::ThreadLocalHistogram1D*> h_pT2_Q2bin;
    std::vector<framework::ThreadLocalHistogram1D*> h_z_xBbin;
    std::vector<framework::ThreadLocalHistogram1D*> h_pT2_zbin;

    // pT broadening and azimuthal moments in 4D bins (Q2, xB, z, pT2)
    framework::ThreadLocalHistogram1D* h_counts_4D = nullptr;
    framework::ThreadLocalHistogram1D* h_sum_pT2_4D = nullptr;
    framework::ThreadLocalHistogram1D* h_sum_cosphi_4D = nullptr;
    framework::ThreadLocalHistogram1D* h_sum_sinphi_4D = nullptr;

    [[nodiscard]] auto flat_4D_index(int iQ2, int ixB, int iz, int ipT2) const -> int {
        int N_xB = static_cast<int>(m_config.xB_edges.size()) - 1;
        int N_z = static_cast<int>(m_config.z_edges.size()) - 1;
        int N_pT2 = static_cast<int>(m_config.pT2_edges.size()) - 1;
        return ((iQ2 * N_xB + ixB) * N_z + iz) * N_pT2 + ipT2;
    }

    [[nodiscard]] auto total_4D_bins() const -> int {
        return static_cast<int>((m_config.Q2_edges.size() - 1) * (m_config.xB_edges.size() - 1) * (m_config.z_edges.size() - 1) * (m_config.pT2_edges.size() - 1));
    }

    auto book_histograms() -> void {
        auto* svc = m_hist.get();
        const std::string dir = "/Pi0Multiplicity";
        {

            // DIS normalization
            h_n_DIS_events = svc->book_hist1D(dir + "/n_DIS_events", "DIS Event Count;count;Events", 1, 0, 1);
            h_n_DIS_vs_Q2 = svc->book_hist1D(dir + "/n_DIS_vs_Q2", "DIS Events vs Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 12);
            h_n_DIS_vs_xB = svc->book_hist1D(dir + "/n_DIS_vs_xB", "DIS Events vs x_{B};x_{B};Events", 100, 0, 0.8);

            // Per-Q2-bin DIS normalization
            h_n_DIS_Q2bin.resize(m_config.Q2_edges.size() - 1);
            for (size_t i = 0; i < m_config.Q2_edges.size() - 1; ++i) {
                h_n_DIS_Q2bin[i] = svc->book_hist1D(
                    fmt::format("{}/norm/n_DIS_Q2bin{}", dir, i),
                    fmt::format("DIS Events Q^{{2}} [{:.1f}, {:.1f}];count;Events", m_config.Q2_edges[i], m_config.Q2_edges[i + 1]),
                    1, 0, 1);
            }

            // Per-xB-bin DIS normalization
            h_n_DIS_xBbin.resize(m_config.xB_edges.size() - 1);
            for (size_t i = 0; i < m_config.xB_edges.size() - 1; ++i) {
                h_n_DIS_xBbin[i] = svc->book_hist1D(
                    fmt::format("{}/norm/n_DIS_xBbin{}", dir, i),
                    fmt::format("DIS Events x_{{B}} [{:.2f}, {:.2f}];count;Events", m_config.xB_edges[i], m_config.xB_edges[i + 1]),
                    1, 0, 1);
            }

            // 1D pi0 distributions
            h_z = svc->book_hist1D(dir + "/z", "#pi^{0} z = E_{#pi^{0}}/#nu;z;Counts", 100, 0, 1);
            h_pT2 = svc->book_hist1D(dir + "/pT2", "#pi^{0} p_{T}^{2};p_{T}^{2} [GeV^{2}];Counts", 100, 0, 2);
            h_phi_h = svc->book_hist1D(dir + "/phi_h", "#pi^{0} #phi_{h};#phi_{h} [deg];Counts", 72, -180, 180);
            h_pi0_mass = svc->book_hist1D(dir + "/pi0_mass", "#pi^{0} Invariant Mass;M_{#gamma#gamma} [GeV/c^{2}];Counts", 200, 0, 0.5);

            // 2D distributions
            h_z_vs_Q2 = svc->book_hist2D(dir + "/z_vs_Q2", "z vs Q^{2};Q^{2} [GeV^{2}];z", 100, 0, 12, 100, 0, 1);
            h_z_vs_pT2 = svc->book_hist2D(dir + "/z_vs_pT2", "z vs p_{T}^{2};p_{T}^{2} [GeV^{2}];z", 100, 0, 2, 100, 0, 1);
            h_pT2_vs_Q2 = svc->book_hist2D(dir + "/pT2_vs_Q2", "p_{T}^{2} vs Q^{2};Q^{2} [GeV^{2}];p_{T}^{2} [GeV^{2}]", 100, 0, 12, 100, 0, 2);

            // Multiplicity: z per Q2 bin
            h_z_Q2bin.resize(m_config.Q2_edges.size() - 1);
            h_pT2_Q2bin.resize(m_config.Q2_edges.size() - 1);
            for (size_t i = 0; i < m_config.Q2_edges.size() - 1; ++i) {
                h_z_Q2bin[i] = svc->book_hist1D(
                    fmt::format("{}/z_Q2bin/z_Q2bin{}", dir, i),
                    fmt::format("#pi^{{0}} z, Q^{{2}} #in [{:.1f}, {:.1f}] GeV^{{2}};z;Counts", m_config.Q2_edges[i], m_config.Q2_edges[i + 1]),
                    100, 0, 1);
                h_pT2_Q2bin[i] = svc->book_hist1D(
                    fmt::format("{}/pT2_Q2bin/pT2_Q2bin{}", dir, i),
                    fmt::format("#pi^{{0}} p_{{T}}^{{2}}, Q^{{2}} #in [{:.1f}, {:.1f}] GeV^{{2}};p_{{T}}^{{2}} [GeV^{{2}}];Counts", m_config.Q2_edges[i], m_config.Q2_edges[i + 1]),
                    100, 0, 2);
            }

            // Multiplicity: z per xB bin
            h_z_xBbin.resize(m_config.xB_edges.size() - 1);
            for (size_t i = 0; i < m_config.xB_edges.size() - 1; ++i) {
                h_z_xBbin[i] = svc->book_hist1D(
                    fmt::format("{}/z_xBbin/z_xBbin{}", dir, i),
                    fmt::format("#pi^{{0}} z, x_{{B}} #in [{:.2f}, {:.2f}];z;Counts", m_config.xB_edges[i], m_config.xB_edges[i + 1]),
                    100, 0, 1);
            }

            // Multiplicity: pT2 per z bin
            h_pT2_zbin.resize(m_config.z_edges.size() - 1);
            for (size_t i = 0; i < m_config.z_edges.size() - 1; ++i) {
                h_pT2_zbin[i] = svc->book_hist1D(
                    fmt::format("{}/pT2_zbin/pT2_zbin{}", dir, i),
                    fmt::format("#pi^{{0}} p_{{T}}^{{2}}, z #in [{:.1f}, {:.1f}];p_{{T}}^{{2}} [GeV^{{2}}];Counts", m_config.z_edges[i], m_config.z_edges[i + 1]),
                    100, 0, 2);
            }

            // 4D flat-indexed histograms for pT broadening and azimuthal moments
            int n4D = total_4D_bins();
            h_counts_4D = svc->book_hist1D(dir + "/4D/counts",
                "4D bin counts (Q^{2}, x_{B}, z, p_{T}^{2});flat bin;Counts", n4D, 0, n4D);
            h_sum_pT2_4D = svc->book_hist1D(dir + "/4D/sum_pT2",
                "#Sigma p_{T}^{2} per 4D bin;flat bin;#Sigma p_{T}^{2}", n4D, 0, n4D);
            h_sum_cosphi_4D = svc->book_hist1D(dir + "/4D/sum_cosphi",
                "#Sigma cos(#phi_{h}) per 4D bin;flat bin;#Sigma cos#phi_{h}", n4D, 0, n4D);
            h_sum_sinphi_4D = svc->book_hist1D(dir + "/4D/sum_sinphi",
                "#Sigma sin(#phi_{h}) per 4D bin;flat bin;#Sigma sin#phi_{h}", n4D, 0, n4D);
        }
    }
};

}  // namespace algorithms
