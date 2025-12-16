# CLAS12 Analysis Framework

**A modern particle physics analysis framework for CLAS12 data**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Framework Version](https://img.shields.io/badge/version-2.0-blue)]()
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()
[![Documentation](https://img.shields.io/badge/docs-GitLab%20Pages-blue)](https://mathieuouillon.gitlab.io/clas-framework)

📚 **[Full Documentation](https://mathieuouillon.gitlab.io/clas-framework)** | 📖 **[API Reference](https://mathieuouillon.gitlab.io/clas-framework/api/)**

---

## Quick Start

### Prerequisites
- C++20 compiler
- ROOT 6.x
- Meson build system

### Installation

```bash
# Clone the repository
git clone https://gitlab.com/MathieuOuillon/clas-framework.git
cd clas-framework

# Configure build
meson setup build

# Compile
meson compile
```

### Your First Analysis

Using the new ApplicationBuilder pattern (recommended):

```cpp
#include "Framework/ApplicationBuilder.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"

int main() {
    auto app = Framework::make_application("MyAnalysis")
        .with_input_files({"data/run_5032.hipo"})
        .with_output("output.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<Algorithms::ElectronSelectionAlg>(
            "ElectronSelection", CLAS::Target::LD2, CLAS::Polarity::Outbending)
        .build();

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

Or using the traditional approach:

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"

int main() {
    Framework::Application app("MyAnalysis");

    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");
    app.service_manager().add_service(std::move(hist_svc));

    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>();
    app.add_algorithm(std::move(electron_alg));

    app.set_input_files({"data/run_5032.hipo"});
    app.set_num_threads(4);

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

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
- `ServiceHandle` for type-safe service access with validation
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
| `src/pi0.cpp` | Pi0 reconstruction using ApplicationBuilder |
| `src/example_builder.cpp` | ApplicationBuilder pattern demo |
| `src/improved.cpp` | Full framework example with all services |
| `src/electron_analysis.cpp` | Electron analysis |
| `src/lambda.cpp` | Lambda reconstruction |

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