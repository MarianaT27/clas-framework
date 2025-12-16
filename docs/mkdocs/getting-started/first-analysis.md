# Your First Analysis

This guide walks you through creating a complete analysis from scratch.

## Step 1: Create Your Algorithm

Create a new header file `include/Algorithms/MyAnalysisAlg.hpp`:

```cpp
#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/Event.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "TH1D.h"

namespace Algorithms {

class MyAnalysisAlg : public Framework::Algorithm {
public:
    explicit MyAnalysisAlg(std::string name = "MyAnalysis")
        : Algorithm(std::move(name))
        , m_cutflow("My Cutflow") {}

    auto initialize() -> Framework::StatusCode override {
        // Get histogram service
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        
        if (!m_hist_svc) {
            Framework::Log::error(name(), "Could not get HistogramService");
            return Framework::StatusCode::FAILURE();
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

        Framework::Log::info(name(), "Initialized successfully");
        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Electron");

        // Select first electron
        const auto& e = electrons[0];

        // Apply momentum cut
        if (e.p() < 1.0) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Momentum > 1 GeV");

        // Fill histograms
        h_momentum->Fill(e.p());
        h_theta->Fill(e.theta());

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        Framework::Log::info(name(), "Finalizing...");
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::Cutflow m_cutflow;
    Framework::HistogramService* m_hist_svc = nullptr;
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
};

}  // namespace Algorithms
```

## Step 2: Create the Main Program

Create `src/my_analysis.cpp`:

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Algorithms/MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    // Create application
    Framework::Application app("MyAnalysis");

    // Add logging service
    auto log_svc = std::make_unique<Framework::LoggingService>();
    log_svc->set_log_level(Framework::LogLevel::INFO);
    app.service_manager().add_service(std::move(log_svc));

    // Add histogram service
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("my_output.root");
    app.service_manager().add_service(std::move(hist_svc));

    // Add algorithm
    app.add_algorithm(std::make_unique<Algorithms::MyAnalysisAlg>());

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

## Step 3: Add to Build System

Add to `meson.build`:

```meson
my_analysis_sources = files(
  'src/CLAS/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/my_analysis.cpp',
)

executable(
  'my_analysis',
  my_analysis_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)
```

## Step 4: Build and Run

```bash
# Rebuild
meson compile -C build

# Run your analysis
./build/my_analysis data/data.hipo

# Check output
root -l my_output.root
root [1] MyAnalysis->cd()
root [2] momentum->Draw()
```

## Alternative: Using ApplicationBuilder

For simpler setup, use the ApplicationBuilder pattern:

```cpp
#include "Framework/ApplicationBuilder.hpp"
#include "Algorithms/MyAnalysisAlg.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <input.hipo>\n", argv[0]);
        return 1;
    }

    auto app = Framework::make_application("MyAnalysis")
        .with_input_files({argv[1]})
        .with_output("my_output.root")
        .with_threads(4)
        .with_standard_services()
        .add_algorithm<Algorithms::MyAnalysisAlg>("MyAnalysis")
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

- Add more cuts using [Physics Cuts](../user-guide/cuts.md)
- Calculate kinematics with [Kinematics](../user-guide/kinematics.md)
- Run on batch with [Batch System](../user-guide/batch-system.md)
- Learn about [ApplicationBuilder](../user-guide/application-builder.md) for cleaner setup
