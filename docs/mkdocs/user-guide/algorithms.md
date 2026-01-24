# Algorithms

Algorithms are the building blocks of analysis. They process events and can access services for common functionality.

## Built-in Algorithms

The framework provides several ready-to-use algorithms organized by category:

### Selection Algorithms

| Algorithm | Location | Description |
|-----------|----------|-------------|
| `ElectronSelectionAlg` | `selection/` | Electron identification with full cut suite |
| `DISSelectionAlg` | `selection/` | DIS event selection with configurable kinematic regions |
| `PionSelectionAlg` | `selection/` | Pion identification |

### Reconstruction Algorithms

| Algorithm | Location | Description |
|-----------|----------|-------------|
| `Pi0ReconstructionAlg` | `reconstruction/` | π⁰ reconstruction from photon pairs |
| `LambdaReconstructionAlg` | `reconstruction/` | Λ reconstruction |
| `Rho0ReconstructionAlg` | `reconstruction/` | ρ⁰ reconstruction |

### Analysis Algorithms

| Algorithm | Location | Description |
|-----------|----------|-------------|
| `ElectronAnalysisAlg` | `analysis/` | Electron analysis with histograms and N-tuples |
| `PionAnalysisAlg` | `analysis/` | Pion analysis with histograms |

### Filter Algorithms

| Algorithm | Location | Description |
|-----------|----------|-------------|
| `TriggerElectronFilter` | `filters/` | Filter events with trigger electron |
| `DiPionFilterAlg` | `filters/` | Filter events with di-pion pairs |

### Monitoring Algorithms

| Algorithm | Location | Description |
|-----------|----------|-------------|
| `EventMonitor` | `monitoring/` | Real-time event processing statistics |

### Using Built-in Algorithms

```cpp
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>

auto app = clas12::framework::make_application("My Analysis")
    .with_standard_services()
    .add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
    .add_algorithm<clas12::algorithms::ElectronSelectionAlg>(
        "ElectronSelection", clas12::Target::LD2, clas12::Polarity::Outbending)
    .add_algorithm<clas12::algorithms::DISSelectionAlg>(
        "DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
    .add_algorithm<clas12::algorithms::Pi0ReconstructionAlg>(
        "Pi0Reconstruction", 0.2, 0.3)  // mass_window, min_energy
    .build();
```

## SimpleAlgorithm Base Class

All algorithms in the framework use the unified `SimpleAlgorithm<T>` CRTP base class. This provides:

- **Automatic service injection** via `require_service<T>()` in `configure()`
- **Clean lifecycle separation** between configuration and initialization
- **Consistent interface** across all algorithm types

```cpp
#include <clas12/framework/core/SimpleAlgorithm.hpp>

class MyAlgorithm : public clas12::framework::SimpleAlgorithm<MyAlgorithm> {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : SimpleAlgorithm(std::move(name)) {}

    auto configure() -> void override {
        // Request services (called automatically before on_initialize)
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        // Book histograms, setup resources
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Process each event
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        // Print statistics, cleanup
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
};
```

## Algorithm Patterns

### Filter Algorithms

Filter events by returning `StatusCode::FILTERED()` and setting decisions:

```cpp
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/PIDs.hpp>

class TriggerElectronFilter : public clas12::framework::SimpleAlgorithm<TriggerElectronFilter> {
public:
    explicit TriggerElectronFilter(std::string name = "TriggerElectronFilter")
        : SimpleAlgorithm(std::move(name)) {}

    auto configure() -> void override {
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        auto& hipo_event = event.hipo_event();
        auto& REC_Particle = hipo_event["REC::Particle"];

        auto particles = m_particle_svc->get()->extract_particles(
            REC_Particle, {clas12::PIDs::ELECTRON});
        const auto& electrons = m_particle_svc->get()->get_electrons(particles);

        if (electrons.empty()) {
            event.set_decision(name(), false);
            return clas12::framework::StatusCode::FILTERED();
        }

        event.set_decision(name(), true);
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
};
```

### Analysis Algorithms

Fill histograms and compute physics quantities:

```cpp
class ElectronAnalysisAlg : public clas12::framework::SimpleAlgorithm<ElectronAnalysisAlg> {
public:
    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysisAlg")
        : SimpleAlgorithm(std::move(name)) {}

    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        h_momentum = m_hist_svc->get()->book_hist1D("/Electron/p", "Momentum;p [GeV];Events", 100, 0, 10);
        h_theta = m_hist_svc->get()->book_hist1D("/Electron/theta", "Theta;#theta [deg];Events", 100, 0, 45);
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        if (!event.passes_decision("TriggerElectronFilter")) {
            return clas12::framework::StatusCode::SUCCESS();
        }

        auto& hipo_event = event.hipo_event();
        auto& REC_Particle = hipo_event["REC::Particle"];
        auto particles = m_particle_svc->get()->extract_particles(REC_Particle, {clas12::PIDs::ELECTRON});

        for (const auto& e : m_particle_svc->get()->get_electrons(particles)) {
            h_momentum->Fill(e.p());
            h_theta->Fill(e.theta() * 180.0 / M_PI);
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
};
```

### Producer Algorithms

Create new data products and store them in the event:

```cpp
#include <clas12/physics/CompositeParticle.hpp>

class Pi0Producer : public clas12::framework::SimpleAlgorithm<Pi0Producer> {
public:
    explicit Pi0Producer(std::string name = "Pi0Producer", double mass_center = 0.135, double mass_width = 0.050)
        : SimpleAlgorithm(std::move(name)), m_mass_center(mass_center), m_mass_width(mass_width) {}

    auto configure() -> void override {
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        auto& hipo_event = event.hipo_event();
        auto& REC_Particle = hipo_event["REC::Particle"];

        auto particles = m_particle_svc->get()->extract_particles(REC_Particle, {clas12::PIDs::PHOTON});
        const auto& photons = m_particle_svc->get()->get_photons(particles);

        auto pi0_candidates = clas12::physics::CompositeParticleBuilder::reconstruct_pi0(photons);

        std::vector<clas12::physics::CompositeParticle> good_pi0s;
        for (const auto& pi0 : pi0_candidates) {
            if (pi0.in_mass_window(m_mass_center, m_mass_width)) {
                good_pi0s.push_back(pi0);
            }
        }

        event.store("Pi0Candidates", good_pi0s);

        if (!good_pi0s.empty()) {
            event.set_decision("HasPi0", true);
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    double m_mass_center;
    double m_mass_width;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
};
```

## Algorithm Sequence

Algorithms execute in the order they are added:

```cpp
// 1. Filter: Select events with trigger electron
app.add_algorithm(std::make_unique<TriggerElectronFilter>());

// 2. Producer: Reconstruct pi0 candidates
app.add_algorithm(std::make_unique<Pi0Producer>());

// 3. Analysis: Fill histograms
app.add_algorithm(std::make_unique<Pi0AnalysisAlg>());
```

## Accessing Services

### Using require_service<T>() (Recommended)

With `SimpleAlgorithm<T>`, services are requested in `configure()` and accessed via `ServiceHandle` pointers:

```cpp
class MyAlgorithm : public clas12::framework::SimpleAlgorithm<MyAlgorithm> {
public:
    auto configure() -> void override {
        // Required service - throws if not available
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");

        // Optional service - returns nullptr if not available
        m_ntuple_svc = &optional_service<clas12::framework::NTupleService>("NTupleService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        // Services are guaranteed to be valid after configure()
        h_momentum = m_hist_svc->get()->book_hist1D("/Analysis/p", "Momentum", 100, 0, 10);

        // Check optional service before use
        if (m_ntuple_svc->is_valid()) {
            m_ntuple_svc->get()->create_ntuple("/Analysis/ntuple");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;
    TH1D* h_momentum = nullptr;
};
```

### Service Access Pattern

| Method | Description |
|--------|-------------|
| `require_service<T>(name)` | Request required service (throws if unavailable) |
| `optional_service<T>(name)` | Request optional service (returns nullptr handle) |
| `m_svc->is_valid()` | Check if service handle is valid |
| `m_svc->get()->method()` | Access service methods |

## Using Cutflow

Track selection efficiency:

```cpp
class MyAlgorithm : public clas12::framework::SimpleAlgorithm<MyAlgorithm> {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : SimpleAlgorithm(std::move(name)), m_cutflow("My Selection") {}

    auto configure() -> void override {
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Momentum > 1 GeV");
        m_cutflow.add_cut("Forward Angle");
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        auto& hipo_event = event.hipo_event();
        auto& REC_Particle = hipo_event["REC::Particle"];
        auto particles = m_particle_svc->get()->extract_particles(REC_Particle, {clas12::PIDs::ELECTRON});
        const auto& electrons = m_particle_svc->get()->get_electrons(particles);

        if (electrons.empty()) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Has Electron");

        if (electrons[0].p() < 1.0) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Momentum > 1 GeV");

        if (electrons[0].theta() > 35.0 * M_PI / 180.0) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Forward Angle");

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
};
```

Output:

```
═══════════════════════════════════════════════════════════════
  Cutflow: My Selection
═══════════════════════════════════════════════════════════════
Cut                                    Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                             100000    100.0%    100.0%
Has Electron                            45230     45.2%     45.2%
Momentum > 1 GeV                        42108     42.1%     93.1%
Forward Angle                           38542     38.5%     91.5%
═══════════════════════════════════════════════════════════════
```

## Configuration

Algorithms can be configured via constructor parameters:

```cpp
class ElectronSelectionAlg : public clas12::framework::SimpleAlgorithm<ElectronSelectionAlg> {
public:
    ElectronSelectionAlg(
        std::string name,
        clas12::Target target,
        clas12::Polarity polarity,
        bool enable_systematics = false
    ) : SimpleAlgorithm(std::move(name))
      , m_target(target)
      , m_polarity(polarity)
      , m_enable_systematics(enable_systematics) {}

    auto configure() -> void override {
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Use m_target, m_polarity in selection logic
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::Target m_target;
    clas12::Polarity m_polarity;
    bool m_enable_systematics;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
};

// Usage
auto alg = std::make_unique<ElectronSelectionAlg>(
    "ElectronSelection",
    clas12::Target::LD2,
    clas12::Polarity::Outbending,
    false
);
```

## Include Paths Reference

| Component | Include Path |
|-----------|-------------|
| Selection algorithms | `<clas12/algorithms/selection/ElectronSelectionAlg.hpp>` |
| Reconstruction algorithms | `<clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>` |
| Analysis algorithms | `<clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>` |
| Filter algorithms | `<clas12/algorithms/filters/TriggerElectronFilter.hpp>` |
| Monitoring algorithms | `<clas12/algorithms/monitoring/EventMonitor.hpp>` |

## Best Practices

!!! tip "Single Responsibility"
    Each algorithm should do one thing well. Split complex analyses into multiple algorithms.

!!! tip "Use Event Store"
    Pass data between algorithms using `event.store()` and `event.retrieve()`.

!!! tip "Use require_service in configure()"
    Request all services in `configure()` using `require_service<T>()`. This ensures services are available before `on_initialize()` is called.

!!! tip "Track Efficiency"
    Use `Cutflow` to track selection efficiency at every step.

!!! tip "Handle Errors"
    Return `StatusCode::FAILURE()` for fatal errors, `StatusCode::FILTERED()` for filtered events, and log warnings for recoverable issues.

!!! tip "Use SimpleAlgorithm<T>"
    Always inherit from `SimpleAlgorithm<T>` for automatic service injection and a clean lifecycle separation.
