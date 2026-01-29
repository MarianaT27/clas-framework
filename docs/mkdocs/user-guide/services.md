# Services

Services are long-lived objects that provide shared functionality to algorithms. They are initialized once at application startup and finalized after all events are processed.

All services are in the `clas12::framework` namespace and can be included from `<clas12/framework/services/...>`.

## HistogramService

Centralized histogram management with automatic ROOT file writing. All histograms are **thread-safe atomic histograms** by default, enabling lock-free parallel event processing.

### Setup

```cpp
#include <clas12/framework/services/HistogramService.hpp>

auto hist_svc = std::make_unique<clas12::framework::HistogramService>();
hist_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(hist_svc));
```

### Usage

```cpp
// Member variables - use AtomicHistogram types
clas12::framework::AtomicHistogram1D* h_momentum = nullptr;
clas12::framework::AtomicHistogram2D* h_p_vs_theta = nullptr;

auto initialize() -> clas12::framework::StatusCode override {
    m_hist_svc = service<clas12::framework::HistogramService>("HistogramService");

    // Book 1D histogram (returns AtomicHistogram1D*)
    h_momentum = m_hist_svc->book_hist1D(
        "/Electron/momentum",           // Path in ROOT file
        "Electron Momentum;p [GeV/c];Events",
        100, 0, 10                      // bins, min, max
    );

    // Book 2D histogram (returns AtomicHistogram2D*)
    h_p_vs_theta = m_hist_svc->book_hist2D(
        "/Electron/p_vs_theta",
        "p vs #theta;#theta [deg];p [GeV/c]",
        180, 0, 180,  // x: theta bins
        100, 0, 10    // y: p bins
    );

    return clas12::framework::StatusCode::SUCCESS();
}

auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Use fill() method (lowercase) for atomic histograms
    h_momentum->fill(electron.p());
    h_p_vs_theta->fill(electron.theta(), electron.p());
    return clas12::framework::StatusCode::SUCCESS();
}
```

### Physics Histogram Templates

The service provides convenient methods for common physics quantities:

```cpp
// DIS kinematics
h_Q2 = m_hist_svc->book_q2_hist("/DIS/Q2", "Q^{2}");
h_W = m_hist_svc->book_w_hist("/DIS/W", "W");
h_xB = m_hist_svc->book_xb_hist("/DIS/xB", "x_{B}");

// Particle kinematics
h_p = m_hist_svc->book_momentum_hist("/Electron/p", "Electron Momentum");
h_theta = m_hist_svc->book_theta_hist("/Electron/theta", "Electron #theta");
h_phi = m_hist_svc->book_phi_hist("/Electron/phi", "Electron #phi");
h_vz = m_hist_svc->book_vz_hist("/Electron/vz", "Electron v_{z}");

// 2D correlations
h_theta_vs_p = m_hist_svc->book_theta_vs_p("/Electron/theta_vs_p", "Electron #theta vs p");
```

### AtomicHistogram API

| Method | Description |
|--------|-------------|
| `fill(value)` | Fill with value (weight = 1) |
| `fill(value, weight)` | Fill with value and weight |
| `entries()` | Get number of entries |
| `to_root(name, title)` | Convert to ROOT TH1D/TH2D for fitting |

### Converting to ROOT for Fitting

When you need to perform fits or use ROOT-specific features:

```cpp
// Convert atomic histogram to ROOT format for fitting
auto root_hist = h_mass->to_root("mass_for_fit", "Invariant Mass");
root_hist->Fit("gaus", "Q");
```

### Features

- **Lock-free atomic filling** - No mutex contention in parallel processing
- Automatic ROOT file management
- Organized directory structure
- Physics-aware histogram templates
- Automatic conversion to ROOT format on finalize
- No manual histogram deletion needed

---

## LoggingService

Structured logging with levels and colors.

### Setup

```cpp
#include <clas12/framework/services/LoggingService.hpp>

auto log_svc = std::make_unique<clas12::framework::LoggingService>();
log_svc->set_log_level(clas12::framework::LogLevel::INFO);
log_svc->set_log_file("analysis.log");  // Optional
app.service_manager().add_service(std::move(log_svc));
```

### Usage

```cpp
// Use the global Log singleton (automatically configured)
clas12::framework::Log::info(name(), "Processing {} events", n_events);
clas12::framework::Log::debug(name(), "Electron momentum: {:.3f} GeV", e.p());
clas12::framework::Log::warning(name(), "Value {} out of range", value);
clas12::framework::Log::error(name(), "Failed to read bank");
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
#include <clas12/framework/services/NTupleService.hpp>

auto ntuple_svc = std::make_unique<clas12::framework::NTupleService>();
ntuple_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(ntuple_svc));
```

### Usage

```cpp
class MyAlgorithm : public clas12::framework::Algorithm {
private:
    clas12::framework::NTupleService* m_ntuple_svc = nullptr;
    double m_e_p = 0.0;
    double m_q2 = 0.0;

    auto initialize() -> clas12::framework::StatusCode override {
        m_ntuple_svc = service<clas12::framework::NTupleService>("NTupleService");

        // Register branches
        m_ntuple_svc->register_branch("electron_data", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("electron_data", "q2", &m_q2);

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_e_p = electron.p();
        m_q2 = kin.q2;

        // Fill N-tuple
        m_ntuple_svc->fill("electron_data");

        return clas12::framework::StatusCode::SUCCESS();
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
#include <clas12/framework/services/ConditionsService.hpp>

auto cond_svc = std::make_unique<clas12::framework::ConditionsService>();
cond_svc->set_run_number(5032);
app.service_manager().add_service(std::move(cond_svc));
```

### Usage

```cpp
auto initialize() -> clas12::framework::StatusCode override {
    m_cond_svc = service<clas12::framework::ConditionsService>("ConditionsService");

    // Get run-dependent constants
    double time_offset = m_cond_svc->get_time_offset("FTOF", sector);

    // Set custom calibrations with IOV
    clas12::framework::IntervalOfValidity iov{5000, 5100};  // Runs 5000-5100
    m_cond_svc->set_value("my_calibration", 1.23, iov);

    return clas12::framework::StatusCode::SUCCESS();
}
```

---

## HipoFileService

Centralized HIPO file I/O management for reading and writing HIPO files.

### Setup

```cpp
#include <clas12/framework/services/HipoFileService.hpp>

auto hipo_svc = std::make_unique<clas12::framework::HipoFileService>();
hipo_svc->add_input_file("data/run_5032.hipo");
hipo_svc->set_output_file("filtered_events.hipo");
app.service_manager().add_service(std::move(hipo_svc));
```

### Usage

```cpp
auto initialize() -> clas12::framework::StatusCode override {
    m_hipo_svc = service<clas12::framework::HipoFileService>("HipoFileService");

    // Get dictionary from input files
    auto* dict = m_hipo_svc->get_dictionary();

    return clas12::framework::StatusCode::SUCCESS();
}

auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Write filtered events to output
    if (passes_selection) {
        m_hipo_svc->write_event(event.hipo_event());
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Features

- Multiple input file support
- Thread-safe event writing
- Automatic dictionary management
- Event statistics tracking

---

## PhotonCutsService

AI/ML-based photon selection using Gradient Boosted Tree (GBT) models.

### Setup

```cpp
#include <clas12/framework/services/PhotonCutsService.hpp>

auto photon_svc = std::make_unique<clas12::framework::PhotonCutsService>();
photon_svc->set_pass(1);  // Reconstruction pass
app.service_manager().add_service(std::move(photon_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    auto& hipo_event = event.hipo_event();
    auto& RUN_config = hipo_event["RUN::config"];
    auto& REC_Particle = hipo_event["REC::Particle"];
    auto& REC_Calorimeter = hipo_event["REC::Calorimeter"];

    // Select photons using AI classification
    auto good_photons = m_photon_svc->select_photons_with_AI(
        RUN_config, REC_Particle, REC_Calorimeter);

    // Individual cuts are also available
    bool pass_fiducial = clas12::framework::PhotonCutsService::pass_pcal_fiducial_cut(v, w);
    bool pass_fd = clas12::framework::PhotonCutsService::pass_forward_detector_cut(theta_deg);
    bool pass_energy = clas12::framework::PhotonCutsService::pass_energy_cut(E);

    return clas12::framework::StatusCode::SUCCESS();
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
#include <clas12/framework/services/ParticleSelectorService.hpp>

auto particle_svc = std::make_unique<clas12::framework::ParticleSelectorService>();
particle_svc->set_default_types({
    clas12::PIDs::ELECTRON, clas12::PIDs::PHOTON,
    clas12::PIDs::PROTON, clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS
});
app.service_manager().add_service(std::move(particle_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    auto& bank = event.get_bank("REC::Particle");

    // Extract all particles
    auto particles = m_particle_svc->extract_particles(bank);

    // Get specific particle types
    const auto& electrons = m_particle_svc->get_electrons(particles);
    const auto& photons = m_particle_svc->get_photons(particles);

    // Apply filters
    auto good_electrons = m_particle_svc->with_momentum_range(electrons, 1.0, 10.0);
    auto fd_electrons = m_particle_svc->get_forward_detector(good_electrons);

    return clas12::framework::StatusCode::SUCCESS();
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
#include <clas12/framework/services/SamplingFractionService.hpp>

auto sf_svc = std::make_unique<clas12::framework::SamplingFractionService>();
app.service_manager().add_service(std::move(sf_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Calculate sampling fraction
    double SF = (pcal_e + ecin_e + ecout_e) / electron.p();

    // Check if passes cut
    bool passes = m_sf_svc->passes_cut(
        SF, electron.p(), sector,
        clas12::Target::LD2, clas12::Polarity::Outbending);

    return clas12::framework::StatusCode::SUCCESS();
}
```

---

## MetadataService

Store analysis metadata for reproducibility.

### Setup

```cpp
#include <clas12/framework/services/MetadataService.hpp>

auto metadata_svc = std::make_unique<clas12::framework::MetadataService>();
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
class MyAlgorithm : public clas12::framework::Algorithm {
private:
    clas12::framework::ServiceHandle<clas12::framework::HistogramService> m_hist_svc{"HistogramService"};
    clas12::framework::ServiceHandle<clas12::framework::LoggingService> m_log_svc{"LoggingService"};

    auto initialize() -> clas12::framework::StatusCode override {
        // Initialize handles - validates service availability
        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            return hist_sc;  // Required service not available
        }

        // Use services via handle
        if (m_hist_svc) {
            h_momentum = m_hist_svc->book_hist1D("/Analysis/p", "Momentum", 100, 0, 10);
        }

        return clas12::framework::StatusCode::SUCCESS();
    }
};
```

---

## Creating Custom Services

Implement the `IService` interface:

```cpp
#pragma once

#include <clas12/framework/core/IService.hpp>

namespace my_analysis {

class MyCustomService : public clas12::framework::IService {
public:
    auto name() const -> const std::string& override {
        static std::string n = "MyCustomService";
        return n;
    }

    auto initialize() -> clas12::framework::StatusCode override {
        // Setup
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto finalize() -> clas12::framework::StatusCode override {
        // Cleanup
        return clas12::framework::StatusCode::SUCCESS();
    }

    // Custom methods
    auto do_something() -> void {
        // ...
    }

private:
    // Private members
};

}  // namespace my_analysis
```

Register and use:

```cpp
app.service_manager().add_service(std::make_unique<my_analysis::MyCustomService>());

// In algorithm
auto* my_svc = service<my_analysis::MyCustomService>("MyCustomService");
my_svc->do_something();
```

---

## MCTruthService

Service for MC truth matching and efficiency studies. Enables comparison of reconstructed particles with generator-level truth.

### Setup

```cpp
#include <clas12/framework/services/MCTruthService.hpp>

auto mc_svc = std::make_unique<clas12::framework::MCTruthService>();
app.service_manager().add_service(std::move(mc_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Load MC particles from bank
    auto& mc_bank = event.get_bank("MC::Particle");
    m_mc_svc->load_mc_particles(mc_bank);

    // Get all MC particles
    const auto& mc_particles = m_mc_svc->mc_particles();

    // Get MC particles of specific type
    auto mc_protons = m_mc_svc->mc_particles(clas12::PIDs::PROTON);

    // Match reconstructed particle to truth
    auto& match_bank = event.get_bank("MC::RecMatch");
    const auto* mc_match = m_mc_svc->match(reco_electron, match_bank);

    if (mc_match) {
        // Calculate momentum resolution
        auto resolution = m_mc_svc->momentum_resolution(reco_electron);
        if (resolution) {
            h_resolution->fill(*resolution);
        }
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Features

- MC particle extraction from HIPO banks
- Truth matching using MC::RecMatch bank
- Momentum/angle resolution calculation
- Reconstruction efficiency determination

---

## FiducialService

Unified detector fiducial cuts for acceptance modeling.

### Setup

```cpp
#include <clas12/framework/services/FiducialService.hpp>

auto fid_svc = std::make_unique<clas12::framework::FiducialService>();
fid_svc->register_pcal_fiducial(9.0, 9.0);  // lu_min, lw_min
fid_svc->register_dc_edge_cuts(1.68, 2.0, 8.75);  // R1, R2, R3 cuts
app.service_manager().add_service(std::move(fid_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    auto& calo_bank = event.get_bank("REC::Calorimeter");
    auto& traj_bank = event.get_bank("REC::Traj");

    // Check individual detector regions
    bool pass_pcal = m_fid_svc->passes_fiducial(
        electron, DetectorRegion::PCAL, calo_bank, traj_bank);

    // Check all fiducial cuts
    bool pass_all = m_fid_svc->passes_all_fiducial(electron, banks);

    // Get acceptance weight for efficiency correction
    double weight = m_fid_svc->acceptance_weight(electron, DetectorRegion::PCAL);

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Detector Regions

| Region | Description |
|--------|-------------|
| `PCAL` | Pre-shower calorimeter |
| `ECin` | Inner electromagnetic calorimeter |
| `ECout` | Outer electromagnetic calorimeter |
| `DC_R1` | Drift chamber region 1 |
| `DC_R2` | Drift chamber region 2 |
| `DC_R3` | Drift chamber region 3 |
| `HTCC` | High threshold Cherenkov counter |
| `FTOF` | Forward time-of-flight |

---

## GoodRunListService

Data quality filtering based on run conditions.

### Setup

```cpp
#include <clas12/framework/services/GoodRunListService.hpp>

auto grl_svc = std::make_unique<clas12::framework::GoodRunListService>();
grl_svc->load_from_file("config/good_runs_rgc.json");
grl_svc->require_flag("beam_stable");
grl_svc->require_flag("torus_nominal");
grl_svc->set_run_range(16000, 17000);
app.service_manager().add_service(std::move(grl_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Check if current run passes quality cuts
    if (!m_grl_svc->check_event(event)) {
        return clas12::framework::StatusCode::RECOVERABLE();
    }

    // Get run-specific information
    auto run_info = m_grl_svc->get_run_info(event.run_number());
    if (run_info) {
        double lumi_weight = m_grl_svc->luminosity_weight(event.run_number());
        h_yield->fill(value, lumi_weight);
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### GRL JSON Format

```json
{
  "runs": [
    {
      "run_number": 16042,
      "is_good": true,
      "quality_flags": "beam_stable,torus_nominal,solenoid_ok",
      "luminosity": 1.234e6,
      "target": "LD2",
      "beam_energy": 10.6
    }
  ]
}
```

---

## EventMixingService

Combinatorial background estimation for resonance analyses.

### Setup

```cpp
#include <clas12/framework/services/EventMixingService.hpp>

auto mix_svc = std::make_unique<clas12::framework::EventMixingService>();
mix_svc->add_vz_bins({-10, -5, 0, 5, 10});
mix_svc->add_Q2_bins({1.0, 2.0, 3.0, 5.0, 8.0});
app.service_manager().add_service(std::move(mix_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Add particles to mixing pool
    m_mix_svc->add_to_pool("piplus", event, pi_plus_particles);
    m_mix_svc->add_to_pool("piminus", event, pi_minus_particles);

    // Get mixed pairs from different events (same kinematic bin)
    auto mixed_pairs = m_mix_svc->get_mixed_pairs<Particle, Particle>(
        "piplus", "piminus", event);

    for (const auto& [pip, pim] : mixed_pairs) {
        double m_mixed = (*pip + *pim).M();
        h_mixed_mass->fill(m_mixed, m_mix_svc->mixed_event_weight());
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Features

- Multi-dimensional binning (vertex z, Q², sector)
- Configurable pool size per bin
- Automatic event weight calculation
- Thread-safe event pooling

---

## SystematicService

Systematic uncertainty framework with histogram cloning.

### Setup

```cpp
#include <clas12/framework/services/SystematicService.hpp>

auto syst_svc = std::make_unique<clas12::framework::SystematicService>();
syst_svc->add_energy_scale(0.01);       // 1% energy scale
syst_svc->add_momentum_resolution(0.005); // 0.5% resolution smearing
app.service_manager().add_service(std::move(syst_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Get current variation name
    std::string var = m_syst_svc->current_variation();

    // Apply variation to particle
    Particle varied_electron = m_syst_svc->vary(electron);

    // Get event weight for this variation
    double weight = m_syst_svc->weight(event);

    // Fill histograms (automatically cloned for each variation)
    h_mass->fill(varied_electron.p(), weight);

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Registering Custom Variations

```cpp
m_syst_svc->add_efficiency_variation("pid_eff",
    [](const Event& e) -> double {
        return 1.0 + 0.02 * (e.run_number() > 16000 ? 1 : -1);
    });
```

---

## MVAService

Machine learning-based classification using TMVA.

### Setup

```cpp
#include <clas12/framework/services/MVAService.hpp>

auto mva_svc = std::make_unique<clas12::framework::MVAService>();
mva_svc->register_model("electron_id", "weights/electron_bdt.xml");
app.service_manager().add_service(std::move(mva_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Evaluate BDT score
    float bdt_score = m_mva_svc->evaluate("electron_id", electron);

    // Check if passes threshold
    if (m_mva_svc->passes_cut("electron_id", electron, 0.85)) {
        // Good electron candidate
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### TOML Configuration

```toml
[MVA]
electron_id = "weights/electron_bdt.xml"
electron_threshold = 0.85
photon_id = "weights/photon_gbt.xml"
photon_threshold = 0.78
```

---

## WeightService

Event weights for acceptance, efficiency, and normalization corrections.

### Setup

```cpp
#include <clas12/framework/services/WeightService.hpp>

auto weight_svc = std::make_unique<clas12::framework::WeightService>();
weight_svc->load_efficiency_map("electron_eff", "efficiency.root", "h_eff_p_theta");
app.service_manager().add_service(std::move(weight_svc));
```

### Usage

```cpp
auto initialize() -> clas12::framework::StatusCode override {
    // Register weight calculators
    m_weight_svc->register_weight("luminosity", WeightType::Luminosity,
        [this](const Event& e) { return m_grl_svc->luminosity_weight(e.run_number()); });

    return clas12::framework::StatusCode::SUCCESS();
}

auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Get total event weight
    auto total_weight = m_weight_svc->get_total_weight(event);
    h_yield->fill(x, total_weight.value);

    // 2D efficiency lookup
    double eff = m_weight_svc->lookup_efficiency("electron_eff",
        electron.p(), electron.theta());

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Weight Types

| Type | Description |
|------|-------------|
| `Luminosity` | Run luminosity normalization |
| `Acceptance` | Geometric acceptance |
| `Efficiency` | Reconstruction/selection efficiency |
| `Trigger` | Trigger efficiency |
| `PID` | Particle ID efficiency |
| `Radiative` | Radiative corrections |
| `Custom` | User-defined |

---

## VertexFittingService

Primary and secondary vertex reconstruction for displaced decay analyses.

### Setup

```cpp
#include <clas12/framework/services/VertexFittingService.hpp>

auto vtx_svc = std::make_unique<clas12::framework::VertexFittingService>();
app.service_manager().add_service(std::move(vtx_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Fit primary vertex
    auto pv_result = m_vtx_svc->fit_primary_vertex_robust(tracks, 9.0);

    if (pv_result.is_good()) {
        // Fit secondary vertex (Lambda decay)
        auto sv_result = m_vtx_svc->fit_secondary_vertex(proton, pion);

        if (sv_result.is_good()) {
            // Calculate flight distance and significance
            auto [flight_dist, flight_err] = m_vtx_svc->flight_distance(pv_result, sv_result);

            // Decay length significance (for lifetime measurements)
            double dls = m_vtx_svc->decay_length_significance(
                pv_result, sv_result, lambda_momentum);

            h_flight_distance->fill(flight_dist);
            h_decay_significance->fill(dls);
        }
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### VertexFitResult Structure

| Field | Description |
|-------|-------------|
| `converged` | Fit convergence status |
| `position` | Fitted vertex position (XYZVector) |
| `covariance` | 3x3 covariance matrix (Eigen) |
| `chi2`, `ndf` | Fit quality |
| `probability` | P(chi2, ndf) |
| `refitted_momenta` | Track momenta at vertex |

---

## DecayTreeFitterService

LHCb-style global topological decay fitting with constraint propagation.

### Setup

```cpp
#include <clas12/framework/services/DecayTreeFitterService.hpp>

auto dtf_svc = std::make_unique<clas12::framework::DecayTreeFitterService>();
app.service_manager().add_service(std::move(dtf_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Fluent interface for complex decays
    auto result = m_dtf_svc->create_fitter(DecayDescriptor::parse("Lambda0 -> p+ pi-"))
        ->add_particle("p+", proton)
        ->add_particle("pi-", pion)
        ->constrain_mass(1.115683)      // Lambda mass constraint
        ->constrain_vertex()            // Common vertex
        ->constrain_pointing(primary_vertex)  // Points to PV
        ->fit();

    if (result.converged && result.probability > 0.01) {
        // Use fitted momenta for improved resolution
        auto fitted_lambda = result.fitted_momenta[0] + result.fitted_momenta[1];
        h_lambda_mass_dtf->fill(fitted_lambda.M());
    }

    // Simple two-body fit
    auto simple_result = m_dtf_svc->fit_two_body(proton, pion, LAMBDA_MASS, primary_vertex);

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Constraint Types

| Constraint | Description |
|------------|-------------|
| `Mass` | Fix invariant mass of particle combination |
| `Vertex` | Common decay vertex for daughters |
| `OriginVertex` | Fix vertex to known position |
| `Pointing` | Momentum points back to origin |

---

## PIDService

Log-likelihood particle identification combining multiple detectors.

### Setup

```cpp
#include <clas12/framework/services/PIDService.hpp>

auto pid_svc = std::make_unique<clas12::framework::PIDService>();
pid_svc->m_use_tof.set(true);
pid_svc->m_use_cherenkov.set(true);
pid_svc->m_use_calorimeter.set(true);
app.service_manager().add_service(std::move(pid_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Full PID evaluation
    auto pid_result = m_pid_svc->compute_pid(track, event);

    // Delta log-likelihood method (LHCb style)
    double dll_k_pi = pid_result.get_dll(PIDHypothesis::Kaon, PIDHypothesis::Pion);

    if (dll_k_pi > 0) {
        // More likely kaon than pion
    }

    // Simple decisions
    if (m_pid_svc->is_kaon(track, event, 0.0)) {
        h_kaon_mass->fill(track.mass());
    }

    // Get efficiency weight for PID selection
    double pid_weight = m_pid_svc->efficiency_weight(track, PIDHypothesis::Kaon, 0.0);

    return clas12::framework::StatusCode::SUCCESS();
}
```

### PID Hypotheses

`Electron`, `Positron`, `Muon`, `Pion`, `Kaon`, `Proton`, `Deuteron`, `Unknown`

### PIDResult Structure

| Field | Description |
|-------|-------------|
| `best_hypothesis` | Most likely particle type |
| `confidence` | Confidence level |
| `log_likelihoods` | LL for each hypothesis |
| `delta_ll` | DLL relative to pion |

---

## sPlotService

Statistical background subtraction using sWeights (LHCb/BaBar technique).

### Setup

```cpp
#include <clas12/framework/services/sPlotService.hpp>

auto splot_svc = std::make_unique<clas12::framework::sPlotService>();
app.service_manager().add_service(std::move(splot_svc));
```

### Usage

```cpp
// Configure sPlot in initialize()
auto initialize() -> clas12::framework::StatusCode override {
    m_splot_svc->create_splot("lambda", "mass", 1.08, 1.18)
               ->add_gaussian_signal("lambda", "signal", 1.1157, 0.002)
               ->add_polynomial_background("lambda", "bkg", 2);

    return clas12::framework::StatusCode::SUCCESS();
}

// Collect data in execute()
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    m_masses.push_back(lambda_mass);
    m_cos_theta.push_back(cos_theta);
    return clas12::framework::StatusCode::SUCCESS();
}

// Compute weights and fill in finalize()
auto finalize() -> clas12::framework::StatusCode override {
    // Fit and compute sWeights
    auto result = m_splot_svc->compute_sweights("lambda", m_masses);

    if (result.success) {
        // Fill background-subtracted distribution
        m_splot_svc->fill_with_sweights("lambda", "signal", h_cos_theta, m_cos_theta);

        fmt::print("Signal yield: {} +/- {}\n",
                   result.signal_yield(), result.yield_errors.at("signal"));
    }

    return clas12::framework::StatusCode::SUCCESS();
}
```

### PDF Types

| Method | Description |
|--------|-------------|
| `add_gaussian_signal` | Gaussian signal shape |
| `add_crystal_ball_signal` | Crystal Ball function |
| `add_double_gaussian_signal` | Double Gaussian |
| `add_polynomial_background` | Polynomial background |
| `add_exponential_background` | Exponential background |

---

## UnfoldingService

Detector response correction for cross-section measurements.

### Setup

```cpp
#include <clas12/framework/services/UnfoldingService.hpp>

auto unfold_svc = std::make_unique<clas12::framework::UnfoldingService>();
unfold_svc->m_bayesian_iterations.set(4);
app.service_manager().add_service(std::move(unfold_svc));
```

### Usage

```cpp
auto finalize() -> clas12::framework::StatusCode override {
    // Build response matrix from MC
    m_unfold_svc->build_response("q2",
        mc_true_values, mc_reco_values,
        10, 10,    // nbins_true, nbins_reco
        1.0, 10.0  // range
    );

    // Unfold measured distribution
    auto result = m_unfold_svc->unfold_bayesian("q2", h_measured, 4);

    if (result.success) {
        // result.unfolded contains corrected distribution
        auto unfolded = result.unfolded->Clone("h_q2_unfolded");

        // Check closure
        double chi2 = result.chi2_refolded;
    }

    // Get diagnostics
    auto purity = m_unfold_svc->get_purity("q2");
    auto stability = m_unfold_svc->get_stability("q2");
    auto efficiency = m_unfold_svc->get_efficiency("q2");

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Unfolding Methods

| Method | Description |
|--------|-------------|
| `Bayesian` | D'Agostini iterative unfolding |
| `SVD` | Singular Value Decomposition |
| `BinByBin` | Simple bin-by-bin correction |
| `MatrixInversion` | Direct matrix inversion |

---

## CrossSectionService

Complete cross-section calculation with acceptance, efficiency, and luminosity.

### Setup

```cpp
#include <clas12/framework/services/CrossSectionService.hpp>

auto xsec_svc = std::make_unique<clas12::framework::CrossSectionService>();
xsec_svc->m_beam_energy.set(10.6);
app.service_manager().add_service(std::move(xsec_svc));
```

### Usage

```cpp
auto finalize() -> clas12::framework::StatusCode override {
    // Define measurement
    m_xsec_svc->define_measurement("q2_xsec", "Q^{2}", "nb/GeV^{2}")
              ->set_luminosity("q2_xsec", 1.2e6)  // /pb
              ->set_yield("q2_xsec", h_yield)
              ->calculate_acceptance("q2_xsec", h_reco_mc, h_gen_mc)
              ->set_efficiency("q2_xsec", h_efficiency)
              ->set_radiative_correction("q2_xsec", h_rad_corr);

    // Calculate: σ = N / (L × A × ε × ΔX × R)
    auto result = m_xsec_svc->calculate("q2_xsec");

    result.print_summary();
    auto graph = result.to_graph();

    // With systematic uncertainties
    m_xsec_svc->add_systematic_yield("q2_xsec", "energy_up", h_yield_eup);
    m_xsec_svc->add_systematic_yield("q2_xsec", "energy_down", h_yield_edown);
    auto result_syst = m_xsec_svc->calculate_with_systematics("q2_xsec");

    return clas12::framework::StatusCode::SUCCESS();
}
```

### CrossSectionResult Structure

| Field | Description |
|-------|-------------|
| `cross_sections` | Differential cross-section per bin |
| `stat_errors` | Statistical uncertainties |
| `syst_errors_up/down` | Systematic uncertainties |
| `total_cross_section` | Integrated cross-section |
| `to_graph()` | Convert to TGraphAsymmErrors |

---

## TrackQualityService

Standardized track quality assessment and selection.

### Setup

```cpp
#include <clas12/framework/services/TrackQualityService.hpp>

auto trkq_svc = std::make_unique<clas12::framework::TrackQualityService>();
trkq_svc->m_chi2_cut_standard.set(10.0);
trkq_svc->m_min_ndf.set(5);
trkq_svc->m_dc_edge_cut.set(5.0);
app.service_manager().add_service(std::move(trkq_svc));
```

### Usage

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Assess track quality
    auto quality_info = m_trkq_svc->assess_quality(track, event);

    // Check specific quality level
    if (m_trkq_svc->passes_quality(track, event, TrackQuality::Standard)) {
        // Good track
    }

    // Filter tracks by quality
    auto good_tracks = m_trkq_svc->filter_by_quality(tracks, event, TrackQuality::Tight);

    // Get best track from duplicates
    auto best = m_trkq_svc->best_track(duplicate_tracks, event);

    // Get continuous quality score
    double score = m_trkq_svc->quality_score(track, event);

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Quality Levels

| Level | Chi2/ndf | DC Edge | Description |
|-------|----------|---------|-------------|
| `VeryLoose` | < 20 | > 2 cm | Minimal requirements |
| `Loose` | < 15 | > 3 cm | Relaxed selection |
| `Standard` | < 10 | > 5 cm | Default selection |
| `Tight` | < 5 | > 7 cm | Strict selection |
| `VeryTight` | < 3 | > 10 cm | Very strict |

---

## Include Paths Reference

| Service | Include Path |
|---------|-------------|
| HistogramService | `<clas12/framework/services/HistogramService.hpp>` |
| LoggingService | `<clas12/framework/services/LoggingService.hpp>` |
| NTupleService | `<clas12/framework/services/NTupleService.hpp>` |
| ConditionsService | `<clas12/framework/services/ConditionsService.hpp>` |
| HipoFileService | `<clas12/framework/services/HipoFileService.hpp>` |
| PhotonCutsService | `<clas12/framework/services/PhotonCutsService.hpp>` |
| ParticleSelectorService | `<clas12/framework/services/ParticleSelectorService.hpp>` |
| SamplingFractionService | `<clas12/framework/services/SamplingFractionService.hpp>` |
| MetadataService | `<clas12/framework/services/MetadataService.hpp>` |
| MCTruthService | `<clas12/framework/services/MCTruthService.hpp>` |
| FiducialService | `<clas12/framework/services/FiducialService.hpp>` |
| GoodRunListService | `<clas12/framework/services/GoodRunListService.hpp>` |
| EventMixingService | `<clas12/framework/services/EventMixingService.hpp>` |
| SystematicService | `<clas12/framework/services/SystematicService.hpp>` |
| MVAService | `<clas12/framework/services/MVAService.hpp>` |
| WeightService | `<clas12/framework/services/WeightService.hpp>` |
| VertexFittingService | `<clas12/framework/services/VertexFittingService.hpp>` |
| DecayTreeFitterService | `<clas12/framework/services/DecayTreeFitterService.hpp>` |
| PIDService | `<clas12/framework/services/PIDService.hpp>` |
| sPlotService | `<clas12/framework/services/sPlotService.hpp>` |
| UnfoldingService | `<clas12/framework/services/UnfoldingService.hpp>` |
| CrossSectionService | `<clas12/framework/services/CrossSectionService.hpp>` |
| TrackQualityService | `<clas12/framework/services/TrackQualityService.hpp>` |
