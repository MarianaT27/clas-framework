# Services

Services are long-lived objects that provide shared functionality to algorithms. They are initialized once at application startup and finalized after all events are processed.

## HistogramService

Centralized histogram management with automatic ROOT file writing.

### Setup

```cpp
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(hist_svc));
```

### Usage

```cpp
auto initialize() -> Framework::StatusCode override {
    m_hist_svc = service<Framework::HistogramService>("HistogramService");

    // Book 1D histogram
    h_momentum = m_hist_svc->book_hist1D(
        "/Electron/momentum",           // Path in ROOT file
        "Electron Momentum;p [GeV/c];Events",
        100, 0, 10                      // bins, min, max
    );

    // Book 2D histogram
    h_p_vs_theta = m_hist_svc->book_hist2D(
        "/Electron/p_vs_theta",
        "p vs #theta;#theta [deg];p [GeV/c]",
        180, 0, 180,  // x: theta bins
        100, 0, 10    // y: p bins
    );

    return Framework::StatusCode::SUCCESS();
}

auto execute(Framework::Event& event) -> Framework::StatusCode override {
    h_momentum->Fill(electron.p());
    h_p_vs_theta->Fill(electron.theta(), electron.p());
    return Framework::StatusCode::SUCCESS();
}
```

### Features

- Automatic ROOT file management
- Thread-safe filling
- Organized directory structure
- No manual histogram deletion needed

---

## LoggingService

Structured logging with levels and colors.

### Setup

```cpp
auto log_svc = std::make_unique<Framework::LoggingService>();
log_svc->set_log_level(Framework::LogLevel::INFO);
log_svc->set_log_file("analysis.log");  // Optional
app.service_manager().add_service(std::move(log_svc));
```

### Usage

```cpp
// Use the global Log singleton (automatically configured)
Framework::Log::info(name(), "Processing {} events", n_events);
Framework::Log::debug(name(), "Electron momentum: {:.3f} GeV", e.p());
Framework::Log::warning(name(), "Value {} out of range", value);
Framework::Log::error(name(), "Failed to read bank");
```

### Log Levels

| Level | Description | Color |
|-------|-------------|-------|
| `DEBUG` | Detailed diagnostic info | Gray |
| `INFO` | General information | White |
| `WARNING` | Warning messages | Yellow |
| `ERROR` | Error messages | Red |
| `CRITICAL` | Critical errors | Red Bold |

---

## NTupleService

ROOT TTree management for event-by-event data.

### Setup

```cpp
auto ntuple_svc = std::make_unique<Framework::NTupleService>();
ntuple_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(ntuple_svc));
```

### Usage

```cpp
class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::NTupleService* m_ntuple_svc = nullptr;
    double m_e_p = 0.0;
    double m_q2 = 0.0;

    auto initialize() -> Framework::StatusCode override {
        m_ntuple_svc = service<Framework::NTupleService>("NTupleService");

        // Register branches
        m_ntuple_svc->register_branch("electron_data", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("electron_data", "q2", &m_q2);

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_e_p = electron.p();
        m_q2 = kin.q2;

        // Fill N-tuple
        m_ntuple_svc->fill("electron_data");

        return Framework::StatusCode::SUCCESS();
    }
};
```

### Access in ROOT

```bash
root output.root
root [1] electron_data->Draw("q2")
root [2] electron_data->Scan("e_p:q2")
```

---

## ConditionsService

Run-dependent calibrations with Interval of Validity (IOV).

### Setup

```cpp
auto cond_svc = std::make_unique<Framework::ConditionsService>();
cond_svc->set_run_number(5032);
app.service_manager().add_service(std::move(cond_svc));
```

### Usage

```cpp
auto initialize() -> Framework::StatusCode override {
    m_cond_svc = service<Framework::ConditionsService>("ConditionsService");

    // Get run-dependent constants
    double time_offset = m_cond_svc->get_time_offset("FTOF", sector);

    // Set custom calibrations with IOV
    Framework::IntervalOfValidity iov{5000, 5100};  // Runs 5000-5100
    m_cond_svc->set_value("my_calibration", 1.23, iov);

    return Framework::StatusCode::SUCCESS();
}
```

---

## HipoFileService

Centralized HIPO file I/O management for reading and writing HIPO files.

### Setup

```cpp
auto hipo_svc = std::make_unique<Framework::HipoFileService>();
hipo_svc->add_input_file("data/run_5032.hipo");
hipo_svc->set_output_file("filtered_events.hipo");
app.service_manager().add_service(std::move(hipo_svc));
```

### Usage

```cpp
auto initialize() -> Framework::StatusCode override {
    m_hipo_svc = service<Framework::HipoFileService>("HipoFileService");

    // Get dictionary from input files
    auto* dict = m_hipo_svc->get_dictionary();

    return Framework::StatusCode::SUCCESS();
}

auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Write filtered events to output
    if (passes_selection) {
        m_hipo_svc->write_event(event.hipo_event());
    }

    return Framework::StatusCode::SUCCESS();
}
```

### Features

- Multiple input file support
- Thread-safe event writing
- Automatic dictionary management
- Event statistics tracking

---

## RootFileService

Centralized ROOT file management for coordinating multiple output files.

### Setup

```cpp
auto root_svc = std::make_unique<Framework::RootFileService>();
root_svc->set_default_output("analysis.root");
app.service_manager().add_service(std::move(root_svc));
```

### Usage

```cpp
auto initialize() -> Framework::StatusCode override {
    m_root_svc = service<Framework::RootFileService>("RootFileService");

    // Open additional output files
    m_root_svc->open_file("systematics", "systematics.root", "RECREATE");

    // Create directory structure
    m_root_svc->mkdir("default", "Electrons/Kinematics");

    return Framework::StatusCode::SUCCESS();
}

auto finalize() -> Framework::StatusCode override {
    // Write objects to specific files
    m_root_svc->write_object("systematics", my_histogram);

    return Framework::StatusCode::SUCCESS();
}
```

### Features

- Multiple file management with aliases
- Thread-safe file operations
- Directory structure creation
- Automatic file lifecycle management

---

## PhotonCutsService

AI/ML-based photon selection using Gradient Boosted Tree (GBT) models.

### Setup

```cpp
auto photon_svc = std::make_unique<Framework::PhotonCutsService>();
photon_svc->set_pass(1);  // Reconstruction pass
app.service_manager().add_service(std::move(photon_svc));
```

### Usage

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    auto& hipo_event = event.hipo_event();
    auto& RUN_config = hipo_event["RUN::config"];
    auto& REC_Particle = hipo_event["REC::Particle"];
    auto& REC_Calorimeter = hipo_event["REC::Calorimeter"];

    // Select photons using AI classification
    auto good_photons = m_photon_svc->select_photons_with_AI(
        RUN_config, REC_Particle, REC_Calorimeter);

    // Individual cuts are also available
    bool pass_fiducial = Framework::PhotonCutsService::pass_pcal_fiducial_cut(v, w);
    bool pass_fd = Framework::PhotonCutsService::pass_forward_detector_cut(theta_deg);
    bool pass_energy = Framework::PhotonCutsService::pass_energy_cut(E);

    return Framework::StatusCode::SUCCESS();
}
```

### Features

- Run-dependent model selection (RGA, RGC campaigns)
- Configurable AI threshold (default: 0.78)
- PCAL fiducial cuts
- Forward detector angle cuts
- Minimum energy cuts

---

## ParticleSelectorService

Service-based particle extraction and filtering.

### Setup

```cpp
auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();
particle_svc->set_default_types({
    CLAS::PIDs::ELECTRON, CLAS::PIDs::PHOTON,
    CLAS::PIDs::PROTON, CLAS::PIDs::PIPLUS, CLAS::PIDs::PIMINUS
});
app.service_manager().add_service(std::move(particle_svc));
```

### Usage

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    auto& bank = event.get_bank("REC::Particle");

    // Extract all particles
    auto particles = m_particle_svc->extract_particles(bank);

    // Get specific particle types
    const auto& electrons = m_particle_svc->get_electrons(particles);
    const auto& photons = m_particle_svc->get_photons(particles);

    // Apply filters
    auto good_electrons = m_particle_svc->with_momentum_range(electrons, 1.0, 10.0);
    auto fd_electrons = m_particle_svc->get_forward_detector(good_electrons);

    return Framework::StatusCode::SUCCESS();
}
```

### Available Filters

| Method | Description |
|--------|-------------|
| `with_momentum_range(particles, min, max)` | Filter by momentum |
| `with_vz_range(particles, min, max)` | Filter by vertex z |
| `with_theta_range(particles, min, max)` | Filter by theta angle |
| `with_chi2pid_range(particles, min, max)` | Filter by chi2pid |
| `with_charge(particles, charge)` | Filter by charge |
| `get_forward_detector(particles)` | Get FD particles |
| `get_central_detector(particles)` | Get CD particles |

---

## SamplingFractionService

Target/polarity-dependent sampling fraction cuts for electron identification.

### Setup

```cpp
auto sf_svc = std::make_unique<Framework::SamplingFractionService>();
app.service_manager().add_service(std::move(sf_svc));
```

### Usage

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Calculate sampling fraction
    double SF = (pcal_e + ecin_e + ecout_e) / electron.p();

    // Check if passes cut
    bool passes = m_sf_svc->passes_cut(
        SF, electron.p(), sector,
        CLAS::Target::LD2, CLAS::Polarity::Outbending);

    return Framework::StatusCode::SUCCESS();
}
```

---

## MetadataService

Store analysis metadata for reproducibility.

### Setup

```cpp
auto metadata_svc = std::make_unique<Framework::MetadataService>();
metadata_svc->set_output_file("output.root");
metadata_svc->set_version("2.0");
metadata_svc->set_beam_energy(10.53);
metadata_svc->set_target("LD2");
app.service_manager().add_service(std::move(metadata_svc));
```

---

## Additional Services

| Service | Description |
|---------|-------------|
| `ProfilingService` | Performance profiling and timing |
| `ConfigService` | Configuration file parsing |
| `WorkflowVisualizationService` | Generate workflow diagrams |
| `SelectionService` | Generic selection criteria |
| `SummaryService` | Analysis summary generation |

---

## ServiceHandle Pattern

Type-safe service access with validation at initialization:

```cpp
class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};

    auto initialize() -> Framework::StatusCode override {
        // Initialize handles - validates service availability
        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            return hist_sc;  // Required service not available
        }

        // Use services via handle
        if (m_hist_svc) {
            h_momentum = m_hist_svc->book_hist1D("/Analysis/p", "Momentum", 100, 0, 10);
        }

        return Framework::StatusCode::SUCCESS();
    }
};
```

---

## Creating Custom Services

Implement the `IService` interface:

```cpp
#pragma once

#include "Framework/IService.hpp"

namespace Framework {

class MyCustomService : public IService {
public:
    auto name() const -> const std::string& override {
        static std::string n = "MyCustomService";
        return n;
    }

    auto initialize() -> StatusCode override {
        // Setup
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        // Cleanup
        return StatusCode::SUCCESS();
    }

    // Custom methods
    auto do_something() -> void {
        // ...
    }

private:
    // Private members
};

}  // namespace Framework
```

Register and use:

```cpp
app.service_manager().add_service(std::make_unique<Framework::MyCustomService>());

// In algorithm
auto* my_svc = service<Framework::MyCustomService>("MyCustomService");
my_svc->do_something();
```
