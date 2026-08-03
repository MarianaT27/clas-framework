# CLAS12 Analysis Framework

**A modern particle physics analysis framework for CLAS12 data**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Framework Version](https://img.shields.io/badge/version-2.0-blue)]()
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()
[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://mathieuouillon.github.io/clas-framework)

📚 **[Full Documentation](https://mathieuouillon.github.io/clas-framework)** | 📖 **[API Reference](https://mathieuouillon.github.io/clas-framework/api/)**

---

## Quick Start

### Prerequisites
- C++20 compiler — GCC 12+ or Clang 14+ (coroutine support required)
- [ROOT](https://root.cern/install/) 6.x with `root-config` on `PATH`
- [Meson](https://mesonbuild.com/) ≥ 1.0 and Ninja
- Eigen3 (`apt install libeigen3-dev` / `brew install eigen`)
- Other deps (fmt, lz4, tomlplusplus, gtest) are vendored as Meson subprojects

### Installation

```bash
# Clone the repository
git clone https://github.com/mathieuouillon/clas-framework.git
cd clas-framework

# Configure build
meson setup build

# Compile
meson compile -C build

# Run the test suite
meson test -C build
```

### Troubleshooting

- **`root-config: command not found`** — ROOT is not on your PATH. Source
  ROOT's `thisroot.sh` (Linux/macOS) or activate the appropriate
  environment module before `meson setup`.
- **`error: 'co_yield' was not declared in this scope`** — your compiler
  is too old. GCC 12+ or Clang 14+ is required for C++20 coroutines.
- **HIPO link errors** — make sure the in-tree HIPO4 build under
  `extern/hipo4` was built (it builds as a meson subdir; a stale
  `build/` directory can hold an old version — `rm -rf build && meson
  setup build` to reset).

### Running an Analysis

Every analysis binary takes exactly one CLI argument: a TOML config file.

```bash
./build/analyses/electron_analysis --config config/electron_analysis.toml
```

The TOML drives every knob — input files, output, threads, percentage,
target/polarity, and per-algorithm `Property` values.

```toml
[run]
input       = ["data/run_5032.hipo"]
output      = "electron_analysis.root"
num_threads = 4
percentage  = 100.0

[analysis]
target   = "LD2"
polarity = "Outbending"

# Per-algorithm sections — one per algorithm the binary registers.
# Section name = the string passed to add_algorithm<...>("...").
[ElectronAnalysis]
enable_chi2pid_cut       = true
enable_momentum_cut      = true
enable_vz_cut            = true
enable_sf_cut            = true
enable_pcal_fiducial_cut = true
enable_dc_chi2ndf_cut    = true
```

Section keys must match `Property::name()` declarations on the
algorithm. Missing keys keep the C++ default; unknown keys log a
warning at startup.

### Writing an Analysis

The `main()` only declares which algorithms + services compose this
analysis; everything else comes from the TOML.

```cpp
#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Electron Analysis", "electron_analysis",
        [](auto& builder, const auto& config) {
            builder
                .add_service<clas12::framework::ElectronCutsService>()
                .add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .add_algorithm<clas12::algorithms::ElectronAnalysisAlg>(
                    "ElectronAnalysis",
                    {.target = config.target, .polarity = config.polarity});
        });
}
```

Sample TOML files are in [config/](config/).

---

## Features

### 🏗️ Architecture

**ApplicationBuilder Pattern** (New in v2.0)
- Fluent API for cleaner application setup
- `with_standard_services()` adds all common services automatically
- Type-safe algorithm configuration with constructor arguments

**Service Layer**
- `HistogramService` - Centralized histogram management with automatic ROOT file writing
- `ConditionsService` - Run-dependent calibrations with Interval of Validity (IOV)
- `PhotonCutsService` - AI/ML-based photon selection with GBT models
- `ParticleSelectorService` - Configurable particle extraction and filtering
- `SamplingFractionService` - Target/polarity-dependent sampling fraction cuts
- `MetadataService` - Analysis metadata storage
- `ServiceManager` - Lifecycle management for all services

**Algorithm Framework**
- Base `Algorithm` class with initialize/execute/finalize lifecycle
- Specialized types: `MonitoringAlgorithm`, `FilterAlgorithm`
- `ServiceRef<T>` for type-safe service access — declared via
  `required_services(...)` in the constructor and auto-injected by the
  framework before `on_initialize()` runs
- Clean separation between framework and physics code

**Built-in Algorithms**
- `ElectronSelectionAlg` - Electron identification with full cut suite
- `DISSelectionAlg` - DIS event selection with configurable kinematic regions
- `Pi0ReconstructionAlg` - π⁰ reconstruction from photon pairs
- `EventMonitor` - Real-time processing statistics

**Event Data Model**
- High-level `Event` class abstracting HIPO banks
- Lazy loading for optimal performance
- Type-safe particle access by PID
- Event store for inter-algorithm communication

### 📊 Analysis Tools

**Cutflow System**
```cpp
Framework::Cutflow cutflow("Electron Selection");
cutflow.add_cut("All Events");
cutflow.add_cut("Trigger Electron");
cutflow.add_cut("Chi2 PID");

// In event loop:
cutflow.pass("All Events");
if (has_trigger) cutflow.pass("Trigger Electron");
if (pass_chi2) cutflow.pass("Chi2 PID");

// Print results:
cutflow.print();
```

**Output:**
```
═══════════════════════════════════════════════════════════════
  Cutflow: Electron Selection
═══════════════════════════════════════════════════════════════
Cut                                    Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                             100000    100.0%    100.0%
Trigger Electron                        45230    45.23%    45.23%
Chi2 PID                                42108    42.11%    93.10%
═══════════════════════════════════════════════════════════════
```

### 📁 Example Analyses

| File | Description |
|------|-------------|
| [analyses/pi0/pi0.cpp](analyses/pi0/pi0.cpp) | Pi0 reconstruction using ApplicationBuilder |
| [examples/02_application_builder/example_builder.cpp](examples/02_application_builder/example_builder.cpp) | ApplicationBuilder pattern demo |
| [examples/04_framework/example_framework.cpp](examples/04_framework/example_framework.cpp) | Full framework example with all services |
| [analyses/electron/electron_analysis.cpp](analyses/electron/electron_analysis.cpp) | Electron analysis |
| [analyses/lambda/lambda.cpp](analyses/lambda/lambda.cpp) | Lambda reconstruction |

See [CONTRIBUTING.md](CONTRIBUTING.md) for coding style, DI conventions,
and the PR checklist.

## References

**Inspiration**
- [Gaudi Framework](https://gaudi.web.cern.ch/) (LHCb/ATLAS)
- [Athena Framework](https://atlassoftwaredocs.web.cern.ch/) (ATLAS)
- [CMSSW](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBook) (CMS)
- [ROOT](https://root.cern.ch/)

**CLAS12**
- [CLAS12 Wiki](https://clasweb.jlab.org/wiki/)
- [HIPO Library](https://github.com/gavalian/hipo)

---

## License

MIT License - See [LICENSE](LICENSE) for details

---

## Authors

Mathieu Ouillon