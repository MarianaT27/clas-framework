# DIS Analysis Example

Deep Inelastic Scattering analysis with full kinematics.

## Overview

This example demonstrates:

- DIS kinematic calculations (Q², W, x, y)
- 2D histogram correlations
- N-tuple output for further analysis

## Code

```cpp
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/PIDs.hpp>

class DISAnalysis : public clas12::framework::Algorithm {
public:
    DISAnalysis() : Algorithm("DISAnalysis") {}

    auto initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");
        m_ntuple_svc = service<clas12::framework::NTupleService>("NTupleService");

        // 1D histograms - returns ThreadLocalHistogram1D* (thread-safe)
        h_q2 = m_hist_svc->book_hist1D("/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_w = m_hist_svc->book_hist1D("/DIS/W", "W;W [GeV];Events", 100, 0, 5);
        h_xb = m_hist_svc->book_hist1D("/DIS/xB", "x_{B};x_{B};Events", 100, 0, 1);
        h_y = m_hist_svc->book_hist1D("/DIS/y", "y;y;Events", 100, 0, 1);

        // 2D correlations - returns ThreadLocalHistogram2D* (thread-safe)
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) return clas12::framework::StatusCode::SUCCESS();

        const auto& e = electrons[0];
        if (std::abs(e.status()) < 2000) return clas12::framework::StatusCode::SUCCESS();

        auto kin = clas12::physics::DISAnalysis::calculate_kinematics(e);

        // DIS cuts
        if (kin.q2 < 1.0 || kin.w < 2.0 || kin.y > 0.85)
            return clas12::framework::StatusCode::SUCCESS();

        // Fill histograms using fill() - thread-safe atomic operation
        h_q2->fill(kin.q2);
        h_w->fill(kin.w);
        h_xb->fill(kin.xb);
        h_y->fill(kin.y);
        h_q2_vs_xb->fill(kin.xb, kin.q2);
        h_w_vs_q2->fill(kin.q2, kin.w);

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

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::NTupleService* m_ntuple_svc = nullptr;
    // Atomic histograms are thread-safe for parallel processing
    clas12::framework::ThreadLocalHistogram1D *h_q2, *h_w, *h_xb, *h_y;
    clas12::framework::ThreadLocalHistogram2D *h_q2_vs_xb, *h_w_vs_q2;
    double m_q2, m_w, m_xb, m_y, m_e_p, m_e_theta;
};
```

## Running

```bash
./build/dis_analysis data/data.hipo
root -l dis_analysis.root
root [1] dis->Draw("q2:xb", "", "colz")
```
