# Automatic Histogram Writing to ROOT Files

## Overview

The framework's **HistogramService** automatically writes all booked histograms to a ROOT file when the application finalizes. **No manual intervention required!**

This feature ensures that:
- ✅ All histograms are automatically saved
- ✅ No histograms are forgotten or lost
- ✅ Directory structure is preserved
- ✅ Works with all histogram types (TH1D, TH2D, TProfile)

## How It Works

### Automatic Writing on Finalize

When the framework application calls `app.finalize()`, the HistogramService automatically:

1. **Opens the output ROOT file** (creates it if it doesn't exist)
2. **Creates the directory structure** based on histogram paths
3. **Writes all booked histograms** (1D, 2D, and profiles)
4. **Closes the file** cleanly
5. **Reports statistics** (number of histograms written)

**You don't need to do anything!** The framework handles everything automatically.

### Default Output File

If no output file is specified, histograms are written to `histograms.root` by default:

```cpp
auto hist_svc = std::make_unique<Framework::HistogramService>();
// Will use "histograms.root" by default
app.service_manager().add_service(std::move(hist_svc));
```

### Custom Output File

Specify a custom output file:

```cpp
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("my_analysis.root");
app.service_manager().add_service(std::move(hist_svc));
```

## Complete Example

```cpp
#include "Framework/Application.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Histogram.hpp"

class MyAnalysis : public Framework::IAlgorithm {
    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");

        // Book histograms
        TH1D* h = m_hist_svc->book_hist1D(
            "/MyAnalysis/momentum",
            "Momentum;p [GeV/c];Events",
            100, 0, 10
        );
        h_momentum = Framework::Hist1D(h);

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // Fill histograms during event loop
        h_momentum.fill(some_value);
        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::HistogramService* m_hist_svc;
    Framework::Hist1D h_momentum;
};

int main() {
    Framework::Application app("My Analysis");

    // 1. Configure HistogramService
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("output.root");  // Specify output file
    app.service_manager().add_service(std::move(hist_svc));

    // 2. Add algorithms
    app.add_algorithm(std::make_unique<MyAnalysis>("MyAnalysis"));

    // 3. Run
    app.initialize();
    app.run();
    app.finalize();  // <-- Histograms automatically written here!

    return 0;
}
```

**Output**: `output.root` contains `/MyAnalysis/momentum` histogram

## Histogram Path Organization

Histograms are organized by their path in the ROOT file:

### Example Paths

```cpp
// Book histograms with organized paths
m_hist_svc->book_hist1D("/Event/n_particles", "...", 50, 0, 50);
m_hist_svc->book_hist1D("/Electron/momentum", "...", 100, 0, 10);
m_hist_svc->book_hist1D("/Electron/theta", "...", 100, 0, 50);
m_hist_svc->book_hist1D("/Pion/momentum", "...", 100, 0, 5);
```

### ROOT File Structure

```
output.root
├── Event/
│   └── n_particles
├── Electron/
│   ├── momentum
│   └── theta
└── Pion/
    └── momentum
```

## Working with HistogramGroup

**HistogramGroup** automatically manages paths for you:

```cpp
auto electron_hists = HistogramGroup(m_hist_svc, "/Electron");
electron_hists.add_hist1D("momentum", "...", 100, 0, 10);
electron_hists.add_hist1D("theta", "...", 100, 0, 50);
electron_hists.add_hist2D("p_vs_theta", "...", 100, 0, 50, 100, 0, 10);

// Automatically creates:
// /Electron/momentum
// /Electron/theta
// /Electron/p_vs_theta
```

All these histograms are **automatically written** to the ROOT file on finalize!

## Working with CutflowHistogram

**CutflowHistogram** creates multiple histograms automatically:

```cpp
auto p_cutflow = CutflowHistogram(
    m_hist_svc, "/Electron/p_cutflow",
    "Momentum;p [GeV/c];Events",
    100, 0, 10
);

p_cutflow.register_cut("All");
p_cutflow.register_cut("PID");
p_cutflow.register_cut("Fiducial");

// Automatically creates:
// /Electron/p_cutflow_All
// /Electron/p_cutflow_PID
// /Electron/p_cutflow_Fiducial
```

All cutflow histograms are **automatically written** to the ROOT file!

## TProfile Support

**TProfile** histograms are also automatically written:

```cpp
auto profile = m_hist_svc->book_profile(
    "/Electron/sf_vs_p",
    "Sampling Fraction vs p;p [GeV/c];<E/p>",
    100, 0, 10
);

// Fill during event loop
profile->Fill(p, E/p);

// Automatically written to ROOT file on finalize!
```

## Advanced Configuration

### Control Auto-Write

You can disable automatic writing if needed:

```cpp
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("output.root");
hist_svc->set_auto_write(false);  // Disable automatic writing
app.service_manager().add_service(std::move(hist_svc));

// Histograms will NOT be written automatically
// You must manually call write_histograms() if needed
```

### Manual Writing

If you disabled auto-write, you can manually write:

```cpp
auto hist_svc = app.service_manager().get_service<HistogramService>("HistogramService");
hist_svc->write_histograms();  // Manually write all histograms
```

### Query Histogram Statistics

```cpp
auto hist_svc = app.service_manager().get_service<HistogramService>("HistogramService");

fmt::print("Booked histograms:\n");
fmt::print("  1D: {}\n", hist_svc->num_hist1D());
fmt::print("  2D: {}\n", hist_svc->num_hist2D());
fmt::print("  Profiles: {}\n", hist_svc->num_profiles());
fmt::print("  Output file: {}\n", hist_svc->get_output_file());
```

## Demonstration Example

A complete demonstration is provided in [example_auto_histograms.cpp](src/example_auto_histograms.cpp):

### Build and Run

```bash
# Build
meson compile -C build

# Run (processes 1000 events by default)
./build/example_auto_histograms data/ 1000

# Output file: demo_histograms.root
```

### What It Demonstrates

The example shows:
1. **Type-safe histogram wrappers** (Hist1D, Hist2D, Profile)
2. **HistogramGroup** for organized collections
3. **CutflowHistogram** for automatic per-cut histograms
4. **TProfile** for correlation studies
5. **Automatic writing** to ROOT file on finalize

### Verify Output

```bash
# Open the ROOT file
root -l demo_histograms.root

# Browse the structure
root [0] .ls
root [1] Event->ls()
root [2] Particles->ls()

# Draw histograms
root [3] Particles->cd()
root [4] momentum->Draw()
root [5] p_cutflow_All->Draw()
```

## Execution Flow

```
┌─────────────────────────────────────┐
│  1. Initialize Application          │
│     - HistogramService::initialize() │
│     - Algorithms::initialize()       │
│       └─ Book histograms             │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│  2. Run Event Loop                  │
│     - Algorithms::execute()          │
│       └─ Fill histograms             │
│     (Repeat for all events)          │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│  3. Finalize Application            │
│     - HistogramService::finalize()   │
│       ├─ Open ROOT file              │
│       ├─ Create directory structure  │
│       ├─ Write all 1D histograms     │
│       ├─ Write all 2D histograms     │
│       ├─ Write all profiles          │
│       ├─ Close ROOT file             │
│       └─ Print statistics            │
│     - Algorithms::finalize()         │
└─────────────────────────────────────┘
                  ↓
           ✅ All histograms saved!
```

## Console Output

When the application finalizes, you'll see:

```
═══════════════════════════════════════
  Finalization
═══════════════════════════════════════

Writing histograms to: demo_histograms.root
Wrote 8 1D histograms, 1 2D histograms, and 1 profiles
HistogramService: Finalized

╔══════════════════════════════════════╗
║      Processing Complete!             ║
╚══════════════════════════════════════╝

Output:
  ROOT file: demo_histograms.root
    /Event/           - Event-level histograms
    /Particles/       - Particle kinematics
    /Particles/p_cutflow_* - Automatic cutflow histograms
```

## Benefits

### 🎯 Zero Manual Work
- No need to call `Write()` on individual histograms
- No need to open/close ROOT files manually
- No need to create directory structures manually

### ✅ Guaranteed Completeness
- All booked histograms are written
- Nothing is forgotten or lost
- Consistent output every time

### 📁 Organized Structure
- Directory hierarchy preserved
- Clean organization in ROOT file
- Easy navigation and analysis

### 🔒 Safe and Robust
- Automatic resource management
- Proper file closing
- Error handling built-in

## Best Practices

### 1. Use Descriptive Paths

```cpp
// Good - organized and clear
m_hist_svc->book_hist1D("/Electron/Kinematics/momentum", "...", 100, 0, 10);
m_hist_svc->book_hist1D("/Electron/Quality/chi2", "...", 100, 0, 10);

// Avoid - flat and unclear
m_hist_svc->book_hist1D("/h1", "...", 100, 0, 10);
m_hist_svc->book_hist1D("/h2", "...", 100, 0, 10);
```

### 2. Use HistogramGroup for Related Histograms

```cpp
// Group related histograms together
auto electron_hists = HistogramGroup(m_hist_svc, "/Electron");
electron_hists.add_hist1D("momentum", "...", 100, 0, 10);
electron_hists.add_hist1D("theta", "...", 100, 0, 50);
electron_hists.add_hist1D("phi", "...", 100, -180, 180);
```

### 3. Use CutflowHistogram for Sequential Cuts

```cpp
// Automatic per-cut histograms
auto momentum_cutflow = CutflowHistogram(
    m_hist_svc, "/Electron/Cutflow/momentum",
    "Momentum;p [GeV/c];Events",
    100, 0, 10
);
momentum_cutflow.register_cut("All");
momentum_cutflow.register_cut("PID");
momentum_cutflow.register_cut("Fiducial");
```

### 4. Set Output File Early

```cpp
// Set output file before initializing
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("my_analysis.root");
app.service_manager().add_service(std::move(hist_svc));
```

## Summary

✅ **Histograms are automatically written to ROOT file**
✅ **No manual intervention required**
✅ **Works with all histogram types** (TH1D, TH2D, TProfile)
✅ **Preserves directory structure** based on paths
✅ **Integrated with HistogramGroup** and **CutflowHistogram**
✅ **Default output file** if none specified
✅ **Statistics printed** on finalize
✅ **Robust and error-free** automatic management

**Just book and fill your histograms - the framework does the rest!** 🎉

---

**See Also:**
- [HISTOGRAM_ENHANCEMENTS.md](HISTOGRAM_ENHANCEMENTS.md) - Type-safe wrappers and organization
- [example_auto_histograms.cpp](src/example_auto_histograms.cpp) - Complete demonstration

**Example**: [example_auto_histograms](src/example_auto_histograms.cpp:1)

**Status**: ✅ Production ready
**Framework Version**: 2.1+
**Date**: 2025-01-24
