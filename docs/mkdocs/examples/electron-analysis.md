# Electron Analysis Example

Complete example of electron selection and DIS analysis.

## Full Code

```cpp
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/Target.hpp>

class ElectronAnalysis : public clas12::framework::Algorithm {
public:
    explicit ElectronAnalysis(clas12::Target target, clas12::Polarity polarity)
        : Algorithm("ElectronAnalysis")
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Electron Selection") {}

    auto initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");
        if (!m_hist_svc) return clas12::framework::StatusCode::FAILURE();
        m_electron_cuts = service<clas12::framework::ElectronCutsService>("ElectronCutsService");
        if (!m_electron_cuts) return clas12::framework::StatusCode::FAILURE();

        // Book histograms - returns ThreadLocalHistogram1D* (thread-safe)
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        // Find trigger electron (FD)
        const clas12::Particle* trigger = nullptr;
        for (const auto& e : electrons) {
            if (std::abs(e.status()) >= 2000 && std::abs(e.status()) < 4000) {
                trigger = &e;
                break;
            }
        }
        if (!trigger) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Forward Detector");

        // Apply cuts
        if (!m_electron_cuts->pass_momentum_cut(trigger->p()))
            return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum");

        if (!m_electron_cuts->pass_vz_cut(trigger->vz(), m_target))
            return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Vertex Z");

        if (!m_electron_cuts->pass_chi2pid_cut(trigger->chi2pid()))
            return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Chi2 PID");

        // Calculate kinematics
        auto kin = clas12::physics::DISAnalysis::calculate_kinematics(*trigger);

        if (kin.q2 < 1.0) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Q² > 1");

        if (kin.w < 2.0) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("W > 2");

        // Fill histograms using fill() - thread-safe atomic operation
        h_p->fill(trigger->p());
        h_theta->fill(trigger->theta());
        h_phi->fill(trigger->phi());
        h_vz->fill(trigger->vz());
        h_q2->fill(kin.q2);
        h_w->fill(kin.w);
        h_xb->fill(kin.xb);

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto finalize() -> clas12::framework::StatusCode override {
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::Target m_target;
    clas12::Polarity m_polarity;
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::ElectronCutsService* m_electron_cuts = nullptr;
    // Atomic histograms are thread-safe for parallel processing
    clas12::framework::ThreadLocalHistogram1D *h_p, *h_theta, *h_phi, *h_vz, *h_q2, *h_w, *h_xb;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    clas12::framework::Application app("ElectronAnalysis");

    auto hist_svc = std::make_unique<clas12::framework::HistogramService>();
    hist_svc->set_output_file("electron_analysis.root");
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(
        std::make_unique<clas12::framework::ElectronCutsService>());

    app.add_algorithm(std::make_unique<ElectronAnalysis>(
        clas12::Target::LD2, clas12::Polarity::Outbending));

    app.set_input_files({argv[1]});
    app.set_num_threads(4);

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

## Running

```bash
./build/analyses/electron_analysis data/sample.hipo
```

## Output

The analysis produces `electron_analysis.root` with histograms and prints a cutflow table:

```
═══════════════════════════════════════════════════════════════
  Cutflow: Electron Selection
═══════════════════════════════════════════════════════════════
Cut                                    Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                             100000    100.0%    100.0%
Has Electron                            45230     45.2%     45.2%
Forward Detector                        42108     42.1%     93.1%
Momentum                                41205     41.2%     97.9%
Vertex Z                                40512     40.5%     98.3%
Chi2 PID                                38945     38.9%     96.1%
Q² > 1                                  35621     35.6%     91.5%
W > 2                                   28453     28.5%     79.9%
═══════════════════════════════════════════════════════════════
```

## Include Paths Used

| Component | Include Path |
|-----------|-------------|
| Application | `<clas12/framework/core/Application.hpp>` |
| ThreadLocalHistogram | `<clas12/framework/core/ThreadLocalHistogram.hpp>` |
| HistogramService | `<clas12/framework/services/HistogramService.hpp>` |
| Cutflow | `<clas12/framework/core/Cutflow.hpp>` |
| ElectronCutsService | `<clas12/framework/services/ElectronCutsService.hpp>` |
| DISAnalysis | `<clas12/physics/DISAnalysis.hpp>` |
| PIDs | `<clas12/PIDs.hpp>` |
| Particle | `<clas12/Particle.hpp>` |
| Target | `<clas12/Target.hpp>` |
