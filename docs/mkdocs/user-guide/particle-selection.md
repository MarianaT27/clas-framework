# Particle Selection

The framework provides convenient methods for selecting and filtering particles.

## Accessing Particles

### From Event

```cpp
auto execute(Framework::Event& event) -> Framework::StatusCode override {
    // Get all particles by type
    const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
    const auto& photons = event.particles(CLAS::PIDs::PHOTON);
    const auto& protons = event.particles(CLAS::PIDs::PROTON);
    const auto& pions_plus = event.particles(CLAS::PIDs::PIPLUS);
    const auto& pions_minus = event.particles(CLAS::PIDs::PIMINUS);
    
    // Get all particles (map of PID -> vector)
    const auto& all_particles = event.particles();
    
    return Framework::StatusCode::SUCCESS();
}
```

### Available Particle Types

| PID | Constant | Particle |
|-----|----------|----------|
| 11 | `CLAS::PIDs::ELECTRON` | Electron |
| -11 | `CLAS::PIDs::POSITRON` | Positron |
| 22 | `CLAS::PIDs::PHOTON` | Photon |
| 2212 | `CLAS::PIDs::PROTON` | Proton |
| 2112 | `CLAS::PIDs::NEUTRON` | Neutron |
| 211 | `CLAS::PIDs::PIPLUS` | π⁺ |
| -211 | `CLAS::PIDs::PIMINUS` | π⁻ |
| 321 | `CLAS::PIDs::KPLUS` | K⁺ |
| -321 | `CLAS::PIDs::KMINUS` | K⁻ |

## Particle Properties

```cpp
const auto& e = electrons[0];

// Kinematics
double p = e.p();           // Momentum magnitude [GeV/c]
double px = e.px();         // x-momentum
double py = e.py();         // y-momentum
double pz = e.pz();         // z-momentum
double E = e.E();           // Energy [GeV]
double theta = e.theta();   // Polar angle [deg]
double phi = e.phi();       // Azimuthal angle [deg]
double pt = e.pt();         // Transverse momentum

// Vertex
double vx = e.vx();         // Vertex x [cm]
double vy = e.vy();         // Vertex y [cm]
double vz = e.vz();         // Vertex z [cm]
double vt = e.vt();         // Vertex time [ns]

// Identification
int pdg = e.pdg();          // PDG code
int status = e.status();    // Status code
int charge = e.charge();    // Charge
double beta = e.beta();     // β = v/c
double chi2pid = e.chi2pid(); // χ² PID

// ROOT vectors
auto p4 = e.PxPyPzEVector();  // ROOT::Math::PxPyPzEVector
auto p3 = e.PxPyPzVector();   // ROOT::Math::XYZVector
auto v3 = e.VxVyVzVector();   // ROOT::Math::XYZVector
```

## Filtering Particles

### Lambda Filters

```cpp
// Filter by momentum
auto high_p_electrons = electrons | std::views::filter([](const auto& e) {
    return e.p() > 1.0 && e.p() < 10.0;
});

// Filter by vertex
auto good_vz = electrons | std::views::filter([](const auto& e) {
    return e.vz() > -15.0 && e.vz() < 5.0;
});

// Filter by detector region (Forward Detector)
auto fd_electrons = electrons | std::views::filter([](const auto& e) {
    return std::abs(e.status()) >= 2000 && std::abs(e.status()) < 4000;
});

// Filter by detector region (Central Detector)
auto cd_electrons = electrons | std::views::filter([](const auto& e) {
    return std::abs(e.status()) >= 4000;
});
```

### Manual Filtering

```cpp
std::vector<CLAS::Particle> good_electrons;
for (const auto& e : electrons) {
    if (e.p() > 1.0 && e.vz() > -15.0 && e.vz() < 5.0) {
        good_electrons.push_back(e);
    }
}
```

## Trigger Electron

The trigger electron is typically the first electron with Forward Detector status:

```cpp
const CLAS::Particle* trigger_electron = nullptr;

for (const auto& e : electrons) {
    if (std::abs(e.status()) >= 2000 && std::abs(e.status()) < 4000) {
        trigger_electron = &e;
        break;
    }
}

if (!trigger_electron) {
    return Framework::StatusCode::SUCCESS();  // No trigger electron
}
```

## Detector Regions

| Status Range | Detector |
|--------------|----------|
| 2000-4000 | Forward Detector (FD) |
| 4000-6000 | Central Detector (CD) |

```cpp
bool is_forward_detector(const CLAS::Particle& p) {
    int status = std::abs(p.status());
    return status >= 2000 && status < 4000;
}

bool is_central_detector(const CLAS::Particle& p) {
    int status = std::abs(p.status());
    return status >= 4000 && status < 6000;
}
```

## Combining Particles

### Invariant Mass

```cpp
#include "Physics/CompositeParticle.hpp"

// Two-photon invariant mass (π⁰ reconstruction)
const auto& photons = event.particles(CLAS::PIDs::PHOTON);

for (size_t i = 0; i < photons.size(); ++i) {
    for (size_t j = i + 1; j < photons.size(); ++j) {
        Physics::CompositeParticle pi0({photons[i], photons[j]});
        double m_gg = pi0.invariant_mass();
        
        if (m_gg > 0.1 && m_gg < 0.17) {
            h_pi0_mass->Fill(m_gg);
        }
    }
}
```

### Opening Angle

```cpp
#include "Physics/Kinematics.hpp"

double angle = Physics::KinematicCalculator::opening_angle(photon1, photon2);
```

## Best Practices

!!! tip "Check Empty Collections"
    Always check if particle collections are empty before accessing elements:
    ```cpp
    if (electrons.empty()) {
        return Framework::StatusCode::SUCCESS();
    }
    ```

!!! tip "Use References"
    Use `const auto&` to avoid copying particles:
    ```cpp
    for (const auto& e : electrons) { ... }
    ```

!!! tip "Trigger First"
    Select the trigger electron first, then apply additional cuts.
