#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/Event.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Core/Cuts.hpp"
#include "Core/Target.hpp"
#include "TH1D.h"
#include <memory>

namespace Algorithms {

/**
 * @brief Algorithm for electron selection and kinematics
 *
 * This algorithm demonstrates the new framework by:
 * - Using services for histogram management and conditions
 * - Implementing a proper cutflow
 * - Separating concerns (selection logic vs. framework)
 */
class ElectronSelectionAlg : public Framework::Algorithm {
public:
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg",
                                  Core::Target target = Core::Target::LD2,
                                  Core::Polarity polarity = Core::Polarity::Outbending)
        : Algorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Electron Selection") {}

    auto initialize() -> Framework::StatusCode override {
        fmt::print("  Initializing {}\n", name());

        // Get services
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        m_cond_svc = service<Framework::ConditionsService>("ConditionsService");

        if (!m_hist_svc) {
            fmt::print(stderr, "ERROR: Could not get HistogramService\n");
            return Framework::StatusCode::FAILURE();
        }

        // Book histograms
        book_histograms();

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons from event
        const auto& electrons = event.particles(Core::PIDs::ELECTRON);

        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        // Select first electron (could be improved with trigger selection)
        const auto& electron = electrons[0];

        // Apply cuts sequentially
        if (!Core::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Chi2 PID");

        double p = electron.p();
        if (!Core::ElectronCuts::pass_electron_momentum_cut(p)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Momentum > 0.8 GeV");

        if (!Core::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Vertex Z");

        // Fill histograms after selection
        h_momentum->Fill(p);
        h_theta->Fill(electron.theta());
        h_phi->Fill(electron.phi());
        h_vz->Fill(electron.vz());

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        fmt::print("\n");
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    Core::Target m_target;
    Core::Polarity m_polarity;
    Framework::Cutflow m_cutflow;

    // Services
    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::ConditionsService* m_cond_svc = nullptr;

    // Histograms (non-owning pointers managed by HistogramService)
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
    TH1D* h_phi = nullptr;
    TH1D* h_vz = nullptr;

    auto book_histograms() -> void {
        h_momentum = m_hist_svc->book_hist1D(
            "/ElectronKinematics/momentum",
            "Electron Momentum;p [GeV/c];Events",
            120, 0, 12
        );

        h_theta = m_hist_svc->book_hist1D(
            "/ElectronKinematics/theta",
            "Electron Theta;#theta [deg];Events",
            180, 0, 180
        );

        h_phi = m_hist_svc->book_hist1D(
            "/ElectronKinematics/phi",
            "Electron Phi;#phi [deg];Events",
            180, -180, 180
        );

        h_vz = m_hist_svc->book_hist1D(
            "/ElectronKinematics/vz",
            "Electron Vertex Z;v_{z} [cm];Events",
            200, -20, 20
        );
    }
};

}  // namespace Algorithms
