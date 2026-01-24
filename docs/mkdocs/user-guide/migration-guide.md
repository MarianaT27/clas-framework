# Migration Guide: Old API to Modern C++20/23 API

This guide helps you migrate existing analysis code to the modern C++20/23 framework features.

## Overview of Changes

The modernized framework uses `SimpleAlgorithm<T>` as the **unified base class** for all algorithms. Legacy specialized classes have been consolidated:

| Old API | Modern API | Notes |
|---------|------------|-------|
| `Algorithm` | `SimpleAlgorithm<T>` | Unified base class with CRTP |
| `FilterAlgorithm` | `SimpleAlgorithm<T>` | Use `StatusCode::FILTERED()` + `event.set_decision()` |
| `ProducerAlgorithm` | `SimpleAlgorithm<T>` | Use `event.store()` to store products |
| `AnalysisAlgorithm` | `SimpleAlgorithm<T>` | Direct replacement |
| `MonitoringAlgorithm` | `SimpleAlgorithm<T>` | Direct replacement |
| `service<T>()` | `require_service<T>()` | Service injection in `configure()` |
| Manual particle loops | Range adaptors | Composable, lazy filtering |
| Manual cut tracking | `CutBuilder` | Fluent API, auto cutflow |
| `std::any` event store | `TypedEventStore` | Compile-time type safety |
| Manual property handling | `Property<T>` | Declarative configuration |

## Method Name Changes

| Old Method | New Method | Called When |
|------------|------------|-------------|
| `initialize()` | `configure()` + `on_initialize()` | `configure()` for services, `on_initialize()` for setup |
| `execute(event)` | `process(event)` | For each event |
| `finalize()` | `on_finalize()` | At end of processing |
| `filter(event) -> bool` | `process(event) -> StatusCode` | Use `FILTERED()` return |
| `produce(event)` | `process(event)` | Direct replacement |
| `monitor(event)` | `process(event)` | Direct replacement |

## Step-by-Step Migration

### 1. Algorithm Base Class

**Before:**
```cpp
class MyAnalysis : public Framework::Algorithm {
public:
    MyAnalysis() : Algorithm("MyAnalysis") {}
    
    auto initialize() -> StatusCode override {
        m_hist_svc = service<HistogramService>("HistogramService");
        if (!m_hist_svc) return StatusCode::FAILURE();
        // ... setup
        return StatusCode::SUCCESS();
    }
    
    auto execute(Event& event) -> StatusCode override {
        // ... processing
    }
    
    auto finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }
    
private:
    HistogramService* m_hist_svc = nullptr;
};
```

**After:**
```cpp
class MyAnalysis : public SimpleAlgorithm<MyAnalysis> {
public:
    MyAnalysis() : SimpleAlgorithm("MyAnalysis") {}

    auto configure() -> void override {
        // Services are automatically injected
        m_hist_svc = &require_service<HistogramService>("HistogramService");
    }

    auto on_initialize() -> StatusCode override {
        // Custom initialization (services already available)
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        // ... processing (replaces execute())
    }

    auto on_finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

private:
    ServiceHandle<HistogramService>* m_hist_svc = nullptr;
};
```

### 1b. Migrating FilterAlgorithm

**Before:**
```cpp
class MyFilter : public FilterAlgorithm {
public:
    MyFilter() : FilterAlgorithm("MyFilter") {}

    auto filter(Event& event) -> bool override {
        // Return true to pass, false to filter
        return has_electron;
    }
};
```

**After:**
```cpp
class MyFilter : public SimpleAlgorithm<MyFilter> {
public:
    MyFilter() : SimpleAlgorithm("MyFilter") {}

    auto process(Event& event) -> StatusCode override {
        if (!has_electron) {
            event.set_decision(name(), false);
            return StatusCode::FILTERED();
        }

        event.set_decision(name(), true);
        return StatusCode::SUCCESS();
    }
};
```

### 1c. Migrating ProducerAlgorithm

**Before:**
```cpp
class MyProducer : public ProducerAlgorithm {
public:
    MyProducer() : ProducerAlgorithm("MyProducer") {}

    auto produce(Event& event) -> StatusCode override {
        // Create and store products
        event.store("products", my_products);
        return StatusCode::SUCCESS();
    }
};
```

**After:**
```cpp
class MyProducer : public SimpleAlgorithm<MyProducer> {
public:
    MyProducer() : SimpleAlgorithm("MyProducer") {}

    auto process(Event& event) -> StatusCode override {
        // Create and store products (same pattern)
        event.store("products", my_products);
        return StatusCode::SUCCESS();
    }
};
```

### 1d. Migrating MonitoringAlgorithm

**Before:**
```cpp
class MyMonitor : public MonitoringAlgorithm {
public:
    MyMonitor() : MonitoringAlgorithm("MyMonitor") {}

    auto monitor(Event& event) -> StatusCode override {
        // Monitor events
        return StatusCode::SUCCESS();
    }
};
```

**After:**
```cpp
class MyMonitor : public SimpleAlgorithm<MyMonitor> {
public:
    MyMonitor() : SimpleAlgorithm("MyMonitor") {}

    auto configure() -> void override {
        m_hist_svc = &require_service<HistogramService>("HistogramService");
    }

    auto process(Event& event) -> StatusCode override {
        // Monitor events (same logic)
        return StatusCode::SUCCESS();
    }

private:
    ServiceHandle<HistogramService>* m_hist_svc = nullptr;
};
```

### 2. Particle Filtering

**Before:**
```cpp
auto execute(Event& event) -> StatusCode override {
    const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
    
    // Manual filtering for FD electrons
    std::vector<CLAS::Particle> fd_electrons;
    for (const auto& e : electrons) {
        int abs_status = std::abs(e.status());
        if (abs_status >= 2000 && abs_status < 4000) {
            if (e.p() > 1.0 && e.p() < 10.0) {
                fd_electrons.push_back(e);
            }
        }
    }
    // ...
}
```

**After:**
```cpp
auto process(Event& event) -> StatusCode override {
    // Composable range adaptors - lazy evaluation
    auto fd_electrons = event.all_particles()
        | Ranges::by_pid(CLAS::PIDs::ELECTRON)
        | Ranges::in_detector(CLAS::FD)
        | Ranges::by_momentum(1.0, 10.0);
    
    // Collect if needed
    std::vector<CLAS::Particle> electrons;
    for (const auto& e : fd_electrons) {
        electrons.push_back(e);
    }
    // ...
}
```

### 3. Cut Application

**Before:**
```cpp
auto execute(Event& event) -> StatusCode override {
    // Manual cut application with cutflow tracking
    if (electron.p() < 1.0 || electron.p() > 10.0) {
        return StatusCode::SUCCESS();
    }
    m_cutflow.pass("Momentum Cut");
    
    if (electron.theta() < 5.0 || electron.theta() > 35.0) {
        return StatusCode::SUCCESS();
    }
    m_cutflow.pass("Theta Cut");
    
    if (electron.vz() < -12.0 || electron.vz() > 10.0) {
        return StatusCode::SUCCESS();
    }
    m_cutflow.pass("Vertex Z Cut");
    // ...
}
```

**After:**
```cpp
auto process(Event& event) -> StatusCode override {
    // Fluent CutBuilder with automatic cutflow tracking
    bool passed = CutBuilder(m_cutflow)
        .require_in_range("Momentum Cut", electron.p(), 1.0, 10.0)
        .require_in_range("Theta Cut", electron.theta(), 5.0, 35.0)
        .require_in_range("Vertex Z Cut", electron.vz(), -12.0, 10.0)
        .evaluate();
    
    if (!passed) {
        return StatusCode::FILTERED();
    }
    // ...
}
```

### 4. Inter-Algorithm Communication

**Before:**
```cpp
// Algorithm 1: Store data
event.store("trigger_electron", trigger_electron);
event.store("has_good_electron", true);

// Algorithm 2: Retrieve data (no type safety)
auto electron_opt = event.retrieve<CLAS::Particle>("trigger_electron");
auto has_electron = event.retrieve<bool>("has_good_electron");
```

**After:**
```cpp
// Algorithm 1: Store with typed keys
TypedEventStore store;
store.store(Keys::TriggerElectron, trigger_electron);
store.store(Keys::HasGoodElectron, true);
event.store("electron_store", store);

// Algorithm 2: Retrieve with compile-time type safety
auto store_opt = event.retrieve<TypedEventStore>("electron_store");
if (store_opt) {
    auto electron = store_opt->retrieve(Keys::TriggerElectron);  // Returns std::optional<CLAS::Particle>
    auto has_electron = store_opt->retrieve(Keys::HasGoodElectron);  // Returns std::optional<bool>
}
```

### 5. Configuration Properties

**Before:**
```cpp
class MyAnalysis : public Algorithm {
public:
    MyAnalysis(double min_p, double max_p) 
        : Algorithm("MyAnalysis")
        , m_min_p(min_p)
        , m_max_p(max_p) {}
    
private:
    double m_min_p;
    double m_max_p;
};

// Usage
auto alg = std::make_unique<MyAnalysis>(1.0, 10.0);
```

**After:**
```cpp
class MyAnalysis : public SimpleAlgorithm<MyAnalysis> {
public:
    MyAnalysis() : SimpleAlgorithm("MyAnalysis") {
        // Declare properties with defaults and descriptions
        declare_property(m_min_p);
        declare_property(m_max_p);
    }
    
private:
    Property<double> m_min_p{"min_momentum", 1.0, "Minimum momentum [GeV]"};
    Property<double> m_max_p{"max_momentum", 10.0, "Maximum momentum [GeV]"};
};

// Usage - properties can be configured from TOML
auto alg = std::make_unique<MyAnalysis>();
// Properties are automatically loaded from config file
```

## Available Range Adaptors

| Adaptor | Description | Example |
|---------|-------------|---------|
| `by_pid(pid)` | Filter by particle ID | `Ranges::by_pid(CLAS::PIDs::ELECTRON)` |
| `by_charge(q)` | Filter by charge | `Ranges::by_charge(-1)` |
| `by_momentum(min, max)` | Filter by momentum range | `Ranges::by_momentum(1.0, 10.0)` |
| `by_min_momentum(min)` | Filter by minimum momentum | `Ranges::by_min_momentum(2.0)` |
| `by_theta(min, max)` | Filter by theta range | `Ranges::by_theta(5.0, 35.0)` |
| `in_detector(det)` | Filter by detector | `Ranges::in_detector(CLAS::FD)` |
| `by_vertex_z(min, max)` | Filter by vertex z | `Ranges::by_vertex_z(-12.0, 10.0)` |
| `valid_only` | Filter out invalid particles | `Ranges::valid_only` |
| `to_lorentz_vector` | Transform to 4-momentum | `Ranges::to_lorentz_vector` |

## Pre-defined TypedEventStore Keys

```cpp
namespace Keys {
    // Particle collections
    TypedKey<std::vector<CLAS::Particle>> Electrons{"electrons"};
    TypedKey<std::vector<CLAS::Particle>> Photons{"photons"};
    TypedKey<std::vector<CLAS::Particle>> Protons{"protons"};
    
    // Kinematic variables
    TypedKey<double> Q2{"Q2"};
    TypedKey<double> W{"W"};
    TypedKey<double> Xbj{"x_bjorken"};
    TypedKey<double> Y{"y"};
    
    // Event flags
    TypedKey<bool> HasGoodElectron{"has_good_electron"};
    TypedKey<bool> IsDIS{"is_dis"};
    
    // Single particles
    TypedKey<CLAS::Particle> TriggerElectron{"trigger_electron"};
}
```

## CutBuilder Methods

| Method | Description | Example |
|--------|-------------|---------|
| `cut(name, predicate)` | Add custom cut | `.cut("Custom", [&]{ return x > 0; })` |
| `require_gt(name, val, threshold)` | Value > threshold | `.require_gt("p > 1", p, 1.0)` |
| `require_gte(name, val, threshold)` | Value >= threshold | `.require_gte("p >= 1", p, 1.0)` |
| `require_lt(name, val, threshold)` | Value < threshold | `.require_lt("p < 10", p, 10.0)` |
| `require_lte(name, val, threshold)` | Value <= threshold | `.require_lte("p <= 10", p, 10.0)` |
| `require_in_range(name, val, min, max)` | min <= value <= max | `.require_in_range("p", p, 1.0, 10.0)` |
| `require_eq(name, val, expected)` | Value == expected | `.require_eq("charge", q, -1)` |
| `evaluate()` | Run all cuts, return bool | `.evaluate()` |
| `apply()` | Run all cuts, return StatusCode | `.apply()` |

## Migration Checklist

When migrating an algorithm to `SimpleAlgorithm<T>`:

- [ ] Change base class from `Algorithm`/`FilterAlgorithm`/etc. to `SimpleAlgorithm<YourClass>`
- [ ] Move service retrieval from `initialize()` to `configure()` using `require_service<T>()`
- [ ] Change service member types from `T*` to `ServiceHandle<T>*`
- [ ] Rename `initialize()` to `on_initialize()`
- [ ] Rename `execute()`/`filter()`/`produce()`/`monitor()` to `process()`
- [ ] Rename `finalize()` to `on_finalize()`
- [ ] Update service access from `m_svc->method()` to `m_svc->get()->method()`
- [ ] For filters: return `StatusCode::FILTERED()` instead of `false`, add `event.set_decision()`

## Using Modern Features

All modern features work with `SimpleAlgorithm<T>`:

```cpp
class ModernAnalysis : public SimpleAlgorithm<ModernAnalysis> {
    auto configure() -> void override {
        m_particle_svc = &require_service<ParticleSelectorService>("ParticleSelectorService");
    }

    auto process(Event& event) -> StatusCode override {
        // Use range adaptors
        auto electrons = event.all_particles()
            | Ranges::by_pid(CLAS::PIDs::ELECTRON)
            | Ranges::in_detector(CLAS::FD);

        // Use CutBuilder
        bool passed = CutBuilder(m_cutflow)
            .require_gt("Momentum", p, 1.0)
            .evaluate();

        return passed ? StatusCode::SUCCESS() : StatusCode::FILTERED();
    }

private:
    ServiceHandle<ParticleSelectorService>* m_particle_svc = nullptr;
};
```
