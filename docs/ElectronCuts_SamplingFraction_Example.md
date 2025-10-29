# Using ElectronCuts with Full Sampling Fraction Cut

This guide shows how to use the `ElectronCuts` class with the full `SamplingFractionCut` that includes target and polarity dependence.

## Quick Example

```cpp
#include <Core/Cuts.hpp>
#include <Core/EventData.hpp>
#include <Core/SamplingFractionCut.hpp>
#include <Core/Target.hpp>

class MyReader {
private:
    Core::Target m_target = Core::Target::LD2;
    Core::Polarity m_polarity = Core::Polarity::Outbending;
    Core::SamplingFractionCut m_sf_cut;  // Initialize once

public:
    void analyze(const std::string& file) {
        for (hipo::hipoeventfile events(file); auto& event : events) {
            Core::EventData evt(event);
            if (!evt.has_particles()) continue;

            for (const auto& electron : evt.electrons()) {
                // Get detector banks
                auto cal = evt.calorimeter(electron);
                auto traj = evt.trajectory(electron);

                // Apply all cuts with full SF cut
                bool passes = Core::ElectronCuts::select_electron(
                    electron,
                    calorimeter,
                    trajectory,
                    m_target,
                    m_polarity,
                    m_sf_cut
                );

                if (passes) {
                    // Process good electron
                }
            }
        }
    }
};
```

## Detailed Usage

### 1. Initialize Components

```cpp
// Setup your analysis configuration
Core::Target target = Core::Target::LD2;
Core::Polarity polarity = Core::Polarity::Outbending;

// Create SF cut object (initialization happens in constructor)
Core::SamplingFractionCut sf_cut;
```

### 2. Apply Individual SF Cut

```cpp
auto cal = evt.calorimeter(electron);

double E_tot = cal.pcal.energy + cal.inner.energy + cal.outer.energy;
double SF = E_tot / electron.p();
double p = electron.p();
int sector = cal.pcal.sector;

// Full production SF cut
bool pass_SF = Core::ElectronCuts::pass_sampling_fraction_cut(
    SF, p, sector, target, polarity, sf_cut
);

// With custom n-sigma
bool pass_SF_5sigma = Core::ElectronCuts::pass_sampling_fraction_cut(
    SF, p, sector, target, polarity, sf_cut, 5.0
);
```

### 3. Apply All Cuts with Detailed Results

```cpp
auto results = Core::ElectronCuts::apply_all_cuts(
    electron,
    calorimeter,
    trajectory,
    target,
    polarity,
    sf_cut,
    theta1,  // Local theta DC R1
    theta2,  // Local theta DC R2
    theta3   // Local theta DC R3
);

// Check individual cuts
if (results.chi2pid) { /* passed chi2pid */ }
if (results.momentum) { /* passed momentum */ }
if (results.vz) { /* passed vertex */ }
if (results.sampling_fraction) { /* passed SF with target/polarity */ }
if (results.pcal_fiducial) { /* passed PCAL */ }
if (results.dc_chi2ndf) { /* passed DC edge */ }

// Check combined
if (results.passes_all_required()) {
    // Passed all cuts except theta
}

if (results.passes_with_theta(1)) {
    // Passed all cuts including 1σ theta
}
```

### 4. Complete Analysis Example

```cpp
#include <Core/Cuts.hpp>
#include <Core/EventData.hpp>
#include <Core/SamplingFractionCut.hpp>
#include <Core/Kinematics.hpp>

class ElectronAnalysis {
private:
    Core::Target m_target;
    Core::Polarity m_polarity;
    Core::SamplingFractionCut m_sf_cut;

    // Counters for cut flow
    struct Counters {
        int total = 0;
        int pass_sf = 0;
        int pass_sf_edge = 0;
        int pass_sf_edge_vw = 0;
        int pass_all = 0;
    } m_counters;

public:
    explicit ElectronAnalysis(Core::Target target, Core::Polarity polarity)
        : m_target(target), m_polarity(polarity) {}

    void process_event(hipo::event& event) {
        Core::EventData evt(event);
        if (!evt.has_particles()) return;

        for (const auto& electron : evt.electrons()) {
            m_counters.total++;

            // Get detector banks
            auto cal = evt.calorimeter(electron);
            auto traj = evt.trajectory(electron);
            auto track = evt.track(electron);

            // Calculate local theta for DC regions
            double theta1 = compute_local_theta_DC(traj.region1, track.sector);
            double theta2 = compute_local_theta_DC(traj.region2, track.sector);
            double theta3 = compute_local_theta_DC(traj.region3, track.sector);

            // Apply all cuts
            auto results = Core::ElectronCuts::apply_all_cuts(
                electron, cal, traj,
                m_target, m_polarity, m_sf_cut,
                theta1, theta2, theta3
            );

            // Cut flow counting
            if (results.sampling_fraction) m_counters.pass_sf++;
            if (results.sampling_fraction && results.dc_chi2ndf) m_counters.pass_sf_edge++;
            if (results.sampling_fraction && results.dc_chi2ndf && results.pcal_fiducial)
                m_counters.pass_sf_edge_vw++;

            // Final selection
            if (!results.passes_all_required()) continue;
            m_counters.pass_all++;

            // Fill histograms after cuts
            hist_p_cut->Fill(electron.p());
            hist_theta_cut->Fill(electron.theta());
            hist_phi_cut->Fill(electron.phi());

            // Calculate physics variables
            double q2 = Core::Kinematics::Q2(electron);
            double w = Core::Kinematics::W(electron);

            hist_Q2->Fill(q2);
            hist_W->Fill(w);
        }
    }

    void print_summary() {
        std::cout << "Cut Flow:\n";
        std::cout << "  Total electrons: " << m_counters.total << "\n";
        std::cout << "  Pass SF: " << m_counters.pass_sf << "\n";
        std::cout << "  Pass SF+Edge: " << m_counters.pass_sf_edge << "\n";
        std::cout << "  Pass SF+Edge+PCAL: " << m_counters.pass_sf_edge_vw << "\n";
        std::cout << "  Pass All: " << m_counters.pass_all << "\n";
    }
};

// Usage
int main() {
    ElectronAnalysis analysis(Core::Target::LD2, Core::Polarity::Outbending);

    for (hipo::hipoeventfile events("data.hipo"); auto& event : events) {
        analysis.process_event(event);
    }

    analysis.print_summary();
}
```

## Alternative: Event Builder Version (No Target/Polarity)

If you don't have access to a `SamplingFractionCut` object or want simpler cuts:

```cpp
// Use Event Builder version (sector-dependent only)
bool passes = Core::ElectronCuts::select_electron_eb(
    electron,
    calorimeter,
    trajectory,
    target  // Only need target for vertex cut
);

// Or with detailed results
auto results = Core::ElectronCuts::apply_all_cuts_eb(
    electron,
    calorimeter,
    trajectory,
    target,
    theta1, theta2, theta3
);
```

## Integration with Your Existing Code

### Your Original Code:
```cpp
const bool pass_SF = Cuts::pass_sampling_fraction_cut(SF, p_e, sector, m_target, m_polarity);
```

### New Equivalent:
```cpp
const bool pass_SF = Core::ElectronCuts::pass_sampling_fraction_cut(
    SF, p_e, sector, m_target, m_polarity, m_sf_cut
);
```

### Complete select_good_electron Replacement:

**Before:**
```cpp
auto Reader::select_good_electron(
    const Core::Particle& electron,
    const hipo::bank& REC_Calorimeter,
    const hipo::bank& REC_Traj,
    const hipo::bank& REC_Track) -> bool {

    // Read banks
    const Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter, electron.index());
    const Core::DCTrajBank traj = Core::read_Traj_bank(REC_Traj, electron.index());

    // 100+ lines of cut logic...
    const bool pass_chi2 = Cuts::MIN_CHI2PID < electron.chi2pid() && electron.chi2pid() < Cuts::MAX_CHI2PID;
    const bool pass_p = p_e > Cuts::MIN_ELECTRON_MOMENTUM;
    const bool pass_vz = Cuts::pass_electron_vz_cut(electron.vz(), m_target);
    const bool pass_SF = Cuts::pass_sampling_fraction_cut(SF, p_e, sector, m_target, m_polarity);
    const bool pass_cal = Cuts::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw);
    const bool pass_chi2ndf = Cuts::pass_chi2ndf_cut(edge1, edge2, edge3);

    return pass_chi2 && pass_p && pass_vz && pass_SF && pass_chi2ndf && pass_cal;
}
```

**After:**
```cpp
auto Reader::select_good_electron(
    const Core::Particle& electron,
    const hipo::bank& REC_Calorimeter,
    const hipo::bank& REC_Traj,
    const hipo::bank& REC_Track) -> bool {

    // Read banks
    const Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter, electron.index());
    const Core::DCTrajBank traj = Core::read_Traj_bank(REC_Traj, electron.index());

    // One line!
    return Core::ElectronCuts::select_electron(
        electron, calorimeter, traj,
        m_target, m_polarity, m_sf_cut
    );
}
```

## Key Points

1. **Initialize `SamplingFractionCut` once** - Store as a member variable in your Reader class
2. **Pass by reference** - The SF cut object is passed by const reference (no copying)
3. **Target and Polarity required** - You must specify both for the full production cut
4. **Backwards compatible** - Individual cut functions still available for custom logic
5. **Two versions available**:
   - `apply_all_cuts()` - Full version with target/polarity/SF cut
   - `apply_all_cuts_eb()` - Event Builder version (simpler, no SF cut object needed)

## Target and Polarity Types

```cpp
// Available targets
Core::Target::LD2
Core::Target::CxC
Core::Target::Cu
Core::Target::Sn
Core::Target::CuSn

// Available polarities
Core::Polarity::Inbending   // "IB"
Core::Polarity::Outbending  // "OB"

// Convert to string for SF cut
std::string target_str = Core::to_string(Core::Target::LD2);        // "LD2"
std::string polarity_str = Core::to_string(Core::Polarity::Outbending);  // "OB"
```

## Summary

The `ElectronCuts` class now fully supports your existing sampling fraction cut with:
- ✅ Target-dependent cuts (LD2, CxC, Cu, Sn, CuSn)
- ✅ Polarity-dependent cuts (Inbending, Outbending)
- ✅ Sector-dependent parameterization
- ✅ Momentum-dependent evaluation
- ✅ Customizable n-sigma levels
- ✅ All your other electron ID cuts integrated

Use `select_electron()` with your `SamplingFractionCut` object for full production quality cuts!
