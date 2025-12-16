# π⁰ Reconstruction Example

Reconstruct π⁰ → γγ from photon pairs.

## Overview

This example demonstrates:

- Photon pair combinatorics
- Invariant mass calculation
- Signal region selection

## Code

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Physics/CompositeParticle.hpp"
#include "Physics/Kinematics.hpp"

class Pi0Reconstruction : public Framework::Algorithm {
public:
    Pi0Reconstruction() : Algorithm("Pi0Reconstruction"), m_cutflow("Pi0 Selection") {}

    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");

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

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        const auto& photons = event.particles(CLAS::PIDs::PHOTON);
        if (photons.size() < 2) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("≥2 Photons");

        // Loop over photon pairs
        for (size_t i = 0; i < photons.size(); ++i) {
            for (size_t j = i + 1; j < photons.size(); ++j) {
                const auto& g1 = photons[i];
                const auto& g2 = photons[j];

                // Fill photon energy
                h_photon_E->Fill(g1.E());
                h_photon_E->Fill(g2.E());

                // Energy cut
                if (g1.E() < 0.3 || g2.E() < 0.3) continue;
                m_cutflow.pass("Photon E > 0.3 GeV");

                // Opening angle
                double angle = Physics::KinematicCalculator::opening_angle(g1, g2);
                h_opening_angle->Fill(angle);

                if (angle < 5.0) continue;
                m_cutflow.pass("Opening Angle > 5°");

                // Invariant mass
                Physics::CompositeParticle pi0({g1, g2});
                double mgg = pi0.invariant_mass();
                h_mgg->Fill(mgg);

                // π⁰ mass window (0.1 - 0.17 GeV)
                if (mgg > 0.1 && mgg < 0.17) {
                    m_cutflow.pass("π⁰ Mass Window");
                    h_mgg_cut->Fill(mgg);
                }
            }
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::Cutflow m_cutflow;
    TH1D *h_mgg, *h_mgg_cut, *h_opening_angle, *h_photon_E;
};
```

## π⁰ Mass

The π⁰ mass is 134.98 MeV. The signal region is typically 100-170 MeV.
