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

Create a new header file in your analysis directory:

```cpp
#pragma once

#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/PIDs.hpp>
#include <TH1D.h>

namespace my_analysis {

class MyAnalysisAlg : public clas12::framework::Algorithm {
public:
    explicit MyAnalysisAlg(std::string name = "MyAnalysis")
        : Algorithm(std::move(name))
        , m_cutflow("My Cutflow") {}

    auto initialize() -> clas12::framework::StatusCode override {
        // Get histogram service
        m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");

        if (!m_hist_svc) {
            clas12::framework::Log::error(name(), "Could not get HistogramService");
            return clas12::framework::StatusCode::FAILURE();
        }

        // Book histograms
        h_momentum = m_hist_svc->book_hist1D(
            "/MyAnalysis/momentum",
            "Electron Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        h_theta = m_hist_svc->book_hist1D(
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

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
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

        // Fill histograms
        h_momentum->Fill(e.p());
        h_theta->Fill(e.theta());

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto finalize() -> clas12::framework::StatusCode override {
        clas12::framework::Log::info(name(), "Finalizing...");
        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
};

}  // namespace my_analysis
```

### Step 2: Create the Main Program

Create `analyses/my_analysis/my_analysis.cpp`:

```cpp
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include "MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    // Create application
    clas12::framework::Application app("MyAnalysis");

    // Add logging service
    auto log_svc = std::make_unique<clas12::framework::LoggingService>();
    log_svc->set_log_level(clas12::framework::LogLevel::INFO);
    app.service_manager().add_service(std::move(log_svc));

    // Add histogram service
    auto hist_svc = std::make_unique<clas12::framework::HistogramService>();
    hist_svc->set_output_file("my_output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add algorithm
    app.add_algorithm(std::make_unique<my_analysis::MyAnalysisAlg>());

    // Configure input
    app.set_input_files({argv[1]});
    app.set_num_threads(4);

    // Run
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

## Alternative: Using ApplicationBuilder

For simpler setup, use the ApplicationBuilder pattern:

```cpp
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include "MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    auto app = clas12::framework::make_application("MyAnalysis")
        .with_input_files({argv[1]})
        .with_output("my_output.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<my_analysis::MyAnalysisAlg>("MyAnalysis")
        .build();

    app.initialize();
    app.run();
    app.finalize();

    return 0;
}
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
- Learn about [ApplicationBuilder](../user-guide/application-builder.md) for cleaner setup
