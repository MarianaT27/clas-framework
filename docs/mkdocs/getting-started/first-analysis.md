# Your First Analysis

This guide walks you through creating a complete analysis from scratch.

## Quick Start: Using the Scaffolding System

The fastest way to create a new analysis is using the scaffolding script:

```bash
# Create a new analysis with an algorithm
python templates/create_analysis.py \
    --name my_analysis \
    --type generic \
    --with-algorithm \
    --output analyses/my_analysis

# Build
meson compile -C build

# Run
./build/analyses/my_analysis data/sample.hipo
```

This creates a complete analysis directory with:

- Main executable file
- Custom algorithm header
- Meson build configuration
- Proper include paths and namespaces

## Manual Approach: Step by Step

### Step 1: Create Your Algorithm

Create a new header file in your analysis directory using `Algorithm`:

```cpp
#pragma once

#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/PIDs.hpp>

namespace my_analysis {

class MyAnalysisAlg : public clas12::framework::Algorithm {
public:
    explicit MyAnalysisAlg(std::string name = "MyAnalysis")
        : Algorithm(std::move(name))
        , m_cutflow("My Cutflow") {}

    auto configure() -> void override {
        // Request histogram service (automatically injected)
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        // Book histograms - returns ThreadLocalHistogram1D* (thread-safe)
        h_momentum = m_hist_svc->get()->book_hist1D(
            "/MyAnalysis/momentum",
            "Electron Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        h_theta = m_hist_svc->get()->book_hist1D(
            "/MyAnalysis/theta",
            "Electron Theta;#theta [deg];Events",
            180, 0, 180
        );

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Momentum > 1 GeV");

        clas12::framework::Log::info(name(), "Initialized successfully");
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) {
            return clas12::framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Electron");

        // Select first electron
        const auto& e = electrons[0];

        // Apply momentum cut
        if (e.p() < 1.0) {
            return clas12::framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Momentum > 1 GeV");

        // Fill histograms using fill() - thread-safe atomic operation
        h_momentum->fill(e.p());
        h_theta->fill(e.theta());

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        clas12::framework::Log::info(name(), "Finalizing...");
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::ServiceRef<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    // Atomic histograms are thread-safe for parallel processing
    clas12::framework::ThreadLocalHistogram1D* h_momentum = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_theta = nullptr;
};

}  // namespace my_analysis
```

### Step 2: Create the Main Program

Create `analyses/my_analysis/my_analysis.cpp` using ApplicationBuilder:

```cpp
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include "MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    // Create application using ApplicationBuilder
    auto app = clas12::framework::make_application("MyAnalysis")
        .with_input_files({argv[1]})
        .with_output("my_output.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<my_analysis::MyAnalysisAlg>("MyAnalysis")
        .build();

    // Run the analysis lifecycle
    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
```

### Step 3: Add to Build System

Create `analyses/my_analysis/meson.build`:

```meson
executable(
    'my_analysis',
    files('my_analysis.cpp'),
    dependencies: [clas12_dep, clas12_alg_dep, hipo_dep, fmt_dep, toml_dep, root_dep, cxxopts_dep],
)
```

Add to `analyses/meson.build`:

```meson
subdir('my_analysis')
```

### Step 4: Build and Run

```bash
# Rebuild
meson compile -C build

# Run your analysis
./build/analyses/my_analysis data/data.hipo

# Check output
root -l my_output.root
root [1] MyAnalysis->cd()
root [2] momentum->Draw()
```

## Understanding the Output

Your analysis produces:

1. **ROOT file** (`my_output.root`) with histograms
2. **Cutflow table** showing selection efficiency
3. **Log messages** for debugging

## Example Cutflow Output

```
═══════════════════════════════════════════════════════════════
  Cutflow: My Cutflow
═══════════════════════════════════════════════════════════════
Cut                                    Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                             100000    100.0%    100.0%
Has Electron                            45230     45.2%     45.2%
Momentum > 1 GeV                        42108     42.1%     93.1%
═══════════════════════════════════════════════════════════════
```

## Next Steps

- [Creating Analyses](../user-guide/scaffolding.md) - More on the scaffolding system
- Add more cuts using [Physics Cuts](../user-guide/cuts.md)
- Calculate kinematics with [Kinematics](../user-guide/kinematics.md)
- Run on batch with [Batch System](../user-guide/batch-system.md)
- [Framework Architecture](../user-guide/framework.md) - Deep dive into the framework
