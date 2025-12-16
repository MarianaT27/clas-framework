# Cuts

Physics cuts for particle selection.

## ElectronCuts

```cpp
namespace CLAS {

class ElectronCuts {
public:
    // Basic cuts
    static auto pass_chi2pid_cut(double chi2pid) -> bool;
    static auto pass_electron_momentum_cut(double p) -> bool;
    static auto pass_electron_vz_cut(double vz, Target target) -> bool;
    
    // Calorimeter cuts
    static auto pass_pcal_fiducial_cut(double lv, double lw) -> bool;
    static auto pass_sampling_fraction_cut(
        double sf, double p, int sector,
        Target target, Polarity polarity) -> bool;
    
    // DC cuts
    static auto pass_chi2ndf_cut(
        double edge1, double edge2, double edge3) -> bool;
    
    // Combined cuts
    static auto apply_basic_cuts(
        const Particle& electron,
        const Calorimeter& cal,
        const Trajectory& traj,
        Target target) -> CutResults;
    
    static auto apply_all_cuts(
        const Particle& electron,
        const Calorimeter& cal,
        const Trajectory& traj,
        Target target) -> CutResults;
};

}
```

## CutResults

```cpp
struct CutResults {
    bool chi2pid = false;
    bool momentum = false;
    bool vz = false;
    bool sampling_fraction = false;
    bool pcal_fiducial = false;
    bool dc_chi2ndf = false;
    
    auto passes_all_required() const -> bool;
};
```

## Target Types

```cpp
enum class Target {
    LD2,    // Liquid Deuterium: vz ∈ (-15.0, 5.0) cm
    CxC,    // Carbon: vz ∈ (-10.6, 5.0) cm
    Cu,     // Copper: vz ∈ (-10.6, -6.5) cm
    Sn,     // Tin: vz ∈ (-5.5, 5.0) cm
    CuSn    // Copper-Tin: vz ∈ (-10.6, 5.0) cm
};
```

## Polarity

```cpp
enum class Polarity {
    Inbending,   // "IB"
    Outbending   // "OB"
};
```
