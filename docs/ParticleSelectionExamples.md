# Improved Particle Selection Guide

This guide demonstrates the new, easier particle selection API for CLAS12 analysis.

## Overview of Improvements

The new `ParticleSelector` class provides:
- **Automatic particle extraction** from HIPO banks into type-safe containers
- **Fluent filtering API** for common selection criteria
- **Type-safe PID checking** without verbose casting
- **Map-based particle storage** for easy multi-particle topology

## Quick Comparison: Before vs After

### OLD WAY (Verbose and Error-Prone)
```cpp
// Old approach - manual particle extraction
std::vector<Core::Particle> electrons;

for (int i = 0; i < REC_Particle.getRows(); i++) {
    const int pid = REC_Particle.get<int>("pid", i);

    // Verbose type checking
    if (pid == static_cast<int>(Core::PIDs::Electron)) {
        // Manually extract all properties
        const int status = REC_Particle.get<int>("status", i);
        const int charge = REC_Particle.get<int>("charge", i);
        const double px = REC_Particle.get<double>("px", i);
        const double py = REC_Particle.get<double>("py", i);
        const double pz = REC_Particle.get<double>("pz", i);
        // ... many more lines ...

        electrons.emplace_back(pid, status, i, charge, mass,
                              px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
    }
}

// Find trigger electron
auto electron = Core::find_trigger_electron(electrons, {2000, 4000});
```

### NEW WAY (Clean and Simple)
```cpp
#include <Core/ParticleSelector.hpp>

// One-line extraction of all particle types
auto particles = Core::ParticleSelector::extract_particles(REC_Particle);

// Access electrons with clean syntax
auto& electrons = particles[Core::PIDs::Electron];

// Find trigger electron (same as before)
auto electron = Core::find_trigger_electron(electrons, {2000, 4000});
```

## Basic Usage Examples

### 1. Extract Specific Particle Types

```cpp
#include <Core/ParticleSelector.hpp>

// Extract only electrons and protons
auto particles = Core::ParticleSelector::extract_particles(
    REC_Particle,
    {Core::PIDs::Electron, Core::PIDs::Proton}
);

// Access each particle type
auto& electrons = particles[Core::PIDs::Electron];
auto& protons = particles[Core::PIDs::Proton];

std::cout << "Found " << electrons.size() << " electrons\n";
std::cout << "Found " << protons.size() << " protons\n";
```

### 2. Extract All Known Particle Types

```cpp
// Extract all particle types (Electron, Positron, Proton, Neutron, Pions, Kaons)
auto particles = Core::ParticleSelector::extract_particles(REC_Particle);

// Iterate over all particle types
for (const auto& [pid_type, particle_vec] : particles) {
    std::cout << Core::pid_name(pid_type) << ": "
              << particle_vec.size() << " found\n";
}
```

### 3. Apply Filters to Particle Selection

```cpp
auto particles = Core::ParticleSelector::extract_particles(REC_Particle);
auto& electrons = particles[Core::PIDs::Electron];

// Filter by momentum range (2.0 to 10.0 GeV/c)
auto high_p_electrons = Core::ParticleSelector::with_momentum_range(
    electrons, 2.0, 10.0
);

// Filter by vertex z-position (-10 to +10 cm)
auto good_vz_electrons = Core::ParticleSelector::with_vz_range(
    electrons, -10.0, 10.0
);

// Filter by chi2pid quality
auto good_pid_electrons = Core::ParticleSelector::with_chi2pid_range(
    electrons, -3.0, 3.0
);

// Get only trigger electrons
auto trigger_electrons = Core::ParticleSelector::get_trigger_particles(
    electrons, {2000, 4000}
);
```

### 4. Chain Multiple Filters

```cpp
// Start with all electrons
auto& electrons = particles[Core::PIDs::Electron];

// Apply multiple filters in sequence
auto filtered = electrons;
filtered = Core::ParticleSelector::with_momentum_range(filtered, 1.0, 10.0);
filtered = Core::ParticleSelector::with_vz_range(filtered, -10.0, 10.0);
filtered = Core::ParticleSelector::with_charge(filtered, -1);  // Only negative

std::cout << "After filtering: " << filtered.size() << " electrons remain\n";
```

### 5. Custom Filters with Lambda Functions

```cpp
// Create custom filter using lambda
auto high_energy_forward = Core::ParticleSelector::filter(
    electrons,
    [](const Core::Particle& p) {
        return p.E() > 5.0 &&           // Energy > 5 GeV
               p.theta() < 30.0 &&      // Forward angle
               p.p() > 2.0;             // Momentum > 2 GeV/c
    }
);

// Complex custom filter
auto custom_selection = Core::ParticleSelector::filter(
    electrons,
    [](const Core::Particle& p) {
        double sampling_fraction = p.E() / p.p();
        return sampling_fraction > 0.2 &&
               sampling_fraction < 0.3 &&
               p.chi2pid() < 3.0;
    }
);
```

### 6. Simplified PID Checking

```cpp
// Old way (verbose)
if (pid == static_cast<int>(Core::PIDs::Electron)) { /* ... */ }

// New way 1: Using matches() function
if (Core::matches(pid, Core::PIDs::Electron)) { /* ... */ }

// New way 2: Using ParticleSelector
if (Core::ParticleSelector::is_type(pid, Core::PIDs::Electron)) { /* ... */ }

// Get PID name for logging
std::cout << "Found " << Core::pid_name(Core::PIDs::Electron) << "\n";
```

## Complete Analysis Example

Here's a complete example showing the improved workflow:

```cpp
#include <Core/ParticleSelector.hpp>
#include <Core/Helpers.hpp>
#include <hipo4/hipoeventiterator.h>

void analyze_electron_proton_events(const std::string& input_file) {

    for (hipo::hipoeventfile events(input_file); auto& event : events) {

        hipo::bank& REC_Particle = event.get_bank("REC::Particle");
        if (REC_Particle.getRows() == 0) continue;

        // Extract particles in one line
        auto particles = Core::ParticleSelector::extract_particles(
            REC_Particle,
            {Core::PIDs::Electron, Core::PIDs::Proton}
        );

        // Get electrons and protons
        auto& electrons = particles[Core::PIDs::Electron];
        auto& protons = particles[Core::PIDs::Proton];

        // Select good quality electrons
        auto good_electrons = electrons;
        good_electrons = Core::ParticleSelector::with_momentum_range(good_electrons, 1.0, 10.0);
        good_electrons = Core::ParticleSelector::with_vz_range(good_electrons, -10.0, 10.0);
        good_electrons = Core::ParticleSelector::with_chi2pid_range(good_electrons, -3.0, 3.0);

        // Find trigger electron
        auto trigger_e = Core::find_trigger_electron(good_electrons, {2000, 4000});
        if (!trigger_e.has_value()) continue;

        // Select good quality protons
        auto good_protons = protons;
        good_protons = Core::ParticleSelector::with_momentum_range(good_protons, 0.3, 5.0);
        good_protons = Core::ParticleSelector::with_vz_range(good_protons, -10.0, 10.0);

        // Require at least one good electron and one good proton
        if (good_electrons.empty() || good_protons.empty()) continue;

        // Perform physics analysis
        const auto& electron = trigger_e.value();
        const auto& proton = good_protons[0];

        double Q2 = compute_Q2(electron);
        double W = compute_W(electron, proton);

        std::cout << "Event: Q2=" << Q2 << ", W=" << W << "\n";
    }
}
```

## Using in Your Reader Class

Update your Reader class to use the new API:

```cpp
// In Reader.hpp
#include <Core/ParticleSelector.hpp>

class Reader {
private:
    // Use ParticleMap instead of custom Topology struct
    using Topology = Core::ParticleMap;

    Histograms& m_histograms;

    auto get_topology(const hipo::bank& REC_Particle) -> Topology;

public:
    explicit Reader(Histograms& histograms);
    auto operator()(const std::string& file) -> void;
};

// In Reader.cpp
auto Reader::get_topology(const hipo::bank& REC_Particle) -> Topology {
    // One line to extract all particles!
    return Core::ParticleSelector::extract_particles(REC_Particle);
}

auto Reader::operator()(const std::string& file) -> void {
    for (hipo::hipoeventfile events(file); auto& event : events) {
        hipo::bank& REC_Particle = event.get_bank("REC::Particle");
        if (REC_Particle.getRows() == 0) continue;

        // Get particle topology
        auto particles = get_topology(REC_Particle);

        // Easy access to any particle type
        auto& electrons = particles[Core::PIDs::Electron];
        auto& protons = particles[Core::PIDs::Proton];
        auto& pions = particles[Core::PIDs::PionPlus];

        // Find trigger electron
        auto electron = Core::find_trigger_electron(electrons, {2000, 4000});
        if (!electron.has_value() || !electron->is_valid()) continue;

        // Continue with your analysis...
    }
}
```

## Benefits Summary

1. **Less Code**: Extract all particles in one line instead of 20+ lines
2. **Type Safety**: Use enum types instead of integer casting
3. **Reusability**: Common filters are pre-built and tested
4. **Readability**: Clear intent with descriptive function names
5. **Flexibility**: Easy to combine filters or write custom ones
6. **Maintainability**: Changes to particle extraction logic in one place

## Migration Guide

To update existing code:

1. Add `#include <Core/ParticleSelector.hpp>` to your Reader files
2. Replace manual particle extraction loops with `ParticleSelector::extract_particles()`
3. Change `Topology` struct to use `Core::ParticleMap` type alias
4. Replace verbose PID checks with `Core::matches()` or `ParticleSelector::is_type()`
5. Use built-in filters instead of manual cuts where applicable

Your old code will still work, but the new API makes things much cleaner!
