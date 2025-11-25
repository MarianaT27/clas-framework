# CLAS12 Analysis Framework v2.0

## Overview

The CLAS12 Analysis Framework has been significantly upgraded following CERN/LHC software best practices (inspired by Gaudi, Athena, and CMSSW). This document describes the new architecture and how to use it.

## What's New in v2.0

### Phase 1 Improvements ✅

1. **Bug Fixes**
   - Fixed logical operator bugs in `Reader.cpp` (bitwise `&` → logical `&&`)

2. **Cutflow System**
   - Automatic event counting and efficiency tracking
   - Thread-safe counters for parallel processing
   - Beautiful formatted output tables

3. **Unit Testing**
   - Google Test integration
   - Test suite for core components
   - Run with: `meson test -C build`

4. **Python Configuration** (optional)
   - CMSSW-style Python job configs
   - No recompilation for parameter changes
   - Bindings via pybind11

### Phase 2 Improvements ✅

5. **Service-Oriented Architecture**
   - `HistogramService` - Centralized histogram management
   - `ConditionsService` - Run-dependent calibrations with IOV
   - `ServiceManager` - Service lifecycle management

6. **Event Data Model (EDM)**
   - High-level `Event` class abstracting HIPO banks
   - Lazy loading for performance
   - Event store for algorithm communication

7. **Algorithm Framework**
   - Base `Algorithm` class with lifecycle (initialize/execute/finalize)
   - Access to services via dependency injection
   - Clear separation of concerns

8. **Conditions Database**
   - Interval of Validity (IOV) system
   - Run-dependent calibrations
   - Ready for SQLite backend integration

---

## Architecture

### Directory Structure

```
clas12-framework/
├── include/
│   ├── Framework/           # Framework core
│   │   ├── IAlgorithm.hpp
│   │   ├── IService.hpp
│   │   ├── Event.hpp
│   │   ├── Cutflow.hpp
│   │   ├── Application.hpp
│   │   └── Services/
│   │       ├── HistogramService.hpp
│   │       └── ConditionsService.hpp
│   ├── Algorithms/          # Analysis algorithms
│   │   └── ElectronSelectionAlg.hpp
│   ├── Core/                # Physics utilities
│   └── study1/              # Legacy code
├── src/
│   ├── example_framework.cpp  # New framework example
│   └── main.cpp               # Legacy entry point
├── test/                    # Unit tests
├── python/                  # Python bindings
└── config/                  # Python configs
```

### Component Overview

```
┌─────────────────────────────────────────────────┐
│              Application                        │
│  - Manages services & algorithms                │
│  - Runs event loop                              │
└─────────────────┬───────────────────────────────┘
                  │
        ┌─────────┴──────────┐
        │                    │
   ┌────▼─────┐       ┌──────▼─────────┐
   │ Services │       │   Algorithms   │
   │          │       │                │
   │ Histogram├──────►│ ElectronSel    │
   │ Conditions│      │ PionID         │
   │ ...      │       │ ...            │
   └──────────┘       └────────┬───────┘
                               │
                        ┌──────▼────────┐
                        │     Event     │
                        │  (EDM Layer)  │
                        └───────────────┘
```

---

## Quick Start

### Building the Framework

```bash
# Configure (first time only)
meson setup build

# Compile
cd build
meson compile

# Run tests
meson test

# Run example
./framework_example
```

### Your First Analysis

#### Option 1: C++ (Recommended for Performance)

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"

int main() {
    // Create application
    Framework::Application app("MyAnalysis");

    // Add services
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add algorithms
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        Core::Target::LD2,
        Core::Polarity::Outbending
    );
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

#### Option 2: Python (Easiest for Prototyping)

```python
import pyclas12 as c12

app = c12.Application("MyAnalysis")

# Services
hist_svc = c12.HistogramService()
hist_svc.set_output_file("output.root")
app.service_manager().add_service(hist_svc)

# Input
app.set_input_files(["data/run_5032.hipo"])
app.set_max_events(10000)
app.set_num_threads(4)

# Run
app.initialize()
app.run()
app.finalize()
```

---

## Writing an Algorithm

### Step 1: Create Your Algorithm

```cpp
// include/Algorithms/MyAnalysisAlg.hpp
#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/Event.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Services/HistogramService.hpp"

class MyAnalysisAlg : public Framework::Algorithm {
public:
    explicit MyAnalysisAlg(std::string name = "MyAnalysis")
        : Algorithm(std::move(name))
        , m_cutflow("My Cutflow") {}

    auto initialize() -> Framework::StatusCode override {
        // Get services
        m_hist_svc = service<Framework::HistogramService>("HistogramService");

        // Book histograms
        h_momentum = m_hist_svc->book_hist1D(
            "/MyAnalysis/momentum",
            "Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Passes Cuts");

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get particles
        const auto& electrons = event.particles(Core::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        // Apply your analysis
        const auto& e = electrons[0];
        if (e.p() > 1.0) {
            m_cutflow.pass("Passes Cuts");
            h_momentum->Fill(e.p());
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::Cutflow m_cutflow;
    Framework::HistogramService* m_hist_svc = nullptr;
    TH1D* h_momentum = nullptr;
};
```

### Step 2: Use It in Your Main Program

```cpp
#include "Algorithms/MyAnalysisAlg.hpp"

int main() {
    Framework::Application app("MyApp");

    // Add services (histogram, conditions, etc.)
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add your algorithm
    app.add_algorithm(std::make_unique<MyAnalysisAlg>());

    // Configure and run
    app.set_input_files({...});
    app.initialize();
    app.run();
    app.finalize();
}
```

---

## Using the Cutflow System

The Cutflow system automatically tracks event counts through your selection:

```cpp
Framework::Cutflow cutflow("Electron Selection");

// Define your cuts
cutflow.add_cut("All Events");
cutflow.add_cut("Trigger Electron");
cutflow.add_cut("Chi2 PID");
cutflow.add_cut("Momentum > 0.8 GeV");
cutflow.add_cut("Vertex Z");

// In event loop
cutflow.pass("All Events");
if (!has_trigger_electron) return;
cutflow.pass("Trigger Electron");
if (!pass_chi2pid) return;
cutflow.pass("Chi2 PID");
// ... etc

// Print cutflow table
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
Momentum > 0.8 GeV                      39887    39.89%    94.72%
Vertex Z                                38234    38.23%    95.86%
═══════════════════════════════════════════════════════════════
```

---

## Using Services

### HistogramService

Centralized histogram management with automatic ROOT file writing:

```cpp
// In initialize()
auto* hist_svc = service<Framework::HistogramService>("HistogramService");

// Book histograms with directory structure
TH1D* h_p = hist_svc->book_hist1D(
    "/ElectronKinematics/momentum",
    "Electron Momentum;p [GeV/c];Events",
    120, 0, 12
);

TH2D* h_p_theta = hist_svc->book_hist2D(
    "/ElectronKinematics/p_vs_theta",
    "p vs #theta;#theta [deg];p [GeV/c]",
    180, 0, 180, 120, 0, 12
);

// Fill in execute()
h_p->Fill(electron.p());
h_p_theta->Fill(electron.theta(), electron.p());

// Histograms are automatically written in finalize()
```

### ConditionsService

Access run-dependent calibrations:

```cpp
// In initialize()
auto* cond_svc = service<Framework::ConditionsService>("ConditionsService");
cond_svc->set_run_number(5032);

// Get calibration constants
auto sf_params = cond_svc->get_sampling_fraction_params(
    Core::Target::LD2,
    Core::Polarity::Outbending,
    sector
);

double time_offset = cond_svc->get_time_offset("FTOF", sector);

// Set custom conditions with IOV
IntervalOfValidity iov{5000, 5100};  // Valid for runs 5000-5100
cond_svc->set_value("my_calib", 1.23, iov);
```

---

## Event Data Model

The `Event` class provides high-level access to event data:

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Get all particles (lazy loaded)
    const auto& all_particles = event.particles();

    // Get specific particle type
    const auto& electrons = event.particles(Core::PIDs::ELECTRON);
    const auto& pions = event.particles(Core::PIDs::PIPLUS);

    // Get event metadata
    double start_time = event.start_time();
    long evt_num = event.event_number();

    // Store/retrieve custom objects for sharing between algorithms
    event.store("selected_electron", electrons[0]);
    auto e = event.retrieve<Core::Particle>("selected_electron");

    return Framework::StatusCode::SUCCESS();
}
```

---

## Running Tests

```bash
cd build

# Run all tests
meson test

# Run specific test
meson test test_cutflow

# Verbose output
meson test --verbose
```

### Writing Your Own Tests

```cpp
// test/test_my_feature.cpp
#include <gtest/gtest.h>
#include "MyFeature.hpp"

TEST(MyFeatureTest, BasicTest) {
    MyFeature feature;
    EXPECT_EQ(feature.compute(), 42);
}

TEST(MyFeatureTest, EdgeCase) {
    MyFeature feature;
    EXPECT_THROW(feature.invalid_operation(), std::runtime_error);
}
```

Add to `test/meson.build`:
```meson
test_my_feature = executable(
  'test_my_feature',
  'test_my_feature.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)
test('My Feature Tests', test_my_feature)
```

---

## Migration Guide

### From Old Framework to v2.0

**Old Way (study1/Reader.cpp):**
```cpp
class Reader {
    void operator()(hipo::iter_event& event) {
        // Manual bank access
        hipo::bank_map REC_Particle = event.get_bank_map("REC::Particle");

        // Inline cuts with manual counters
        m_event_count++;
        if (!has_electron) return;
        m_nb_trigger_electron++;

        // Manual histogram filling
        m_histograms.electron.hist_momentum->Get()->Fill(p);
    }
};
```

**New Way (Framework v2.0):**
```cpp
class ElectronSelectionAlg : public Framework::Algorithm {
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // High-level particle access
        const auto& electrons = event.particles(Core::PIDs::ELECTRON);

        // Automatic cutflow tracking
        m_cutflow.pass("All Events");
        if (electrons.empty()) return StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        // Service-managed histograms
        h_momentum->Fill(electrons[0].p());

        return StatusCode::SUCCESS();
    }
};
```

**Benefits:**
- Cleaner, more maintainable code
- Automatic cutflow management
- Easier to add/remove cuts
- Better separation of concerns
- Testable components

---

## Performance Considerations

### Parallel Processing

The framework uses HIPO's parallel processing capabilities:

```cpp
app.set_num_threads(8);  // Use 8 threads
```

All histograms are thread-safe via ROOT's `TThreadedObject`.

### Memory Management

- Services are managed by `ServiceManager`
- Algorithms are managed by `Application`
- Histograms are owned by `HistogramService`
- Events use lazy loading to minimize memory

---

## Best Practices

### 1. One Algorithm, One Purpose

Each algorithm should do one thing well:
- ✅ `ElectronSelectionAlg` - Select electrons
- ✅ `PionIdentificationAlg` - Identify pions
- ❌ `DoEverythingAlg` - Bad!

### 2. Use Services for Shared Functionality

- ✅ Histogram management → `HistogramService`
- ✅ Calibrations → `ConditionsService`
- ✅ Particle ID → `ParticleIDService` (to be implemented)

### 3. Always Use Cutflow

Track your selection efficiency:
```cpp
m_cutflow.add_cut("All Events");
m_cutflow.add_cut("Each");
m_cutflow.add_cut("Cut");
m_cutflow.add_cut("You");
m_cutflow.add_cut("Apply");
```

### 4. Write Tests

Test your physics cuts and utilities:
```cpp
TEST(ElectronCuts, MomentumCut) {
    EXPECT_TRUE(ElectronCuts::pass_electron_momentum_cut(2.5));
    EXPECT_FALSE(ElectronCuts::pass_electron_momentum_cut(0.5));
}
```

### 5. Document Your Code

Use Doxygen-style comments:
```cpp
/**
 * @brief Select high-momentum electrons
 * @param electrons Input electron collection
 * @param min_p Minimum momentum in GeV/c
 * @return Selected electrons
 */
auto select_electrons(const std::vector<Electron>& electrons, double min_p);
```

---

## Troubleshooting

### Build Issues

**Problem:** Can't find ROOT
```bash
# Make sure ROOT is in your PATH
source /path/to/root/bin/thisroot.sh
```

**Problem:** Tests fail to compile
```bash
# Make sure gtest.wrap is present
ls subprojects/gtest.wrap
```

### Runtime Issues

**Problem:** Service not found
```cpp
// Make sure you added the service!
app.service_manager().add_service(std::make_unique<MyService>());
```

**Problem:** Histogram is nullptr
```cpp
// Book histogram in initialize(), not constructor!
auto initialize() -> StatusCode override {
    h_momentum = m_hist_svc->book_hist1D(...);
}
```

---

## What's Next?

### Planned Features (Phase 3 & 4)

- RDataFrame integration for columnar analysis
- Composite particle reconstruction
- Systematic uncertainty framework
- Batch job submission tools
- Web-based monitoring
- Container distribution

### Contributing

We welcome contributions! Areas where help is needed:
- More physics algorithms (pion ID, DIS selection, etc.)
- Additional services (geometry, MC matching)
- Documentation improvements
- Example analyses

---

## References

- **Gaudi Framework**: https://gaudi.web.cern.ch/
- **Athena Framework**: https://atlassoftwaredocs.web.cern.ch/
- **CMSSW**: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBook
- **ROOT**: https://root.cern.ch/

---

## Support

- GitHub Issues: https://github.com/your-repo/issues
- Documentation: `docs/` directory
- Examples: `src/example_framework.cpp`

**Happy analyzing!** 🎉
