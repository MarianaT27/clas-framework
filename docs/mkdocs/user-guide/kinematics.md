# Kinematics Calculations

Standard DIS and exclusive kinematics calculations.

## DIS Kinematics

```cpp
#include <Physics/Kinematics.hpp>
#include <Physics/DISAnalysis.hpp>

// Create beam and target particles
CLAS::Particle beam(11, 0, -1, -1, CLAS::Constants::ELECTRONMASS,
                    0, 0, 10.53, 10.53, 0, 0, 0, 0, 1.0, 0);
CLAS::Particle target(2212, 0, -1, 1, CLAS::Constants::PROTONMASS,
                      0, 0, 0, CLAS::Constants::PROTONMASS, 0, 0, 0, 0, 0, 0);

// Calculate DIS variables using KinematicCalculator
double q2 = Physics::KinematicCalculator::calculate_Q2(beam, electron);
double w = Physics::KinematicCalculator::calculate_W(beam, electron, target);
double y = Physics::KinematicCalculator::calculate_y(beam, electron);
double nu_val = beam.E() - electron.E();
double xb = Physics::KinematicCalculator::calculate_x_bjorken(q2, nu_val);

std::cout << "Q² = " << q2 << " GeV²\n";
std::cout << "W = " << w << " GeV\n";
std::cout << "x_B = " << xb << "\n";

// Or use DISAnalysis for convenience
auto kin = Physics::DISAnalysis::calculate_kinematics(electron);
std::cout << "Q² = " << kin.q2 << ", W = " << kin.w << "\n";
```

## Exclusive Reactions

```cpp
// Missing mass using KinematicCalculator
double mm = Physics::KinematicCalculator::missing_mass(beam, target, {electron, proton});

// Invariant mass using CompositeParticle
Physics::CompositeParticle pi0({photon1, photon2});
double m_gg = pi0.invariant_mass();

// Opening angle
double angle = Physics::KinematicCalculator::opening_angle(photon1, photon2);
double p_miss = missing_momentum(electron, {proton, pion}, beam);
```

## Geometric Calculations

```cpp
// Angle between particles
double angle = angle_between(particle1, particle2);  // degrees

// Relative phi angle
double dphi = delta_phi(particle1, particle2);  // [-180, 180]
```

## Available Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `Q2(e, beam)` | Four-momentum transfer squared | GeV² |
| `W(e, beam)` | Invariant mass of hadronic system | GeV |
| `nu(e, beam)` | Energy transfer | GeV |
| `x_bjorken(e, beam)` | Bjorken x | dimensionless |
| `inelasticity(e, beam)` | Inelasticity y | dimensionless |
| `missing_mass(e, p, beam)` | Missing mass | GeV |
| `invariant_mass(p1, p2)` | Two-particle invariant mass | GeV |
| `momentum_transfer_t(e, meson, beam)` | -t for meson production | GeV² |
| `missing_energy(e, particles, beam)` | Missing energy | GeV |
| `missing_momentum(e, particles, beam)` | Missing momentum | GeV |
| `angle_between(p1, p2)` | Opening angle | degrees |
| `delta_phi(p1, p2)` | Relative phi angle | degrees |

## DIS Kinematic Regions

Use `DISSelectionAlg` for DIS event selection with configurable regions:

```cpp
#include "Algorithms/DISSelectionAlg.hpp"

// Standard DIS region
auto dis_region = Physics::DISAnalysis::standard_dis_region();

// Custom region
Physics::DISAnalysis::KinematicRegion custom_region;
custom_region.q2_min = 1.0;
custom_region.q2_max = 10.0;
custom_region.w_min = 2.0;
custom_region.w_max = 5.0;
custom_region.y_min = 0.0;
custom_region.y_max = 0.85;

// Use in algorithm
.add_algorithm<Algorithms::DISSelectionAlg>("DISSelection", custom_region)
```
