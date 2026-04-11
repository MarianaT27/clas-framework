# Cuts

Physics cuts for particle selection. For electron identification, use
[`ElectronCutsService`](../framework/services.md) — all electron cut
functionality lives on the framework service side.

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
