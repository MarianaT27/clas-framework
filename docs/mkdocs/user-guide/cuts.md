# Physics Cuts

The framework provides reusable, composable cuts for particle selection.
All electron identification cuts live on `ElectronCutsService`; the
obsolete static `clas12::ElectronCuts` helper was removed.

## Electron Cuts

Register `ElectronCutsService` in the application builder, then look it
up from your algorithm:

```cpp
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>

auto app = clas12::framework::make_application("MyAnalysis")
    .with_standard_services()
    .add_service<clas12::framework::ElectronCutsService>()
    .add_service<clas12::framework::SamplingFractionService>()
    .build();
```

Inside an algorithm:

```cpp
auto on_initialize() -> clas12::framework::StatusCode override {
    m_electron_cuts = svc<clas12::framework::ElectronCutsService>();
    if (!m_electron_cuts) {
        return clas12::framework::StatusCode::FAILURE();
    }

    // Optional: override defaults
    clas12::framework::ElectronCutsService::Config config;
    config.target = clas12::Target::LD2;
    config.polarity = clas12::Polarity::Outbending;
    config.pcal_level = clas12::framework::ElectronCutsService::PCALFiducialLevel::Medium;
    m_electron_cuts->set_config(config);

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Individual Cuts

```cpp
// Chi2 PID cut (uses configured [min_chi2pid, max_chi2pid])
bool pass_chi2 = m_electron_cuts->pass_chi2pid_cut(electron.chi2pid());

// Momentum cut (uses configured min_electron_momentum)
bool pass_p = m_electron_cuts->pass_momentum_cut(electron.p());

// Vertex Z cut (target-specific; uses the configured target)
bool pass_vz = m_electron_cuts->pass_vz_cut(electron.vz());

// Or override target per call:
bool pass_vz2 = m_electron_cuts->pass_vz_cut(electron.vz(), clas12::Target::LD2);

// PCAL fiducial cut (uses the configured PCALFiducialLevel)
bool pass_pcal = m_electron_cuts->pass_pcal_fiducial_cut(
    calorimeter.pcal.lv,
    calorimeter.pcal.lw);

// Drift chamber edge cut (static)
bool pass_dc = clas12::framework::ElectronCutsService::pass_dc_edge_cut(
    traj.region1.edge,
    traj.region2.edge,
    traj.region3.edge);
```

### Sampling Fraction

The sampling fraction cut needs `SamplingFractionService` — wire it into
`ElectronCutsService` during initialization:

```cpp
auto* sf_svc = svc<clas12::framework::SamplingFractionService>();
m_electron_cuts->set_sf_service(sf_svc);

bool pass_sf = m_electron_cuts->pass_sampling_fraction_cut(
    SF, momentum, sector);
```

### Detailed Cut Results

```cpp
auto results = m_electron_cuts->apply_basic_cuts(electron, calorimeter, traj);

if (results.chi2pid)        { /* passed */ }
if (results.momentum)       { /* passed */ }
if (results.vz)             { /* passed */ }
if (results.pcal_fiducial)  { /* passed */ }
if (results.dc_chi2ndf)     { /* passed */ }

if (m_electron_cuts->passes_basic_cuts(electron, calorimeter, traj)) {
    // Electron passed everything except sampling fraction
}
```

## Target Types

| Target | Vertex Z Range (cm) |
|--------|---------------------|
| `clas12::Target::LD2`  | (-15.0, 5.0) |
| `clas12::Target::CxC`  | (-10.6, 5.0) |
| `clas12::Target::Cu`   | (-10.6, -6.5) |
| `clas12::Target::Sn`   | (-5.5, 5.0) |
| `clas12::Target::CuSn` | (-10.6, 5.0) |

## Polarity

```cpp
clas12::Polarity::Inbending    // "IB"
clas12::Polarity::Outbending   // "OB"
```
