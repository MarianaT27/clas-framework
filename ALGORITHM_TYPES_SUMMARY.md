# Algorithm Types Enhancement - Summary

## What Was Implemented

Enhanced the CLAS12 framework with **6 specialized algorithm types** following ATLAS Athena, CMS CMSSW, and LHCb Gaudi patterns, providing a professional foundation for complex analysis workflows.

## New Algorithm Types

### 1. FilterAlgorithm ✨
**Purpose**: Event selection with automatic decision setting

```cpp
class TriggerElectronFilter : public FilterAlgorithm {
    auto filter(Event& event) -> bool override {
        // Return true/false - decision set automatically!
        return has_good_trigger_electron(event);
    }
};
```

**Key Features**:
- Implements `filter()` instead of `execute()`
- Decision automatically set with algorithm name
- Clean yes/no event selection

### 2. ProducerAlgorithm ✨
**Purpose**: Create new data products (composite particles, jets, etc.)

```cpp
class Pi0Producer : public ProducerAlgorithm {
    auto produce(Event& event) -> StatusCode override {
        auto pi0s = reconstruct_pi0_from_photons(event);
        event.store("Pi0Candidates", pi0s);  // Store for downstream
        return StatusCode::SUCCESS();
    }
};
```

**Key Features**:
- Implements `produce()` instead of `execute()`
- Creates and stores data products
- Used for reconstruction

### 3. AnalysisAlgorithm ✨
**Purpose**: Fill histograms and measurements (read-only)

```cpp
class ElectronAnalysis : public AnalysisAlgorithm {
    auto analyze(Event& event) -> StatusCode override {
        // Read event, fill histograms - don't modify event
        for (const auto& e : get_electrons(event)) {
            h_momentum->Fill(e.p());
        }
        return StatusCode::SUCCESS();
    }
};
```

**Key Features**:
- Implements `analyze()` instead of `execute()`
- Read-only event access
- Physics measurements

### 4. MonitoringAlgorithm ✨
**Purpose**: Track performance and data quality

```cpp
class EventMonitor : public MonitoringAlgorithm {
    auto monitor(Event& event) -> StatusCode override {
        m_events_processed++;
        // Track rates, occupancies, quality metrics
        return StatusCode::SUCCESS();
    }
};
```

**Key Features**:
- Implements `monitor()` instead of `execute()`
- Performance tracking
- Quality assurance

### 5. AlgorithmSequence ✨
**Purpose**: Group related algorithms

```cpp
auto electron_seq = std::make_unique<AlgorithmSequence>("ElectronSeq");
electron_seq->add_algorithm(std::make_unique<ElectronFilter>(...));
electron_seq->add_algorithm(std::make_unique<ElectronAnalysis>(...));
app.add_algorithm(std::move(electron_seq));
```

**Key Features**:
- Contains multiple algorithms
- Executes in order
- Organizational clarity

### 6. ConditionalSequence ✨
**Purpose**: Run sequence only if decision passes

```cpp
auto pion_seq = std::make_unique<ConditionalSequence>(
    "PionSeq",
    "TriggerFilter"  // Required decision
);
pion_seq->add_algorithm(std::make_unique<PionAnalysis>(...));
// Entire sequence skipped if TriggerFilter = false
```

**Key Features**:
- Conditional execution
- Decision-based workflow
- Efficient skipping

## Files Created

### Core Framework (1 file)
- **[Framework/AlgorithmTypes.hpp](include/Framework/AlgorithmTypes.hpp:1)** (280 lines)
  - FilterAlgorithm base class
  - ProducerAlgorithm base class
  - AnalysisAlgorithm base class
  - MonitoringAlgorithm base class
  - AlgorithmSequence class
  - ConditionalSequence class

### Example Implementations (3 files)
- **[Algorithms/TriggerElectronFilter.hpp](include/Algorithms/TriggerElectronFilter.hpp:1)** (105 lines)
  - FilterAlgorithm example
  - Trigger electron selection

- **[Algorithms/Pi0Producer.hpp](include/Algorithms/Pi0Producer.hpp:1)** (95 lines)
  - ProducerAlgorithm example
  - π⁰ reconstruction from photons

- **[Algorithms/EventMonitor.hpp](include/Algorithms/EventMonitor.hpp:1)** (115 lines)
  - MonitoringAlgorithm example
  - Event and performance monitoring

### Complete Demonstration (1 file)
- **[src/main_advanced.cpp](src/main_advanced.cpp:1)** (195 lines)
  - Shows all algorithm types working together
  - Demonstrates sequences and conditional execution
  - Professional workflow organization

### Build System (1 file modified)
- **[meson.build](meson.build:79-90)** - Added `main_advanced` executable

### Documentation (2 files)
- **[ALGORITHM_TYPES.md](ALGORITHM_TYPES.md)** (600+ lines) - Complete guide
- **[ALGORITHM_TYPES_SUMMARY.md](ALGORITHM_TYPES_SUMMARY.md)** - This file

## Example Workflow

```cpp
// main_advanced.cpp demonstrates this complete workflow:

// 1. Monitor all events
app.add_algorithm(std::make_unique<EventMonitor>("Monitor"));

// 2. Filter events (automatic decision)
app.add_algorithm(std::make_unique<TriggerElectronFilter>("TriggerFilter"));

// 3. Conditional sequence (runs only if filter passes)
auto electron_seq = std::make_unique<ConditionalSequence>(
    "ElectronSequence", "TriggerFilter");
electron_seq->add_algorithm(std::make_unique<ElectronAnalysis>(...));
electron_seq->add_algorithm(std::make_unique<PionSelection>(...));
app.add_algorithm(std::move(electron_seq));

// 4. Produce composite particles (all events)
app.add_algorithm(std::make_unique<Pi0Producer>("Pi0Producer"));
```

**Execution Flow**:
```
Event 1 (no trigger):
  EventMonitor      → Tracks statistics
  TriggerFilter     → Returns false (decision = false)
  ElectronSequence  → SKIPPED (decision failed)
  Pi0Producer       → Runs, creates π⁰ candidates

Event 2 (has trigger):
  EventMonitor      → Tracks statistics
  TriggerFilter     → Returns true (decision = true)
  ElectronSequence  → RUNS
    ├─ ElectronAnalysis
    └─ PionSelection
  Pi0Producer       → Runs, creates π⁰ candidates
```

## Benefits

### Code Organization
**Before** (Single Algorithm type):
```cpp
class MyAlgorithm : public Algorithm {
    auto execute(Event& event) -> StatusCode override {
        // Everything mixed together:
        // - Selection logic
        // - Product creation
        // - Histogram filling
        // - Monitoring
        // Manual decision setting
    }
};
```

**After** (Specialized types):
```cpp
class MyFilter : public FilterAlgorithm {
    auto filter(Event&) -> bool { /* selection only */ }
    // Decision set automatically!
};

class MyProducer : public ProducerAlgorithm {
    auto produce(Event&) -> StatusCode { /* create products */ }
};

class MyAnalysis : public AnalysisAlgorithm {
    auto analyze(Event&) -> StatusCode { /* fill histograms */ }
};

class MyMonitor : public MonitoringAlgorithm {
    auto monitor(Event&) -> StatusCode { /* track performance */ }
};
```

### Clarity & Maintainability
✅ **Separation of Concerns** - Each algorithm has one clear purpose
✅ **Self-Documenting** - Type indicates what algorithm does
✅ **Testable** - Easy to unit test individual components
✅ **Reusable** - Algorithms can be shared across analyses
✅ **Industry Standard** - Follows ATLAS/CMS/LHCb patterns

### Performance
✅ **Efficient Skipping** - ConditionalSequence skips entire groups
✅ **Clear Dependencies** - Algorithm ordering well-defined
✅ **Monitoring Built-in** - Performance tracking is first-class

## LHC Framework Patterns Implemented

| Framework | Pattern | CLAS12 Implementation |
|-----------|---------|----------------------|
| **ATLAS Athena** | IReentrantAlgorithm (filter) | FilterAlgorithm |
| **ATLAS Athena** | AthAnalysisAlgorithm | AnalysisAlgorithm |
| **ATLAS Athena** | AthMonitorAlgorithm | MonitoringAlgorithm |
| **ATLAS Athena** | AthSequencer | AlgorithmSequence |
| **CMS CMSSW** | EDFilter | FilterAlgorithm |
| **CMS CMSSW** | EDProducer | ProducerAlgorithm |
| **CMS CMSSW** | EDAnalyzer | AnalysisAlgorithm |
| **CMS CMSSW** | Sequence | AlgorithmSequence |
| **LHCb Gaudi** | GaudiAlgorithm (filter) | FilterAlgorithm |
| **LHCb Gaudi** | GaudiSequencer | ConditionalSequence |

## Build & Test

### Compile
```bash
meson setup build --reconfigure
meson compile -C build
```

**Status**: ✅ Compiles successfully

### Run
```bash
./build/main_advanced data/ 100000 8
```

**Output**: `advanced_analysis.root` with:
- `/Monitor/` - Monitoring histograms
- `/Electron/` - Electron analysis
- `/Pion/` - Pion selection

### Verify
Check output for algorithm flow:
```
[1] Global Monitoring (runs on all events)
  ✓ EventMonitor (MonitoringAlgorithm)

[2] Event Selection Filter
  ✓ TriggerElectronFilter (FilterAlgorithm)
    → Sets decision: 'TriggerElectronFilter' (true/false)

[3] Electron-Tagged Event Sequence (conditional)
  ✓ ElectronAnalysis (AnalysisAlgorithm)
  ✓ PionSelection (AnalysisAlgorithm)
  → Sequence runs only if 'TriggerElectronFilter' = true

[4] Neutral Particle Production (runs on all events)
  ✓ Pi0Producer (ProducerAlgorithm)
    → Produces: 'Pi0Candidates' in event store
    → Sets decision: 'HasPi0' if π⁰ found
```

## Migration Guide

### Converting Existing Algorithms

**Option 1**: Keep as-is
```cpp
// Existing algorithms still work!
class MyAlg : public Framework::Algorithm {
    auto execute(Event& event) -> StatusCode override { ... }
};
```

**Option 2**: Convert to specialized type
```cpp
// If it's mainly selection:
class MyAlg : public Framework::FilterAlgorithm {
    auto filter(Event& event) -> bool override { ... }
};

// If it's mainly analysis:
class MyAlg : public Framework::AnalysisAlgorithm {
    auto analyze(Event& event) -> StatusCode override { ... }
};

// If it's mainly production:
class MyAlg : public Framework::ProducerAlgorithm {
    auto produce(Event& event) -> StatusCode override { ... }
};
```

**Backward Compatible**: ✅ Old algorithms continue to work!

## Code Metrics

| Metric | Value |
|--------|-------|
| **New Algorithm Types** | 6 |
| **New Core Classes** | 6 (in AlgorithmTypes.hpp) |
| **Example Implementations** | 3 |
| **Lines of Code (framework)** | 280 |
| **Lines of Code (examples)** | 315 |
| **Lines of Code (demo)** | 195 |
| **Documentation** | 600+ lines |
| **Backward Compatible** | ✅ 100% |

## Summary

✅ **6 specialized algorithm types** following LHC patterns
✅ **Automatic decision management** for FilterAlgorithm
✅ **Sequence organization** with conditional execution
✅ **Complete examples** with working demonstration
✅ **Comprehensive documentation** with migration guide
✅ **100% backward compatible** with existing algorithms
✅ **Industry-standard patterns** from ATLAS/CMS/LHCb

**The framework now provides production-level organization matching major LHC experiments!** 🎉

---

**Status**: ✅ Complete and tested
**Build**: ✅ Compiles successfully (310 KB executable)
**Framework Version**: 2.0+ (Algorithm Types)
**Pattern**: ATLAS Athena + CMS CMSSW + LHCb Gaudi
**Executables**: 4 (starter, framework_example, main_improved, **main_advanced**)
**Date**: 2025-01-24
