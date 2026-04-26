# ApplicationBuilder

Fluent builder for constructing `Application` instances.

## Header

```cpp
#include "Framework/ApplicationBuilder.hpp"
```

## Class Definition

```cpp
namespace Framework {

class ApplicationBuilder {
public:
    explicit ApplicationBuilder(std::string name);

    // Input/Output
    auto with_input_files(std::vector<std::string> files) -> ApplicationBuilder&;
    auto with_input_file(std::string file) -> ApplicationBuilder&;
    auto with_output(std::string filename) -> ApplicationBuilder&;

    // Processing
    auto with_percentage(double pct) -> ApplicationBuilder&;
    auto with_threads(int n) -> ApplicationBuilder&;

    // Services
    auto with_standard_services() -> ApplicationBuilder&;
    template <typename ServiceT, typename... Args>
    auto add_service(Args&&... args) -> ApplicationBuilder&;
    template <typename ServiceT>
    auto configure_service(std::function<void(ServiceT&)> configurator) -> ApplicationBuilder&;

    // Algorithms
    template <typename AlgT, typename... Args>
    auto add_algorithm(Args&&... args) -> ApplicationBuilder&;
    auto add_algorithm(std::unique_ptr<Algorithm> alg) -> ApplicationBuilder&;

    // Metadata
    auto with_version(std::string version) -> ApplicationBuilder&;
    auto with_beam_energy(double energy) -> ApplicationBuilder&;
    auto with_target(std::string target) -> ApplicationBuilder&;

    // Build
    [[nodiscard]] auto build() -> Application;
};

// Helper function
[[nodiscard]] auto make_application(std::string name) -> ApplicationBuilder;

}  // namespace Framework
```

## Factory Function

### `make_application`

```cpp
[[nodiscard]] auto make_application(std::string name) -> ApplicationBuilder;
```

Creates an `ApplicationBuilder` with the given application name.

**Parameters:**

- `name` - Name of the application

**Returns:** `ApplicationBuilder` instance

**Example:**

```cpp
auto app = Framework::make_application("My Analysis")
    .with_input_files(files)
    .build();
```

## Methods

### Input/Output Configuration

#### `with_input_files`

```cpp
auto with_input_files(std::vector<std::string> files) -> ApplicationBuilder&;
```

Set input HIPO files.

#### `with_input_file`

```cpp
auto with_input_file(std::string file) -> ApplicationBuilder&;
```

Set a single input file.

#### `with_output`

```cpp
auto with_output(std::string filename) -> ApplicationBuilder&;
```

Set output ROOT file (used by histogram and ntuple services).

### Processing Configuration

#### `with_percentage`

```cpp
auto with_percentage(double pct) -> ApplicationBuilder&;
```

Set percentage of events to process (0-100).

#### `with_threads`

```cpp
auto with_threads(int n) -> ApplicationBuilder&;
```

Set number of processing threads.

### Service Configuration

#### `with_standard_services`

```cpp
auto with_standard_services() -> ApplicationBuilder&;
```

Add standard services: LoggingService, HistogramService, NTupleService, ConditionsService, ParticleSelectorService, SamplingFractionService, HipoFileService, RootFileService, MetadataService, ProfilingService.

#### `add_service`

```cpp
template <typename ServiceT, typename... Args>
auto add_service(Args&&... args) -> ApplicationBuilder&;
```

Add a custom service with constructor arguments.

**Example:**

```cpp
.add_service<Framework::PhotonCutsService>()
.add_service<MyCustomService>(arg1, arg2)
```

### Algorithm Configuration

#### `add_algorithm`

```cpp
template <typename AlgT, typename... Args>
auto add_algorithm(Args&&... args) -> ApplicationBuilder&;
```

Add an algorithm by type with constructor arguments.

**Example:**

```cpp
.add_algorithm<Algorithms::ElectronSelectionAlg>(
    "ElectronSelection", target, polarity)
```

### Metadata Configuration

#### `with_version`

```cpp
auto with_version(std::string version) -> ApplicationBuilder&;
```

Set analysis version metadata.

#### `with_beam_energy`

```cpp
auto with_beam_energy(double energy) -> ApplicationBuilder&;
```

Set beam energy metadata (GeV).

#### `with_target`

```cpp
auto with_target(std::string target) -> ApplicationBuilder&;
```

Set target name metadata.

### Build

#### `build`

```cpp
[[nodiscard]] auto build() -> Application;
```

Build and return the configured `Application`.

## Complete Example

```cpp
#include "Framework/ApplicationBuilder.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Framework/Services/PhotonCutsService.hpp"

int main() {
    auto app = Framework::make_application("Pi0 Analysis")
        .with_input_files({"data/run_5032.hipo"})
        .with_output("pi0_analysis.root")
        .with_percentage(100.0)
        .with_threads(4)
        .with_standard_services()
        .with_version("2.0")
        .with_beam_energy(10.53)
        .with_target("LD2")
        .add_service<Framework::PhotonCutsService>()
        .add_algorithm<Algorithms::ElectronSelectionAlg>(
            "ElectronSelection", CLAS::Target::LD2, CLAS::Polarity::Outbending)
        .add_algorithm<Algorithms::Pi0ReconstructionAlg>(
            "Pi0Reconstruction", 0.2, 0.3)
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        return EXIT_FAILURE;
    }

    if (auto sc = app.run(); sc.is_failure()) {
        return EXIT_FAILURE;
    }

    app.finalize();
    return EXIT_SUCCESS;
}
```
