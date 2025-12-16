# Physics Cuts

The framework provides reusable, composable cuts for particle selection.

## Electron Cuts

### Basic Cuts

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
```

### Calorimeter Cuts

```cpp
// PCAL fiducial cut
bool pass_pcal = CLAS::ElectronCuts::pass_pcal_fiducial_cut(
    calorimeter.pcal.lv,
    calorimeter.pcal.lw
);

// Sampling fraction cut
bool pass_sf = CLAS::ElectronCuts::pass_sampling_fraction_cut(
    SF, momentum, sector,
    CLAS::Target::LD2,
    CLAS::Polarity::Outbending
);
```

### DC Cuts

```cpp
// DC chi2/ndf cut
bool pass_dc = CLAS::ElectronCuts::pass_chi2ndf_cut(
    traj.region1.edge,
    traj.region2.edge,
    traj.region3.edge
);
```
