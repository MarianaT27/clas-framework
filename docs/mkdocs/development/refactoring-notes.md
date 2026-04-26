# Refactoring Notes

History of major refactoring efforts.

## Version 2.0 Changes

### Phase 3: ApplicationBuilder & New Features

**ApplicationBuilder Pattern**

New fluent API for cleaner application setup:

```cpp
auto app = Framework::make_application("My Analysis")
    .with_input_files(files)
    .with_output("output.root")
    .with_threads(4)
    .with_standard_services()
    .add_algorithm<Algorithms::ElectronSelectionAlg>("ElectronSelection", target, polarity)
    .build();
```

**New Algorithms**

- `Pi0ReconstructionAlg` - π⁰ reconstruction from photon pairs
- `DISSelectionAlg` - DIS event selection with configurable kinematic regions
- `EventMonitor` - Real-time event processing statistics

**New Services**

- `PhotonCutsService` - AI/ML-based photon selection with GBT models
- `ParticleSelectorService` - Service-based particle extraction
- `MetadataService` - Analysis metadata storage
- `ProfilingService` - Performance profiling
- `HipoFileService` - HIPO file management
- `OutputFileService` - ROOT file management

**ServiceRef Pattern**

Type-safe service access with validation:

```cpp
Framework::ServiceRef<Framework::HistogramService> m_hist_svc{"HistogramService"};

auto initialize() -> Framework::StatusCode override {
    auto sc = initialize_handle(m_hist_svc);
    if (sc.is_failure()) return sc;
    return Framework::StatusCode::SUCCESS();
}
```

### Phase 1 & 2: Service-Oriented Architecture

Migrated from monolithic design to service-based architecture:

- `HistogramService` - Centralized histogram management
- `LoggingService` - Structured logging
- `NTupleService` - ROOT TTree management
- `ConditionsService` - Run-dependent calibrations
- `SamplingFractionService` - Sampling fraction cuts

### Algorithm Framework

Introduced `Algorithm` interface with lifecycle:

- `initialize()` - Setup phase
- `execute(event)` - Per-event processing
- `finalize()` - Cleanup phase

### Event Data Model

New `Event` class with:

- Lazy loading of particles
- Event store for inter-algorithm communication
- Algorithm decision flags

### Bug Fixes

Fixed 13 logical operator bugs in `Reader.cpp`:

```cpp
// Before (incorrect)
if (value > min && value > max)

// After (correct)
if (value > min && value < max)
```

### Code Quality

- Adopted C++20 standard
- Added `.clang-format` configuration
- Introduced Google Test for unit testing
- Added Doxygen documentation

## Migration Guide

### From v1.x to v2.0

1. Replace direct histogram creation with `HistogramService`
2. Inherit from `Algorithm` base class
3. Use `Event` class instead of raw HIPO access
4. Add `Cutflow` for efficiency tracking

### Example Migration

Before:

```cpp
void analyze(hipo::event& evt) {
    TH1D* h = new TH1D("h", "h", 100, 0, 10);
    // ...
}
```

After:

```cpp
class MyAlg : public clas12::framework::Algorithm {
    auto initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");
        // Returns ThreadLocalHistogram1D* (thread-safe)
        h = m_hist_svc->book_hist1D("/path/h", "h", 100, 0, 10);
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        h->fill(value);  // Thread-safe atomic fill
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h = nullptr;
};
```
