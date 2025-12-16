# ServiceHandle

Type-safe service access with validation at initialization.

## Header

```cpp
#include "Framework/ServiceHandle.hpp"
```

## Class Definition

```cpp
namespace Framework {

template <typename ServiceT>
class ServiceHandle {
public:
    explicit ServiceHandle(std::string service_name);

    // Initialize the handle from a ServiceManager
    auto initialize(ServiceManager& mgr) -> StatusCode;

    // Check if handle is valid
    [[nodiscard]] auto is_valid() const -> bool;
    explicit operator bool() const;

    // Access the service
    auto operator->() -> ServiceT*;
    auto operator->() const -> const ServiceT*;
    auto get() -> ServiceT*;
    auto get() const -> const ServiceT*;
};

}  // namespace Framework
```

## Usage

### Declaration

Declare service handles as class members:

```cpp
class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};
};
```

### Initialization

Initialize handles in the `initialize()` method:

```cpp
auto initialize() -> Framework::StatusCode override {
    // Initialize required service
    auto hist_sc = initialize_handle(m_hist_svc);
    if (hist_sc.is_failure()) {
        return hist_sc;  // Required service not available
    }

    // Initialize optional service (don't fail if not available)
    auto log_sc = initialize_handle(m_log_svc);
    if (log_sc.is_failure()) {
        fmt::print(stderr, "WARNING: LoggingService not available\n");
    }

    return Framework::StatusCode::SUCCESS();
}
```

### Access

Use the handle like a pointer:

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Check if valid before use
    if (m_hist_svc) {
        m_hist_svc->book_hist1D("/Analysis/p", "Momentum", 100, 0, 10);
    }

    // Or use get()
    if (auto* svc = m_hist_svc.get(); svc != nullptr) {
        svc->book_hist1D("/Analysis/theta", "Theta", 180, 0, 180);
    }

    return Framework::StatusCode::SUCCESS();
}
```

## Methods

### Constructor

```cpp
explicit ServiceHandle(std::string service_name);
```

Create a handle for the named service.

**Parameters:**

- `service_name` - Name of the service to retrieve

### `initialize`

```cpp
auto initialize(ServiceManager& mgr) -> StatusCode;
```

Initialize the handle from a ServiceManager.

**Parameters:**

- `mgr` - The ServiceManager to retrieve the service from

**Returns:** `StatusCode::SUCCESS()` if service found, `StatusCode::FAILURE()` otherwise

### `is_valid`

```cpp
[[nodiscard]] auto is_valid() const -> bool;
```

Check if the handle points to a valid service.

### `operator bool`

```cpp
explicit operator bool() const;
```

Allows using the handle in boolean context.

### `operator->`

```cpp
auto operator->() -> ServiceT*;
auto operator->() const -> const ServiceT*;
```

Access service members directly.

### `get`

```cpp
auto get() -> ServiceT*;
auto get() const -> const ServiceT*;
```

Get the underlying service pointer.

## Helper Function

### `initialize_handle`

In algorithms, use the `initialize_handle` helper:

```cpp
template <typename T>
auto initialize_handle(ServiceHandle<T>& handle) -> StatusCode {
    if (m_svc_mgr) {
        return handle.initialize(*m_svc_mgr);
    }
    return StatusCode::FAILURE();
}
```

## Complete Example

```cpp
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"

class MyAlgorithm : public Framework::Algorithm {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : Algorithm(std::move(name)) {}

    auto initialize() -> Framework::StatusCode override {
        // Initialize required services
        if (auto sc = initialize_handle(m_hist_svc); sc.is_failure()) {
            Framework::Log::error(name(), "HistogramService required but not available");
            return sc;
        }

        // Initialize optional services
        initialize_handle(m_log_svc);
        initialize_handle(m_ntuple_svc);

        // Book histograms
        if (m_hist_svc) {
            h_momentum = m_hist_svc->book_hist1D(
                "/MyAlgorithm/momentum",
                "Momentum;p [GeV/c];Events",
                100, 0, 10);
        }

        // Setup N-tuple (optional)
        if (m_ntuple_svc) {
            m_ntuple_svc->register_branch("my_data", "momentum", &m_momentum);
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // Use services
        if (m_hist_svc && h_momentum) {
            h_momentum->Fill(electron.p());
        }

        if (m_ntuple_svc) {
            m_momentum = electron.p();
            m_ntuple_svc->fill("my_data");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        if (m_log_svc) {
            Framework::Log::info(name(), "Finalized successfully");
        }
        return Framework::StatusCode::SUCCESS();
    }

private:
    // Service handles
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};

    // Histograms
    TH1D* h_momentum = nullptr;

    // N-tuple variables
    double m_momentum = 0.0;
};
```

## Benefits

!!! success "Type Safety"
    Template-based handles ensure type safety at compile time.

!!! success "Validation"
    Explicit initialization allows checking service availability.

!!! success "Clean Syntax"
    Pointer-like access with `->` operator.

!!! success "Optional Services"
    Easy to handle optional services that may not be available.
