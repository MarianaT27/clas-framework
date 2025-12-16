# Algorithms

Algorithms are the building blocks of analysis. They process events and can access services for common functionality.

## Built-in Algorithms

The framework provides several ready-to-use algorithms:

| Algorithm | Description |
|-----------|-------------|
| `ElectronSelectionAlg` | Electron identification with full cut suite |
| `ElectronAnalysisAlg` | Electron analysis with histograms and N-tuples |
| `DISSelectionAlg` | DIS event selection with configurable kinematic regions |
| `Pi0ReconstructionAlg` | π⁰ reconstruction from photon pairs |
| `EventMonitor` | Real-time event processing statistics |
| `PionSelectionAlg` | Pion identification |
| `LambdaReconstructionAlg` | Λ reconstruction |
| `TriggerElectronFilter` | Filter events with trigger electron |

### Using Built-in Algorithms

```cpp
#include "Framework/ApplicationBuilder.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"

auto app = Framework::make_application("My Analysis")
    .with_standard_services()
    .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
    .add_algorithm<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection", CLAS::Target::LD2, CLAS::Polarity::Outbending)
    .add_algorithm<Algorithms::DISSelectionAlg>(
        "DISSelection", Physics::DISAnalysis::standard_dis_region())
    .add_algorithm<Algorithms::Pi0ReconstructionAlg>(
        "Pi0Reconstruction", 0.2, 0.3)  // mass_window, min_energy
    .build();
```

## Algorithm Base Classes

The framework provides specialized base classes:

| Base Class | Purpose |
|------------|---------|
| `Algorithm` | Standard algorithm with initialize/execute/finalize |
| `MonitoringAlgorithm` | For event monitoring (uses `monitor()` instead of `execute()`) |
| `FilterAlgorithm` | For event filtering (uses `filter()` returning bool) |

## Algorithm Types

### Filter Algorithms

Return true/false to select events:

```cpp
class TriggerElectronFilter : public Framework::Algorithm {
public:
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        
        bool has_trigger = !electrons.empty() && 
                          std::abs(electrons[0].status()) >= 2000;
        
        event.set_decision("HasTriggerElectron", has_trigger);
        
        return Framework::StatusCode::SUCCESS();
    }
};
```

### Analysis Algorithms

Fill histograms and compute physics:

```cpp
class ElectronAnalysisAlg : public Framework::Algorithm {
public:
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        if (!event.passes_decision("HasTriggerElectron")) {
            return Framework::StatusCode::SUCCESS();
        }
        
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        for (const auto& e : electrons) {
            h_momentum->Fill(e.p());
            h_theta->Fill(e.theta());
        }
        
        return Framework::StatusCode::SUCCESS();
    }
};
```

### Producer Algorithms

Create new data products:

```cpp
class Pi0Producer : public Framework::Algorithm {
public:
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        const auto& photons = event.particles(CLAS::PIDs::PHOTON);
        
        std::vector<Physics::CompositeParticle> pi0_candidates;
        
        for (size_t i = 0; i < photons.size(); ++i) {
            for (size_t j = i + 1; j < photons.size(); ++j) {
                Physics::CompositeParticle pi0({photons[i], photons[j]});
                if (pi0.invariant_mass() > 0.1 && pi0.invariant_mass() < 0.17) {
                    pi0_candidates.push_back(pi0);
                }
            }
        }
        
        event.store_particles("Pi0Candidates", pi0_candidates);
        
        return Framework::StatusCode::SUCCESS();
    }
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

### Using ServiceHandle (Recommended)

```cpp
class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};

    auto initialize() -> Framework::StatusCode override {
        // Initialize required service
        if (auto sc = initialize_handle(m_hist_svc); sc.is_failure()) {
            return sc;
        }

        // Initialize optional service
        initialize_handle(m_log_svc);

        // Use service
        if (m_hist_svc) {
            h_momentum = m_hist_svc->book_hist1D("/Analysis/p", "Momentum", 100, 0, 10);
        }

        return Framework::StatusCode::SUCCESS();
    }
};
```

### Using service<T>() Helper

```cpp
auto initialize() -> Framework::StatusCode override {
    m_hist_svc = service<Framework::HistogramService>("HistogramService");
    m_log_svc = service<Framework::LoggingService>("LoggingService");
    
    if (!m_hist_svc) {
        Framework::Log::error(name(), "HistogramService not found");
        return Framework::StatusCode::FAILURE();
    }
    
    return Framework::StatusCode::SUCCESS();
}
```

## Using Cutflow

Track selection efficiency:

```cpp
class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::Cutflow m_cutflow{"My Selection"};

public:
    auto initialize() -> Framework::StatusCode override {
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Momentum > 1 GeV");
        m_cutflow.add_cut("Forward Angle");
        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");
        
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");
        
        if (electrons[0].p() < 1.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum > 1 GeV");
        
        if (electrons[0].theta() > 35.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Forward Angle");
        
        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }
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
class ElectronSelectionAlg : public Framework::Algorithm {
public:
    ElectronSelectionAlg(
        std::string name,
        CLAS::Target target,
        CLAS::Polarity polarity,
        bool enable_systematics = false
    ) : Algorithm(std::move(name))
      , m_target(target)
      , m_polarity(polarity)
      , m_enable_systematics(enable_systematics) {}

private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    bool m_enable_systematics;
};

// Usage
auto alg = std::make_unique<ElectronSelectionAlg>(
    "ElectronSelection",
    CLAS::Target::LD2,
    CLAS::Polarity::Outbending,
    false
);
```

## Best Practices

!!! tip "Single Responsibility"
    Each algorithm should do one thing well. Split complex analyses into multiple algorithms.

!!! tip "Use Event Store"
    Pass data between algorithms using `event.store()` and `event.retrieve()`.

!!! tip "Check Services"
    Always verify services were retrieved successfully in `initialize()`.

!!! tip "Track Efficiency"
    Use `Cutflow` to track selection efficiency at every step.

!!! tip "Handle Errors"
    Return `StatusCode::FAILURE()` for fatal errors, log warnings for recoverable issues.
