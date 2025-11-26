# CLAS12 Analysis Framework

**A modern particle physics analysis framework for CLAS12 data**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Framework Version](https://img.shields.io/badge/version-1.0-blue)]()
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

---

## Quick Start

### Prerequisites
- C++20 compiler
- ROOT 6.x
- Meson build system

### Installation

```bash
# Clone the repository
git clone https://gitlab.com/MathieuOuillon/clas-analysis clas12-framework
git checkout test2
cd clas12-framework

# Configure build
meson setup build

# Compile
meson compile
```

### Your First Analysis

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"

int main() {
    // Create application
    Framework::Application app("MyAnalysis");

    // Add histogram service
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add electron selection algorithm
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>();
    app.add_algorithm(std::move(electron_alg));

    // Configure input
    app.set_input_files({"data/run_5032.hipo"});
    app.set_max_events(10000);
    app.set_num_threads(4);

    // Run
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

---

## Features

### 🏗️ Architecture

**Service Layer**
- `HistogramService` - Centralized histogram management with automatic ROOT file writing
- `ConditionsService` - Run-dependent calibrations with Interval of Validity (IOV)
- `ServiceManager` - Lifecycle management for all services

**Algorithm Framework**
- Base `Algorithm` class with initialize/execute/finalize lifecycle
- Automatic service access via dependency injection
- Clean separation between framework and physics code

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