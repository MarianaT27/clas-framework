# Electron Analysis Example

Complete example of electron selection and DIS analysis.

## Full Code

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Cutflow.hpp"
#include "CLAS/ElectronCuts.hpp"
#include "Physics/DISAnalysis.hpp"

class ElectronAnalysis : public Framework::Algorithm {
public:
    explicit ElectronAnalysis(CLAS::Target target, CLAS::Polarity polarity)
        : Algorithm("ElectronAnalysis")
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Electron Selection") {}

    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        if (!m_hist_svc) return Framework::StatusCode::FAILURE();

        // Book histograms
        h_p = m_hist_svc->book_hist1D("/Electron/p", "p;p [GeV/c];Events", 100, 0, 10);
        h_theta = m_hist_svc->book_hist1D("/Electron/theta", "#theta;#theta [deg];Events", 90, 0, 45);
        h_phi = m_hist_svc->book_hist1D("/Electron/phi", "#phi;#phi [deg];Events", 360, -180, 180);
        h_vz = m_hist_svc->book_hist1D("/Electron/vz", "vz;vz [cm];Events", 100, -20, 10);
        h_q2 = m_hist_svc->book_hist1D("/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_w = m_hist_svc->book_hist1D("/DIS/W", "W;W [GeV];Events", 100, 0, 5);
        h_xb = m_hist_svc->book_hist1D("/DIS/xB", "x_{B};x_{B};Events", 100, 0, 1);

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Forward Detector");
        m_cutflow.add_cut("Momentum");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Q² > 1");
        m_cutflow.add_cut("W > 2");

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        // Find trigger electron (FD)
        const CLAS::Particle* trigger = nullptr;
        for (const auto& e : electrons) {
            if (std::abs(e.status()) >= 2000 && std::abs(e.status()) < 4000) {
                trigger = &e;
                break;
            }
        }
        if (!trigger) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Forward Detector");

        // Apply cuts
        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(trigger->p()))
            return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum");

        if (!CLAS::ElectronCuts::pass_electron_vz_cut(trigger->vz(), m_target))
            return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Vertex Z");

        if (!CLAS::ElectronCuts::pass_chi2pid_cut(trigger->chi2pid()))
            return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Chi2 PID");

        // Calculate kinematics
        auto kin = Physics::DISAnalysis::calculate_kinematics(*trigger);

        if (kin.q2 < 1.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Q² > 1");

        if (kin.w < 2.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("W > 2");

        // Fill histograms
        h_p->Fill(trigger->p());
        h_theta->Fill(trigger->theta());
        h_phi->Fill(trigger->phi());
        h_vz->Fill(trigger->vz());
        h_q2->Fill(kin.q2);
        h_w->Fill(kin.w);
        h_xb->Fill(kin.xb);

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    Framework::Cutflow m_cutflow;
    Framework::HistogramService* m_hist_svc = nullptr;
    TH1D *h_p, *h_theta, *h_phi, *h_vz, *h_q2, *h_w, *h_xb;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    Framework::Application app("ElectronAnalysis");

    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("electron_analysis.root");
    app.service_manager().add_service(std::move(hist_svc));

    app.add_algorithm(std::make_unique<ElectronAnalysis>(
        CLAS::Target::LD2, CLAS::Polarity::Outbending));

    app.set_input_files({argv[1]});
    app.set_num_threads(4);

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

## Output

The analysis produces `electron_analysis.root` with histograms and prints a cutflow table.
