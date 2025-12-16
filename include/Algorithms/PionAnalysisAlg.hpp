#pragma once

/**
 * @file PionAnalysisAlg.hpp
 * @brief Pion analysis algorithm with calorimeter and DC quality histograms
 *
 * Produces histograms for:
 * - lv vs Sampling Fraction
 * - lw vs Sampling Fraction
 * - chi2/ndf vs DC edge (all 3 regions)
 *
 * Requires a selected electron in the event (HasGoodElectron decision).
 */

#include "CLAS/Constants.hpp"
#include "CLAS/ElectronCuts.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "TH1D.h"
#include "TH2D.h"

#include <cmath>
#include <memory>

namespace Algorithms {

using Framework::Log;

/**
 * @brief Pion analysis with calorimeter SF and DC quality histograms
 */
class PionAnalysisAlg : public Framework::Algorithm {
public:
    explicit PionAnalysisAlg(std::string name = "PionAnalysis",
                             CLAS::Target target = CLAS::Target::LD2,
                             CLAS::Polarity polarity = CLAS::Polarity::Outbending)
        : Algorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow_piplus("π⁺ Analysis")
        , m_cutflow_piminus("π⁻ Analysis") {}

    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        m_particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");

        if (!m_hist_svc) {
            Log::error(name(), "HistogramService not available");
            return Framework::StatusCode::FAILURE();
        }

        if (!m_particle_svc) {
            Log::error(name(), "ParticleSelectorService not available");
            return Framework::StatusCode::FAILURE();
        }

        Log::info(name(), "Initializing pion analysis");
        Log::info(name(), "Target: {}, Polarity: {}",
                  CLAS::to_string(m_target), CLAS::to_string(m_polarity));

        setup_cutflows();
        book_histograms();

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // Require selected electron
        if (!event.passes_decision("HasGoodElectron")) {
            return Framework::StatusCode::FILTERED();
        }

        auto& hipo_event = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Calorimeter, REC_Traj, REC_Track] =
            hipo_event.banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Traj", "REC::Track");

        if (REC_Event->rows() == 0) return Framework::StatusCode::FILTERED();

        // Extract pions
        auto particles = m_particle_svc->extract_particles(*REC_Particle, {CLAS::PIDs::PIPLUS, CLAS::PIDs::PIMINUS});

        // Process π⁺
        for (const auto& pion : m_particle_svc->get_pions_plus(particles)) {
            process_pion(pion, *REC_Calorimeter, *REC_Traj, *REC_Track, true);
        }

        // Process π⁻
        for (const auto& pion : m_particle_svc->get_pions_minus(particles)) {
            process_pion(pion, *REC_Calorimeter, *REC_Traj, *REC_Track, false);
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        Log::info(name(), "Finalizing pion analysis");

        m_cutflow_piplus.print();
        m_cutflow_piminus.print();

        Log::info(name(), "Pion Analysis Summary:");
        Log::info(name(), "     - π⁺ with calorimeter info: {}", m_cutflow_piplus.get_count("Has Calorimeter"));
        Log::info(name(), "     - π⁻ with calorimeter info: {}", m_cutflow_piminus.get_count("Has Calorimeter"));

        return Framework::StatusCode::SUCCESS();
    }

private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    Framework::Cutflow m_cutflow_piplus;
    Framework::Cutflow m_cutflow_piminus;

    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::ParticleSelectorService* m_particle_svc = nullptr;

    // Histogram set for each pion charge
    struct PionHistograms {
        // Calorimeter: lv, lw vs SF
        TH2D* sf_vs_lv = nullptr;
        TH2D* sf_vs_lw = nullptr;
        TH1D* sf = nullptr;
        TH1D* lv = nullptr;
        TH1D* lw = nullptr;

        // DC: chi2/ndf vs edge
        TH2D* chi2ndf_vs_edge_r1 = nullptr;
        TH2D* chi2ndf_vs_edge_r2 = nullptr;
        TH2D* chi2ndf_vs_edge_r3 = nullptr;
        TH1D* chi2ndf = nullptr;
        TH1D* edge_r1 = nullptr;
        TH1D* edge_r2 = nullptr;
        TH1D* edge_r3 = nullptr;

        // Kinematics
        TH1D* momentum = nullptr;
        TH1D* theta = nullptr;
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    void setup_cutflows() {
        auto setup = [](Framework::Cutflow& cf) {
            cf.add_cut("All Pions");
            cf.add_cut("In the forward detector");
            cf.add_cut("Momentum > 0.3 GeV");
            cf.add_cut("Has Calorimeter");
            cf.add_cut("Has DC Track");
        };
        setup(m_cutflow_piplus);
        setup(m_cutflow_piminus);
    }

    void book_histograms() {
        book_pion_histograms("/PionAnalysis/PiPlus", h_piplus, "#pi^{+}");
        book_pion_histograms("/PionAnalysis/PiMinus", h_piminus, "#pi^{-}");
    }

    void book_pion_histograms(const std::string& dir, PionHistograms& h, const std::string& label) {
        // Sampling fraction histograms
        h.sf = m_hist_svc->book_hist1D(
            dir + "/sf", fmt::format("{} Sampling Fraction;SF;Events", label),
            100, 0, 0.4);
        h.lv = m_hist_svc->book_hist1D(
            dir + "/lv", fmt::format("{} PCAL lv;lv [cm];Events", label),
            100, 0, 500);
        h.lw = m_hist_svc->book_hist1D(
            dir + "/lw", fmt::format("{} PCAL lw;lw [cm];Events", label),
            100, 0, 500);

        h.sf_vs_lv = m_hist_svc->book_hist2D(
            dir + "/sf_vs_lv",
            fmt::format("{} Sf vs lv;lv [cm];SF", label),
            200, 0, 70, 200, 0, 0.4);
        h.sf_vs_lw = m_hist_svc->book_hist2D(
            dir + "/sf_vs_lw",
            fmt::format("{} Sf vs lw;lw [cm];SF", label),
            200, 0, 70, 200, 0, 0.4);
        // DC chi2/ndf vs edge histograms
        h.chi2ndf = m_hist_svc->book_hist1D(
            dir + "/chi2ndf", fmt::format("{} DC #chi^{{2}}/NDF;#chi^{{2}}/NDF;Events", label),
            100, 0, 20);
        h.edge_r1 = m_hist_svc->book_hist1D(
            dir + "/edge_r1", fmt::format("{} DC Edge R1;Edge [cm];Events", label),
            100, -10, 50);
        h.edge_r2 = m_hist_svc->book_hist1D(
            dir + "/edge_r2", fmt::format("{} DC Edge R2;Edge [cm];Events", label),
            100, -10, 50);
        h.edge_r3 = m_hist_svc->book_hist1D(
            dir + "/edge_r3", fmt::format("{} DC Edge R3;Edge [cm];Events", label),
            100, -10, 50);

        h.chi2ndf_vs_edge_r1 = m_hist_svc->book_hist2D(
            dir + "/chi2ndf_vs_edge_r1",
            fmt::format("{} #chi^{{2}}/NDF vs Edge R1;Edge R1 [cm];#chi^{{2}}/NDF", label),
            200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2 = m_hist_svc->book_hist2D(
            dir + "/chi2ndf_vs_edge_r2",
            fmt::format("{} #chi^{{2}}/NDF vs Edge R2;Edge R2 [cm];#chi^{{2}}/NDF", label),
            200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3 = m_hist_svc->book_hist2D(
            dir + "/chi2ndf_vs_edge_r3",
            fmt::format("{} #chi^{{2}}/NDF vs Edge R3;Edge R3 [cm];#chi^{{2}}/NDF", label),
            200, 0, 50, 200, 0, 100);
        // Basic kinematics
        h.momentum = m_hist_svc->book_hist1D(
            dir + "/momentum", fmt::format("{} Momentum;p [GeV/c];Events", label),
            100, 0, 10);
        h.theta = m_hist_svc->book_hist1D(
            dir + "/theta", fmt::format("{} #theta;#theta [deg];Events", label),
            180, 0, 40);
    }

    void process_pion(const CLAS::Particle& pion,
                      hipo::BankView& REC_Calorimeter,
                      hipo::BankView& REC_Traj,
                      hipo::BankView& REC_Track,
                      bool is_piplus) {
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;
        auto& h = is_piplus ? h_piplus : h_piminus;

        cutflow.pass("All Pions");

        // Pion in the forward detector (2000<=|status| < 4000)
        int status = pion.status();
        if (std::abs(status) < 2000 || std::abs(status) >= 4000) return;
        cutflow.pass("In the forward detector");


        // Basic momentum cut
        double p = pion.p();
        if (p < 0.3) return;
        cutflow.pass("Momentum > 0.3 GeV");

        // Read calorimeter info
        auto calo = CLAS::read_Calorimeter_bank(REC_Calorimeter, pion.index());
        double E_tot = calo.pcal.energy + calo.inner.energy + calo.outer.energy;

        bool has_calo = (E_tot > 0.01);  // At least 10 MeV deposited
        if (!has_calo) return;
        cutflow.pass("Has Calorimeter");

        // Read DC track info
        auto track = CLAS::read_Track_bank(REC_Track, pion.index());
        auto traj = CLAS::read_Traj_bank(REC_Traj, pion.index());

        bool has_dc = (track.ndf > 0);
        if (!has_dc) return;
        cutflow.pass("Has DC Track");

        // Calculate quantities
        double SF = E_tot / p;
        double lv = calo.pcal.lv;
        double lw = calo.pcal.lw;
        double chi2ndf = (track.ndf > 0) ? track.chi2 / track.ndf : 0.0;
        double edge_r1 = traj.region1.edge;
        double edge_r2 = traj.region2.edge;
        double edge_r3 = traj.region3.edge;

        // Fill histograms
        h.momentum->Fill(p);
        h.theta->Fill(pion.theta());

        // Calorimeter histograms
        h.sf->Fill(SF);
        h.lv->Fill(lv);
        h.lw->Fill(lw);
        h.sf_vs_lv->Fill(lv, SF);
        h.sf_vs_lw->Fill(lw, SF);

        // DC histograms
        h.chi2ndf->Fill(chi2ndf);
        h.edge_r1->Fill(edge_r1);
        h.edge_r2->Fill(edge_r2);
        h.edge_r3->Fill(edge_r3);
        h.chi2ndf_vs_edge_r1->Fill(edge_r1, chi2ndf);
        h.chi2ndf_vs_edge_r2->Fill(edge_r2, chi2ndf);
        h.chi2ndf_vs_edge_r3->Fill(edge_r3, chi2ndf);
    }
};

}  // namespace Algorithms
