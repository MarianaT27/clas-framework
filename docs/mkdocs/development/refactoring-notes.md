# Refactoring Notes

History of major refactoring efforts.

## Version 2.0 Changes

### Service-Oriented Architecture

Migrated from monolithic design to service-based architecture:

- `HistogramService` - Centralized histogram management
- `LoggingService` - Structured logging
- `NTupleService` - ROOT TTree management
- `ConditionsService` - Run-dependent calibrations

### Algorithm Framework

Introduced `IAlgorithm` interface with lifecycle:

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
class MyAlg : public Framework::Algorithm {
    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        h = m_hist_svc->book_hist1D("/path/h", "h", 100, 0, 10);
        return Framework::StatusCode::SUCCESS();
    }
    
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // ...
        return Framework::StatusCode::SUCCESS();
    }
};
```
