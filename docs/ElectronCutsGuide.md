# Electron Cuts Guide - CLAS12 Analysis

This guide shows how to use the comprehensive `ElectronCuts` class for CLAS12 electron identification.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Individual Cut Functions](#individual-cut-functions)
3. [Complete Selection](#complete-selection)
4. [Cut Results Structure](#cut-results-structure)
5. [Integration with Your Analysis](#integration-with-your-analysis)
6. [Complete Example](#complete-example)

---

## Quick Start

### Basic Usage

```cpp
#include <Core/Cuts.hpp>
#include <Core/EventData.hpp>

// Set your target
using Target = Core::ElectronCuts::Target;
Target target = Target::LD2;

for (hipo::hipoeventfile events(file); auto& event : events) {
    Core::EventData evt(event);

    for (const auto& electron : evt.electrons()) {
        // Get detector banks
        auto cal = evt.calorimeter(electron);
        auto traj = evt.trajectory(electron);

        // Apply all cuts
        bool passes = Core::ElectronCuts::select_electron(electron, cal, traj, target);

        if (passes) {
            // Process good electron
        }
    }
}
```

---

## Individual Cut Functions

### 1. Chi2 PID Cut

```cpp
double chi2pid = electron.chi2pid();
bool passes = Core::ElectronCuts::pass_chi2pid_cut(chi2pid);

// Range: [-5.0, 5.0]
```

### 2. Momentum Cut

```cpp
double p = electron.p();
bool passes = Core::ElectronCuts::pass_electron_momentum_cut(p);

// Minimum: 0.8 GeV
```

### 3. Vertex Z Cut (Target-Specific)

```cpp
using Target = Core::ElectronCuts::Target;

// Available targets
Target::LD2     // Liquid Deuterium: vz ∈ (-15.0, 5.0) cm
Target::CxC     // Carbon on Carbon: vz ∈ (-10.6, 5.0) cm
Target::Cu      // Copper: vz ∈ (-10.6, -6.5) cm
Target::Sn      // Tin: vz ∈ (-5.5, 5.0) cm
Target::CuSn    // Copper-Tin: vz ∈ (-10.6, 5.0) cm

double vz = electron.vz();
bool passes = Core::ElectronCuts::pass_electron_vz_cut(vz, Target::LD2);

// Or get the range for your target
auto range = Core::ElectronCuts::get_vz_range(Target::LD2);
std::cout << "Vz range: [" << range.min << ", " << range.max << "]\n";
```

### 4. PCAL Fiducial Cuts

```cpp
auto cal = evt.calorimeter(electron);

// Individual cuts
bool pass_v = Core::ElectronCuts::pass_pcal_v_cut(cal.pcal.lv);  // > 9.0 cm
bool pass_w = Core::ElectronCuts::pass_pcal_w_cut(cal.pcal.lw);  // > 9.0 cm

// Combined
bool passes = Core::ElectronCuts::pass_pcal_fiducial_cut(cal.pcal.lv, cal.pcal.lw);
```

### 5. Sampling Fraction Cut (Event Builder)

```cpp
auto cal = evt.calorimeter(electron);

double E_tot = cal.pcal.energy + cal.inner.energy + cal.outer.energy;
double SF = E_tot / electron.p();
int sector = cal.pcal.sector;

bool passes = Core::ElectronCuts::pass_sampling_fraction_cut_eb(SF, E_tot, sector);

// With custom n-sigma
bool passes = Core::ElectronCuts::pass_sampling_fraction_cut_eb(SF, E_tot, sector, 5.0);
```

### 6. Drift Chamber Chi2/NDF Edge Cuts

```cpp
auto traj = evt.trajectory(electron);

double edge1 = traj.region1.edge;  // > 1.680 cm
double edge2 = traj.region2.edge;  // > 2.0 cm
double edge3 = traj.region3.edge;  // > 8.750 cm

bool passes = Core::ElectronCuts::pass_chi2ndf_cut(edge1, edge2, edge3);

// Or using EdgeCuts struct
Core::ElectronCuts::EdgeCuts edges{edge1, edge2, edge3};
bool passes = Core::ElectronCuts::pass_chi2ndf_cut(edges);
```

### 7. Drift Chamber Theta-Dependent Cuts

```cpp
auto traj = evt.trajectory(electron);

// Calculate local theta for each region (you need to implement compute_local_theta_DC)
double theta1 = compute_local_theta_DC(traj.region1.x, traj.region1.y, traj.region1.z, sector);
double theta2 = compute_local_theta_DC(traj.region2.x, traj.region2.y, traj.region2.z, sector);
double theta3 = compute_local_theta_DC(traj.region3.x, traj.region3.y, traj.region3.z, sector);

// Check single region at 1σ
bool pass_R1 = Core::ElectronCuts::pass_dc_theta_cut_sigma(1, traj.region1.edge, theta1, 1);

// Check all regions at once (1σ, 2σ, or 3σ)
bool pass_1sigma = Core::ElectronCuts::pass_all_dc_theta_cuts_sigma(
    traj.region1.edge, traj.region2.edge, traj.region3.edge,
    theta1, theta2, theta3,
    1  // sigma level
);

bool pass_2sigma = Core::ElectronCuts::pass_all_dc_theta_cuts_sigma(
    traj.region1.edge, traj.region2.edge, traj.region3.edge,
    theta1, theta2, theta3,
    2  // sigma level
);
```

---

## Complete Selection

### Method 1: Simple Selection (No Theta Cuts)

```cpp
using Target = Core::ElectronCuts::Target;

// Apply all required cuts (chi2pid, momentum, vz, SF, PCAL, DC chi2/ndf)
bool passes = Core::ElectronCuts::select_electron(
    electron,
    calorimeter,
    trajectory,
    Target::LD2
);
```

### Method 2: Selection with Theta Cuts

```cpp
// Include theta-dependent cuts at specified sigma level
bool passes = Core::ElectronCuts::select_electron_with_theta(
    electron,
    calorimeter,
    trajectory,
    Target::LD2,
    theta1,  // Local theta DC region 1
    theta2,  // Local theta DC region 2
    theta3,  // Local theta DC region 3
    1        // Sigma level (1, 2, or 3)
);
```

### Method 3: Get Detailed Cut Results

```cpp
auto results = Core::ElectronCuts::apply_all_cuts(
    electron,
    calorimeter,
    trajectory,
    Target::LD2,
    theta1, theta2, theta3
);

// Inspect individual cuts
if (results.chi2pid) { /* passed chi2pid cut */ }
if (results.momentum) { /* passed momentum cut */ }
if (results.vz) { /* passed vertex cut */ }
if (results.sampling_fraction) { /* passed SF cut */ }
if (results.pcal_fiducial) { /* passed PCAL fiducial */ }
if (results.dc_chi2ndf) { /* passed DC chi2/ndf cut */ }
if (results.dc_theta_1sigma) { /* passed 1σ theta cut */ }
if (results.dc_theta_2sigma) { /* passed 2σ theta cut */ }
if (results.dc_theta_3sigma) { /* passed 3σ theta cut */ }

// Check if passes all required cuts
if (results.passes_all_required()) {
    // Passes everything except theta
}

// Check if passes all cuts at specific sigma level
if (results.passes_with_theta(1)) {
    // Passes all cuts including 1σ theta
}
```

---

## Cut Results Structure

The `CutResults` structure provides detailed information about which cuts passed:

```cpp
struct CutResults {
    bool chi2pid;              // Chi2 PID cut
    bool momentum;             // Momentum > 0.8 GeV
    bool vz;                   // Vertex Z cut (target-specific)
    bool sampling_fraction;    // Sampling fraction cut
    bool pcal_fiducial;        // PCAL V and W cuts
    bool dc_chi2ndf;           // DC edge cuts (chi2/ndf based)
    bool dc_theta_1sigma;      // DC theta cuts at 1σ
    bool dc_theta_2sigma;      // DC theta cuts at 2σ
    bool dc_theta_3sigma;      // DC theta cuts at 3σ

    // Helper methods
    bool passes_all_required() const;           // All except theta
    bool passes_with_theta(int sigma_level) const;  // All including theta
};
```

---

## Integration with Your Analysis

### Example: Replacing Your select_good_electron Function

**Before (Your Code):**
```cpp
auto Reader::select_good_electron(
    const Core::Particle& electron,
    const hipo::bank& REC_Calorimeter,
    const hipo::bank& REC_Traj,
    const hipo::bank& REC_Track) -> bool {

    // 100+ lines of cut logic...
    const bool pass_chi2 = Cuts::MIN_CHI2PID < electron.chi2pid() && electron.chi2pid() < Cuts::MAX_CHI2PID;
    const bool pass_p = p_e > Cuts::MIN_ELECTRON_MOMENTUM;
    const bool pass_vz = Cuts::pass_electron_vz_cut(electron.vz(), m_target);
    // ... many more lines ...

    return pass_chi2 && pass_p && pass_vz && pass_SF && pass_cal && pass_chi2ndf;
}
```

**After (Using ElectronCuts):**
```cpp
auto Reader::select_good_electron(
    const Core::Particle& electron,
    const Core::CalorimeterBank& calorimeter,
    const Core::DCTrajBank& trajectory) -> bool {

    // One line!
    return Core::ElectronCuts::select_electron(
        electron, calorimeter, trajectory, m_target
    );
}
```

### With Detailed Results for Cut Flow

```cpp
auto Reader::select_good_electron_with_cutflow(
    const Core::Particle& electron,
    const Core::CalorimeterBank& calorimeter,
    const Core::DCTrajBank& trajectory) -> bool {

    // Get detailed results
    auto results = Core::ElectronCuts::apply_all_cuts(
        electron, calorimeter, trajectory, m_target
    );

    // Fill cutflow histograms
    if (results.sampling_fraction) m_counter.nb_of_event_sf++;
    if (results.sampling_fraction && results.dc_chi2ndf) m_counter.nb_of_event_sf_edge++;
    if (results.sampling_fraction && results.dc_chi2ndf && results.pcal_fiducial)
        m_counter.nb_of_event_sf_edge_vw++;
    // ... etc

    // Fill histograms before cuts
    m_histograms.electrons.hist1D_p->Get()->Fill(electron.p());
    // ...

    // Apply cuts
    bool passes = results.passes_all_required();

    if (passes) {
        // Fill histograms after cuts
        m_histograms.electrons.hist1D_p_cut->Get()->Fill(electron.p());
        // ...
    }

    return passes;
}
```

---

## Complete Example

### Full Analysis with ElectronCuts

```cpp
#include <Core/EventData.hpp>
#include <Core/Cuts.hpp>
#include <Core/Kinematics.hpp>

class MyAnalysis {
private:
    using Target = Core::ElectronCuts::Target;
    Target m_target = Target::LD2;

public:
    void analyze(const std::string& file) {
        using namespace Core::Kinematics;

        BeamConfig beam;  // 10.6 GeV default

        for (hipo::hipoeventfile events(file); auto& event : events) {
            Core::EventData evt(event);
            if (!evt.has_particles()) continue;

            for (const auto& electron : evt.electrons()) {
                // Get detector banks
                auto cal = evt.calorimeter(electron);
                auto traj = evt.trajectory(electron);
                auto track = evt.track(electron);

                // Calculate local theta (implement this based on your geometry)
                double theta1 = compute_local_theta_DC(traj.region1, track.sector);
                double theta2 = compute_local_theta_DC(traj.region2, track.sector);
                double theta3 = compute_local_theta_DC(traj.region3, track.sector);

                // Apply cuts and get detailed results
                auto results = Core::ElectronCuts::apply_all_cuts(
                    electron, cal, traj, m_target,
                    theta1, theta2, theta3
                );

                // Fill pre-cut histograms
                hist_p_before->Fill(electron.p());
                hist_theta_before->Fill(electron.theta());
                hist_vz_before->Fill(electron.vz());

                // Check cuts
                if (!results.passes_all_required()) continue;

                // Fill post-cut histograms
                hist_p_after->Fill(electron.p());
                hist_theta_after->Fill(electron.theta());
                hist_vz_after->Fill(electron.vz());

                // Calculate physics variables
                double q2 = Q2(electron, beam);
                double w = W(electron, beam);
                double xb = x_bjorken(electron, beam);

                // Fill physics histograms
                hist_Q2->Fill(q2);
                hist_W->Fill(w);
                hist_xB->Fill(xb);

                // Apply additional theta cuts if needed
                if (results.dc_theta_1sigma) {
                    hist_Q2_tight->Fill(q2);
                }
            }
        }
    }
};
```

---

## Constants Reference

All cut values are defined as `constexpr` in the `ElectronCuts` class:

| Constant | Value | Description |
|----------|-------|-------------|
| `MIN_CHI2PID` | -5.0 | Minimum chi2 PID |
| `MAX_CHI2PID` | 5.0 | Maximum chi2 PID |
| `MIN_ELECTRON_MOMENTUM` | 0.8 GeV | Minimum electron momentum |
| `MIN_PCAL_V` | 9.0 cm | PCAL V fiducial cut |
| `MIN_PCAL_W` | 9.0 cm | PCAL W fiducial cut |
| `MIN_EDGE_R1` | 1.680 cm | DC region 1 edge cut |
| `MIN_EDGE_R2` | 2.0 cm | DC region 2 edge cut |
| `MIN_EDGE_R3` | 8.750 cm | DC region 3 edge cut |
| `SF_N_SIGMA` | 3.5 | Sampling fraction n-sigma |
| `EB_SF_N_SIGMA` | 5.0 | Event builder SF n-sigma |

To customize these values, you can access them like:

```cpp
double min_p = Core::ElectronCuts::MIN_ELECTRON_MOMENTUM;
double min_chi2 = Core::ElectronCuts::MIN_CHI2PID;
```

---

## Summary

The `ElectronCuts` class provides:

✅ **All your existing cuts** - chi2PID, momentum, vertex, SF, PCAL, DC edge, DC theta
✅ **Target-specific** vertex cuts (LD2, CxC, Cu, Sn, CuSn)
✅ **Sector-dependent** sampling fraction cuts
✅ **Sigma-level** DC theta cuts (1σ, 2σ, 3σ)
✅ **Individual** and **combined** cut functions
✅ **Detailed results** structure for cut flow analysis
✅ **Zero overhead** - all functions are `inline` and `constexpr` where possible

This replaces your ~100 line `select_good_electron` function with a clean, reusable interface!
