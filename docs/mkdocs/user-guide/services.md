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
