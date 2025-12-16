# Λ Analysis Example

Reconstruct Λ → pπ⁻ from proton-pion pairs.

## Overview

This example demonstrates:

- Multi-particle final state reconstruction
- Invariant mass from charged particles
- Vertex constraints

## Code

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Physics/CompositeParticle.hpp"

class LambdaReconstruction : public Framework::Algorithm {
public:
    LambdaReconstruction() : Algorithm("LambdaReconstruction"), m_cutflow("Lambda Selection") {}

    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");

        h_mppi = m_hist_svc->book_hist1D("/Lambda/mppi", "M_{p#pi^{-}};M_{p#pi^{-}} [GeV];Events", 200, 1.0, 1.3);
        h_mppi_cut = m_hist_svc->book_hist1D("/Lambda/mppi_cut", "M_{p#pi^{-}} (cuts);M_{p#pi^{-}} [GeV];Events", 200, 1.0, 1.3);
        h_proton_p = m_hist_svc->book_hist1D("/Lambda/proton_p", "Proton p;p [GeV/c];Events", 100, 0, 5);
        h_pion_p = m_hist_svc->book_hist1D("/Lambda/pion_p", "#pi^{-} p;p [GeV/c];Events", 100, 0, 3);

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Has Proton");
        m_cutflow.add_cut("Has π⁻");
        m_cutflow.add_cut("Proton p > 0.4 GeV");
        m_cutflow.add_cut("π⁻ p > 0.2 GeV");
        m_cutflow.add_cut("Λ Mass Window");

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        const auto& protons = event.particles(CLAS::PIDs::PROTON);
        if (protons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Proton");

        const auto& pions = event.particles(CLAS::PIDs::PIMINUS);
        if (pions.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has π⁻");

        // Loop over proton-pion pairs
        for (const auto& p : protons) {
            h_proton_p->Fill(p.p());
            if (p.p() < 0.4) continue;
            m_cutflow.pass("Proton p > 0.4 GeV");

            for (const auto& pi : pions) {
                h_pion_p->Fill(pi.p());
                if (pi.p() < 0.2) continue;
                m_cutflow.pass("π⁻ p > 0.2 GeV");

                // Invariant mass
                Physics::CompositeParticle lambda({p, pi});
                double mppi = lambda.invariant_mass();
                h_mppi->Fill(mppi);

                // Λ mass window (1.105 - 1.125 GeV)
                if (mppi > 1.105 && mppi < 1.125) {
                    m_cutflow.pass("Λ Mass Window");
                    h_mppi_cut->Fill(mppi);
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
    TH1D *h_mppi, *h_mppi_cut, *h_proton_p, *h_pion_p;
};
```

## Λ Properties

- Mass: 1115.68 MeV
- Decay: Λ → pπ⁻ (63.9%)
- cτ: 7.89 cm (displaced vertex)

## Signal Region

The Λ mass window is typically 1105-1125 MeV (±10 MeV around PDG mass).
