# π⁰ Reconstruction Example

Reconstruct π⁰ → γγ from photon pairs using the framework's built-in `Pi0ReconstructionAlg` or a custom implementation.

## Overview

This example demonstrates:

- Using the ApplicationBuilder pattern
- AI-based photon selection with PhotonCutsService
- Photon pair combinatorics
- Invariant mass calculation
- Signal region selection

## Using Built-in Algorithm

The simplest approach uses the built-in `Pi0ReconstructionAlg`:

```cpp
#include "Framework/ApplicationBuilder.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Framework/Services/PhotonCutsService.hpp"

int main() {
    auto app = Framework::make_application("Pi0 Analysis")
        .with_input_files(input_files)
        .with_output("pi0_analysis.root")
        .with_threads(4)
        .with_standard_services()
        .with_version("2.0")
        .with_beam_energy(10.53)
        .with_target("LD2")
        .add_service<Framework::PhotonCutsService>()
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<Algorithms::ElectronSelectionAlg>(
            "ElectronSelection", CLAS::Target::LD2, CLAS::Polarity::Outbending)
        .add_algorithm<Algorithms::DISSelectionAlg>(
            "DISSelection", Physics::DISAnalysis::standard_dis_region())
        .add_algorithm<Algorithms::Pi0ReconstructionAlg>(
            "Pi0Reconstruction", 0.2, 0.3)  // mass_window, min_photon_energy
        .build();

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

## Custom Implementation

For more control, implement a custom algorithm:

```cpp
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/physics/CompositeParticle.hpp>
#include <clas12/physics/Kinematics.hpp>
#include <clas12/PIDs.hpp>

class Pi0Reconstruction : public clas12::framework::Algorithm {
public:
    Pi0Reconstruction() : Algorithm("Pi0Reconstruction"), m_cutflow("Pi0 Selection") {}

    auto initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");

        // Book histograms - returns ThreadLocalHistogram1D* (thread-safe)
        h_mgg = m_hist_svc->book_hist1D("/Pi0/mgg", "M_{#gamma#gamma};M_{#gamma#gamma} [GeV];Events", 200, 0, 0.3);
        h_mgg_cut = m_hist_svc->book_hist1D("/Pi0/mgg_cut", "M_{#gamma#gamma} (cuts);M_{#gamma#gamma} [GeV];Events", 200, 0, 0.3);
        h_opening_angle = m_hist_svc->book_hist1D("/Pi0/opening_angle", "Opening Angle;#theta_{#gamma#gamma} [deg];Events", 180, 0, 180);
        h_photon_E = m_hist_svc->book_hist1D("/Pi0/photon_E", "Photon Energy;E [GeV];Events", 100, 0, 5);

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("≥2 Photons");
        m_cutflow.add_cut("Photon E > 0.3 GeV");
        m_cutflow.add_cut("Opening Angle > 5°");
        m_cutflow.add_cut("π⁰ Mass Window");

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        const auto& photons = event.particles(clas12::PIDs::PHOTON);
        if (photons.size() < 2) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("≥2 Photons");

        // Loop over photon pairs
        for (size_t i = 0; i < photons.size(); ++i) {
            for (size_t j = i + 1; j < photons.size(); ++j) {
                const auto& g1 = photons[i];
                const auto& g2 = photons[j];

                // Fill photon energy - thread-safe atomic fill
                h_photon_E->fill(g1.E());
                h_photon_E->fill(g2.E());

                // Energy cut
                if (g1.E() < 0.3 || g2.E() < 0.3) continue;
                m_cutflow.pass("Photon E > 0.3 GeV");

                // Opening angle
                double angle = clas12::physics::KinematicCalculator::opening_angle(g1, g2);
                h_opening_angle->fill(angle);  // Thread-safe atomic fill

                if (angle < 5.0) continue;
                m_cutflow.pass("Opening Angle > 5°");

                // Invariant mass
                clas12::physics::CompositeParticle pi0({g1, g2});
                double mgg = pi0.invariant_mass();
                h_mgg->fill(mgg);  // Thread-safe atomic fill

                // π⁰ mass window (0.1 - 0.17 GeV)
                if (mgg > 0.1 && mgg < 0.17) {
                    m_cutflow.pass("π⁰ Mass Window");
                    h_mgg_cut->fill(mgg);  // Thread-safe atomic fill
                }
            }
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto finalize() -> clas12::framework::StatusCode override {
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::Cutflow m_cutflow;
    // Atomic histograms are thread-safe for parallel processing
    clas12::framework::ThreadLocalHistogram1D *h_mgg, *h_mgg_cut, *h_opening_angle, *h_photon_E;
};
```

## π⁰ Mass

The π⁰ mass is 134.98 MeV. The signal region is typically 100-170 MeV.

## Using AI-Based Photon Selection

The `PhotonCutsService` provides AI/ML-based photon selection:

```cpp
auto initialize() -> Framework::StatusCode override {
    m_photon_svc = service<Framework::PhotonCutsService>("PhotonCutsService");
    m_photon_svc->set_pass(1);  // Reconstruction pass
    return Framework::StatusCode::SUCCESS();
}

auto execute(Framework::Event& event) -> Framework::StatusCode override {
    auto& hipo_event = event.hipo_event();
    auto& RUN_config = hipo_event["RUN::config"];
    auto& REC_Particle = hipo_event["REC::Particle"];
    auto& REC_Calorimeter = hipo_event["REC::Calorimeter"];

    // Select photons using AI classification
    auto good_photons = m_photon_svc->select_photons_with_AI(
        RUN_config, REC_Particle, REC_Calorimeter);

    // Continue with pair combinatorics...
    return Framework::StatusCode::SUCCESS();
}
```

## Command Line Usage

The `pi0` executable takes one flag — `--config <path.toml>`. Every
knob (input, output, threads, target/polarity, per-algorithm
properties) lives in the TOML.

```bash
./builddir/pi0 --config config/pi0.toml
```

Example TOML:

```toml
[run]
input       = "/path/to/data"
output      = "pi0_analysis.root"
num_threads = 4

[analysis]
target   = "LD2"
polarity = "Outbending"

[Pi0Reconstruction]
mass_window = 0.2

[PhotonSelection]
min_energy = 0.3
```
