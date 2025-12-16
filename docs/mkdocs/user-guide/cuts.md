# Physics Cuts

The framework provides reusable, composable cuts for particle selection.

## Electron Cuts

Complete electron identification cuts using services:

```cpp
#include <CLAS/ElectronCuts.hpp>
#include <Framework/Services/SamplingFractionService.hpp>

// Get the SamplingFractionService
auto sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");

// Apply basic cuts (excluding sampling fraction)
auto results = CLAS::ElectronCuts::apply_basic_cuts(
    electron, calorimeter, trajectory, CLAS::Target::LD2
);

// Apply sampling fraction cut using the service
bool pass_sf = sf_svc->passes_cut(SF, momentum, sector, CLAS::Target::LD2, CLAS::Polarity::Outbending);
```

### Individual Cuts

```cpp
#include <CLAS/ElectronCuts.hpp>

// Chi2 PID cut
bool pass_chi2 = CLAS::ElectronCuts::pass_chi2pid_cut(electron.chi2pid());

// Momentum cut
bool pass_p = CLAS::ElectronCuts::pass_electron_momentum_cut(electron.p());

// Vertex Z cut (target-specific)
bool pass_vz = CLAS::ElectronCuts::pass_electron_vz_cut(
    electron.vz(),
    CLAS::Target::LD2
);

// PCAL fiducial cut
bool pass_pcal = CLAS::ElectronCuts::pass_pcal_fiducial_cut(
    calorimeter.pcal.lv,
    calorimeter.pcal.lw
);

// Sampling fraction cut (using service)
auto sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");
bool pass_sf = sf_svc->passes_cut(SF, momentum, sector, CLAS::Target::LD2, CLAS::Polarity::Outbending);

// DC chi2/ndf cut
bool pass_dc = CLAS::ElectronCuts::pass_chi2ndf_cut(
    traj.region1.edge,
    traj.region2.edge,
    traj.region3.edge
);
```

### Detailed Cut Results

```cpp
auto results = CLAS::ElectronCuts::apply_all_cuts(
    electron,
    calorimeter,
    trajectory,
    CLAS::Target::LD2
);

// Check individual cuts
if (results.chi2pid) { /* passed */ }
if (results.momentum) { /* passed */ }
if (results.vz) { /* passed */ }
if (results.sampling_fraction) { /* passed */ }
if (results.pcal_fiducial) { /* passed */ }
if (results.dc_chi2ndf) { /* passed */ }

// Check combined
if (results.passes_all_required()) {
    // Passed all cuts
}
```

## Target Types

| Target | Vertex Z Range (cm) |
|--------|---------------------|
| `CLAS::Target::LD2` | (-15.0, 5.0) |
| `CLAS::Target::CxC` | (-10.6, 5.0) |
| `CLAS::Target::Cu` | (-10.6, -6.5) |
| `CLAS::Target::Sn` | (-5.5, 5.0) |
| `CLAS::Target::CuSn` | (-10.6, 5.0) |

## Polarity

```cpp
CLAS::Polarity::Inbending    // "IB"
CLAS::Polarity::Outbending   // "OB"
```

## Cut Services

### ElectronCutsService

```cpp
#include <Framework/Services/ElectronCutsService.hpp>

auto electron_cuts = service<Framework::ElectronCutsService>("ElectronCutsService");
electron_cuts->set_target(CLAS::Target::LD2);
electron_cuts->set_polarity(CLAS::Polarity::Outbending);

if (electron_cuts->pass_chi2pid_cut(electron.chi2pid()) &&
    electron_cuts->pass_momentum_cut(electron.p()) &&
    electron_cuts->pass_vz_cut(electron.vz())) {
    // Electron passes basic cuts
}
```

### SamplingFractionService

```cpp
#include <Framework/Services/SamplingFractionService.hpp>

auto sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");

bool passes = sf_svc->passes_cut(SF, momentum, sector, 
    CLAS::Target::LD2, CLAS::Polarity::Outbending);
```
