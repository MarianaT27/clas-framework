# ServiceHandle

Type-safe service access with validation at initialization.

## Header

```cpp
#include <clas12/framework/core/ServiceHandle.hpp>
```

## Class Definition

```cpp
namespace clas12::framework {

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

}  // namespace clas12::framework
```

## Usage

### Declaration

Declare service handles as class members:

```cpp
class MyAlgorithm : public clas12::framework::Algorithm {
private:
    clas12::framework::ServiceHandle<clas12::framework::HistogramService> m_hist_svc{"HistogramService"};
    clas12::framework::ServiceHandle<clas12::framework::LoggingService> m_log_svc{"LoggingService"};
    clas12::framework::ServiceHandle<clas12::framework::NTupleService> m_ntuple_svc{"NTupleService"};
};
```

### Initialization

Initialize handles in the `initialize()` method:

```cpp
auto initialize() -> clas12::framework::StatusCode override {
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

    return clas12::framework::StatusCode::SUCCESS();
}
```

### Access

Use the handle like a pointer:

```cpp
auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
    // Check if valid before use - returns AtomicHistogram1D* (thread-safe)
    if (m_hist_svc) {
        m_hist_svc->book_hist1D("/Analysis/p", "Momentum", 100, 0, 10);
    }

    // Or use get()
    if (auto* svc = m_hist_svc.get(); svc != nullptr) {
        svc->book_hist1D("/Analysis/theta", "Theta", 180, 0, 180);
    }

    return clas12::framework::StatusCode::SUCCESS();
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
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/ServiceHandle.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>

class MyAlgorithm : public clas12::framework::Algorithm {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : Algorithm(std::move(name)) {}

    auto initialize() -> clas12::framework::StatusCode override {
        // Initialize required services
        if (auto sc = initialize_handle(m_hist_svc); sc.is_failure()) {
            clas12::framework::Log::error(name(), "HistogramService required but not available");
            return sc;
        }

        // Initialize optional services
        initialize_handle(m_log_svc);
        initialize_handle(m_ntuple_svc);

        // Book histograms - returns AtomicHistogram1D* (thread-safe)
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto execute(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Use services
        if (m_hist_svc && h_momentum) {
            h_momentum->fill(electron.p());  // Thread-safe atomic fill
        }

        if (m_ntuple_svc) {
            m_momentum = electron.p();
            m_ntuple_svc->fill("my_data");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto finalize() -> clas12::framework::StatusCode override {
        if (m_log_svc) {
            clas12::framework::Log::info(name(), "Finalized successfully");
        }
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    // Service handles
    clas12::framework::ServiceHandle<clas12::framework::HistogramService> m_hist_svc{"HistogramService"};
    clas12::framework::ServiceHandle<clas12::framework::LoggingService> m_log_svc{"LoggingService"};
    clas12::framework::ServiceHandle<clas12::framework::NTupleService> m_ntuple_svc{"NTupleService"};

    // Atomic histograms are thread-safe for parallel processing
    clas12::framework::AtomicHistogram1D* h_momentum = nullptr;

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
