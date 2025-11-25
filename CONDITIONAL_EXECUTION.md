# Conditional Algorithm Execution

## Overview

The framework now supports **conditional algorithm execution**, where downstream algorithms can check the decisions of upstream algorithms before processing events. This is essential for exclusive event selection in particle physics analyses.

## Use Case

**Problem**: In an exclusive electron-pion analysis, you only want to analyze pions in events that contain a good trigger electron. Running pion selection on all events wastes CPU time and may contaminate your sample.

**Solution**: The ElectronAnalysisAlg sets a decision flag when it finds a good electron. The PionSelectionAlg checks this flag and skips events without a good electron.

## Implementation

### Event Decision Storage

The `Framework::Event` class provides two convenience methods for inter-algorithm communication:

```cpp
// Set a decision flag
event.set_decision("DecisionName", true);

// Check a decision flag
if (event.passes_decision("DecisionName")) {
    // Process event
}
```

### Example: Electron → Pion Selection Chain

#### Upstream Algorithm (ElectronAnalysisAlg)

The electron algorithm sets a decision when all cuts pass:

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // ... apply all electron cuts ...

    if (!Core::ElectronCuts::pass_chi2ndf_cut(...)) {
        return Framework::StatusCode::SUCCESS();  // Failed - no decision set
    }
    m_cutflow.pass("DC Chi2/NDF");

    // All cuts passed!
    fill_final_histograms(electron, calorimeter, traj, track);

    // Set decision for downstream algorithms
    event.set_decision("HasGoodElectron", true);

    return Framework::StatusCode::SUCCESS();
}
```

**Key Points**:
- Decision is only set when **all cuts pass**
- If any cut fails, the method returns early without setting the decision
- Decision name should be descriptive: `"HasGoodElectron"`, `"PassesTrigger"`, etc.

#### Downstream Algorithm (PionSelectionAlg)

The pion algorithm checks for the decision at the start of execution:

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Only process pions if a good electron was found
    if (!event.passes_decision("HasGoodElectron")) {
        return Framework::StatusCode::SUCCESS();
    }

    // ... rest of pion selection ...
}
```

**Key Points**:
- Check happens **before** any expensive operations
- `passes_decision()` returns `false` if:
  - Decision was never set (upstream algorithm failed cuts)
  - Decision was set to `false`
  - Decision name doesn't exist
- Algorithm returns `SUCCESS` immediately - this is normal, not an error

## Efficiency Impact

### Without Conditional Execution
```
Total events:        100,000
Electron selection:   10,000 good electrons (10% efficiency)
Pion processing:     100,000 events processed (wasted 90k)
```

### With Conditional Execution
```
Total events:        100,000
Electron selection:   10,000 good electrons (10% efficiency)
Pion processing:      10,000 events processed (10× speedup!)
```

**Result**: The pion algorithm only processes 10% of events, significantly reducing CPU time.

## Algorithm Ordering

The order in which algorithms are added to the application **matters**:

```cpp
// Correct order: electron first, then pion
app.add_algorithm(std::move(electron_alg));  // Sets decision
app.add_algorithm(std::move(pion_alg));      // Checks decision

// Wrong order: pion first, then electron
app.add_algorithm(std::move(pion_alg));      // Checks decision (not set yet!)
app.add_algorithm(std::move(electron_alg));  // Sets decision (too late!)
```

**In main_improved.cpp**:
```cpp
// Configure Algorithms
auto electron_alg = std::make_unique<Algorithms::ElectronAnalysisAlg>(...);
app.add_algorithm(std::move(electron_alg));  // Added first

auto pion_alg = std::make_unique<Algorithms::PionSelectionAlg>(...);
app.add_algorithm(std::move(pion_alg));      // Added second (correct!)
```

## Multiple Decisions

An algorithm can set multiple decisions:

```cpp
// In ElectronAnalysisAlg
event.set_decision("HasGoodElectron", true);
event.set_decision("ElectronInForwardRegion", theta < 35.0);
event.set_decision("HighMomentumElectron", p > 2.0);

// In PionSelectionAlg
if (!event.passes_decision("HasGoodElectron")) return;
if (!event.passes_decision("HighMomentumElectron")) return;
// ... process pions for high-momentum electron events only
```

## Decision Naming Convention

Use clear, descriptive names following these patterns:

✅ **Good Names**:
- `"HasGoodElectron"` - Clear what it means
- `"PassesDISCuts"` - Specific selection
- `"TriggerElectronFound"` - Describes particle role
- `"IsExclusiveEvent"` - Event-level property

❌ **Bad Names**:
- `"electron"` - Ambiguous
- `"pass"` - What passed?
- `"flag1"` - Not descriptive
- `"test"` - Not production-ready

## Advanced: Storing Custom Objects

The Event store supports any type, not just booleans:

```cpp
// Store the trigger electron for downstream use
event.store("TriggerElectron", electron);

// Retrieve in downstream algorithm
auto trigger_e = event.retrieve<Core::Particle>("TriggerElectron");
if (trigger_e.has_value()) {
    double e_p = trigger_e->p();
    // Use trigger electron momentum
}
```

## Cutflow Behavior

When using conditional execution, cutflow tables accurately reflect processing:

### ElectronAnalysisAlg Cutflow (runs on all events)
```
┌──────────────────┬──────────┬────────────┬───────────┐
│ Cut              │ Events   │ Absolute % │ Relative %│
├──────────────────┼──────────┼────────────┼───────────┤
│ All Events       │  100000  │   100.00   │  100.00   │
│ Trigger Electron │   20000  │    20.00   │   20.00   │
│ Chi2 PID         │   15000  │    15.00   │   75.00   │
│ Momentum         │   14500  │    14.50   │   96.67   │
│ ...              │     ...  │      ...   │     ...   │
│ DC Chi2/NDF      │   10000  │    10.00   │   92.86   │
└──────────────────┴──────────┴────────────┴───────────┘
```

### PionSelectionAlg Cutflow (runs only on 10,000 events with good electrons)
```
┌──────────────────┬──────────┬────────────┬───────────┐
│ Cut              │ Events   │ Absolute % │ Relative %│
├──────────────────┼──────────┼────────────┼───────────┤
│ All Events       │   10000  │   100.00   │  100.00   │  ← Only events with e⁻
│ Has π⁺           │    3200  │    32.00   │   32.00   │
│ Chi2 PID         │    2850  │    28.50   │   89.06   │
│ ...              │     ...  │      ...   │     ...   │
└──────────────────┴──────────┴────────────┴───────────┘
```

**Note**: "All Events" in PionSelectionAlg means "all events that reached this algorithm", not all events in the file.

## Best Practices

### 1. Check Decisions Early
```cpp
// Good - check immediately
auto execute(Event& event) -> StatusCode override {
    if (!event.passes_decision("Prerequisite")) return SUCCESS();
    // ... expensive operations
}

// Bad - check after expensive operations
auto execute(Event& event) -> StatusCode override {
    load_all_particles();  // Expensive!
    read_all_banks();      // Expensive!
    if (!event.passes_decision("Prerequisite")) return SUCCESS();  // Too late!
}
```

### 2. Document Dependencies
```cpp
/**
 * @brief Pion selection algorithm
 *
 * Requires: ElectronAnalysisAlg must run first and set "HasGoodElectron"
 */
class PionSelectionAlg : public Framework::Algorithm {
    // ...
};
```

### 3. Fail Gracefully
```cpp
// Don't treat missing decision as an error
if (!event.passes_decision("MaybeSet")) {
    // This is normal - just skip processing
    return StatusCode::SUCCESS();
}
```

## Framework Patterns

This follows standard patterns from LHC experiments:

| Framework | Feature Name | Purpose |
|-----------|-------------|---------|
| **Athena** | StoreGate decisions | Inter-algorithm communication |
| **CMSSW** | Event filters | Conditional processing |
| **Gaudi** | Selection cuts | Event-level decisions |
| **CLAS12** | Event decisions | Same pattern! ✅ |

## Performance Testing

To verify the performance impact:

```bash
# Run with both algorithms
./main_improved data/ 100000 8

# Check output:
# - ElectronAnalysisAlg cutflow shows all 100k events processed
# - PionSelectionAlg cutflow shows only events with good electrons
# - Total processing time is significantly reduced
```

## Summary

✅ **Decision Pattern Enables**:
- Efficient exclusive event selection
- Clear algorithm dependencies
- Reduced CPU usage
- Modular algorithm design

✅ **Implementation**:
- `event.set_decision(name, bool)` - Set decision
- `event.passes_decision(name)` - Check decision
- Return `SUCCESS` if decision not passed

✅ **Best Practices**:
- Check decisions early
- Use descriptive names
- Document dependencies
- Respect algorithm ordering

---

**Status**: ✅ Implemented and tested
**Framework Version**: 2.0 (Phase 3+)
**Date**: 2025-01-24
