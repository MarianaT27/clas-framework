# CLAS12 Analysis Framework

**A modern particle physics analysis framework for CLAS12 data**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Framework Version](https://img.shields.io/badge/version-2.0-blue)]()
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

---

## Overview

The CLAS12 Analysis Framework is a modern C++20 framework designed to make particle physics analysis:

- **Cleaner** - Less boilerplate, more physics
- **Safer** - Type-safe interfaces, automated testing
- **Faster** - Built-in parallelization, optimized data structures
- **Maintainable** - Modular design, reusable components

## Key Features

!!! success "ApplicationBuilder Pattern"
    Fluent API for cleaner application setup with `with_standard_services()` and type-safe algorithm configuration.

!!! success "Service-Oriented Architecture"
    Modular, reusable services including HistogramService, PhotonCutsService (AI/ML-based), ParticleSelectorService, and more.

!!! success "Algorithm Framework"
    Clean separation with initialize/execute/finalize lifecycle following Gaudi/Athena patterns. Includes specialized types like `MonitoringAlgorithm` and `FilterAlgorithm`.

!!! success "Event Data Model"
    High-level interface abstracting HIPO banks with lazy loading for optimal performance.

!!! success "Cutflow System"
    Automatic efficiency tracking with formatted output tables.

!!! success "Built-in Algorithms"
    Ready-to-use algorithms: ElectronSelectionAlg, DISSelectionAlg, Pi0ReconstructionAlg, EventMonitor.

## Quick Start

```bash
# Clone the repository
git clone https://gitlab.com/MathieuOuillon/clas-analysis clas12-framework
cd clas12-framework

# Configure and build
meson setup build
meson compile -C build

# Run example
./build/main_improved data/data.hipo
```

## Architecture

```mermaid
graph TB
    A[ApplicationBuilder] --> B[Application]
    B --> C[ServiceManager]
    B --> D[Algorithms]
    C --> E[HistogramService]
    C --> F[LoggingService]
    C --> G[PhotonCutsService]
    C --> H[ParticleSelectorService]
    C --> I[MetadataService]
    D --> J[EventMonitor]
    D --> K[ElectronSelectionAlg]
    D --> L[DISSelectionAlg]
    D --> M[Pi0ReconstructionAlg]
    J --> N[Event]
    K --> N
    L --> N
    M --> N
    N --> O[HIPO Banks]
```

## Example Analysis

Using the new **ApplicationBuilder** pattern (recommended):

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
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

## Documentation

| Section | Description |
|---------|-------------|
| [Getting Started](getting-started/installation.md) | Installation and setup |
| [User Guide](user-guide/framework.md) | Detailed framework documentation |
| [Examples](examples/electron-analysis.md) | Complete analysis examples |
| [API Reference](api/index.md) | Full API documentation |

## References

- [Gaudi Framework](https://gaudi.web.cern.ch/) (LHCb/ATLAS)
- [Athena Framework](https://atlassoftwaredocs.web.cern.ch/) (ATLAS)
- [CLAS12 Wiki](https://clasweb.jlab.org/wiki/)
- [HIPO Library](https://github.com/gavalian/hipo)

## License

MIT License - See [LICENSE](https://gitlab.com/MathieuOuillon/clas-framework/-/blob/main/LICENSE) for details.
