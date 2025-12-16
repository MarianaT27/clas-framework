# DIS Analysis Example

Deep Inelastic Scattering analysis with full kinematics.

## Overview

This example demonstrates:

- DIS kinematic calculations (Q², W, x, y)
- 2D histogram correlations
- N-tuple output for further analysis

## Code

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Physics/DISAnalysis.hpp"

class DISAnalysis : public Framework::Algorithm {
public:
    DISAnalysis() : Algorithm("DISAnalysis") {}

    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = service<Framework::NTupleService>("NTupleService");

        // 1D histograms
        h_q2 = m_hist_svc->book_hist1D("/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_w = m_hist_svc->book_hist1D("/DIS/W", "W;W [GeV];Events", 100, 0, 5);
        h_xb = m_hist_svc->book_hist1D("/DIS/xB", "x_{B};x_{B};Events", 100, 0, 1);
        h_y = m_hist_svc->book_hist1D("/DIS/y", "y;y;Events", 100, 0, 1);

        // 2D correlations
        h_q2_vs_xb = m_hist_svc->book_hist2D("/DIS/Q2_vs_xB", 
            "Q^{2} vs x_{B};x_{B};Q^{2} [GeV^{2}]", 100, 0, 1, 100, 0, 10);
        h_w_vs_q2 = m_hist_svc->book_hist2D("/DIS/W_vs_Q2",
            "W vs Q^{2};Q^{2} [GeV^{2}];W [GeV]", 100, 0, 10, 100, 0, 5);

        // N-tuple branches
        if (m_ntuple_svc) {
            m_ntuple_svc->register_branch("dis", "q2", &m_q2);
            m_ntuple_svc->register_branch("dis", "w", &m_w);
            m_ntuple_svc->register_branch("dis", "xb", &m_xb);
            m_ntuple_svc->register_branch("dis", "y", &m_y);
            m_ntuple_svc->register_branch("dis", "e_p", &m_e_p);
            m_ntuple_svc->register_branch("dis", "e_theta", &m_e_theta);
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();

        const auto& e = electrons[0];
        if (std::abs(e.status()) < 2000) return Framework::StatusCode::SUCCESS();

        auto kin = Physics::DISAnalysis::calculate_kinematics(e);

        // DIS cuts
        if (kin.q2 < 1.0 || kin.w < 2.0 || kin.y > 0.85)
            return Framework::StatusCode::SUCCESS();

        // Fill histograms
        h_q2->Fill(kin.q2);
        h_w->Fill(kin.w);
        h_xb->Fill(kin.xb);
        h_y->Fill(kin.y);
        h_q2_vs_xb->Fill(kin.xb, kin.q2);
        h_w_vs_q2->Fill(kin.q2, kin.w);

        // Fill N-tuple
        if (m_ntuple_svc) {
            m_q2 = kin.q2;
            m_w = kin.w;
            m_xb = kin.xb;
            m_y = kin.y;
            m_e_p = e.p();
            m_e_theta = e.theta();
            m_ntuple_svc->fill("dis");
        }

        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::NTupleService* m_ntuple_svc = nullptr;
    TH1D *h_q2, *h_w, *h_xb, *h_y;
    TH2D *h_q2_vs_xb, *h_w_vs_q2;
    double m_q2, m_w, m_xb, m_y, m_e_p, m_e_theta;
};
```

## Running

```bash
./build/dis_analysis data/data.hipo
root -l dis_analysis.root
root [1] dis->Draw("q2:xb", "", "colz")
```
