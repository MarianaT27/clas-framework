# Histogram System Enhancements

## Overview

The framework's histogram system has been enhanced with type-safe wrappers, organizational tools, and automated histogram management. This follows patterns from modern HEP frameworks like ATLAS AthHistogramming and CMS DQMStore.

## What Was Enhanced

### 1. Type-Safe Histogram Wrappers

**Before** (Direct ROOT pointers):
```cpp
TH1D* h_momentum = m_hist_svc->book_hist1D("/Electron/momentum", "...", 100, 0, 10);
h_momentum->Fill(electron.p());
```

**After** (Type-safe wrappers with fluent API):
```cpp
Hist1D h_momentum = m_hist_svc->book_hist1D("/Electron/momentum", "...", 100, 0, 10);
h_momentum.fill(electron.p())
          .fill_if(passes_cut, electron.p())
          .fill_weighted(electron.p(), weight);
```

**Benefits**:
- Null-safe (checks for nullptr automatically)
- Fluent API for method chaining
- Conditional filling built-in
- Weight support

### 2. HistogramGroup - Organized Collections

**Purpose**: Group related histograms together for easier management

**Example**:
```cpp
auto electron_hists = HistogramGroup(m_hist_svc, "/Electron");

// Book histograms in the group
electron_hists.add_hist1D("momentum", "Momentum;p [GeV/c];Events", 100, 0, 10);
electron_hists.add_hist1D("theta", "Theta;#theta [deg];Events", 100, 0, 50);
electron_hists.add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
                          100, 0, 50, 100, 0, 10);

// Fill histograms by name
electron_hists["momentum"].fill(electron.p());
electron_hists["theta"].fill(electron.theta() * TMath::RadToDeg());
electron_hists.fill_2D("p_vs_theta", theta_deg, p);
```

**Benefits**:
- Automatic path management (no manual "/Electron/" prefixes)
- Access by name with `operator[]`
- All histograms in one logical unit
- Clear organizational structure

### 3. CutflowHistogram - Automatic Per-Cut Histograms

**Purpose**: Automatically create and fill histograms at each stage of a cutflow

**Example**:
```cpp
// Create cutflow histogram for momentum
auto momentum_cutflow = CutflowHistogram(
    m_hist_svc, "/Electron/momentum_cutflow",
    "Electron Momentum;p [GeV/c];Events",
    100, 0, 10
);

// Register each cut stage
momentum_cutflow.register_cut("All");
momentum_cutflow.register_cut("PID");
momentum_cutflow.register_cut("Fiducial");
momentum_cutflow.register_cut("SamplingFraction");

// Fill at each stage
momentum_cutflow.fill("All", electron.p());
if (!passes_pid) return;
momentum_cutflow.fill("PID", electron.p());
if (!passes_fiducial) return;
momentum_cutflow.fill("Fiducial", electron.p());
if (!passes_sf) return;
momentum_cutflow.fill("SamplingFraction", electron.p());
```

**Output**: Creates separate histograms:
- `/Electron/momentum_cutflow_All` - All electrons
- `/Electron/momentum_cutflow_PID` - After PID cut
- `/Electron/momentum_cutflow_Fiducial` - After fiducial cut
- `/Electron/momentum_cutflow_SamplingFraction` - After all cuts

**Benefits**:
- Automatic histogram creation for each cut stage
- No manual booking of per-cut histograms
- Clean visualization of how distributions change through cuts
- Self-documenting cutflow progression

### 4. TProfile Support in HistogramService

**Enhancement**: Added TProfile histogram support to HistogramService

**New Methods**:
```cpp
// Book a TProfile
auto profile = m_hist_svc->book_profile(
    "/Electron/sf_vs_p",
    "Sampling Fraction vs Momentum;p [GeV/c];<E/p>",
    100, 0, 10
);

// Retrieve existing profile
auto prof = m_hist_svc->get_profile("/Electron/sf_vs_p");
if (prof) {
    prof->Fill(p, E/p);
}
```

**Features**:
- Same pattern as book_hist1D/book_hist2D
- Automatic directory creation
- Written to output file with histograms

## Implementation Files

### Core Framework (2 files)

1. **[Framework/Histogram.hpp](include/Framework/Histogram.hpp)** (320 lines)
   - `Hist1D` class - Type-safe 1D histogram wrapper
   - `Hist2D` class - Type-safe 2D histogram wrapper
   - `Profile` class - Type-safe TProfile wrapper
   - `HistogramGroup` class - Organized histogram collections
   - `CutflowHistogram` class - Automatic per-cut histograms

2. **[Framework/Histogram.cpp](src/Framework/Histogram.cpp)** (70 lines)
   - HistogramGroup implementations
   - CutflowHistogram implementations
   - Path and name utilities

### Enhanced Services (1 file modified)

3. **[Services/HistogramService.hpp](include/Framework/Services/HistogramService.hpp)** (Modified)
   - Added `#include "TProfile.h"` (line 6)
   - Added `book_profile()` method (lines 139-154)
   - Added `get_profile()` method (lines 161-167)
   - Added `m_profiles` member variable (line 203)
   - Enhanced `write_histograms()` to write profiles (lines 194-198)
   - Updated output message to include profile count (line 201)

### Build System (1 file modified)

4. **[meson.build](meson.build)** - Added Histogram.cpp to all executables
   - Line 41: Added to `starter` sources
   - Line 57: Added to `framework_example` sources
   - Line 71: Added to `main_improved` sources
   - Line 85: Added to `main_advanced` sources

## API Reference

### Hist1D Class

```cpp
class Hist1D {
public:
    // Fluent API for filling
    auto fill(double value) -> Hist1D&;
    auto fill_weighted(double value, double weight) -> Hist1D&;
    auto fill_if(bool condition, double value) -> Hist1D&;

    // Direct ROOT access
    auto get() -> TH1D*;
    operator bool() const;  // Check if valid
};
```

### Hist2D Class

```cpp
class Hist2D {
public:
    auto fill(double x, double y) -> Hist2D&;
    auto fill_weighted(double x, double y, double weight) -> Hist2D&;
    auto fill_if(bool condition, double x, double y) -> Hist2D&;

    auto get() -> TH2D*;
    operator bool() const;
};
```

### Profile Class

```cpp
class Profile {
public:
    auto fill(double x, double y) -> Profile&;
    auto fill_weighted(double x, double y, double weight) -> Profile&;
    auto fill_if(bool condition, double x, double y) -> Profile&;

    auto get() -> TProfile*;
    operator bool() const;
};
```

### HistogramGroup Class

```cpp
class HistogramGroup {
public:
    explicit HistogramGroup(HistogramService* service, std::string base_path);

    // Book histograms
    auto add_hist1D(const std::string& name, const std::string& title,
                    int nbins, double xmin, double xmax) -> Hist1D&;

    auto add_hist2D(const std::string& name, const std::string& title,
                    int nbinsx, double xmin, double xmax,
                    int nbinsy, double ymin, double ymax) -> Hist2D&;

    auto add_profile(const std::string& name, const std::string& title,
                     int nbins, double xmin, double xmax) -> Profile&;

    // Access histograms
    auto operator[](const std::string& name) -> Hist1D&;
    auto get_2D(const std::string& name) -> Hist2D&;
    auto get_profile(const std::string& name) -> Profile&;

    // Convenience filling
    auto fill_2D(const std::string& name, double x, double y) -> void;
};
```

### CutflowHistogram Class

```cpp
class CutflowHistogram {
public:
    CutflowHistogram(HistogramService* service,
                     const std::string& base_path,
                     const std::string& title,
                     int nbins, double xmin, double xmax);

    // Register cut stages
    auto register_cut(const std::string& cut_name) -> void;

    // Fill at specific cut stage
    auto fill(const std::string& cut_name, double value) -> void;

    // Access individual histograms
    auto operator[](const std::string& cut_name) -> Hist1D&;
};
```

## Usage Examples

### Example 1: Basic Usage with Type-Safe Wrappers

```cpp
class MyAnalysis : public Framework::AnalysisAlgorithm {
    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");

        // Book histograms using type-safe wrappers
        h_momentum = m_hist_svc->book_hist1D(
            "/Analysis/momentum", "Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        return StatusCode::SUCCESS();
    }

    auto analyze(Event& event) -> StatusCode override {
        auto electrons = get_electrons(event);

        for (const auto& e : electrons) {
            // Fluent API with method chaining
            h_momentum.fill(e.p())
                      .fill_if(e.charge() < 0, e.p())  // Only fill if negative
                      .fill_weighted(e.p(), event_weight);
        }

        return StatusCode::SUCCESS();
    }

private:
    HistogramService* m_hist_svc;
    Hist1D h_momentum;  // Type-safe wrapper instead of TH1D*
};
```

### Example 2: HistogramGroup Organization

```cpp
class ElectronAnalysis : public Framework::AnalysisAlgorithm {
    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");

        // Create histogram group
        m_electron_hists = std::make_unique<HistogramGroup>(
            m_hist_svc, "/Electron"
        );

        // Book all electron histograms
        m_electron_hists->add_hist1D("momentum", "p;p [GeV/c];Events", 100, 0, 10);
        m_electron_hists->add_hist1D("theta", "#theta;#theta [deg];Events", 100, 0, 50);
        m_electron_hists->add_hist1D("phi", "#phi;#phi [deg];Events", 100, -180, 180);
        m_electron_hists->add_hist2D("p_vs_theta", "p vs #theta;#theta;p",
                                      100, 0, 50, 100, 0, 10);

        return StatusCode::SUCCESS();
    }

    auto analyze(Event& event) -> StatusCode override {
        auto electrons = get_electrons(event);

        for (const auto& e : electrons) {
            double p = e.p();
            double theta = e.theta() * TMath::RadToDeg();
            double phi = e.phi() * TMath::RadToDeg();

            // Fill by name - clean and organized
            (*m_electron_hists)["momentum"].fill(p);
            (*m_electron_hists)["theta"].fill(theta);
            (*m_electron_hists)["phi"].fill(phi);
            m_electron_hists->fill_2D("p_vs_theta", theta, p);
        }

        return StatusCode::SUCCESS();
    }

private:
    HistogramService* m_hist_svc;
    std::unique_ptr<HistogramGroup> m_electron_hists;
};
```

### Example 3: Automatic Cutflow Histograms

```cpp
class ElectronFilter : public Framework::FilterAlgorithm {
    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");

        // Create cutflow histogram for momentum
        m_p_cutflow = std::make_unique<CutflowHistogram>(
            m_hist_svc, "/Electron/p_cutflow",
            "Electron Momentum;p [GeV/c];Events",
            100, 0, 10
        );

        // Register all cut stages
        m_p_cutflow->register_cut("All");
        m_p_cutflow->register_cut("PID");
        m_p_cutflow->register_cut("Fiducial");
        m_p_cutflow->register_cut("SamplingFraction");
        m_p_cutflow->register_cut("Final");

        return StatusCode::SUCCESS();
    }

    auto filter(Event& event) -> bool override {
        auto electrons = get_electrons(event);
        if (electrons.empty()) return false;

        auto& e = electrons[0];
        double p = e.p();

        // Fill at each cut stage
        m_p_cutflow->fill("All", p);

        if (!passes_pid(e)) return false;
        m_p_cutflow->fill("PID", p);

        if (!passes_fiducial(e)) return false;
        m_p_cutflow->fill("Fiducial", p);

        if (!passes_sampling_fraction(e)) return false;
        m_p_cutflow->fill("SamplingFraction", p);

        m_p_cutflow->fill("Final", p);
        return true;
    }

private:
    HistogramService* m_hist_svc;
    std::unique_ptr<CutflowHistogram> m_p_cutflow;
};
```

### Example 4: TProfile Usage

```cpp
class CalibrationAnalysis : public Framework::AnalysisAlgorithm {
    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");

        // Book profile for sampling fraction vs momentum
        p_sf_vs_p = m_hist_svc->book_profile(
            "/Electron/sf_vs_p",
            "Sampling Fraction vs Momentum;p [GeV/c];<E/p>",
            100, 0, 10
        );

        return StatusCode::SUCCESS();
    }

    auto analyze(Event& event) -> StatusCode override {
        auto electrons = get_electrons(event);

        for (const auto& e : electrons) {
            auto cal = get_calorimeter(event, e.pindex());
            if (!cal.has_value()) continue;

            double p = e.p();
            double E = cal->energy();
            double sf = E / p;

            // Fill profile - automatically computes mean and RMS
            if (p_sf_vs_p) {
                p_sf_vs_p->Fill(p, sf);
            }
        }

        return StatusCode::SUCCESS();
    }

private:
    HistogramService* m_hist_svc;
    TProfile* p_sf_vs_p;
};
```

## Benefits Summary

### Code Quality
✅ **Type Safety** - Wrappers prevent nullptr dereferencing
✅ **Fluent API** - Method chaining for cleaner code
✅ **Organization** - HistogramGroup keeps related histograms together
✅ **Automation** - CutflowHistogram eliminates boilerplate

### Maintainability
✅ **Self-Documenting** - Clear what each histogram group contains
✅ **Less Boilerplate** - No manual path construction
✅ **Easy Refactoring** - Change base path in one place
✅ **Clear Dependencies** - Histogram creation separate from filling

### Analysis Workflow
✅ **Cutflow Visualization** - Automatic per-cut histograms
✅ **Profile Support** - Built-in TProfile management
✅ **Conditional Filling** - fill_if() reduces if-statement clutter
✅ **Weight Support** - Easy to add event weights

## Migration Guide

### Migrating Existing Code

**Option 1**: Keep using TH1D* directly (backward compatible)
```cpp
// Old code still works!
TH1D* h = m_hist_svc->book_hist1D("/path", "title", 100, 0, 10);
h->Fill(value);
```

**Option 2**: Use type-safe wrappers
```cpp
// Convert to wrapper
Hist1D h = m_hist_svc->book_hist1D("/path", "title", 100, 0, 10);
h.fill(value);
```

**Option 3**: Organize into groups
```cpp
// Use HistogramGroup
auto group = HistogramGroup(m_hist_svc, "/MyAnalysis");
group.add_hist1D("name", "title", 100, 0, 10);
group["name"].fill(value);
```

## Build Status

**Compilation**: ✅ Successful
**Executables**: All 4 executables compile and link
- starter (305 KB)
- framework_example
- main_improved
- main_advanced

**Warnings**: Only minor ROOT version warnings (C++23 vs C++17)
**Errors**: None

## Summary

✅ **Type-safe histogram wrappers** (Hist1D, Hist2D, Profile)
✅ **HistogramGroup** for organized collections
✅ **CutflowHistogram** for automatic per-cut histograms
✅ **TProfile support** in HistogramService
✅ **Fluent API** with method chaining
✅ **Conditional filling** built-in
✅ **100% backward compatible** with existing code

**The histogram system now provides modern, type-safe, and automated histogram management!** 🎉

---

**Status**: ✅ Complete and tested
**Build**: ✅ All executables compile successfully
**Framework Version**: 2.1 (Histogram Enhancements)
**Pattern**: ATLAS AthHistogramming + CMS DQMStore
**Date**: 2025-01-24
