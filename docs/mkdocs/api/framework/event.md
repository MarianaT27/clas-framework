# Event

Event Data Model for CLAS12 analysis.

## Class Definition

```cpp
namespace Framework {

class Event {
public:
    explicit Event(hipo::iter_event& hipo_event);
    
    // Particle access
    auto particles() -> const ParticleMap&;
    auto particles(CLAS::PIDs pid) -> const std::vector<CLAS::Particle>&;
    
    // Bank access
    auto get_bank(const std::string& bank_name) -> hipo::bank_map;
    auto hipo_event() -> hipo::iter_event&;
    
    // Event info
    auto start_time() -> double;
    auto event_number() const -> long;
    auto file_index() const -> int;
    
    // Event store
    template<typename T>
    auto store(const std::string& key, T value) -> void;
    
    template<typename T>
    auto retrieve(const std::string& key) const -> std::optional<T>;
    
    auto contains(const std::string& key) const -> bool;
    
    // Algorithm decisions
    auto set_decision(const std::string& name, bool passed) -> void;
    auto passes_decision(const std::string& name) -> bool;
};

}
```

## Particle Access

```cpp
// Get all particles of a type
const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
const auto& photons = event.particles(CLAS::PIDs::PHOTON);

// Get all particles (map of PID -> vector)
const auto& all = event.particles();
```

## Event Store

Inter-algorithm communication:

```cpp
// Store data
event.store("selected_electron", electrons[0]);
event.store("q2", 2.5);

// Retrieve data
auto e = event.retrieve<CLAS::Particle>("selected_electron");
auto q2 = event.retrieve<double>("q2");

// Check existence
if (event.contains("selected_electron")) { ... }
```

## Algorithm Decisions

```cpp
// Set decision
event.set_decision("HasGoodElectron", true);

// Check decision
if (event.passes_decision("HasGoodElectron")) {
    // Process event
}
```

## Bank Access

```cpp
auto rec_particle = event.get_bank("REC::Particle");
auto rec_calorimeter = event.get_bank("REC::Calorimeter");
```
