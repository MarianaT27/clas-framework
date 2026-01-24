# Creating Analyses with Scaffolding

The framework provides a Python scaffolding script to quickly generate new analysis directories with all the necessary boilerplate code, build configuration, and proper structure.

## Quick Start

```bash
# Create a new electron analysis
python templates/create_analysis.py \
    --name my_electron_study \
    --type electron \
    --with-algorithm \
    --output analyses/my_electron_study

# Build and run
meson compile -C build
./build/analyses/my_electron_study data/sample.hipo
```

## Scaffolding Script Options

```bash
python templates/create_analysis.py --help
```

| Option | Description |
|--------|-------------|
| `--name NAME` | Name for the analysis (required) |
| `--type TYPE` | Analysis type: `electron`, `pion`, `pi0`, `lambda`, `generic` |
| `--with-algorithm` | Generate a custom algorithm header |
| `--namespace NS` | Custom namespace (default: analysis name) |
| `--output DIR` | Output directory (default: `analyses/NAME`) |

## Analysis Types

### Generic Analysis

Basic analysis skeleton for custom implementations:

```bash
python templates/create_analysis.py --name my_analysis --type generic
```

### Electron Analysis

Pre-configured for electron selection and DIS studies:

```bash
python templates/create_analysis.py --name electron_study --type electron --with-algorithm
```

Includes:
- Electron selection cuts
- DIS kinematics calculation
- Standard electron histograms

### Pion Analysis

Pre-configured for pion identification:

```bash
python templates/create_analysis.py --name pion_study --type pion --with-algorithm
```

### Pi0 Analysis

Pre-configured for neutral pion reconstruction:

```bash
python templates/create_analysis.py --name pi0_study --type pi0 --with-algorithm
```

Includes:
- Photon selection
- Two-photon invariant mass
- Pi0 reconstruction

### Lambda Analysis

Pre-configured for Lambda baryon reconstruction:

```bash
python templates/create_analysis.py --name lambda_study --type lambda --with-algorithm
```

## Generated Structure

When you run the scaffolding script with `--with-algorithm`, it creates:

```
analyses/my_analysis/
├── my_analysis.cpp          # Main executable
├── MyAnalysisAlg.hpp        # Custom algorithm header
└── meson.build              # Build configuration
```

### Generated Algorithm

The generated algorithm follows the CRTP-based `SimpleAlgorithm` pattern:

```cpp
#pragma once

#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/PIDs.hpp>
#include <TH1D.h>

namespace my_analysis {

class MyAnalysisAlg : public clas12::framework::SimpleAlgorithm<MyAnalysisAlg> {
public:
    explicit MyAnalysisAlg(std::string name = "MyAnalysisAlg")
        : SimpleAlgorithm(std::move(name)) {}

    auto do_initialize() -> clas12::framework::StatusCode {
        // Get histogram service (automatically injected)
        h_momentum = histogram_service()->book_hist1D(
            "/MyAnalysis/momentum",
            "Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto do_execute(clas12::framework::Event& event) -> clas12::framework::StatusCode {
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);

        for (const auto& e : electrons) {
            h_momentum->Fill(e.p());
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto do_finalize() -> clas12::framework::StatusCode {
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    TH1D* h_momentum = nullptr;
};

}  // namespace my_analysis
```

### Generated Main Executable

```cpp
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include "MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo> [options]\n", argv[0]);
        return 1;
    }

    auto app = clas12::framework::make_application("MyAnalysis")
        .with_input_files({argv[1]})
        .with_output("my_analysis.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<my_analysis::MyAnalysisAlg>("MyAnalysisAlg")
        .build();

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

### Generated Meson Build

```meson
executable(
    'my_analysis',
    files('my_analysis.cpp'),
    dependencies: [clas12_dep, clas12_alg_dep, hipo_dep, fmt_dep, toml_dep, root_dep, cxxopts_dep],
)
```

## Adding to Build System

After generating a new analysis, add it to the build:

1. Edit `analyses/meson.build`:

```meson
# Existing analyses
subdir('electron')
subdir('pion')
# ...

# Add your new analysis
subdir('my_analysis')
```

2. Reconfigure and build:

```bash
meson setup build --wipe
meson compile -C build
```

## Customizing Generated Code

After scaffolding, you can customize:

1. **Add more histograms** in `do_initialize()`
2. **Add selection cuts** in `do_execute()`
3. **Add cutflow tracking** with `clas12::framework::Cutflow`
4. **Use additional services** like `NTupleService`

### Example: Adding Cutflow

```cpp
class MyAnalysisAlg : public clas12::framework::SimpleAlgorithm<MyAnalysisAlg> {
private:
    clas12::framework::Cutflow m_cutflow{"My Selection"};

public:
    auto do_initialize() -> clas12::framework::StatusCode {
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Momentum > 1 GeV");
        // ... book histograms
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto do_execute(clas12::framework::Event& event) -> clas12::framework::StatusCode {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        if (electrons[0].p() < 1.0) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum > 1 GeV");

        // ... fill histograms
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto do_finalize() -> clas12::framework::StatusCode {
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }
};
```

## Template Files

The scaffolding script uses templates from `templates/`:

| Template | Description |
|----------|-------------|
| `algorithm/MyAlgorithm.hpp.template` | CRTP-based algorithm |
| `analysis/my_analysis.cpp.template` | Main executable |
| `analysis/meson.build.template` | Build configuration |

You can modify these templates to change the default generated code.

## Best Practices

!!! tip "Namespace Isolation"
    Each analysis gets its own namespace to avoid symbol conflicts.

!!! tip "Start Simple"
    Begin with `--type generic` and add complexity as needed.

!!! tip "Use Built-in Algorithms"
    Combine your custom algorithm with built-in ones like `EventMonitor`.

!!! tip "Version Control"
    Commit your generated analysis to version control, then customize.
