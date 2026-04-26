# Algorithm

`Algorithm` is the single base class for everything that processes events
(formerly `IAlgorithm` / `Algorithm` / `SimpleAlgorithm` — merged in 2026).

```cpp
namespace clas12::framework {

class Algorithm : public PropertyMixin {
public:
    explicit Algorithm(std::string name);
    virtual ~Algorithm() = default;

    // User-facing hooks — override in derived classes.
    virtual auto configure()                      -> void {}
    virtual auto on_initialize()                  -> StatusCode { return StatusCode::SUCCESS(); }
    virtual auto process(Event& event)            -> StatusCode = 0;
    virtual auto on_finalize()                    -> StatusCode { return StatusCode::SUCCESS(); }

    // Framework-driven lifecycle (do not override).
    auto initialize()                             -> StatusCode;
    auto execute(Event& event)                    -> StatusCode;   // calls process()
    auto finalize()                               -> StatusCode;

    // Public API used by Application / ApplicationBuilder.
    [[nodiscard]] auto name()  const              -> const std::string&;
    [[nodiscard]] auto state() const              -> AlgorithmState;
    auto set_service_manager(ServiceManager*)     -> void;
    [[nodiscard]] auto required_banks() const     -> std::vector<std::string>;
    [[nodiscard]] auto optional_banks() const     -> std::vector<std::string>;

protected:
    // Helpers for derived classes (selected). Full API in the header.
    template <ServiceConcept T>
    auto svc()                                    -> T*;            // null if not registered

    template <ServiceConcept T>
    void require_service(ServiceRef<T>& member);                    // call in constructor

    template <typename... Args>
    void required_services(Args&... member_refs);                   // variadic

    void declare_bank(std::string_view bank_name);
    void declare_optional_bank(std::string_view bank_name);

    template <typename T>
    auto declare_property(Property<T>& property)  -> Property<T>&;  // PropertyMixin
};

}
```

## Lifecycle

1. **Construction** — algorithm is created with a name. Bank
   declarations and `Property<T>` registration happen here.
2. **`configure()`** — optional. Runs once at the start of
   `initialize()` if you need late-bound declarations.
3. **TOML application** — the framework looks up
   `[<algorithm.name()>]` in the analysis's TOML file and applies
   matching keys to declared `Property<T>` members.
4. **`on_initialize()`** — called once after services are injected
   and properties are populated. Book histograms, set up state.
5. **`process(event)`** — called per event, concurrently from N
   worker threads.
6. **`on_finalize()`** — called once after the event loop joins.

## Example

```cpp
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>

class MyAlgorithm : public clas12::framework::Algorithm {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : Algorithm(std::move(name)) {
        required_services(m_hist, m_particles);
        declare_bank("REC::Particle");
        declare_property(m_min_p);
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        h_momentum = m_hist->book_hist1D("/MyAlg/p", "p;p [GeV];Events", 100, 0, 10);
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        auto& rec = event.bank<clas12::bankdefs::REC_Particle>();
        auto particles = m_particles->extract_particles(rec);
        for (const auto& e : particles.electrons()) {
            if (e.p() > m_min_p.get()) h_momentum->fill(e.p());
        }
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::ServiceRef<clas12::framework::HistogramService>          m_hist;
    clas12::framework::ServiceRef<clas12::framework::ParticleSelectorService>   m_particles;
    clas12::framework::Property<double>                                         m_min_p{"min_p", 1.0, "GeV"};
    clas12::framework::ThreadLocalHistogram1D*                                  h_momentum = nullptr;
};
```

The `[MyAlgorithm]` section in the analysis TOML drives the
property:

```toml
[MyAlgorithm]
min_p = 2.0
```

## Service access

| Method | Description |
|--------|-------------|
| `svc<T>()` | Look up a service by its static `service_name()`. Returns `nullptr` if not registered. |
| `require_service(ref)` | Declare a required dependency in the constructor; the framework injects it before `on_initialize()` and fails fast if the service is missing. |
| `required_services(refs...)` | Variadic form of `require_service`. |

## StatusCode returns

| Status | Usage |
|--------|-------|
| `StatusCode::SUCCESS()` | Event processed successfully — continue with the rest of the algorithm chain. |
| `StatusCode::FAILURE()` | Fatal error — stop the entire run. |
| `StatusCode::FILTERED()` | Skip remaining algorithms for this event. The framework short-circuits the chain. |

## Conditional execution

There used to be `AlgorithmSequence` / `TypedConditionalSequence` /
`FilterSequence` types for conditional algorithm chains. They were
removed in favour of returning `StatusCode::FILTERED()` from
`process()` — the framework already short-circuits the rest of the
chain for that event when an upstream algorithm filters it out.

```cpp
auto process(Event& event) -> StatusCode override {
    if (!event.passes_decision_typed(Decisions::HasTriggerElectron))
        return StatusCode::FILTERED();
    // ...real work...
    return StatusCode::SUCCESS();
}
```

## Thread safety

`process()` is called concurrently from N worker threads on the
**same** algorithm instance. Mutable state must be one of:

1. **Atomic** (`std::atomic<T>`, `Cutflow` counters).
2. **Thread-local** (`ThreadLocalHistogram` and friends, which the
   framework merges at finalize).
3. **Mutex-guarded**.
4. **Append-only after `initialize()`** (configuration, `ServiceRef`).

`initialize()` and `finalize()` run serially, so anything set up
there can be read freely from `process()`.
