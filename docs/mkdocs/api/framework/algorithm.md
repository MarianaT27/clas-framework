# Algorithm

Base interface and classes for all algorithms.

## SimpleAlgorithm (Recommended)

The `SimpleAlgorithm<T>` CRTP base class is the unified algorithm type for all analyses:

```cpp
namespace clas12::framework {

template<typename Derived>
class SimpleAlgorithm : public Algorithm {
public:
    explicit SimpleAlgorithm(std::string name);

    // Override these in your derived class
    virtual auto configure() -> void {}
    virtual auto on_initialize() -> StatusCode { return StatusCode::SUCCESS(); }
    virtual auto process(Event& event) -> StatusCode = 0;
    virtual auto on_finalize() -> StatusCode { return StatusCode::SUCCESS(); }

protected:
    // Service injection helpers
    template<typename T>
    auto require_service(const std::string& name) -> ServiceHandle<T>&;

    template<typename T>
    auto optional_service(const std::string& name) -> ServiceHandle<T>&;
};

}
```

## Lifecycle

1. **Construction** - Algorithm is created with a name
2. **configure()** - Request services using `require_service<T>()` or `optional_service<T>()`
3. **on_initialize()** - Book histograms, setup resources (services are available)
4. **process(event)** - Called for each event
5. **on_finalize()** - Print statistics, cleanup

## Example Implementation

```cpp
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>

class MyAlgorithm : public clas12::framework::SimpleAlgorithm<MyAlgorithm> {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : SimpleAlgorithm(std::move(name)) {}

    auto configure() -> void override {
        // Required service - throws if not available
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        // Book histograms - returns AtomicHistogram1D* (thread-safe)
        h_momentum = m_hist_svc->get()->book_hist1D("/MyAlg/p", "p;p [GeV];Events", 100, 0, 10);
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        auto& hipo_event = event.hipo_event();
        auto& REC_Particle = hipo_event["REC::Particle"];

        auto particles = m_particle_svc->get()->extract_particles(
            REC_Particle, {clas12::PIDs::ELECTRON});

        for (const auto& e : m_particle_svc->get()->get_electrons(particles)) {
            h_momentum->fill(e.p());  // Thread-safe atomic fill
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        clas12::framework::Log::info(name(), "Processed events");
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
    // Atomic histograms are thread-safe for parallel processing
    clas12::framework::AtomicHistogram1D* h_momentum = nullptr;
};
```

## Service Injection

| Method | Description |
|--------|-------------|
| `require_service<T>(name)` | Request a required service. Throws if unavailable. |
| `optional_service<T>(name)` | Request an optional service. Returns invalid handle if unavailable. |
| `m_svc->is_valid()` | Check if service handle points to a valid service |
| `m_svc->get()` | Access the underlying service pointer |

## StatusCode Returns

| Status | Usage |
|--------|-------|
| `StatusCode::SUCCESS()` | Event processed successfully |
| `StatusCode::FAILURE()` | Fatal error, stop processing |
| `StatusCode::FILTERED()` | Event filtered out (continue to next event) |

## IAlgorithm Interface

The underlying interface that all algorithms implement:

```cpp
namespace clas12::framework {

class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;

    virtual auto initialize() -> StatusCode = 0;
    virtual auto execute(Event& event) -> StatusCode = 0;
    virtual auto finalize() -> StatusCode = 0;
    virtual auto name() const -> const std::string& = 0;
    virtual auto set_service_manager(ServiceManager* svc_mgr) -> void = 0;
};

}
```

## Algorithm Sequences

For grouping algorithms:

| Class | Description |
|-------|-------------|
| `AlgorithmSequence` | Groups algorithms to execute together |
| `ConditionalSequence` | Runs sequence only if a decision passes |
| `FilterSequence` | Stops on first `FILTERED` status |

```cpp
// Create a conditional sequence
auto electron_seq = std::make_unique<clas12::framework::ConditionalSequence>(
    "ElectronSequence", "HasTriggerElectron");

electron_seq->add_algorithm(std::make_unique<ElectronAnalysisAlg>());
electron_seq->add_algorithm(std::make_unique<ElectronMonitor>());

app.add_algorithm(std::move(electron_seq));
```
