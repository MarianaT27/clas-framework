# Algorithm Types Framework

## Overview

The framework now supports **five specialized algorithm types**, following the design patterns from ATLAS Athena, CMS CMSSW, and LHCb Gaudi. This provides a more expressive and organized way to structure your analysis workflows.

## Algorithm Type Hierarchy

```
IAlgorithm (base interface)
    │
    ├── Algorithm (base implementation)
    │   │
    │   ├── FilterAlgorithm      → Event selection (yes/no decisions)
    │   ├── ProducerAlgorithm    → Create new data products
    │   ├── AnalysisAlgorithm    → Fill histograms, measurements
    │   ├── MonitoringAlgorithm  → Performance & quality tracking
    │   └── AlgorithmSequence    → Group algorithms together
    │       │
    │       └── ConditionalSequence → Conditional algorithm groups
```

## 1. FilterAlgorithm

**Purpose**: Make yes/no decisions about events (event selection, triggers)

**Key Features**:
- Implements `filter(Event&) -> bool` instead of `execute()`
- Automatically sets decision with algorithm name
- Used for event selection, trigger simulation

**Pattern**: ATLAS IReentrantAlgorithm (filter mode), CMS EDFilter

### Example: Trigger Electron Filter

```cpp
class TriggerElectronFilter : public Framework::FilterAlgorithm {
public:
    explicit TriggerElectronFilter(std::string name = "TriggerFilter")
        : FilterAlgorithm(std::move(name)) {}

protected:
    auto filter(Framework::Event& event) -> bool override {
        // Find trigger electron
        auto electrons = get_electrons(event);
        auto trigger_e = find_trigger(electrons);

        if (!trigger_e.has_value()) return false;  // Fail

        // Store for downstream use
        event.store("TriggerElectron", trigger_e.value());

        return true;  // Pass - decision automatically set!
    }
};
```

**Usage**:
```cpp
auto filter = std::make_unique<TriggerElectronFilter>("MyFilter");
app.add_algorithm(std::move(filter));

// Downstream algorithms can check:
if (event.passes_decision("MyFilter")) {
    // Event passed the filter
}
```

**Automatic Behavior**:
- Decision name = algorithm name
- `filter() -> true` sets `decision[name] = true`
- `filter() -> false` sets `decision[name] = false`

---

## 2. ProducerAlgorithm

**Purpose**: Create new data products from existing data

**Key Features**:
- Implements `produce(Event&) -> StatusCode` instead of `execute()`
- Creates composite particles, reconstructed objects
- Stores products in event store for downstream use

**Pattern**: ATLAS AlgTool, CMS EDProducer

### Example: π⁰ Producer

```cpp
class Pi0Producer : public Framework::ProducerAlgorithm {
public:
    explicit Pi0Producer(std::string name = "Pi0Producer")
        : ProducerAlgorithm(std::move(name)) {}

protected:
    auto produce(Framework::Event& event) -> StatusCode override {
        auto photons = get_photons(event);

        // Reconstruct π⁰ from photon pairs
        auto pi0_candidates = reconstruct_pi0(photons);

        // Filter by mass window
        std::vector<CompositeParticle> good_pi0s;
        for (const auto& pi0 : pi0_candidates) {
            if (pi0.in_mass_window(0.135, 0.050)) {
                good_pi0s.push_back(pi0);
            }
        }

        // Store for downstream algorithms
        event.store("Pi0Candidates", good_pi0s);

        // Set decision if we found any
        if (!good_pi0s.empty()) {
            event.set_decision("HasPi0", true);
        }

        return StatusCode::SUCCESS();
    }
};
```

**Usage**:
```cpp
auto producer = std::make_unique<Pi0Producer>("Pi0Producer");
app.add_algorithm(std::move(producer));

// Downstream algorithms can retrieve:
auto pi0s = event.retrieve<std::vector<CompositeParticle>>("Pi0Candidates");
if (pi0s.has_value()) {
    for (const auto& pi0 : pi0s.value()) {
        // Use reconstructed π⁰
    }
}
```

**Common Products**:
- Composite particles (π⁰, η, ρ, J/ψ, etc.)
- Jets
- Missing momentum/energy
- Reconstructed vertices
- Calibrated momenta

---

## 3. AnalysisAlgorithm

**Purpose**: Fill histograms and perform measurements (read-only)

**Key Features**:
- Implements `analyze(Event&) -> StatusCode` instead of `execute()`
- Should not modify event data
- Used for physics measurements, cross-sections, asymmetries

**Pattern**: ATLAS AthAnalysisAlgorithm, CMS EDAnalyzer

### Example: Electron Analysis

```cpp
class ElectronKinematicsAnalysis : public Framework::AnalysisAlgorithm {
public:
    explicit ElectronKinematicsAnalysis(std::string name = "ElectronAnalysis")
        : AnalysisAlgorithm(std::move(name)) {}

    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");

        h_momentum = m_hist_svc->book_hist1D(
            "/Electron/momentum", "Electron Momentum;p [GeV/c];Events",
            100, 0, 10);

        return StatusCode::SUCCESS();
    }

protected:
    auto analyze(Framework::Event& event) -> StatusCode override {
        // Read-only access - don't modify event
        auto electrons = get_electrons(event);

        for (const auto& e : electrons) {
            h_momentum->Fill(e.p());
        }

        return StatusCode::SUCCESS();
    }

private:
    HistogramService* m_hist_svc = nullptr;
    TH1D* h_momentum = nullptr;
};
```

**Best Practices**:
- Don't set decisions (use FilterAlgorithm)
- Don't create products (use ProducerAlgorithm)
- Only read from event, fill histograms
- Track cutflows for efficiency

---

## 4. MonitoringAlgorithm

**Purpose**: Track performance metrics and data quality

**Key Features**:
- Implements `monitor(Event&) -> StatusCode` instead of `execute()`
- Monitors processing rates, occupancies, quality
- Typically runs on all events

**Pattern**: ATLAS AthMonitorAlgorithm, CMS DQM modules

### Example: Event Monitor

```cpp
class EventMonitor : public Framework::MonitoringAlgorithm {
public:
    explicit EventMonitor(std::string name = "EventMonitor")
        : MonitoringAlgorithm(std::move(name)) {}

    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");

        h_particles_per_event = m_hist_svc->book_hist1D(
            "/Monitor/particles_per_event",
            "Particles per Event;N_{particles};Events",
            50, 0, 50);

        m_start_time = std::chrono::high_resolution_clock::now();

        return StatusCode::SUCCESS();
    }

protected:
    auto monitor(Framework::Event& event) -> StatusCode override {
        m_events_processed++;

        // Monitor event characteristics
        auto particles = get_all_particles(event);
        int n_particles = particles.size();

        h_particles_per_event->Fill(n_particles);

        // Print periodic updates
        if (m_events_processed % 10000 == 0) {
            fmt::print("  Processed {} events...\n", m_events_processed);
        }

        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            end_time - m_start_time);

        fmt::print("Event Monitor Statistics:\n");
        fmt::print("  Total events: {}\n", m_events_processed);
        fmt::print("  Runtime: {} seconds\n", duration.count());
        fmt::print("  Rate: {:.1f} events/sec\n",
                   double(m_events_processed) / duration.count());

        return StatusCode::SUCCESS();
    }

private:
    size_t m_events_processed = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};
```

**Monitoring Use Cases**:
- Event processing rate
- Particle multiplicities
- Detector occupancy
- Resource usage (memory, CPU)
- Data quality checks

---

## 5. AlgorithmSequence

**Purpose**: Group related algorithms into logical units

**Key Features**:
- Contains multiple algorithms
- All execute in order
- Useful for organizing workflow

**Pattern**: ATLAS AthSequencer, CMS Sequence, Gaudi Sequencer

### Example: Electron Sequence

```cpp
auto electron_seq = std::make_unique<AlgorithmSequence>("ElectronSequence");

// Add algorithms to sequence
electron_seq->add_algorithm(std::make_unique<ElectronFilter>(...));
electron_seq->add_algorithm(std::make_unique<ElectronAnalysis>(...));
electron_seq->add_algorithm(std::make_unique<ElectronMonitor>(...));

app.add_algorithm(std::move(electron_seq));
```

**Execution**:
```
For each event:
    ElectronFilter.execute()
    ElectronAnalysis.execute()
    ElectronMonitor.execute()
```

---

## 6. ConditionalSequence

**Purpose**: Run a sequence only if a decision passes

**Key Features**:
- Extends AlgorithmSequence
- Checks required decision before executing
- Entire sequence skipped if decision fails

### Example: Conditional Pion Analysis

```cpp
// Create conditional sequence
auto pion_seq = std::make_unique<ConditionalSequence>(
    "PionSequence",
    "TriggerElectronFilter"  // Required decision
);

// Add algorithms
pion_seq->add_algorithm(std::make_unique<PionSelection>(...));
pion_seq->add_algorithm(std::make_unique<PionAnalysis>(...));

app.add_algorithm(std::move(pion_seq));
```

**Execution Logic**:
```
For each event:
    if (event.passes_decision("TriggerElectronFilter")) {
        PionSelection.execute()
        PionAnalysis.execute()
    } else {
        // Skip entire sequence
    }
```

**Benefits**:
- Clean conditional logic
- Efficient (skips all algorithms at once)
- Self-documenting dependencies

---

## Complete Workflow Example

See [main_advanced.cpp](src/main_advanced.cpp:1) for a complete example:

```cpp
Framework::Application app("CLAS12 Advanced Analysis");

// 1. Event monitor (runs on all events)
app.add_algorithm(std::make_unique<EventMonitor>("Monitor"));

// 2. Trigger filter (selects events)
app.add_algorithm(std::make_unique<TriggerElectronFilter>("TriggerFilter"));

// 3. Conditional sequence (runs only if trigger passes)
auto electron_seq = std::make_unique<ConditionalSequence>(
    "ElectronSequence", "TriggerFilter");
electron_seq->add_algorithm(std::make_unique<ElectronAnalysis>(...));
electron_seq->add_algorithm(std::make_unique<PionSelection>(...));
app.add_algorithm(std::move(electron_seq));

// 4. Neutral particle producer (runs on all events)
app.add_algorithm(std::make_unique<Pi0Producer>("Pi0Producer"));
```

**Execution Flow**:
```
Event 1 (no trigger electron):
  1. EventMonitor → Track statistics
  2. TriggerFilter → Returns false, sets decision = false
  3. ElectronSequence → SKIPPED (decision failed)
  4. Pi0Producer → Runs, creates π⁰ candidates

Event 2 (has trigger electron):
  1. EventMonitor → Track statistics
  2. TriggerFilter → Returns true, sets decision = true
  3. ElectronSequence → RUNS
     a. ElectronAnalysis → Fill histograms
     b. PionSelection → Fill histograms
  4. Pi0Producer → Runs, creates π⁰ candidates
```

---

## When to Use Each Type

| Algorithm Type | When to Use | Example Use Cases |
|---------------|-------------|-------------------|
| **FilterAlgorithm** | Event selection, triggers | Trigger electron, DIS selection, exclusive event filter |
| **ProducerAlgorithm** | Creating new objects | π⁰ reconstruction, jet building, vertex finding |
| **AnalysisAlgorithm** | Physics measurements | Kinematics plots, cross-sections, asymmetries |
| **MonitoringAlgorithm** | Performance tracking | Event rates, detector occupancy, data quality |
| **AlgorithmSequence** | Grouping algorithms | Electron sequence, pion sequence, calibration sequence |
| **ConditionalSequence** | Conditional groups | Run analysis only on selected events |

---

## Comparison with Standard Algorithm

### Old Way (Standard Algorithm)

```cpp
class MyAlgorithm : public Framework::Algorithm {
    auto execute(Event& event) -> StatusCode override {
        // Do everything in one method
        bool passes = apply_filter(event);
        event.set_decision("MyAlg", passes);  // Manual

        if (passes) {
            fill_histograms(event);
            create_products(event);
            monitor_performance(event);
        }

        return StatusCode::SUCCESS();
    }
};
```

### New Way (Specialized Algorithms)

```cpp
// Separate concerns into specialized algorithms
class MyFilter : public FilterAlgorithm {
    auto filter(Event& event) -> bool override {
        return apply_filter(event);  // Decision set automatically!
    }
};

class MyProducer : public ProducerAlgorithm {
    auto produce(Event& event) -> StatusCode override {
        create_products(event);
        return StatusCode::SUCCESS();
    }
};

class MyAnalysis : public AnalysisAlgorithm {
    auto analyze(Event& event) -> StatusCode override {
        fill_histograms(event);
        return StatusCode::SUCCESS();
    }
};

class MyMonitor : public MonitoringAlgorithm {
    auto monitor(Event& event) -> StatusCode override {
        monitor_performance(event);
        return StatusCode::SUCCESS();
    }
};

// Organize into sequence
auto seq = std::make_unique<ConditionalSequence>("MySequence", "MyFilter");
seq->add_algorithm(std::make_unique<MyProducer>(...));
seq->add_algorithm(std::make_unique<MyAnalysis>(...));
seq->add_algorithm(std::make_unique<MyMonitor>(...));
```

**Benefits**:
✅ Clear separation of concerns
✅ Easier to test individual components
✅ More reusable
✅ Self-documenting
✅ Follows LHC framework patterns

---

## Implementation Files

### Framework Core
- [Framework/AlgorithmTypes.hpp](include/Framework/AlgorithmTypes.hpp:1) - Base classes for all algorithm types

### Example Implementations
- [Algorithms/TriggerElectronFilter.hpp](include/Algorithms/TriggerElectronFilter.hpp:1) - FilterAlgorithm example
- [Algorithms/Pi0Producer.hpp](include/Algorithms/Pi0Producer.hpp:1) - ProducerAlgorithm example
- [Algorithms/EventMonitor.hpp](include/Algorithms/EventMonitor.hpp:1) - MonitoringAlgorithm example

### Complete Demo
- [src/main_advanced.cpp](src/main_advanced.cpp:1) - Full workflow demonstration

---

## Build and Run

### Build
```bash
meson setup build --reconfigure
meson compile -C build
```

### Run Advanced Demo
```bash
./build/main_advanced data/ 100000 8
```

### Output
```
advanced_analysis.root
  /Monitor/     - Performance monitoring histograms
  /Electron/    - Electron analysis histograms
  /Pion/        - Pion selection histograms
```

---

## Summary

| Feature | Before | After |
|---------|--------|-------|
| **Algorithm Types** | 1 (generic) | 6 (specialized) |
| **Decision Handling** | Manual | Automatic (FilterAlgorithm) |
| **Organization** | Flat list | Sequences + Conditionals |
| **LHC Pattern** | Basic | Full (Athena/CMSSW/Gaudi) |
| **Code Clarity** | Mixed concerns | Separated concerns |

**Framework is now production-ready with industry-standard patterns!** ✅

---

**Status**: ✅ Implemented and tested
**Framework Version**: 2.0+ (Algorithm Types)
**Pattern**: ATLAS Athena + CMS CMSSW + LHCb Gaudi
**Date**: 2025-01-24
