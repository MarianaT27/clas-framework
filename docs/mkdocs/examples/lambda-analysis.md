# Λ Analysis Example

Reconstruct Λ → pπ⁻ from proton-pion pairs.

## Overview

This example demonstrates:

- Multi-particle final state reconstruction
- Invariant mass from charged particles
- Vertex constraints

## Code

```cpp
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/physics/CompositeParticle.hpp>
#include <clas12/PIDs.hpp>

class LambdaReconstruction : public clas12::framework::Algorithm {
public:
    LambdaReconstruction() : Algorithm("LambdaReconstruction"), m_cutflow("Lambda Selection") {}

    auto initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");

        // Book histograms - returns AtomicHistogram1D* (thread-safe)
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        const auto& protons = event.particles(clas12::PIDs::PROTON);
        if (protons.empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Proton");

        const auto& pions = event.particles(clas12::PIDs::PIMINUS);
        if (pions.empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has π⁻");

        // Loop over proton-pion pairs
        for (const auto& p : protons) {
            h_proton_p->fill(p.p());  // Thread-safe atomic fill
            if (p.p() < 0.4) continue;
            m_cutflow.pass("Proton p > 0.4 GeV");

            for (const auto& pi : pions) {
                h_pion_p->fill(pi.p());  // Thread-safe atomic fill
                if (pi.p() < 0.2) continue;
                m_cutflow.pass("π⁻ p > 0.2 GeV");

                // Invariant mass
                clas12::physics::CompositeParticle lambda({p, pi});
                double mppi = lambda.invariant_mass();
                h_mppi->fill(mppi);  // Thread-safe atomic fill

                // Λ mass window (1.105 - 1.125 GeV)
                if (mppi > 1.105 && mppi < 1.125) {
                    m_cutflow.pass("Λ Mass Window");
                    h_mppi_cut->fill(mppi);  // Thread-safe atomic fill
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
    clas12::framework::AtomicHistogram1D *h_mppi, *h_mppi_cut, *h_proton_p, *h_pion_p;
};
```

## Λ Properties

- Mass: 1115.68 MeV
- Decay: Λ → pπ⁻ (63.9%)
- cτ: 7.89 cm (displaced vertex)

## Signal Region

The Λ mass window is typically 1105-1125 MeV (±10 MeV around PDG mass).
