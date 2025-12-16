# Kinematics Calculations

Standard DIS and exclusive kinematics calculations.

## DIS Kinematics

```cpp
#include <Physics/Kinematics.hpp>
#include <Physics/DISAnalysis.hpp>

// Using KinematicCalculator
double q2 = Physics::KinematicCalculator::calculate_Q2(beam, electron);
double w = Physics::KinematicCalculator::calculate_W(beam, electron, target);
double y = Physics::KinematicCalculator::calculate_y(beam, electron);
double nu = beam.E() - electron.E();
double xb = Physics::KinematicCalculator::calculate_x_bjorken(q2, nu);

// Using DISAnalysis convenience method
auto kin = Physics::DISAnalysis::calculate_kinematics(electron);
std::cout << "Q² = " << kin.q2 << ", W = " << kin.w << "\n";
```

## Exclusive Reactions

```cpp
// Missing mass
double mm = Physics::KinematicCalculator::missing_mass(
    beam, target, {electron, proton}
);

// Invariant mass (π⁰ → γγ)
Physics::CompositeParticle pi0({photon1, photon2});
double m_gg = pi0.invariant_mass();

// Opening angle
double angle = Physics::KinematicCalculator::opening_angle(photon1, photon2);
```

## Available Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `calculate_Q2()` | Four-momentum transfer squared | GeV² |
| `calculate_W()` | Invariant mass of hadronic system | GeV |
| `calculate_y()` | Inelasticity | dimensionless |
| `calculate_x_bjorken()` | Bjorken x | dimensionless |
| `missing_mass()` | Missing mass | GeV |
| `opening_angle()` | Angle between particles | degrees |
