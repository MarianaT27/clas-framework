# Quick Start

Get up and running with the CLAS12 Analysis Framework in 5 minutes.

## Build and Run

```bash
# Build the framework
meson compile -C build

# Run the example analysis
./build/examples/example_simple_algorithm data/data.hipo
```

## Basic Analysis Structure

Using the **ApplicationBuilder** pattern:

```cpp
#include <clas12/framework/core/ApplicationBuilder.hpp>

int main() {
    auto app = clas12::framework::make_application("MyAnalysis")
        .with_input_files({"data.hipo"})
        .with_output("output.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<MyAlgorithm>("MyAlg", arg1, arg2)
        .build();

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

## Key Concepts

### Namespaces

| Namespace | Description |
|-----------|-------------|
| `clas12::framework` | Core framework (Application, Algorithm, Services) |
| `clas12` | CLAS12-specific (Particle, PIDs, Target, etc.) |
| `clas12::algorithms` | Standard algorithms (ElectronSelectionAlg, etc.) |
| `clas12::physics` | Physics utilities (Kinematics, DISAnalysis, etc.) |

### Services

Services provide shared functionality:

| Service | Purpose |
|---------|---------|
| `HistogramService` | Manage ROOT histograms |
| `LoggingService` | Structured logging |
| `NTupleService` | ROOT TTree management |
| `ConditionsService` | Run-dependent calibrations |
| `PhotonCutsService` | AI/ML-based photon selection |
| `ParticleSelectorService` | Particle extraction and filtering |
| `SamplingFractionService` | Sampling fraction cuts |
| `MetadataService` | Analysis metadata storage |

!!! tip "Standard Services"
    Use `with_standard_services()` in ApplicationBuilder to add all common services automatically.

### Algorithms

Algorithms inherit from `Algorithm` and follow this lifecycle:

1. `configure()` - Request services via `require_service<T>()`
2. `on_initialize()` - Setup (book histograms, configure resources)
3. `process(event)` - Process each event
4. `on_finalize()` - Cleanup (print statistics)

### Events

The `Event` class provides access to particle data:

```cpp
auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Get electrons
    const auto& electrons = event.particles(clas12::PIDs::ELECTRON);

    for (const auto& e : electrons) {
        // Use fill() for thread-safe atomic histograms
        h_momentum->fill(e.p());
        h_theta->fill(e.theta());
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

## Include Paths

| Component | Include Path |
|-----------|-------------|
| Framework core | `<clas12/framework/core/Application.hpp>` |
| Framework services | `<clas12/framework/services/HistogramService.hpp>` |
| CLAS12 utilities | `<clas12/Particle.hpp>`, `<clas12/PIDs.hpp>` |
| Physics | `<clas12/physics/Kinematics.hpp>` |
| Algorithms | `<clas12/algorithms/selection/ElectronSelectionAlg.hpp>` |

!!! tip "Convenience Headers"
    Use `<clas12/framework/Core.hpp>` to include all framework components, or `<clas12/CLAS12.hpp>` to include all CLAS12 utilities.

## Example Output

```
═══════════════════════════════════════════════════════════════
  Initializing Application: MyAnalysis
═══════════════════════════════════════════════════════════════

Services initialized

Initializing Algorithms:
  - ElectronSelection

Initialization complete!
═══════════════════════════════════════════════════════════════

Starting event processing...
  Input files: 1
  Algorithms: 1
  Threads: 4

Processing: 100% [████████████████████████████████] 10000/10000

═══════════════════════════════════════════════════════════════
  Cutflow: Electron Selection
═══════════════════════════════════════════════════════════════
Cut                                    Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                             10000    100.0%    100.0%
Trigger Electron                        4523     45.2%     45.2%
Chi2 PID                                4211     42.1%     93.1%
═══════════════════════════════════════════════════════════════
```

## Next Steps

- [Your First Analysis](first-analysis.md) - Write a complete analysis
- [Creating Analyses](../user-guide/scaffolding.md) - Use the scaffolding system
- [Framework Guide](../user-guide/framework.md) - Deep dive into the framework
- [Examples](../examples/electron-analysis.md) - See complete examples
