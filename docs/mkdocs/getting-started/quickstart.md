# Quick Start

Get up and running with the CLAS12 Analysis Framework in 5 minutes.

## Build and Run

```bash
# Build the framework
meson compile -C build

# Run the example analysis
./build/main_improved data/data.hipo
```

## Basic Analysis Structure

Using the **ApplicationBuilder** pattern (recommended):

```cpp
#include "Framework/ApplicationBuilder.hpp"

int main() {
    auto app = Framework::make_application("MyAnalysis")
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

Or using the traditional approach:

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"

int main() {
    // 1. Create application
    Framework::Application app("MyAnalysis");

    // 2. Add services
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // 3. Add algorithms
    auto my_alg = std::make_unique<MyAlgorithm>();
    app.add_algorithm(std::move(my_alg));

    // 4. Configure input
    app.set_input_files({"data.hipo"});
    app.set_num_threads(4);

    // 5. Run
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

## Key Concepts

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

Algorithms process events with a lifecycle:

1. `initialize()` - Setup (book histograms, get services)
2. `execute(event)` - Process each event
3. `finalize()` - Cleanup (print statistics)

### Events

The `Event` class provides access to particle data:

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Get electrons
    const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
    
    for (const auto& e : electrons) {
        h_momentum->Fill(e.p());
        h_theta->Fill(e.theta());
    }
    
    return Framework::StatusCode::SUCCESS();
}
```

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
- [Framework Guide](../user-guide/framework.md) - Deep dive into the framework
- [Examples](../examples/electron-analysis.md) - See complete examples
