# Algorithm

Base interface and class for all algorithms.

## IAlgorithm Interface

```cpp
namespace Framework {

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

## Algorithm Base Class

```cpp
namespace Framework {

class Algorithm : public IAlgorithm {
public:
    explicit Algorithm(std::string name);
    
    auto name() const -> const std::string& override;
    auto set_service_manager(ServiceManager* svc_mgr) -> void override;

protected:
    template<typename T>
    auto service(const std::string& service_name) -> T*;
    
    template<typename T>
    auto initialize_handle(ServiceHandle<T>& handle) -> StatusCode;
};

}
```

## Lifecycle

1. **Construction** - Algorithm is created
2. **initialize()** - Called once before event processing
3. **execute(event)** - Called for each event
4. **finalize()** - Called once after all events

## Example Implementation

```cpp
class MyAlgorithm : public Framework::Algorithm {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : Algorithm(std::move(name)) {}

    auto initialize() -> Framework::StatusCode override {
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        if (!m_hist_svc) {
            return Framework::StatusCode::FAILURE();
        }
        
        h_momentum = m_hist_svc->book_hist1D("/MyAlg/p", "p;p [GeV];Events", 100, 0, 10);
        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        for (const auto& e : electrons) {
            h_momentum->Fill(e.p());
        }
        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        Framework::Log::info(name(), "Processed events");
        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::HistogramService* m_hist_svc = nullptr;
    TH1D* h_momentum = nullptr;
};
```
