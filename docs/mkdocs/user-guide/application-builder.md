# ApplicationBuilder Pattern

The `ApplicationBuilder` provides a fluent API for cleaner, more readable application setup. This is the recommended way to configure applications in v2.0.

## Basic Usage

```cpp
#include "Framework/ApplicationBuilder.hpp"

auto app = Framework::make_application("My Analysis")
    .with_input_files(input_files)
    .with_output("output.root")
    .with_threads(4)
    .with_standard_services()
    .add_algorithm<MyAlgorithm>("MyAlg", arg1, arg2)
    .build();

app.initialize();
app.run();
app.finalize();
```

## Complete Example

```cpp
#include "Framework/ApplicationBuilder.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Framework/Services/PhotonCutsService.hpp"

auto app = Framework::make_application("Pi0 Analysis")
    .with_input_files(config.input_files)
    .with_output(config.output_file)
    .with_percentage(config.percentage)
    .with_threads(config.num_threads)
    .with_standard_services()
    .with_version("2.0")
    .with_beam_energy(10.53)
    .with_target("LD2")
    .add_service<Framework::PhotonCutsService>()
    .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
    .add_algorithm<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection", target, polarity)
    .add_algorithm<Algorithms::DISSelectionAlg>(
        "DISSelection", Physics::DISAnalysis::standard_dis_region())
    .add_algorithm<Algorithms::Pi0ReconstructionAlg>(
        "Pi0Reconstruction", mass_window, min_energy)
    .build();
```

## Builder Methods

### Input/Output Configuration

| Method | Description |
|--------|-------------|
| `with_input_files(files)` | Set input HIPO files (vector) |
| `with_input_file(file)` | Set single input file |
| `with_output(filename)` | Set output ROOT file |

### Processing Configuration

| Method | Description |
|--------|-------------|
| `with_percentage(pct)` | Set percentage of events to process |
| `with_threads(n)` | Set number of processing threads |

### Service Configuration

| Method | Description |
|--------|-------------|
| `with_standard_services()` | Add all standard services |
| `add_service<T>(args...)` | Add a custom service |
| `configure_service<T>(func)` | Configure a service after creation |

### Algorithm Configuration

| Method | Description |
|--------|-------------|
| `add_algorithm<T>(args...)` | Add an algorithm with constructor args |
| `add_algorithm(unique_ptr)` | Add a pre-constructed algorithm |

### Metadata Configuration

| Method | Description |
|--------|-------------|
| `with_version(version)` | Set analysis version |
| `with_beam_energy(energy)` | Set beam energy |
| `with_target(target)` | Set target name |

## Standard Services

When you call `with_standard_services()`, the following services are automatically added:

- `LoggingService` - Structured logging
- `HistogramService` - ROOT histogram management
- `NTupleService` - ROOT TTree management
- `ConditionsService` - Run-dependent calibrations
- `ParticleSelectorService` - Particle extraction
- `SamplingFractionService` - Sampling fraction cuts
- `HipoFileService` - HIPO file management
- `OutputFileService` - ROOT file management
- `MetadataService` - Analysis metadata
- `ProfilingService` - Performance profiling

## Adding Custom Services

```cpp
auto app = Framework::make_application("My Analysis")
    .with_standard_services()
    .add_service<Framework::PhotonCutsService>()
    .add_service<MyCustomService>(arg1, arg2)
    .build();
```

## Comparison with Traditional Approach

=== "ApplicationBuilder (Recommended)"

    ```cpp
    auto app = Framework::make_application("MyAnalysis")
        .with_input_files({"data.hipo"})
        .with_output("output.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<Algorithms::ElectronSelectionAlg>(
            "ElectronSelection", target, polarity)
        .build();

    app.initialize();
    app.run();
    app.finalize();
    ```

=== "Traditional"

    ```cpp
    Framework::Application app("MyAnalysis");

    auto log_svc = std::make_unique<Framework::LoggingService>();
    app.service_manager().add_service(std::move(log_svc));

    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // ... add more services ...

    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection", target, polarity);
    app.add_algorithm(std::move(electron_alg));

    app.set_input_files({"data.hipo"});
    app.set_num_threads(4);

    app.initialize();
    app.run();
    app.finalize();
    ```

## Benefits

!!! success "Cleaner Code"
    Fluent API reduces boilerplate and improves readability.

!!! success "Type Safety"
    Template-based algorithm addition ensures type safety at compile time.

!!! success "Automatic Configuration"
    `with_standard_services()` handles common service setup automatically.

!!! success "Metadata Tracking"
    Built-in support for version, beam energy, and target metadata.
