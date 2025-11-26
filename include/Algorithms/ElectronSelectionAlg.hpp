#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/Event.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Core/Cuts.hpp"
#include "Core/Target.hpp"
#include "TH1D.h"

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
        // Get services
        m_log_svc = service<Framework::LoggingService>("LoggingService");
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = service<Framework::NTupleService>("NTupleService");
        m_cond_svc = service<Framework::ConditionsService>("ConditionsService");

        if (m_log_svc) {
            LOG_INFO(m_log_svc, "Initializing algorithm");
        }

        if (!m_hist_svc) {
            if (m_log_svc) {
                LOG_ERROR(m_log_svc, "Could not get HistogramService");
            }
            fmt::print(stderr, "ERROR: Could not get HistogramService\n");
            return Framework::StatusCode::FAILURE();
        }

        if (!m_ntuple_svc) {
            if (m_log_svc) {
                LOG_WARNING(m_log_svc, "Could not get NTupleService - N-tuple output disabled");
            }
        }

        // Book histograms
        book_histograms();

        // Setup N-tuple branches
        if (m_ntuple_svc) {
            setup_ntuple();
        }

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        if (m_log_svc) {
            LOG_INFO(m_log_svc, "Initialized with 8 cuts in cutflow");
        }

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

        // Fill N-tuple if service is available
        if (m_ntuple_svc) {
            m_e_p = p;
            m_e_theta = electron.theta();
            m_e_phi = electron.phi();
            m_e_vz = electron.vz();
            m_e_chi2pid = electron.chi2pid();
            m_ntuple_svc->fill("electron_kinematics");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        if (m_log_svc) {
            LOG_INFO(m_log_svc, "Finalizing algorithm");
            if (m_ntuple_svc) {
                auto entries = m_ntuple_svc->get_entries("electron_kinematics");
                LOG_INFO(m_log_svc, "N-tuple has {} entries", entries);
            }
        }

        fmt::print("\n");
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    Core::Target m_target;
    Core::Polarity m_polarity;
    Framework::Cutflow m_cutflow;

    // Services
    Framework::LoggingService* m_log_svc = nullptr;
    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::NTupleService* m_ntuple_svc = nullptr;
    Framework::ConditionsService* m_cond_svc = nullptr;

    // Histograms (non-owning pointers managed by HistogramService)
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
    TH1D* h_phi = nullptr;
    TH1D* h_vz = nullptr;

    // N-tuple variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_e_chi2pid = 0.0;

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

    auto setup_ntuple() -> void {
        // Create and register branches for electron kinematics
        m_ntuple_svc->register_branch("electron_kinematics", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("electron_kinematics", "e_theta", &m_e_theta);
        m_ntuple_svc->register_branch("electron_kinematics", "e_phi", &m_e_phi);
        m_ntuple_svc->register_branch("electron_kinematics", "e_vz", &m_e_vz);
        m_ntuple_svc->register_branch("electron_kinematics", "e_chi2pid", &m_e_chi2pid);

        if (m_log_svc) {
            LOG_INFO(m_log_svc, "N-tuple branches registered for electron kinematics");
        }
    }
};

}  // namespace Algorithms
