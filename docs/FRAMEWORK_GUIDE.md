# CLAS12 Analysis Framework - Complete Guide

**A modern particle physics analysis framework for CLAS12 data**

Version 2.0 | Following CERN/LHC software best practices (Gaudi/Athena/CMSSW)

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Quick Start](#2-quick-start)
3. [Framework Architecture](#3-framework-architecture)
4. [Writing Your First Analysis](#4-writing-your-first-analysis)
5. [Services](#5-services)
6. [Particle Selection](#6-particle-selection)
7. [Physics Cuts](#7-physics-cuts)
8. [Kinematics Calculations](#8-kinematics-calculations)
9. [Complete Examples](#9-complete-examples)
10. [Advanced Features](#10-advanced-features)
11. [Migration Guide](#11-migration-guide)
12. [Troubleshooting](#12-troubleshooting)

---

## 1. Introduction

### What is this Framework?

The CLAS12 Analysis Framework is a modern C++20 framework designed to make physics analysis:
- **Cleaner** - Less boilerplate, more physics
- **Safer** - Type-safe interfaces, automated testing
- **Faster** - Built-in parallelization, optimized data structures
- **Maintainable** - Modular design, reusable components

### Key Features

✅ **Service-Oriented Architecture** - Modular, reusable services (Histogram, Logging, N-tuples, Conditions)
✅ **Algorithm Framework** - Clean separation with initialize/execute/finalize lifecycle
✅ **Event Data Model** - High-level interface abstracting HIPO banks
✅ **Cutflow System** - Automatic efficiency tracking with formatted output
✅ **Unit Testing** - Google Test integration
✅ **Particle Selection** - One-line particle extraction
✅ **Physics Cuts** - Reusable, composable cut system
✅ **Kinematics** - Standard DIS and exclusive calculations

### What's New in v2.0

**Phase 1 & 2 Complete:**
- Service-oriented architecture (HistogramService, ConditionsService, LoggingService, NTupleService)
- Algorithm framework with lifecycle management
- Event Data Model with lazy loading
- Cutflow system with automatic tracking
- Comprehensive physics utilities (ParticleSelector, Cuts, Kinematics)
- Unit testing infrastructure
- Bug fixes (13 logical operator bugs in Reader.cpp)

---

## 2. Quick Start

### Prerequisites

- **C++20 compiler** (GCC 10+, Clang 12+, or LLVM)
- **ROOT 6.x** (for histograms and trees)
- **Meson build system**
- **Python 3** (optional, for Python bindings)

### Building the Framework

```bash
# Clone or navigate to framework directory
cd clas12-framework

# Configure build (first time only)
meson setup build

# Compile
meson compile -C build

# Run tests
meson test -C build

# Run example
./build/main_improved
```

### Your First Analysis (5 Minutes)

Create a file `my_analysis.cpp`:

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"

int main() {
    // Create application
    Framework::Application app("MyAnalysis");

    // Add histogram service
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("my_output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add electron selection algorithm
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        CLAS::Target::LD2,
        CLAS::Polarity::Outbending
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

Compile and run:
```bash
g++ -std=c++20 my_analysis.cpp -o my_analysis `root-config --cflags --libs`
./my_analysis
```

---

## 3. Framework Architecture

### Component Overview

```
┌─────────────────────────────────────────────────┐
│              Application                        │
│  - Manages services & algorithms                │
│  - Runs event loop                              │
│  - Handles parallelization                      │
└─────────────┬───────────────────────────────────┘
              │
        ┌─────┴──────────┐
        │                │
   ┌────▼─────┐    ┌─────▼──────────┐
   │ Services │    │   Algorithms   │
   │          │    │                │
   │ Histogram│◄───┤ ElectronSel    │
   │ Logging  │    │ PionID         │
   │ NTuple   │    │ Analysis       │
   │ Conditions    │ Monitor        │
   └──────────┘    └────────┬───────┘
                            │
                     ┌──────▼────────┐
                     │     Event     │
                     │  (EDM Layer)  │
                     │ - Particles   │
                     │ - Detector    │
                     │ - Store       │
                     └───────────────┘
```

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
│   │       ├── LoggingService.hpp
│   │       ├── NTupleService.hpp
│   │       └── ConditionsService.hpp
│   ├── Algorithms/          # Analysis algorithms
│   │   ├── ElectronSelectionAlg.hpp
│   │   ├── ElectronAnalysisAlg.hpp
│   │   └── PionSelectionAlg.hpp
│   ├── Core/                # Physics utilities
│   │   ├── Particle.hpp
│   │   ├── ParticleSelector.hpp
│   │   ├── Cuts.hpp
│   │   ├── Kinematics.hpp
│   │   ├── ReadBank.hpp
│   │   └── Target.hpp
│   └── Physics/             # Physics calculations
├── src/
│   ├── main_improved.cpp    # Main example
│   └── Framework/
│       └── Histogram.cpp
├── test/                    # Unit tests
├── docs/                    # Documentation
└── meson.build              # Build configuration
```

### Key Concepts

**Services**: Long-lived objects providing functionality to algorithms
- HistogramService: Manage histograms and ROOT files
- LoggingService: Structured logging with levels
- NTupleService: ROOT TTree management
- ConditionsService: Run-dependent calibrations

**Algorithms**: Process events with initialize/execute/finalize lifecycle
- FilterAlgorithm: Event selection (returns true/false)
- AnalysisAlgorithm: Fill histograms, compute physics
- ProducerAlgorithm: Create new data products

**Event**: High-level interface to event data
- Lazy loading for performance
- Access particles, detector banks
- Store/retrieve custom objects

---

## 4. Writing Your First Analysis

### Step 1: Create Your Algorithm

Create `include/Algorithms/MyAnalysisAlg.hpp`:

```cpp
#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/Event.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "TH1D.h"

namespace Algorithms {

class MyAnalysisAlg : public Framework::Algorithm {
public:
    explicit MyAnalysisAlg(std::string name = "MyAnalysis")
        : Algorithm(std::move(name))
        , m_cutflow("My Cutflow") {}

    auto initialize() -> Framework::StatusCode override {
        // Get services
        m_log_svc = service<Framework::LoggingService>("LoggingService");
        m_hist_svc = service<Framework::HistogramService>("HistogramService");

        if (m_log_svc) {
            LOG_INFO(m_log_svc, "Initializing algorithm");
        }

        if (!m_hist_svc) {
            fmt::print(stderr, "ERROR: Could not get HistogramService\n");
            return Framework::StatusCode::FAILURE();
        }

        // Book histograms
        h_momentum = m_hist_svc->book_hist1D(
            "/MyAnalysis/momentum",
            "Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        h_theta = m_hist_svc->book_hist1D(
            "/MyAnalysis/theta",
            "Theta;#theta [deg];Events",
            180, 0, 180
        );

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Momentum > 1 GeV");
        m_cutflow.add_cut("Forward Angle");

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        // Select first electron
        const auto& e = electrons[0];

        // Apply cuts
        if (e.p() < 1.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum > 1 GeV");

        if (e.theta() > 35.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Forward Angle");

        // Fill histograms
        h_momentum->Fill(e.p());
        h_theta->Fill(e.theta());

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        if (m_log_svc) {
            LOG_INFO(m_log_svc, "Finalizing algorithm");
        }

        fmt::print("\n");
        m_cutflow.print();

        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::Cutflow m_cutflow;
    Framework::LoggingService* m_log_svc = nullptr;
    Framework::HistogramService* m_hist_svc = nullptr;

    // Histograms
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
};

}  // namespace Algorithms
```

### Step 2: Use Your Algorithm

Create `src/my_main.cpp`:

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Algorithms/MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    // Create application
    Framework::Application app("MyAnalysis");

    // Add services
    auto log_svc = std::make_unique<Framework::LoggingService>();
    log_svc->set_log_level(Framework::LogLevel::INFO);
    app.service_manager().add_service(std::move(log_svc));

    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("my_output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add algorithm
    app.add_algorithm(std::make_unique<Algorithms::MyAnalysisAlg>());

    // Configure input
    app.set_input_files({argv[1]});
    app.set_max_events(10000);
    app.set_num_threads(4);

    // Run
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

### Step 3: Build and Run

Add to `meson.build`:
```meson
my_analysis_sources = files(
  'src/Core/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/my_main.cpp',
)

executable(
  'my_analysis',
  my_analysis_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)
```

Build and run:
```bash
meson compile -C build
./build/my_analysis data/run_5032.hipo
```

---

## 5. Services

Services are long-lived objects that provide functionality to algorithms. They are managed by the ServiceManager and initialized once at the start of the application.

### 5.1 HistogramService

Centralized histogram management with automatic ROOT file writing.

**Setup:**
```cpp
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(hist_svc));
```

**Usage in Algorithm:**
```cpp
auto initialize() -> Framework::StatusCode override {
    m_hist_svc = service<Framework::HistogramService>("HistogramService");

    // Book 1D histogram
    h_momentum = m_hist_svc->book_hist1D(
        "/Electron/momentum",           // Path in ROOT file
        "Electron Momentum;p [GeV/c];Events",
        100, 0, 10                      // bins, min, max
    );

    // Book 2D histogram
    h_p_vs_theta = m_hist_svc->book_hist2D(
        "/Electron/p_vs_theta",
        "p vs #theta;#theta [deg];p [GeV/c]",
        180, 0, 180,  // theta bins
        100, 0, 10    // p bins
    );

    return Framework::StatusCode::SUCCESS();
}

auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Fill histograms
    h_momentum->Fill(electron.p());
    h_p_vs_theta->Fill(electron.theta(), electron.p());

    return Framework::StatusCode::SUCCESS();
}
// Histograms automatically written in finalize()
```

**Features:**
- Automatic ROOT file management
- Thread-safe filling
- Organized directory structure
- No manual histogram deletion needed

### 5.2 LoggingService

Structured logging with levels and colors.

**Setup:**
```cpp
auto log_svc = std::make_unique<Framework::LoggingService>();
log_svc->set_log_level(Framework::LogLevel::INFO);
log_svc->set_log_file("analysis.log");  // Optional file output
app.service_manager().add_service(std::move(log_svc));
```

**Usage:**
```cpp
auto initialize() -> Framework::StatusCode override {
    m_log_svc = service<Framework::LoggingService>("LoggingService");

    if (m_log_svc) {
        LOG_DEBUG(m_log_svc, "Debug message");
        LOG_INFO(m_log_svc, "Info message");
        LOG_WARNING(m_log_svc, "Warning message");
        LOG_ERROR(m_log_svc, "Error message");
        LOG_CRITICAL(m_log_svc, "Critical message");

        // With formatting
        LOG_INFO(m_log_svc, "Processing run {}, {} events", run, n_events);
    }

    return Framework::StatusCode::SUCCESS();
}
```

**Log Levels:**
- `DEBUG`: Detailed diagnostic information
- `INFO`: General information
- `WARNING`: Warning messages
- `ERROR`: Error messages
- `CRITICAL`: Critical errors

### 5.3 NTupleService

ROOT TTree management for event-by-event data.

**Setup:**
```cpp
auto ntuple_svc = std::make_unique<Framework::NTupleService>();
ntuple_svc->set_output_file("output.root");  // Can share with histograms
app.service_manager().add_service(std::move(ntuple_svc));
```

**Usage:**
```cpp
class MyAnalysisAlg : public Framework::Algorithm {
private:
    Framework::NTupleService* m_ntuple_svc = nullptr;

    // N-tuple variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_q2 = 0.0;
    double m_w = 0.0;

    auto initialize() -> Framework::StatusCode override {
        m_ntuple_svc = service<Framework::NTupleService>("NTupleService");

        if (m_ntuple_svc) {
            // Register branches
            m_ntuple_svc->register_branch("electron_data", "e_p", &m_e_p);
            m_ntuple_svc->register_branch("electron_data", "e_theta", &m_e_theta);
            m_ntuple_svc->register_branch("electron_data", "e_phi", &m_e_phi);
            m_ntuple_svc->register_branch("electron_data", "q2", &m_q2);
            m_ntuple_svc->register_branch("electron_data", "w", &m_w);
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // Get electron
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();

        const auto& e = electrons[0];

        // Fill variables
        m_e_p = e.p();
        m_e_theta = e.theta();
        m_e_phi = e.phi();
        auto kin = Physics::DISAnalysis::calculate_kinematics(e);
        m_q2 = kin.q2;
        m_w = kin.w;

        // Fill N-tuple
        if (m_ntuple_svc) {
            m_ntuple_svc->fill("electron_data");
        }

        return Framework::StatusCode::SUCCESS();
    }
};
```

**Access in ROOT:**
```bash
root output.root
root [1] electron_data->Draw("q2:w")
root [2] electron_data->Scan("e_p:e_theta:q2")
```

### 5.4 ConditionsService

Run-dependent calibrations with Interval of Validity (IOV).

**Setup:**
```cpp
auto cond_svc = std::make_unique<Framework::ConditionsService>();
cond_svc->set_run_number(5032);
app.service_manager().add_service(std::move(cond_svc));
```

**Usage:**
```cpp
auto initialize() -> Framework::StatusCode override {
    m_cond_svc = service<Framework::ConditionsService>("ConditionsService");

    if (m_cond_svc) {
        // Get run-dependent constants
        double time_offset = m_cond_svc->get_time_offset("FTOF", sector);

        // Set custom calibrations with IOV
        Framework::IntervalOfValidity iov{5000, 5100};  // Runs 5000-5100
        m_cond_svc->set_value("my_calibration", 1.23, iov);
    }

    return Framework::StatusCode::SUCCESS();
}
```

---

## 6. Particle Selection

Particle selection is handled through the `ParticleSelectorService`, which provides a service-based, configurable approach that can be shared across algorithms.

### 6.1 ParticleSelectorService

The service-based approach integrates with the framework architecture.

**Setup:**
```cpp
auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();
// Optionally configure default particle types
particle_svc->set_default_types({
    CLAS::PIDs::ELECTRON, CLAS::PIDs::PHOTON,
    CLAS::PIDs::PROTON, CLAS::PIDs::PIPLUS, CLAS::PIDs::PIMINUS
});
app.service_manager().add_service(std::move(particle_svc));
```

**Usage in Algorithm:**
```cpp
#include <Framework/Services/ParticleSelectorService.hpp>

class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::ParticleSelectorService* m_particle_svc = nullptr;

    auto initialize() -> Framework::StatusCode override {
        m_particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");
        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        auto& bank = event.get_bank("REC::Particle");

        // Extract all particles
        auto particles = m_particle_svc->extract_particles(bank);

        // Get specific particle types
        const auto& electrons = m_particle_svc->get_electrons(particles);
        const auto& photons = m_particle_svc->get_photons(particles);
        const auto& protons = m_particle_svc->get_protons(particles);

        // Apply filters
        auto good_electrons = m_particle_svc->with_momentum_range(electrons, 1.0, 10.0);
        auto fd_electrons = m_particle_svc->get_forward_detector(good_electrons);

        return Framework::StatusCode::SUCCESS();
    }
};
```

**Available Filters:**
```cpp
// Momentum range
auto filtered = m_particle_svc->with_momentum_range(particles, min_p, max_p);

// Vertex z range
auto filtered = m_particle_svc->with_vz_range(particles, min_vz, max_vz);

// Theta angle range
auto filtered = m_particle_svc->with_theta_range(particles, min_theta, max_theta);

// Chi2pid range
auto filtered = m_particle_svc->with_chi2pid_range(particles, min_chi2, max_chi2);

// Charge
auto filtered = m_particle_svc->with_charge(particles, charge);

// Detector regions
auto fd = m_particle_svc->get_forward_detector(particles);   // Status 2000-4000
auto cd = m_particle_svc->get_central_detector(particles);   // Status 4000-6000

// Custom filter
auto filtered = m_particle_svc->filter(particles, [](const CLAS::Particle& p) {
    return p.E() > 5.0 && p.theta() < 30.0;
});
```

### 6.2 Using with Event Store

The `Event` class provides convenient particle access:

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Get particles by type (lazy loaded)
    const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
    const auto& pions = event.particles(CLAS::PIDs::PIPLUS);

    // Filter using service or static methods
    auto good_electrons = m_particle_svc->with_momentum_range(electrons, 1.0, 10.0);

    return Framework::StatusCode::SUCCESS();
}
```

---

## 7. Physics Cuts

The cuts system provides reusable, composable cuts for particle selection.

### Electron Cuts

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

**Individual Cuts:**

```cpp
// Chi2 PID
bool pass_chi2 = CLAS::ElectronCuts::pass_chi2pid_cut(electron.chi2pid());

// Momentum
bool pass_p = CLAS::ElectronCuts::pass_electron_momentum_cut(electron.p());

// Vertex Z (target-specific)
bool pass_vz = CLAS::ElectronCuts::pass_electron_vz_cut(
    electron.vz(),
    CLAS::Target::LD2
);

// PCAL fiducial
bool pass_pcal = CLAS::ElectronCuts::pass_pcal_fiducial_cut(
    calorimeter.pcal.lv,
    calorimeter.pcal.lw
);

// Sampling fraction (using service)
auto sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");
bool pass_sf = sf_svc->passes_cut(SF, momentum, sector, CLAS::Target::LD2, CLAS::Polarity::Outbending);

// DC chi2/ndf
bool pass_dc = CLAS::ElectronCuts::pass_chi2ndf_cut(
    traj.region1.edge,
    traj.region2.edge,
    traj.region3.edge
);
```

**Detailed Cut Results:**

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

### Target Types

```cpp
CLAS::Target::LD2     // Liquid Deuterium: vz ∈ (-15.0, 5.0) cm
CLAS::Target::CxC     // Carbon: vz ∈ (-10.6, 5.0) cm
CLAS::Target::Cu      // Copper: vz ∈ (-10.6, -6.5) cm
CLAS::Target::Sn      // Tin: vz ∈ (-5.5, 5.0) cm
CLAS::Target::CuSn    // Copper-Tin: vz ∈ (-10.6, 5.0) cm
```

### Polarity

```cpp
CLAS::Polarity::Inbending    // "IB"
CLAS::Polarity::Outbending   // "OB"
```

---

## 8. Kinematics Calculations

Standard DIS and exclusive kinematics calculations.

### DIS Kinematics

```cpp
#include <Physics/Kinematics.hpp>
#include <Physics/DISAnalysis.hpp>

// Create beam and target particles
CLAS::Particle beam(11, 0, -1, -1, CLAS::Constants::ELECTRONMASS,
                    0, 0, 10.53, 10.53, 0, 0, 0, 0, 1.0, 0);
CLAS::Particle target(2212, 0, -1, 1, CLAS::Constants::PROTONMASS,
                      0, 0, 0, CLAS::Constants::PROTONMASS, 0, 0, 0, 0, 0, 0);

// Calculate DIS variables using KinematicCalculator
double q2 = Physics::KinematicCalculator::calculate_Q2(beam, electron);
double w = Physics::KinematicCalculator::calculate_W(beam, electron, target);
double y = Physics::KinematicCalculator::calculate_y(beam, electron);
double nu_val = beam.E() - electron.E();
double xb = Physics::KinematicCalculator::calculate_x_bjorken(q2, nu_val);

std::cout << "Q² = " << q2 << " GeV²\n";
std::cout << "W = " << w << " GeV\n";
std::cout << "x_B = " << xb << "\n";

// Or use DISAnalysis for convenience
auto kin = Physics::DISAnalysis::calculate_kinematics(electron);
std::cout << "Q² = " << kin.q2 << ", W = " << kin.w << "\n";
```

### Exclusive Reactions

```cpp
// Missing mass using KinematicCalculator
double mm = Physics::KinematicCalculator::missing_mass(beam, target, {electron, proton});

// Invariant mass using CompositeParticle
Physics::CompositeParticle pi0({photon1, photon2});
double m_gg = pi0.invariant_mass();

// Opening angle
double angle = Physics::KinematicCalculator::opening_angle(photon1, photon2);
double p_miss = missing_momentum(electron, {proton, pion}, beam);
```

### Geometric Calculations

```cpp
// Angle between particles
double angle = angle_between(particle1, particle2);  // degrees

// Relative phi angle
double dphi = delta_phi(particle1, particle2);  // [-180, 180]
```

### Available Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `Q2(e, beam)` | Four-momentum transfer squared | GeV² |
| `W(e, beam)` | Invariant mass of hadronic system | GeV |
| `nu(e, beam)` | Energy transfer | GeV |
| `x_bjorken(e, beam)` | Bjorken x | dimensionless |
| `inelasticity(e, beam)` | Inelasticity y | dimensionless |
| `missing_mass(e, p, beam)` | Missing mass | GeV |
| `invariant_mass(p1, p2)` | Two-particle invariant mass | GeV |
| `momentum_transfer_t(e, meson, beam)` | -t for meson production | GeV² |
| `missing_energy(e, particles, beam)` | Missing energy | GeV |
| `missing_momentum(e, particles, beam)` | Missing momentum | GeV |
| `angle_between(p1, p2)` | Opening angle | degrees |
| `delta_phi(p1, p2)` | Relative phi angle | degrees |

---

## 9. Complete Examples

### Example 1: Basic Electron Analysis

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "CLAS/Target.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    // Create application
    Framework::Application app("ElectronAnalysis");

    // Setup services
    auto log_svc = std::make_unique<Framework::LoggingService>();
    log_svc->set_log_level(Framework::LogLevel::INFO);
    log_svc->set_log_file("electron_analysis.log");
    app.service_manager().add_service(std::move(log_svc));

    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("electron_analysis.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add algorithm
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        CLAS::Target::LD2,
        CLAS::Polarity::Outbending
    );
    app.add_algorithm(std::move(electron_alg));

    // Configure
    app.set_input_files({argv[1]});
    app.set_max_events(100000);
    app.set_num_threads(4);

    // Run
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

### Example 2: DIS Analysis with Kinematics

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Algorithms/ElectronAnalysisAlg.hpp"
#include "CLAS/Target.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    Framework::Application app("DIS_Analysis");

    // Services
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("dis_analysis.root");
    app.service_manager().add_service(std::move(hist_svc));

    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    ntuple_svc->set_output_file("dis_analysis.root");
    app.service_manager().add_service(std::move(ntuple_svc));

    // Algorithm with full cuts and kinematics
    auto analysis_alg = std::make_unique<Algorithms::ElectronAnalysisAlg>(
        "ElectronAnalysis",
        CLAS::Target::LD2,
        CLAS::Polarity::Outbending,
        false  // systematics disabled
    );
    app.add_algorithm(std::move(analysis_alg));

    // Configure
    app.set_input_files({argv[1]});
    app.set_max_events(-1);  // Process all events
    app.set_num_threads(8);

    // Run
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

### Example 3: Multi-Particle Analysis (ep → e'p'π+)

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/PionSelectionAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    Framework::Application app("ExclusivePion");

    // Services
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("exclusive_pion.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Algorithms
    // 1. Electron selection (sets "HasGoodElectron" decision)
    auto electron_alg = std::make_unique<Algorithms::ElectronAnalysisAlg>(
        "ElectronAnalysis",
        CLAS::Target::LD2,
        CLAS::Polarity::Outbending
    );
    app.add_algorithm(std::move(electron_alg));

    // 2. Pion selection (runs only if HasGoodElectron)
    auto pion_alg = std::make_unique<Algorithms::PionSelectionAlg>(
        "PionSelection",
        CLAS::Target::LD2,
        true  // separate pi+ and pi-
    );
    app.add_algorithm(std::move(pion_alg));

    // Configure
    app.set_input_files({argv[1]});
    app.set_max_events(100000);
    app.set_num_threads(4);

    // Run
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

---

## 10. Advanced Features

### 10.1 Cutflow System

The Cutflow system automatically tracks event counts through your selection:

```cpp
Framework::Cutflow cutflow("Electron Selection");

// Define cuts
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
Cut                          Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                   100000    100.0%    100.0%
Trigger Electron              45230    45.23%    45.23%
Chi2 PID                      42108    42.11%    93.10%
Momentum > 0.8 GeV            39887    39.89%    94.72%
Vertex Z                      38234    38.23%    95.86%
═══════════════════════════════════════════════════════════════
```

### 10.2 Parallel Processing

The framework uses HIPO's parallel processing capabilities:

```cpp
app.set_num_threads(8);  // Use 8 threads
```

All histograms and services are thread-safe.

### 10.3 Event Store

Share data between algorithms:

```cpp
// In first algorithm
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
    auto trigger_e = find_trigger_electron(electrons);

    // Store for downstream algorithms
    event.store("trigger_electron", trigger_e.value());
    event.set_decision("HasGoodElectron", true);

    return Framework::StatusCode::SUCCESS();
}

// In second algorithm
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Only process if first algorithm passed
    if (!event.passes_decision("HasGoodElectron")) {
        return Framework::StatusCode::SUCCESS();
    }

    // Retrieve stored electron
    auto electron = event.retrieve<CLAS::Particle>("trigger_electron");

    // Use electron...

    return Framework::StatusCode::SUCCESS();
}
```

### 10.4 Systematic Variations

Framework support for systematic uncertainties:

```cpp
// Enable systematics in algorithm constructor
auto analysis_alg = std::make_unique<Algorithms::ElectronAnalysisAlg>(
    "ElectronAnalysis",
    CLAS::Target::LD2,
    CLAS::Polarity::Outbending,
    true  // enable systematics
);
```

The framework will produce separate histograms for each systematic variation.

---

## 11. Migration Guide

### From Old Framework to v2.0

**Step 1: Update Includes**

**Before:**
```cpp
#include "study1/Reader.hpp"
#include "study1/Drawing.hpp"
```

**After:**
```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
```

**Step 2: Replace Manual Particle Extraction**

**Before:**
```cpp
std::vector<CLAS::Particle> electrons;
for (int i = 0; i < REC_Particle.getRows(); i++) {
    // Manual extraction...
}
```

**After:**
```cpp
const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
```

**Step 3: Use Algorithm Framework**

**Before:**
```cpp
class Reader {
    void operator()(hipo::iter_event& event) {
        // Inline processing
    }
};
```

**After:**
```cpp
class MyAnalysisAlg : public Framework::Algorithm {
    auto initialize() -> Framework::StatusCode override { /* ... */ }
    auto execute(Framework::Event& event) -> Framework::StatusCode override { /* ... */ }
    auto finalize() -> Framework::StatusCode override { /* ... */ }
};
```

**Step 4: Use Services**

**Before:**
```cpp
TFile* file = new TFile("output.root", "RECREATE");
TH1D* hist = new TH1D("h", "title", 100, 0, 10);
// ...
hist->Write();
file->Close();
delete hist;
delete file;
```

**After:**
```cpp
// In main
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(hist_svc));

// In algorithm
h_momentum = m_hist_svc->book_hist1D("/Analysis/momentum", "Momentum;p [GeV/c];Events", 100, 0, 10);
// Histograms automatically written, no manual cleanup
```

**Step 5: Use Built-in Cuts and Kinematics**

**Before:**
```cpp
// Manual cuts
bool pass = p > 1.0 && p < 10.0 &&
            vz > -10.0 && vz < 10.0 &&
            chi2pid > -3.0 && chi2pid < 3.0;

// Manual Q² calculation
double E_beam = 10.6;
double E_e = electron.E();
// ... many lines ...
double q2 = ...;
```

**After:**
```cpp
// Use ElectronCuts
bool passes = CLAS::ElectronCuts::select_electron(
    electron, calorimeter, trajectory, CLAS::Target::LD2
);

// Use Physics::DISAnalysis for kinematics
auto kin = Physics::DISAnalysis::calculate_kinematics(electron);
double q2 = kin.q2;
```

---

## 12. Troubleshooting

### Build Issues

**Problem:** Can't find ROOT
```bash
# Make sure ROOT is in your PATH
source /path/to/root/bin/thisroot.sh

# Or set ROOT environment
export ROOTSYS=/path/to/root
export PATH=$ROOTSYS/bin:$PATH
```

**Problem:** Tests fail to compile
```bash
# Make sure subprojects are downloaded
cd subprojects
# Download missing dependencies if needed
```

**Problem:** Compiler errors with LLVM 20 and fmt
```bash
# This is a known compatibility issue
# Solutions:
# 1. Use LLVM 18 or 19 instead
# 2. Update fmt subproject to latest version
# 3. Temporarily disable colored logging
```

### Runtime Issues

**Problem:** Service not found
```cpp
// Make sure you added the service!
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(hist_svc));
```

**Problem:** Histogram is nullptr
```cpp
// Book histogram in initialize(), not constructor!
auto initialize() -> StatusCode override {
    m_hist_svc = service<Framework::HistogramService>("HistogramService");
    h_momentum = m_hist_svc->book_hist1D(...);
}
```

**Problem:** Segmentation fault
```cpp
// Check that services are not null before using
if (m_hist_svc) {
    h_momentum->Fill(p);
}
```

**Problem:** Empty histograms
```cpp
// Make sure you're calling Fill() in execute()
// Check that cutflow is passing events
m_cutflow.print();  // Shows how many events pass each cut
```

### Performance Issues

**Problem:** Slow processing
```bash
# Increase number of threads
app.set_num_threads(8);

# Process fewer events for testing
app.set_max_events(10000);
```

**Problem:** High memory usage
```bash
# Use lazy loading (already default in Event)
# Process files in chunks
# Reduce histogram binning
```

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
- ✅ Logging → `LoggingService`
- ✅ N-tuples → `NTupleService`

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

## Summary

The CLAS12 Analysis Framework v2.0 provides:

✅ **Cleaner Code** - Less boilerplate, more physics
✅ **Type Safety** - Compile-time checking, fewer runtime errors
✅ **Performance** - Built-in parallelization, optimized data structures
✅ **Reusability** - Shared services and utilities
✅ **Maintainability** - Modular design, clear interfaces
✅ **Testing** - Unit tests for quality assurance
✅ **Documentation** - Comprehensive guides and examples

**Benefits:**
- ~75 lines saved per analysis
- Improved code readability
- Reduced error rates
- Faster development
- Better collaboration

---

## Resources

- **Examples**: `src/main_improved.cpp`, `src/example_framework.cpp`
- **Documentation**: `docs/` directory
- **Tests**: `test/` directory
- **Issue Tracker**: GitHub Issues

**References:**
- Gaudi Framework: https://gaudi.web.cern.ch/
- Athena Framework: https://atlassoftwaredocs.web.cern.ch/
- CMSSW: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBook
- ROOT: https://root.cern.ch/

---

**Happy analyzing!** 🎉

For questions or contributions, please open an issue or submit a pull request.
