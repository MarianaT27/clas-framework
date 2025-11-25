# Framework Improvements - Comprehensive Enhancement Package

## Overview

This document describes **12 major improvements** implemented to enhance the CLAS12 analysis framework. These improvements provide production-level features for:
- Logging and debugging
- Performance monitoring
- Analysis reporting
- Histogram management
- Thread safety
- N-tuple output
- Progress tracking
- Metadata and provenance
- Event selection
- Detector corrections
- Workflow visualization

All features follow patterns from major HEP experiments (ATLAS, CMS, LHCb).

---

## 1. Logging Service 📝

**File**: [Framework/Services/LoggingService.hpp](include/Framework/Services/LoggingService.hpp:1)

### Features
- **5 log levels**: DEBUG, INFO, WARNING, ERROR, CRITICAL
- **Colored console output** for easy reading
- **File logging** support
- **Thread-safe** operations
- **Timestamps** on all messages
- **Source tracking** (algorithm/service name)

### Usage

```cpp
// In your algorithm
auto initialize() -> StatusCode override {
    m_log = service<LoggingService>("LoggingService");
    return StatusCode::SUCCESS();
}

auto execute(Event& event) -> StatusCode override {
    // Using macros (automatic source name)
    LOG_INFO(m_log, "Processing event {}", event_number);
    LOG_WARNING(m_log, "No trigger electron found");
    LOG_ERROR(m_log, "Failed to read bank: {}", bank_name);

    // Or manually
    m_log->info(this->name(), "Processing event {}", event_number);

    return StatusCode::SUCCESS();
}
```

### Configuration

```cpp
// In main()
auto log_svc = std::make_unique<LoggingService>();
log_svc->set_log_level(LogLevel::INFO);     // Minimum level
log_svc->set_colored_output(true);          // Enable colors
log_svc->set_log_file("analysis.log");      // File output
app.service_manager().add_service(std::move(log_svc));
```

### Output Example

```
[14:32:15.234] INFO  ElectronAnalysis     Processing event 1000
[14:32:15.235] WARN  PionSelection        Low momentum pion detected
[14:32:15.236] ERROR EventMonitor         Failed to retrieve calorimeter data
```

---

## 2. Algorithm Performance Profiling ⏱️

**File**: [Framework/Services/ProfilingService.hpp](include/Framework/Services/ProfilingService.hpp:1)

### Features
- **Automatic timing** of algorithm execution
- **Statistics tracking**: min, max, avg, total time
- **Bottleneck identification** (>20% of time)
- **Throughput calculation** (events/second)
- **Thread-safe** recording

### Usage

```cpp
// Method 1: Manual timing
auto execute(Event& event) -> StatusCode override {
    auto timer = m_profiling->start_timer(this->name());

    // ... algorithm code ...

    m_profiling->stop_timer(timer);
    return StatusCode::SUCCESS();
}

// Method 2: RAII Auto-timer (recommended)
auto execute(Event& event) -> StatusCode override {
    AutoTimer timer(m_profiling, this->name());

    // ... algorithm code ...
    // Timer automatically stopped on scope exit

    return StatusCode::SUCCESS();
}
```

### Configuration

```cpp
auto prof_svc = std::make_unique<ProfilingService>();
app.service_manager().add_service(std::move(prof_svc));
```

### Output Example

```
╔════════════════════════════════════════════════════════════════╗
║              Algorithm Performance Summary                     ║
╚════════════════════════════════════════════════════════════════╝

Total execution time: 45.23 seconds

Algorithm                      Calls   Total (ms)   Avg (ms)   Min (ms)   Max (ms)   % Time
------------------------------ ---------- ------------ ---------- ---------- ---------- --------
ElectronAnalysis                   10000    12345.67       1.235      0.856      5.234    45.2%
PionSelection                      10000     8901.23       0.890      0.654      3.123    32.6%
EventMonitor                       10000     5432.10       0.543      0.432      2.111    19.9%

Performance Analysis:
  ⚠️  ElectronAnalysis is a bottleneck (45.2% of total time)
  ℹ️  Average throughput: 221.3 events/second
```

---

## 3. Analysis Summary Report 📊

**File**: [Framework/Services/SummaryService.hpp](include/Framework/Services/SummaryService.hpp:1)

### Features
- **Event statistics** (processed, accepted, efficiency)
- **Cutflow efficiencies** (stage-by-stage)
- **Histogram summary** (counts by type)
- **Performance highlights** (slowest algorithms)
- **Custom statistics** support

### Usage

```cpp
// Record statistics during analysis
auto summary = service<SummaryService>("SummaryService");

// Record events
summary->record_events_processed(total_events);
summary->record_events_accepted(accepted_events);

// Add cutflow stages
summary->add_cutflow_stage("All Events", 10000);
summary->add_cutflow_stage("Has Electron", 8500);
summary->add_cutflow_stage("DIS Selection", 6200);
summary->add_cutflow_stage("Final Selection", 4100);

// Add custom statistics
summary->add_statistic("Average Q2", 2.45);
summary->add_statistic("Average W", 2.12);
```

### Output Example

```
╔════════════════════════════════════════════════════════════════╗
║                   Analysis Summary Report                      ║
╚════════════════════════════════════════════════════════════════╝

─── Timing Information ─────────────────────────────────────────
  Total runtime: 45.23 seconds
  Processing rate: 221.1 events/second
  Time per event: 4.52 ms

─── Event Statistics ───────────────────────────────────────────
  Events processed: 10000
  Events accepted: 4100
  Overall efficiency: 41.00%

─── Cutflow Efficiency ─────────────────────────────────────────
  Cut Stage                       Events    Efficiency    Cumulative
  ------------------------------ ------------ ------------ ------------
  All Events                          10000       100.00%       100.00%
  Has Electron                         8500        85.00%        85.00%
  DIS Selection                        6200        72.94%        62.00%
  Final Selection                      4100        66.13%        41.00%

─── Histogram Summary ──────────────────────────────────────────
  Total histograms: 245
    - 1D histograms: 180
    - 2D histograms: 50
    - TProfile: 15
  Output file: analysis.root

─── Custom Statistics ──────────────────────────────────────────
  Average Q2: 2.45
  Average W: 2.12
```

---

## 4. Enhanced Histogram Verification ✅

**File**: [Framework/Services/HistogramService.hpp](include/Framework/Services/HistogramService.hpp:270) (enhanced)

### Features
- **Automatic verification** after writing
- **Integrity checks** (file open, histogram presence)
- **Empty histogram detection**
- **Missing histogram reporting**
- **Single ROOT file guarantee** (all histograms in one file)

### Usage

```cpp
// Enable verification
auto hist_svc = std::make_unique<HistogramService>();
hist_svc->set_output_file("output.root");
hist_svc->set_verification(true);  // Enable verification
app.service_manager().add_service(std::move(hist_svc));
```

### Output Example

```
Writing 245 histograms to: output.root
✓ Wrote 180 1D, 50 2D histograms, and 15 profiles

Verifying histograms in: output.root
✓ Verification passed: All 245 histograms found
  ⚠️  Warning: 3 histogram(s) are empty
```

---

## 5. Histogram Templates 🎨

**File**: [Framework/HistogramTemplates.hpp](include/Framework/HistogramTemplates.hpp:1)

### Features
- **Predefined histogram sets** for common analyses
- **Standard binning** following community conventions
- **Reusable configurations**
- **Easy customization**

### Templates Available

1. **ParticleKinematics** - Standard kinematic variables (p, θ, φ, E)
2. **ElectronID** - Electron identification (PID, sampling fraction, fiducial)
3. **QualityAssurance** - Data quality monitoring
4. **MissingMassHistograms** - Missing mass analysis
5. **InvariantMassHistograms** - Composite particle reconstruction
6. **TimeOfFlightHistograms** - TOF and beta analysis
7. **DISKinematics** - Deep inelastic scattering variables (Q², W, x, y)

### Usage

```cpp
// Method 1: Direct creation
auto electron_hists = HistogramTemplates::ParticleKinematics::create(
    hist_svc, "/Electron"
);

// Method 2: Helper functions
auto electron_hists = HistogramTemplates::create_particle_histograms(
    hist_svc, "Electron"
);

auto pi0_hists = HistogramTemplates::create_pi0_histograms(hist_svc);

// Fill histograms
(*electron_hists)["p"].fill(electron.p());
(*electron_hists)["theta"].fill(electron.theta() * TMath::RadToDeg());
electron_hists->hist2d("p_vs_theta").fill(theta, p);
```

### Available Templates

```cpp
// Particle kinematics (p, pt, pz, theta, phi, eta, E, 2D correlations)
auto kinematics = HistogramTemplates::ParticleKinematics::create(hist_svc, "/Electron");

// Electron ID (chi2pid, sampling fraction, fiducial, tracking)
auto electron_id = HistogramTemplates::ElectronID::create(hist_svc, "/ElectronID");

// Quality assurance (multiplicities, timing, run conditions)
auto qa = HistogramTemplates::QualityAssurance::create(hist_svc, "/QA");

// Missing mass (mm, mm2, pmiss, angles, correlations)
auto missing = HistogramTemplates::MissingMassHistograms::create(hist_svc, "/Missing");

// Invariant mass (mass, mass vs kinematics, particle kinematics)
auto pi0 = HistogramTemplates::InvariantMassHistograms::create(
    hist_svc, "/Pi0", "(γγ)", 0.0, 0.5, 200
);

// Time of flight (beta, delta-t, tof, path)
auto tof = HistogramTemplates::TimeOfFlightHistograms::create(hist_svc, "/TOF");

// DIS kinematics (Q2, W, nu, x, y, correlations)
auto dis = HistogramTemplates::DISKinematics::create(hist_svc, "/DIS");
```

---

## 6. Thread-Safe Histogram Filling 🔒

**File**: [Framework/ThreadSafeHistogram.hpp](include/Framework/ThreadSafeHistogram.hpp:1)

### Features
- **Mutex-protected filling** for parallel processing
- **Lock-free reading** of underlying histograms
- **Same API** as regular histogram wrappers
- **OpenMP compatible**

### Usage

```cpp
// Book histogram
TH1D* h = hist_svc->book_hist1D("/path", "title", 100, 0, 10);

// Wrap for thread-safe access
ThreadSafeHist1D thread_safe_h(h);

// Fill from multiple threads - thread safe!
#pragma omp parallel for
for (int i = 0; i < n_events; ++i) {
    thread_safe_h.fill(event_data[i]);
}

// Also available:
ThreadSafeHist2D  // For TH2D
ThreadSafeProfile // For TProfile
```

### Important Notes

⚠️ **ROOT histograms are NOT thread-safe by default!**
- Always use `ThreadSafeHistX` wrappers for parallel processing
- Each `fill()` operation is atomic
- No data races or corruption

---

## 7. N-tuple/TTree Output Service 🌳

**File**: [Framework/Services/NTupleService.hpp](include/Framework/Services/NTupleService.hpp:1)

### Features
- **Centralized TTree management**
- **Automatic branch registration**
- **Vector branch support**
- **Same output file** as histograms (optional)
- **Multiple TTrees** support
- **Auto-save** configuration

### Usage

```cpp
// In initialize()
m_ntuple = service<NTupleService>("NTupleService");

// Register branches
m_ntuple->register_branch("events", "electron_p", &m_electron_p);
m_ntuple->register_branch("events", "electron_theta", &m_electron_theta);
m_ntuple->register_branch("events", "Q2", &m_Q2);

// For vectors
m_ntuple->register_vector_branch("events", "pion_momenta", &m_pion_momenta);

// In execute()
m_electron_p = electron.p();
m_electron_theta = electron.theta();
m_Q2 = calculate_Q2(electron);
m_pion_momenta = get_pion_momenta(event);

m_ntuple->fill("events");  // Fill tree
```

### Configuration

```cpp
auto ntuple_svc = std::make_unique<NTupleService>();
ntuple_svc->set_output_file("output.root");  // Same file as histograms!
app.service_manager().add_service(std::move(ntuple_svc));
```

### RAII Helper

```cpp
auto execute(Event& event) -> StatusCode override {
    TreeFiller filler(m_ntuple, "events");

    // Set branch variables
    m_electron_p = electron.p();
    m_electron_theta = electron.theta();

    // Automatically fills tree on scope exit
    return StatusCode::SUCCESS();
}
```

### Output

Verification:
```
Writing 3 TTrees to: output.root
  ✓ Wrote tree 'events' (10000 entries)
  ✓ Wrote tree 'systematics' (10000 entries)
  ✓ Wrote tree 'metadata' (1 entries)
✓ Successfully wrote 3 TTrees
```

Access in ROOT:
```bash
$ root -l output.root
root [0] events->Print()
root [1] events->Draw("electron_p")
root [2] events->Scan("electron_p:electron_theta:Q2")
```

---

## 8. Progress Bar with ETA ⏳

**File**: [Framework/ProgressBar.hpp](include/Framework/ProgressBar.hpp:1)

### Features
- **Real-time progress indicator** with visual bar
- **ETA calculation** (estimated time remaining)
- **Processing rate** (events/second)
- **Percentage complete**
- **Colored output** (green when complete)
- **Throttled updates** (doesn't spam terminal)

### Usage

```cpp
// Simple usage
ProgressBar progress(total_events, "Processing");
for (size_t i = 0; i < total_events; ++i) {
    // ... process event ...
    progress.update(i + 1);
}
progress.finish();

// Or increment by one
for (size_t i = 0; i < total_events; ++i) {
    // ... process event ...
    progress.increment();
}
progress.finish();
```

### Output Example

```
Processing: [████████████████▌                 ] 45% (4500/10000) [01:23<02:34] 123 evt/s
```

When complete:
```
✓ Processing: [██████████████████████████████████] 100% (10000/10000) [03:45] 44.4 evt/s
```

### Features
- **Visual bar** with Unicode block characters
- **ETA** in HH:MM:SS format
- **Rate** in events/second (or k evt/s for high rates)
- **Throttling** - updates every 100ms (configurable)
- **Clean finish** - newline on completion

---

## 9. Metadata Service 📋

**File**: [Framework/Services/MetadataService.hpp](include/Framework/Services/MetadataService.hpp:1)

### Features
- **Analysis provenance** tracking
- **Reproducibility** information
- **Software version** tracking
- **Git commit hash** storage
- **Input file** list
- **Command line** arguments
- **Analysis parameters** recording
- **Timestamps** (start/end time)
- **Writes to ROOT file** as TTree

### Usage

```cpp
// In main()
auto metadata = std::make_unique<MetadataService>();
metadata->set_output_file("output.root");
metadata->set_version("2.1.0");
metadata->set_git_hash("a1b2c3d4");
metadata->set_run_number(5032);
metadata->set_target("proton");
metadata->set_beam_energy(10.6);  // GeV

// Record input files
for (const auto& file : input_files) {
    metadata->add_input_file(file);
}

// Record command line
metadata->set_command_line(argc, argv);

// Record analysis parameters
metadata->add_parameter("min_Q2", 1.0);
metadata->add_parameter("min_W", 2.0);
metadata->add_parameter("electron_chi2pid", 3.0);

app.service_manager().add_service(std::move(metadata));
```

### Output Example

Console output:
```
╔════════════════════════════════════════════════════════════════╗
║                    Analysis Metadata                           ║
╚════════════════════════════════════════════════════════════════╝

Software:
  Version: 2.1.0
  Git hash: a1b2c3d4

Run Information:
  Run number: 5032
  Target: proton
  Beam energy: 10.60 GeV

Input Files: 15
  - data_001.hipo
  - data_002.hipo
  - data_003.hipo
  - ... (12 more)

Analysis Parameters:
  min_Q2: 1.0
  min_W: 2.0
  electron_chi2pid: 3.0

Timing:
  Start: Mon Jan 24 14:32:15 2025
  End: Mon Jan 24 14:35:59 2025
  Duration: 224 seconds
  Events processed: 50000
```

ROOT file output:
```bash
$ root -l output.root
root [0] metadata->Print()
root [1] metadata->Scan("version:run_number:events_processed")
```

---

## 10. Event Selection Service ✂️

**File**: [Framework/Services/SelectionService.hpp](include/Framework/Services/SelectionService.hpp:1)

### Features
- **Centralized event selection**
- **Named selection criteria**
- **Boolean combinations** (AND/OR/NOT)
- **Selection statistics** with efficiencies
- **Reusable** across algorithms
- **Predefined common selections**

### Usage

```cpp
// In initialize()
auto selection = service<SelectionService>("SelectionService");

// Define selections
selection->define("has_electron", [](const Event& e) {
    return e.passes_decision("ElectronFilter");
});

selection->define("dis_cut", [](const Event& e) {
    auto Q2 = e.retrieve<double>("Q2");
    auto W = e.retrieve<double>("W");
    return Q2.has_value() && Q2.value() > 1.0 &&
           W.has_value() && W.value() > 2.0;
});

selection->define("vertex_cut", [](const Event& e) {
    auto vz = e.retrieve<double>("vertex_z");
    return vz.has_value() && std::abs(vz.value()) < 3.0;
});

// Define composite selections
selection->define_and("full_selection", {"has_electron", "dis_cut", "vertex_cut"});

// In execute()
if (selection->passes("has_electron", event)) {
    // ... process electron ...
}

// Multiple selections
if (selection->passes_all({"has_electron", "dis_cut"}, event)) {
    // ... DIS analysis ...
}

// Negation
if (selection->passes_not("has_electron", event)) {
    // ... background events ...
}
```

### Predefined Selections

```cpp
// DIS selection
selection->define("dis", CommonSelections::DIS_Selection(1.0, 2.0));

// Trigger electron
selection->define("trigger", CommonSelections::HasTriggerElectron());

// Exclusive event
selection->define("exclusive", CommonSelections::ExclusiveEvent(0.938, 0.3));
```

### Output Example

```
╔════════════════════════════════════════════════════════════════╗
║                  Selection Statistics                          ║
╚════════════════════════════════════════════════════════════════╝

Selection                      Passed          Total     Efficiency
------------------------------ ------------ ------------ ------------
has_electron                          8532        10000       85.32%
dis_cut                               6821         8532       79.95%
vertex_cut                            6543         6821       95.93%
full_selection                        6543        10000       65.43%
```

---

## 11. Corrections/Calibrations Database 🔧

**File**: [Framework/Services/CorrectionsService.hpp](include/Framework/Services/CorrectionsService.hpp:1)

### Features
- **Detector corrections** and calibrations
- **1D, 2D, 3D corrections**
- **Run-dependent** calibrations
- **Efficiency corrections**
- **Enable/disable** individual corrections
- **Load from file** support
- **Predefined correction functions**

### Usage

```cpp
// In initialize()
auto corrections = service<CorrectionsService>("CorrectionsService");

// Define corrections
corrections->define_correction_2d("electron_momentum",
    [](double p, double sector) {
        // Sector-dependent momentum correction
        return p * (1.0 + 0.001 * (sector - 3));
    });

corrections->define_correction_1d("ecal_energy",
    [](double E) {
        return E * 1.002;  // Energy scale correction
    });

// In execute()
double p_corrected = corrections->apply("electron_momentum", p_raw, sector);
double E_corrected = corrections->apply("ecal_energy", E_raw);

// Enable/disable corrections
corrections->enable_correction("electron_momentum", true);
corrections->enable_correction("ecal_energy", false);
```

### Predefined Correction Functions

```cpp
// Polynomial correction
auto poly_corr = Corrections::polynomial({1.0, 0.01, -0.001});  // 1 + 0.01x - 0.001x^2
corrections->define_correction_1d("momentum", poly_corr);

// Exponential correction
auto exp_corr = Corrections::exponential(1.0, -0.1);  // e^(-0.1x)
corrections->define_correction_1d("efficiency", exp_corr);

// Sector-dependent correction
std::array<double, 6> sector_corrs = {1.00, 1.01, 0.99, 1.02, 0.98, 1.00};
auto sector_corr = Corrections::sector_dependent(sector_corrs);
corrections->define_correction_2d("sector", sector_corr);

// Run-dependent correction
Corrections::RunDependentCorrection run_corr;
run_corr.add_run_correction(5032, 1.005);
run_corr.add_run_correction(5033, 1.003);
double corr = run_corr.apply(value, run_number);

// Efficiency correction from histogram
TH1D* eff_hist = /* load efficiency histogram */;
Corrections::EfficiencyCorrection eff_corr(eff_hist);
double efficiency = eff_corr.get_efficiency(momentum);
double weight = eff_corr.get_weight(momentum);  // 1/efficiency
```

### Load from File

```cpp
// corrections.txt format:
// # Comment
// electron_momentum polynomial 1.0 0.01 -0.001
// ecal_energy scale 1.002

corrections->load_corrections_from_file("corrections.txt");
```

### Output Example

```
Corrections Summary:
  Total corrections: 8
    1D: 3
    2D: 4
    3D: 1
  Enabled: 7
```

---

## 12. Workflow Visualization 📊

**File**: [Framework/Services/WorkflowVisualizationService.hpp](include/Framework/Services/WorkflowVisualizationService.hpp:1)

### Features
- **Algorithm dependency graph**
- **DOT/GraphViz output**
- **Conditional execution** visualization
- **Service usage** tracking
- **Data flow** analysis
- **Circular dependency detection**
- **Execution order** display
- **Complexity estimation**

### Usage

```cpp
// In main() - register workflow
auto viz = std::make_unique<WorkflowVisualizationService>();
viz->set_output_file("workflow.dot");

// Register algorithms
viz->register_algorithm("EventReader");
viz->register_filter("ElectronFilter");
viz->register_algorithm("KinematicsCalculator", {"ElectronFilter"});
viz->register_algorithm("PionSelection", {"KinematicsCalculator"});

// Register execution order
viz->register_execution_order("EventReader", "ElectronFilter");
viz->register_execution_order("ElectronFilter", "KinematicsCalculator");

// Register conditional execution
viz->register_conditional("ElectronFilter", "KinematicsCalculator");

// Register service usage
viz->register_service_use("KinematicsCalculator", "HistogramService");
viz->register_service_use("PionSelection", "CorrectionsService");

// Register data flow
viz->register_data_flow("KinematicsCalculator", "Q2", "DISAnalysis");
viz->register_data_flow("PionSelection", "Pions", "Pi0Analysis");

app.service_manager().add_service(std::move(viz));
```

### Fluent API

```cpp
// Alternative: Use builder pattern
WorkflowBuilder builder(viz.get());
builder.add_filter("ElectronFilter")
       .then("KinematicsCalculator")
       .uses_service("HistogramService")
       .produces("Q2")
       .consumed_by("DISAnalysis");
```

### Output Example

Console output:
```
╔════════════════════════════════════════════════════════════════╗
║                    Workflow Execution Order                    ║
╚════════════════════════════════════════════════════════════════╝

1. EventReader [Algorithm]
2. ElectronFilter [Filter]
3. KinematicsCalculator [Algorithm]
   Inputs: ElectronFilter
   Outputs: Q2, W, nu
4. PionSelection [Algorithm]
   Inputs: Q2
   Outputs: Pions
5. Pi0Analysis [Algorithm]
   Inputs: Pions

Workflow Statistics:
  Total nodes: 10
  Total edges: 15
    Algorithms: 6
    Filters: 2
    Services: 2
    Data objects: 3
  ✓ No circular dependencies
```

DOT file generation:
```
✓ Generated workflow visualization: workflow.dot
  Render with: dot -Tpng workflow.dot -o workflow.png
  Or view with: xdot workflow.dot
```

### Visualization Output

The generated DOT file can be rendered with GraphViz:

```bash
# Generate PNG
dot -Tpng workflow.dot -o workflow.png

# Generate SVG
dot -Tsvg workflow.dot -o workflow.svg

# Interactive viewing
xdot workflow.dot
```

The graph shows:
- **Blue boxes** - Algorithms
- **Yellow diamonds** - Filters (conditional)
- **Green ellipses** - Services
- **Pink folders** - Data objects
- **Solid black arrows** - Execution order
- **Solid blue arrows** - Data dependencies
- **Dashed orange arrows** - Conditional execution
- **Dotted green arrows** - Service usage

---

## Integration Example

Complete example showing all services working together:

```cpp
#include "Framework/Application.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/ProfilingService.hpp"
#include "Framework/Services/SummaryService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/SelectionService.hpp"
#include "Framework/Services/CorrectionsService.hpp"
#include "Framework/Services/WorkflowVisualizationService.hpp"
#include "Framework/HistogramTemplates.hpp"
#include "Framework/ProgressBar.hpp"

int main(int argc, char** argv) {
    Framework::Application app("Advanced Analysis");

    // 1. Logging Service
    auto log_svc = std::make_unique<LoggingService>();
    log_svc->set_log_level(LogLevel::INFO);
    log_svc->set_log_file("analysis.log");
    app.service_manager().add_service(std::move(log_svc));

    // 2. Profiling Service
    auto prof_svc = std::make_unique<ProfilingService>();
    app.service_manager().add_service(std::move(prof_svc));

    // 3. Summary Service
    auto summary_svc = std::make_unique<SummaryService>();
    app.service_manager().add_service(std::move(summary_svc));

    // 4. Histogram Service (with verification)
    auto hist_svc = std::make_unique<HistogramService>();
    hist_svc->set_output_file("output.root");
    hist_svc->set_verification(true);
    app.service_manager().add_service(std::move(hist_svc));

    // 5. N-tuple Service
    auto ntuple_svc = std::make_unique<NTupleService>();
    ntuple_svc->set_output_file("output.root");  // Same file!
    app.service_manager().add_service(std::move(ntuple_svc));

    // 6. Metadata Service
    auto metadata_svc = std::make_unique<MetadataService>();
    metadata_svc->set_output_file("output.root");  // Same file!
    metadata_svc->set_version("2.2.0");
    metadata_svc->set_git_hash("a1b2c3d4");
    metadata_svc->set_run_number(5032);
    metadata_svc->set_command_line(argc, argv);
    metadata_svc->add_parameter("min_Q2", 1.0);
    app.service_manager().add_service(std::move(metadata_svc));

    // 7. Selection Service
    auto selection_svc = std::make_unique<SelectionService>();
    app.service_manager().add_service(std::move(selection_svc));

    // 8. Corrections Service
    auto corrections_svc = std::make_unique<CorrectionsService>();
    app.service_manager().add_service(std::move(corrections_svc));

    // 9. Workflow Visualization
    auto viz_svc = std::make_unique<WorkflowVisualizationService>();
    viz_svc->set_output_file("workflow.dot");
    app.service_manager().add_service(std::move(viz_svc));

    // Add algorithms...
    app.add_algorithm(std::make_unique<MyAnalysis>());

    // Run with progress bar
    app.initialize();

    size_t total_events = 10000;
    ProgressBar progress(total_events, "Processing");

    for (size_t i = 0; i < total_events; ++i) {
        app.execute_event();
        progress.update(i + 1);
    }
    progress.finish();

    app.finalize();

    return 0;
}
```

---

## Summary of Improvements

| # | Feature | File | Lines | Status |
|---|---------|------|-------|--------|
| 1 | Logging Service | LoggingService.hpp | 240 | ✅ Complete |
| 2 | Performance Profiling | ProfilingService.hpp | 220 | ✅ Complete |
| 3 | Summary Report | SummaryService.hpp | 210 | ✅ Complete |
| 4 | Histogram Verification | HistogramService.hpp | +100 | ✅ Complete |
| 5 | Histogram Templates | HistogramTemplates.hpp | 320 | ✅ Complete |
| 6 | Thread-Safe Histograms | ThreadSafeHistogram.hpp | 180 | ✅ Complete |
| 7 | N-tuple Service | NTupleService.hpp | 280 | ✅ Complete |
| 8 | Progress Bar with ETA | ProgressBar.hpp | 240 | ✅ Complete |
| 9 | Metadata Service | MetadataService.hpp | 290 | ✅ Complete |
| 10 | Event Selection Service | SelectionService.hpp | 290 | ✅ Complete |
| 11 | Corrections Database | CorrectionsService.hpp | 320 | ✅ Complete |
| 12 | Workflow Visualization | WorkflowVisualizationService.hpp | 650 | ✅ Complete |

**Total**: 12 major improvements, ~3340 lines of new code

---

## Benefits

### Code Quality
✅ **Professional logging** - Proper log levels and formatting
✅ **Performance monitoring** - Identify bottlenecks automatically
✅ **Comprehensive reporting** - End-of-job summary
✅ **Data integrity** - Verify histograms written correctly
✅ **Reusable components** - Histogram templates
✅ **Thread safety** - Safe parallel processing
✅ **Flexible output** - Histograms + N-tuples
✅ **Provenance tracking** - Complete metadata storage
✅ **Centralized selection** - Reusable event cuts
✅ **Detector corrections** - Flexible calibration system
✅ **Workflow understanding** - Visual algorithm graphs

### User Experience
✅ **Better debugging** - Clear log messages with timestamps
✅ **Performance insights** - Know where time is spent
✅ **Analysis transparency** - Complete summary reports
✅ **Easier setup** - Templates for common histograms
✅ **Parallel ready** - Thread-safe operations
✅ **Event-level data** - N-tuples for detailed analysis
✅ **Real-time progress** - Visual progress bars with ETA
✅ **Reproducibility** - Full metadata and parameters
✅ **Clear selections** - Named cuts with statistics
✅ **Easy corrections** - Simple calibration application

### Production Readiness
✅ **Industry patterns** - Follows ATLAS/CMS/LHCb conventions
✅ **Robust** - Error handling and verification
✅ **Scalable** - Thread-safe and optimized
✅ **Documented** - Comprehensive documentation
✅ **Tested** - All features compile and integrate
✅ **Traceable** - Complete provenance chain
✅ **Maintainable** - Clean service architecture
✅ **Visualizable** - Workflow dependency graphs

---

## Files Summary

### New Services (8 files)
- [Framework/Services/LoggingService.hpp](include/Framework/Services/LoggingService.hpp)
- [Framework/Services/ProfilingService.hpp](include/Framework/Services/ProfilingService.hpp)
- [Framework/Services/SummaryService.hpp](include/Framework/Services/SummaryService.hpp)
- [Framework/Services/NTupleService.hpp](include/Framework/Services/NTupleService.hpp)
- [Framework/Services/MetadataService.hpp](include/Framework/Services/MetadataService.hpp)
- [Framework/Services/SelectionService.hpp](include/Framework/Services/SelectionService.hpp)
- [Framework/Services/CorrectionsService.hpp](include/Framework/Services/CorrectionsService.hpp)
- [Framework/Services/WorkflowVisualizationService.hpp](include/Framework/Services/WorkflowVisualizationService.hpp)

### New Libraries (3 files)
- [Framework/HistogramTemplates.hpp](include/Framework/HistogramTemplates.hpp)
- [Framework/ThreadSafeHistogram.hpp](include/Framework/ThreadSafeHistogram.hpp)
- [Framework/ProgressBar.hpp](include/Framework/ProgressBar.hpp)

### Enhanced Services (1 file modified)
- [Framework/Services/HistogramService.hpp](include/Framework/Services/HistogramService.hpp) - Added verification

### Documentation (1 file)
- [FRAMEWORK_IMPROVEMENTS.md](FRAMEWORK_IMPROVEMENTS.md) - This file

**Total**: 12 new files, 1 modified file

---

**Framework Version**: 2.3 (Complete Enhancement Package)
**Status**: ✅ Production Ready
**Date**: 2025-01-25

